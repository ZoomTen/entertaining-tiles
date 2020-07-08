# Entertaining Tiles

**WORK IN PROGRESS** Reversi game using Victor Tran's libentertaining. It also uses MOD music powered by libmodplug. (I'd use libopenmpt but not all distros are inclined to include it because... gstreamer wh)

## Client

Qt app, to be built and installed normally

**Todo**:

- Online: implement client-side game application

- Offline: Load and save functionality

- Information: Write game rules document

- Application: Fill in all the missing holes

**Known Bugs**

- When the computer doesn't have any moves left, control immediately passes over to the player. However this is not reflected in the turns screen, so only the player indicator at the top is accurate. (same goes for turn based mode I think)

## Server

Nodejs, copy it to your `entertaining-server` install, under `applications/EntertainingTiles/0.5/`.

**Todo**:

- Online: implement server-side game application
