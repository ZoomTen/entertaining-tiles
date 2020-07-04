const winston = require('winston');
const crypto = require('crypto');

// database
const db = require('../../../db.js');

// room creation functs
const Room = require('./room');

///////////////////////////////////////////////////

function colourFromUserId(userId) {
    // Algorithm for converting between HSV taken from Wikipedia:
    // https://en.wikipedia.org/wiki/HSL_and_HSV#HSV_to_RGB

    let h = userId % 360;
    let s = 1;
    let v = 0.5;

    let c = v * s;
    let hPr = h / 60;
    let x = c * (1 - Math.abs(hPr % 2 - 1));

    let rgb1;
    if (hPr <= 1) {
        rgb1 = [c, x, 0];
    } else if (hPr <= 2) {
        rgb1 = [x, c, 0];
    } else if (hPr <= 3) {
        rgb1 = [0, c, x];
    } else if (hPr <= 4) {
        rgb1 = [0, x, c];
    } else if (hPr <= 5) {
        rgb1 = [x, 0, c];
    } else {
        rgb1 = [c, 0, x];
    }

    let m = v - c;
    let rgb = rgb1.map(y => {
        return Math.round((y + m) * 255);
    });
    console.log(rgb);

    let colBuf = Buffer.allocUnsafe(4);
    colBuf.writeUInt8(0xFF);
    Buffer.from(rgb).copy(colBuf, 1);


    return colBuf.readUInt32BE(0);
}

///////////////////////////////////////////////////

const States = {
    idle: 0,
    lobby: 1,
    game: 2
}

class Game {
    // public:
    //  (none)
    // private:
    #ws;            // web socket
    #username;
    #userId;
    #picture;
    #sessionId;
    #state;
    #room;
    #color;

    constructor(ws, username, userId) {
        this.#ws = ws;              // web socket object
        this.#username = username;
        this.#userId = userId;
        this.#state = States.idle;  // default state
        this.#room = null;          // default room

        // set session ID
        this.#sessionId = Math.floor(Math.random() * 1000000);

        // generate a random color based on the user ID
        this.#color = colourFromUserId(userId);

        // close / crash message
        ws.on("close", (code, reason) => {
            winston.log('silly', `Entertaining Tiles client closed, code ${code}, reason: ${reason}`);
        });

        db.userForUsername(username).then(info => {
            this.#picture = info.gravHash;
        });

        // install command handlers
        ws.on("jsonMessage", this.handleMessage.bind(this));

        // initialized
        winston.log('verbose', `Entertaining Tiles client initialized for user ${username} (${userId})`);

        // submit session ID
        setTimeout(() => {
            this.#ws.sendObject({
                type: "sessionIdChanged",
                session: this.#sessionId
            });
        }, 1000);
    }

    // message handler

    async handleMessage(message) {
        //Don't handle system messages
        if (message.system) return;

        // examine received message
        console.log(message);
        console.log(message.type);

        // Bind the functions
        const handlers = {
            "openRoom": this.openRoom.bind(this),
            "joinRoom": this.joinRoom.bind(this),
            "quitRoom": this.quitRoom.bind(this),
            "listRooms": this.listRooms.bind(this)
        };

        if (handlers.hasOwnProperty(message.type)) {
            // on receive {"type": "whatever"}
            // then call the function binded in handler["whatever"]
            handlers[message.type](message);
        } else if (this.#room) {
            // if no "type" specified
            this.#room.processMessage(this, message);
        }
    }

    // bind functions

    openRoom(){
        if (this.#state != States.idle) {
            this.#ws.close(1002); // we must be idle
        } else {
            let room = new Room();  // create a room
            this.joinRoom({         // immediately join the room
                "roomId": room.id
            });
        }
    }

    joinRoom(message){
        if (this.#state != States.idle) {
            this.#ws.close(1002); // we must be idle
        } else {
            console.log(message);
            // join a room ID
            let room = Room.roomById(message.roomId);

            if (!room) {
                // room not found
                this.#ws.sendObject({
                    type: "joinRoomFailed",
                    reason: "room.invalid"
                });
                return;
            }

            // try to add user to the room
            let addUserResponse = room.addUser(this);

            if (addUserResponse !== "ok") {
                this.#ws.sendObject({
                    type: "joinRoomFailed",
                    reason: addUserResponse
                });
                return;
            }

            // if we're successful, set the current user's room
            this.#room = room;

            this.changeState("lobby") // user is now in the lobby, they
                                      // can't open another room at the
                                      // same time
        }
    }

    quitRoom(){
        if (this.#state == States.idle) {
            this.#ws.close(1002); // we must NOT be in idle
        } else {
            this.#room.removeUser(this);
            this.#room = null;

            this.changeState("idle");
        }
    }

    async listRooms(){
        // initialize rooms list
        let rooms = [];

        // get all the user's friends
        let friends = await db.friendsForUserId(this.#userId);

        // should be a list, get all open rooms
        for (let friend of friends) {
            let rs = Room.roomsByUserId(friend.userId);
            for (let room of rs) {
                room.friendName = friend.username;
                rooms.push(room);
            }
        }

        // send off the room IDs
        this.#ws.sendObject({
            type: "availableRoomsReply",
            rooms: rooms.map(room => {
                return {
                    friend: room.friendName,
                    roomId: room.id,
                    pinRequired: room.pinRequired
                };
            })
        });
    }

    // general functions

    changeState(state) {
        this.#ws.sendObject({
            type: "stateChange",
            newState: state
        });
        this.#state = States[state];
    }

    // getters

    static displayName() {
        return "Entertaining Tiles";
    }

    get username() {
        return this.#username;
    }

    get userId() {
        return this.#userId;
    }

    get ws() {
        return this.#ws;
    }

    get picture() {
        return this.#picture;
    }

    get color() {
        return this.#color;
    }

    get sessionId() {
        return this.#sessionId;
    }
}

module.exports = Game;
