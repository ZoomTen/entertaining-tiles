#include "regulargame.h"
#include "ui_regulargame.h"

#include "game/reversitile.h"

#include <QVector>

#include <QDebug>

#include <QKeyEvent>

#include <QTime>
#include <QTimer>

#include "audio/backgroundmusic.h"

#include <libentertaining/musicengine.h>

#include "screens/pause/pausescreen.h"
#include "screens/game/turnsscreen.h"

// CPP
#include <random>

struct RegularGamePrivate{
    QVector<ReversiTile*> tiles;
    int gameType;
    int squareBoardSize;

    int currentPlayer;
    bool isComputer;

    int firstPlayerRunOut; // first player to run out of moves

    bool playerCanPlay;

    bool showTurnsScreen;

    QList<QString> playerNames;

    QVector<bool> calculatedMoves;
    QPair<int, int> focusedCoords;

    QPair<int, int> darkLightCount;

    QTime gameTime;
    QTimer updater;

    BackgroundMusic* bgMusic;
};

RegularGame::RegularGame(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RegularGame)
{
    ui->setupUi(this);
    d = new RegularGamePrivate();

    // bind gamepad options
    // assign "A" / Enter button to flip tiles
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonA, tr("Flip Tile"));
    ui->gamepadHud->bindKey(Qt::Key_Return, QGamepadManager::ButtonA);
    ui->gamepadHud->bindKey(Qt::Key_Space, QGamepadManager::ButtonX);

    // assign "L1" / H button to show hints (lol?)
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonL1, tr("Hint"));
    ui->gamepadHud->bindKey(Qt::Key_H, QGamepadManager::ButtonL1);

    // assign "Start" / Esc button to pause
    ui->gamepadHud->setButtonText(QGamepadManager::ButtonStart, tr("Pause"));
    ui->gamepadHud->bindKey(Qt::Key_Escape, QGamepadManager::ButtonStart);

    connect(this, &RegularGame::numMovesChanged,
            this, [=](int movesLeft){
        qDebug() << "Moves left:" << movesLeft;
        /* BUG: on vs. computer sometimes the "computer" screen will display
         * when it's actually the player's turn due to the computer
         * running out of moves
         */
        if (movesLeft == 0){
            // pass it back to the other player

            d->showTurnsScreen = false;
            if (d->firstPlayerRunOut == 0){
                d->firstPlayerRunOut = d->currentPlayer;
                switchPlayers();
                calculateLegalMoves(d->squareBoardSize); // calculate moves for the other player
            } else {
                // the game ends here
                emit gameOver();
            }
        } else {
            /* clear "first player to run out of moves" flag since
             * we can still play
             */

            d->showTurnsScreen = true;
            d->firstPlayerRunOut = 0;

            /* If we're playing in computer mode,
             * let's run the AI
             */
            if (d->gameType == VsComputer){
                if(!d->isComputer){
                if(d->currentPlayer == ReversiTile::Light){
                    // AI time
                    d->isComputer = true;
                    emit setHighlightVisibility(false);
                    QTimer::singleShot(2500, this, [=]{
                        performComputer();
                        d->isComputer = false;
                    });
                }
                }
            }
        }
    });

    connect(this, &RegularGame::tileCountChanged,
            this, [=](QList<int> tileCounts){
        d->darkLightCount = QPair<int,int> (tileCounts[1], tileCounts[2]);
        ui->darkCount->setText(QString("%1: %2").arg(d->playerNames[1]).arg(tileCounts[1]));
        ui->lightCount->setText(QString("%1: %2").arg(d->playerNames[2]).arg(tileCounts[2]));
    });

    connect(this, &RegularGame::playerChanged,
            this, [=](int player){
        ui->playerName->setText(d->playerNames[player]);
    });

    // on game over
    connect(this, &RegularGame::gameOver,
            this, [=](){
        ui->noMoreMoves->show();

        // disable hint button
        ui->hintButton->setDisabled(true);
        ui->gamepadHud->removeText(QGamepadManager::ButtonL1);

        // flip tile button
        ui->gamepadHud->removeText(QGamepadManager::ButtonA);
        ui->gamepadHud->setButtonText(QGamepadManager::ButtonX, "Select");

        d->updater.stop();

        QTimer::singleShot(5500, this, [=]{
            d->bgMusic->stopMusic();
            d->bgMusic = new BackgroundMusic(this, ":/audio/bgm/game-online.mod");
            d->bgMusic->startMusic();

            if (d->darkLightCount.first != d->darkLightCount.second){
                int winTileCount;
                QString winnerName;
                if (d->darkLightCount.first > d->darkLightCount.second){
                    winTileCount = d->darkLightCount.first;
                    winnerName = d->playerNames[1];
                } else {
                    winTileCount = d->darkLightCount.second;
                    winnerName = d->playerNames[2];
                }
                ui->winText->setText(tr("The winner is %1, with %2 tiles!")
                                     .arg(winnerName)
                                     .arg(winTileCount));
            } else {
                ui->winText->setText(tr("It's a %1-tile tie!")
                                     .arg(d->darkLightCount.first));
            }
            ui->gameCanvas->setCurrentWidget(ui->overScreen);
        });
    });

    connect(ui->hintButton, &QPushButton::clicked,
            this, [=](bool){
        if (d->playerCanPlay){
            for (int i = 0; i < (d->squareBoardSize*d->squareBoardSize); ++i) {
                if(d->calculatedMoves[i]){
                    d->tiles[i]->flashTile();
                    break;
                }
            }
        }
    });

    connect(ui->backToMenu, &QPushButton::clicked,
            this, [=](bool){
        emit returnToMainMenu();
    });

    connect(this, &RegularGame::returnToMainMenu,
            this, [=]{
        d->bgMusic->stopMusic();
    });

    connect(&d->updater, &QTimer::timeout,
            this,       [=]{
        d->gameTime = d->gameTime.addSecs(1);
        ui->timerDisplay->setText(tr("Time: %1").arg(d->gameTime.toString("mm:ss")));
    });


}

RegularGame::~RegularGame()
{
    clearTiles();
    delete d;
    delete ui;
}

// player control

void RegularGame::setActivePlayer(int player)
{
    d->currentPlayer = player;
    emit playerChanged(player);
    playersTurnScreen();
}

void RegularGame::switchPlayers()
{
    switch (d->currentPlayer) {
    case ReversiTile::Light:
        setActivePlayer(ReversiTile::Dark);
        break;
    case ReversiTile::Dark:
        setActivePlayer(ReversiTile::Light);
        break;
    default:
        break;
    }
}

void RegularGame::lockPlayer()
{
    d->playerCanPlay = false;
    emit setHighlightVisibility(false);
}

void RegularGame::unlockPlayer()
{
    d->playerCanPlay = true;
    emit setHighlightVisibility(true);
}

bool RegularGame::playerIsComputer()
{
    return d->isComputer;
}

void RegularGame::performComputer()
{
    QList<int> legalmovesList;
    std::random_device rd;
    std::mt19937 g(rd());

    calculateLegalMoves(d->squareBoardSize);
    for (int i = 0; i < (d->squareBoardSize*d->squareBoardSize); ++i) {
        if(d->calculatedMoves[i]){
            legalmovesList.append(i);
        }
    }

    if (legalmovesList.size() != 0){
        std::shuffle(legalmovesList.begin(), legalmovesList.end(), g);
        processMove(legalmovesList[0]);
    }
}

//  getting and checking tiles

ReversiTile* RegularGame::getTileAt(int row, int col)
{
    if (isInBoard(row, col)){
        return d->tiles[getIndex(row,col)];
    } else {
        return nullptr;  // invalid tile
    }
}

ReversiTile *RegularGame::getTileAtIndex(int index)
{
    if ((0 <= index) &&
             (index < (d->squareBoardSize * d->squareBoardSize))
    ){
        return d->tiles[index];
    } else {
        return nullptr;
    }
}

int RegularGame::getIndex(int row, int col)
{
    return (row * d->squareBoardSize) + col;
}

bool RegularGame::isInBoard(int row, int col)
{
    // greater than /or equal/ because zero indexed stuff
    if ((row < 0) || (row >= d->squareBoardSize)) return false;
    if ((col < 0) || (col >= d->squareBoardSize)) return false;
    return true;
}

QPair<int, int> RegularGame::rowAndColFromID(int tileId)
{
    if ((0 <= tileId) &&
             (tileId < (d->squareBoardSize * d->squareBoardSize))
    ){
        return QPair<int,int>(tileId / d->squareBoardSize,
                              tileId % d->squareBoardSize);
    } else {
        // invalid tile ID
        return QPair<int,int>(-1,-1);
    }
}

void RegularGame::refreshTileCount(int size)
{
    int darkCount = 0;
    int lightCount = 0;
    int emptyCount = 0;
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){
            switch (getTileAt(row, col)->getType()){
            case ReversiTile::Dark:
                darkCount++;
                break;
            case ReversiTile::Light:
                lightCount++;
                break;
            case ReversiTile::Empty:
                emptyCount++;
                break;
            default:
                break;
            }
        }
    }

    QList<int> tileCounts;
    tileCounts.append(emptyCount);
    tileCounts.append(darkCount);
    tileCounts.append(lightCount);

    emit tileCountChanged(tileCounts);
}

// game control

void RegularGame::startGame(int size, int gameType, QList<QString> names)
{
    // TODO: size should be 6 <= x <= 20 and must be even
    // if a board exists, delete all the tiles first
    if (d->tiles.count() != 0){
        clearTiles();
    }

    d->squareBoardSize = size;
    d->gameType = gameType;
    d->firstPlayerRunOut = ReversiTile::Empty;
    d->focusedCoords = {0, 0};
    d->playerNames = names;
    d->showTurnsScreen = true;

    d->playerCanPlay = false;
    d->isComputer = false;

    // create new tiles
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){
            // make tiles
            ReversiTile* tile = new ReversiTile(this,
                                                8*row + col);

            // link the window's resize event to set the proper tile width
            connect(this, &RegularGame::windowResized,
                    tile, &ReversiTile::setAppropriateSize);

            connect(this, &RegularGame::setHighlightVisibility,
                    tile, &ReversiTile::setHighlightIsVisible);

            // resizeEvent
            QSize boardSize = QSize(d->squareBoardSize,d->squareBoardSize);
            emit windowResized(boardSize, ui->gameWidget->size());

            // if clicked on a tile, process the move
            connect(tile, &ReversiTile::clickedTileID,
                    this, [=](int tileId){
                if (d->playerCanPlay){
                    processMove(tileId);
                }
            });

            // add tile to game board
            d->tiles.append(tile);
            ui->gameGrid->addWidget(tile, row, col);
        }
    }

    /* initialize calculated legal moves grid
     * this isn't used for main functions, but rather
     * to be used for hints and computer moves
     */
    d->calculatedMoves.resize(size * size);

    // default positions
    // TODO: this assumes a 8x8 board.
    d->tiles[27]->setType(ReversiTile::Light);
    d->tiles[28]->setType(ReversiTile::Dark);
    d->tiles[35]->setType(ReversiTile::Dark);
    d->tiles[36]->setType(ReversiTile::Light);

    // UI functions
    ui->noMoreMoves->hide();

    // start clock
    d->gameTime = QTime();
    d->gameTime.setHMS(0,0,0);
    d->updater.setInterval(1000);
    d->updater.start();

    // TODO: make first player adjustable
    setActivePlayer(ReversiTile::Dark); // dark goes first

    // count all the tiles
    refreshTileCount(d->squareBoardSize);

    calculateLegalMoves(size);

    // set focus
    d->tiles.first()->setFocus();
    this->setFocusProxy(d->tiles.first());

    d->bgMusic = new BackgroundMusic(this, ":/audio/bgm/game.mod");
    d->bgMusic->startMusic();

    ui->gameCanvas->setCurrentAnimation(tStackedWidget::SlideHorizontal);
}

QMap<QString, QVariant> RegularGame::getFlippable(int from, int row_offset, int col_offset, int row, int col)
{
    /* checks the tile at row, col and traverses in the direction of
     * row_offset and col_offset (call that a "vector") to see if there
     * is a flippable tile 1 step in that direction
     *
     * that tile is only flippable if it is the other players', and that the
     * tile 1 step next to it is ours
     *
     * if so, we return a QMap specifying if we can flip a tile <bool>,
     * and if so, which ones <ReversiTile*>
     */
    QMap<QString, QVariant> map;
    int opposingTile;

    bool          canFlip = false;
    ReversiTile*  tileToFlip = nullptr;

    // determine opposing tile from the tile which this was called from
    switch(from){
    case ReversiTile::Dark:
        opposingTile = ReversiTile::Light;
        break;
    case ReversiTile::Light:
        opposingTile = ReversiTile::Dark;
        break;
    default:
        opposingTile = -1;  // no opposing tile
        break;
    }

    // check the middle tile
    ReversiTile* middleTileType = getTileAt(row+row_offset, col+col_offset);
    ReversiTile* lastTileType = getTileAt(row+(row_offset*2), col+(col_offset*2));

    if (lastTileType != nullptr){
        // if last tile exists
        // surely the middle one must exist as well
        if(middleTileType->getType() == opposingTile){
            if(lastTileType->getType() == from){
                // if last tile is the same as player tile
                canFlip = true;
                tileToFlip = getTileAt(row+row_offset, col+col_offset);
            }
        }
    }

    map["canFlip"]    = canFlip;
    map["tileToFlip"] = QVariant::fromValue(tileToFlip);

    return map;
}

void RegularGame::flipRelevantTiles(int row, int col)
{
    bool anyFlipped = false;
    for (int row_offset = -1; row_offset < 2; ++row_offset){
    for (int col_offset = -1; col_offset < 2; ++col_offset){
        QMap<QString, QVariant> flippable = getFlippable(d->currentPlayer,
                                                         row_offset,
                                                         col_offset,
                                                         row,
                                                         col);
        if (flippable["canFlip"].toBool()){
            qDebug() << "Flipping" << row  << col <<
                        ".." <<
                        rowAndColFromID(flippable["tileToFlip"]
                        .value<ReversiTile*>()
                        ->getID());

            // add our tile to the board
            getTileAt(row, col)->setType(d->currentPlayer);
            flippable["tileToFlip"].value<ReversiTile*>()->flip();

            anyFlipped = true;
        }
    }
    }

    if (anyFlipped){
        // play sound
        MusicEngine::playSoundEffect(MusicEngine::Warning);
    }

}

int RegularGame::calculateLegalMoves(int size)
{
    /* traverses the entire grid to see if there's any moves that can be
     * made by the player
     *
     * highlightable tile == player can perform move on that tile
     */
    int numLegalMoves = 0;
    for (int row = 0; row < size; ++row){
        for (int col = 0; col < size; ++col){

            // clear out the highlightable for the tile
            getTileAt(row, col)->setHighlightable(false);

            // similarly, clear out our list "cache"
            d->calculatedMoves[getIndex(row,col)] = false;

            // check every direction
            if  (getTileAt(row, col)->getType() == ReversiTile::Empty){
            for (int row_offset = -1; row_offset < 2; ++row_offset){
            for (int col_offset = -1; col_offset < 2; ++col_offset){
                QMap<QString, QVariant> flippable = getFlippable(d->currentPlayer,
                                                                 row_offset,
                                                                 col_offset,
                                                                 row,
                                                                 col);
                if (flippable["canFlip"].toBool()){
                    /*qDebug() << "Legal at" << row  << col <<
                                "Affecting" <<
                                rowAndColFromID(flippable["tileToFlip"]
                                .value<ReversiTile*>()
                                ->getID());*/
                    // set legal move on the origin tile
                    ReversiTile* tile = getTileAt(row, col);
                    tile->setHighlightable(true);

                    // set it on our "cache" too
                    d->calculatedMoves[getIndex(row,col)] = true;

                    // count available moves
                    ++numLegalMoves;
                }
            }
            }
            }
        }
    }
    emit numMovesChanged(numLegalMoves);
    return numLegalMoves;
}

void RegularGame::processMove(int tileId)
{
    /* called on tile click
     * if tile is highlightable (i.e. legal move), flip every tile that
     * can be flipped
     */
    QPair<int,int> rowCol = rowAndColFromID(tileId);
    int row = rowCol.first;
    int col = rowCol.second;

    if (d->tiles[tileId]->getHighlightable()){
        flipRelevantTiles(row, col);

        // take turns
        lockPlayer();
        switchPlayers();

        calculateLegalMoves(d->squareBoardSize);
        refreshTileCount(d->squareBoardSize);
    }
}

void RegularGame::clearTiles()
{
    for(ReversiTile* tile : d->tiles){
        ui->gameGrid->removeWidget(tile);
        tile->deleteLater();
    }
    d->tiles.clear();
}

// overlays

void RegularGame::pauseSession()
{
    d->bgMusic->pauseMusic();

    d->updater.stop();

    MusicEngine::playSoundEffect(MusicEngine::Pause);

    PauseScreen* screen = new PauseScreen(this);
    connect(screen, &PauseScreen::resume, this, [=] {
        screen->deleteLater();
        d->bgMusic->resumeMusic();
        ReversiTile* focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
        focused->setFocus();
        this->setFocusProxy(focused);

        // if we haven't run out of moves yet
        if (d->firstPlayerRunOut == 0) d->updater.start();
    });
    connect(screen, &PauseScreen::newGame, this, [=] {
        screen->deleteLater();
        d->bgMusic->stopMusic();
        startGame(8, d->gameType, d->playerNames);
    });
    connect(screen, &PauseScreen::mainMenu, this, [=] {
        screen->deleteLater();
        emit returnToMainMenu();
    });
    connect(screen, &PauseScreen::provideMetadata, this, [=](QVariantMap* metadata) {
        //TODO
//        QStringList description;
//        description.append(tr("%1 × %2 board").arg(d->width).arg(boardDimensions().height()));
//        description.append(tr("%1 mines").arg(d->mines));
//        description.append(tr("%1 flagged").arg(d->mines - d->minesRemaining));
//        description.append(tr("%1 mines to go").arg(d->minesRemaining));
//        description.append(tr("%1% cleared").arg(static_cast<int>(static_cast<float>(d->remainingTileCount) / d->tiles.count() * 100)));

//        metadata->insert("description", description.join(" ∙ "));
    });
//    connect(screen, &PauseScreen::provideSaveData, this, &GameScreen::saveGame);
    screen->show();
}

void RegularGame::playersTurnScreen()
{
    if(d->showTurnsScreen){
        TurnsScreen* screen = new TurnsScreen(this, d->playerNames[d->currentPlayer]);
        QTimer::singleShot(500, this, [=]{
            connect(screen, &TurnsScreen::doneShown, this, [=] {
                screen->deleteLater();

                d->focusedCoords = QPair<int,int>(0, 0);
                ReversiTile* focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
                focused->setFocus();
                this->setFocusProxy(focused);

                if (!d->isComputer){
                    unlockPlayer();
                }
            });
            screen->show();
        });
    }
}

// events

void RegularGame::resizeEvent(QResizeEvent*)
{
    // resize the tiles sensibly
    QSize boardSize = QSize(d->squareBoardSize,d->squareBoardSize);

    emit windowResized(boardSize, ui->gameWidget->size());
}

void RegularGame::keyPressEvent(QKeyEvent *e)
{
    ReversiTile* focused;
    //QEvent clickEvent(QEvent::MouseButtonPress);
    int newID;

    if (ui->gameCanvas->currentWidget() == ui->mainGame){
    switch (e->key()) {

    case Qt::Key_Up:
    case Qt::Key_W:
        if (--d->focusedCoords.first > -1){
            focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
            focused->setFocus();
            this->setFocusProxy(focused);
        } else ++d->focusedCoords.first;
        break;

    case Qt::Key_Down:
    case Qt::Key_S:
        if (++d->focusedCoords.first < d->squareBoardSize){
            focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
            focused->setFocus();
            this->setFocusProxy(focused);
        } else --d->focusedCoords.first;
        break;

    case Qt::Key_Left:
    case Qt::Key_A:
        if (--d->focusedCoords.second > -1){
            focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
            focused->setFocus();
            this->setFocusProxy(focused);
        } else ++d->focusedCoords.second;
        break;

    case Qt::Key_Right:
    case Qt::Key_D:
        if (++d->focusedCoords.second < d->squareBoardSize){
            focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
            focused->setFocus();
            this->setFocusProxy(focused);
        } else --d->focusedCoords.second;
        break;

    case Qt::Key_Enter:
    case Qt::Key_Return:
        focused = getTileAt(d->focusedCoords.first, d->focusedCoords.second);
        focused->click();   // simulate click
        //QApplication::sendEvent(this->focusWidget(), &clickEvent);
        // TODO: space is used to click buttons on the Game over screen
        break;

    case Qt::Key_H:
        // Hint button hardcode to H
        ui->hintButton->click();
        break;

    case Qt::Key_Escape:
        pauseSession();
        break;

    case Qt::Key_Tab:
        // forwards focus
        newID = getIndex(d->focusedCoords.first,d->focusedCoords.second)+1;
        if (newID >= (d->squareBoardSize*d->squareBoardSize)) newID = 0;
        focused = getTileAtIndex(newID);
        focused->setFocus();
        this->setFocusProxy(focused);
        d->focusedCoords = rowAndColFromID(newID);
        break;

    case Qt::Key_Backtab:
        // reverse focus
        newID = getIndex(d->focusedCoords.first,d->focusedCoords.second)-1;
        if (newID < 0) newID = (d->squareBoardSize*d->squareBoardSize)-1;
        focused = getTileAtIndex(newID);
        focused->setFocus();
        this->setFocusProxy(focused);
        d->focusedCoords = rowAndColFromID(newID);
        break;
    default:
        break;
    }
    } else {
        // game over
        switch (e->key()) {

        case Qt::Key_Left:
        case Qt::Key_Backtab:
        case Qt::Key_Right:
        case Qt::Key_Tab:
            if (ui->backToMenu->hasFocus()){
                ui->backToBoard->setFocus();
            } else {
                ui->backToMenu->setFocus();
            }
            break;
        default:
            break;
        }
    }
}

bool RegularGame::focusNextPrevChild(bool)
{
    // focusing is handled manually by keyPressEvent
    return false;
}

void RegularGame::on_pauseButton_clicked()
{
    pauseSession();
}

void RegularGame::on_backToBoard_clicked()
{
    ui->gameCanvas->setCurrentWidget(ui->mainGame);
}
