const winston = require('winston');

// at start of server, we have an empty rooms list
let rooms = {};

class Room {
    #id;
    #users;
    #playing;
    #maxUsers;
    #board;

    // begin room

    constructor() {
        this.#id = Room.generateId();
        this.#users = [];
        this.#playing = false;
        this.#maxUsers = 2;

        // add a Room object at the generated key
        rooms[this.#id] = this;

        winston.log("verbose", `Entertaining Tiles room #${this.#id} created`);
    }

    static generateId() {
        return Math.floor(Math.random() * 10000000 + 1000000);
    }

    // fetch rooms

    static roomById(id) {
        return rooms[id];
    }

    static roomsByUserId(userId) {
        // Find all the rooms where this user is a member
        let rs = Object.values(rooms).filter(room => {
            for (let user of room.users) {
                if (user.userId === userId) return true;
            }
            return false;
        });

        return rs;
    }

    // handler install

    processMessage(user, message) {
        // lobby-time messages
        if (this.isHost(user)) {
            const handlers = {
                "startGame": this.startGame.bind(this)
            };

            // register handlers
            if (handlers.hasOwnProperty(message.type)) {
                handlers[message.type](user, message);
            }
        }

        // game-time messages
        const handlers = {
            "currentTileChanged": this.currentTileChanged.bind(this),
            //"sendMessage": this.sendCannedMessage.bind(this)
        };
        if (handlers.hasOwnProperty(message.type)) {
            handlers[message.type](user, message);
        }
    }

    // user functions

    addUser(user) {
        if (this.#playing) {
            return "room.closed";
        }
        if (this.#users.count >= this.#maxUsers) {
            return "room.full";
        }

        // add user to room users list
        this.#users.push(user);

        // remove on unexpected disconnect
        user.ws.on("close", () => {
            this.removeUser(user);
        });

        // let user client know the lobby is changed
        user.ws.sendObject({
            type: "lobbyChange",
            lobbyId: this.#id
        });

        this.beamRoomUpdate();

        return "ok";
    }

    removeUser(user) {
        if (this.#users.includes(user)) {
            this.#users.splice(this.#users.indexOf(user), 1);

            if (this.#playing) {
                this.#board.removeUser(user);
            }

            user.ws.sendObject({
                type: "lobbyChange",
                lobbyId: -1
            });

            if (this.#users.length == 0) {
                this.deleteRoom();
            } else {
                this.beamRoomUpdate();
            }
        }
    }

    deleteRoom(){
        //Close this room
        delete rooms[this.#id];
        winston.log("verbose", `Entertaining Tiles room #${this.#id} closed`);
    }

    // send to clients

    beamRoomUpdate() {
        // let everyone in the room know about changes in setup
        let users = [];

        // add every user we can find to the list
        for (let user of this.#users) {
            users.push({
                session: user.sessionId,
                username: user.username,
                picture: user.picture,
                color: user.color,
                isHost: this.isHost(user)
            });
        }

        // send list of users to every client
        let message = {
            type: "roomUpdate",
            users: users,
            maxUsers: this.#maxUsers
        };
        this.beam(message);
    }

    beam(message) {
        // send a json object to all clients currently connected
        for (let user of this.#users) {
            this.sendMessage(user, message);
        }
    }

    sendMessage(user, message) {
        // send a json object to a specific user
        user.ws.sendObject(message);
    }

    // game control

    startGame(user, message) {
        // Start the game state
        this.#playing = true;

        // everyone is now playing the game
        for (let user of this.#users) {
            user.changeState("game");
        }
    }

    endGame() {
        // everyone is off the game
        for (let user of this.#users) {
            user.changeState("lobby");
        }

        this.beamRoomUpdate();

        // End the game state
        this.#playing = false;
    }

    // signals

    currentTileChanged(user, message) {
        if (this.#playing) {
            // TODO: make handler
            console.log(user, message)
        }
    }

    changeBoardParams(user, message) {
        // TODO: make handler
    };

    // getters

    isHost(user) {
        return this.#users[0] === user;
    }

    get id() {
        return this.#id;
    }

    get users() {
        return this.#users;
    }
}

module.exports = Room