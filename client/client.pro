#### Application Meta ####

# do NOT put quotes around APP_NAME!
# this is so that the name conversion works properly

APP_NAME    = Entertaining Tiles
APP_VERSION = 0.1

AUTHOR_DOMAIN = zumid.com

# for author data and such, see main.cpp...

#### Source file list ####
SOURCES += \
    audio/backgroundmusic.cpp \
    game/reversitile.cpp \
    gamewindow.cpp \
    main.cpp \
    onlinecontroller/onlinecontroller.cpp \
    screens/game/regulargame.cpp \
    screens/game/turnsscreen.cpp \
    screens/main/mainscreen.cpp \
    screens/online/lobby/lobbyscreen.cpp \
    screens/online/menu/onlinemenuscreen.cpp \
    screens/online/onlinescreen.cpp \
    screens/online/pending/onlinependingscreen.cpp \
    screens/pause/pausescreen.cpp

HEADERS += \
    audio/backgroundmusic.h \
    game/reversitile.h \
    gamewindow.h \
    onlinecontroller/onlinecontroller.h \
    screens/game/regulargame.h \
    screens/game/turnsscreen.h \
    screens/main/mainscreen.h \
    screens/online/lobby/lobbyscreen.h \
    screens/online/menu/onlinemenuscreen.h \
    screens/online/onlinescreen.h \
    screens/online/pending/onlinependingscreen.h \
    screens/pause/pausescreen.h

FORMS += \
    gamewindow.ui \
    screens/game/regulargame.ui \
    screens/game/turnsscreen.ui \
    screens/main/mainscreen.ui \
    screens/online/lobby/lobbyscreen.ui \
    screens/online/menu/onlinemenuscreen.ui \
    screens/online/onlinescreen.ui \
    screens/online/pending/onlinependingscreen.ui \
    screens/pause/pausescreen.ui

#### Automatic naming ####
# fix app name
LOWER_DASHED_APP_NAME = $$lower($$join(APP_NAME,-))

AUTHOR_DOMAIN_SPLIT           = $$split(AUTHOR_DOMAIN,.)
AUTHOR_DOMAIN_SPLIT_REVERSED  = $$reverse(AUTHOR_DOMAIN_SPLIT)
AUTHOR_DOMAIN_REVERSED        = $$join(AUTHOR_DOMAIN_SPLIT_REVERSED,.)

# name to use for /usr/share/* directory, default is just the APP_NAME
SHARE_APP_NAME = $$LOWER_DASHED_APP_NAME

# executable file name, by default this is the APP_NAME turned lowercase
TARGET = $$LOWER_DASHED_APP_NAME

# set git commit ID, useful for bug reports and such
GIT_VERSION = $$system(git rev-parse --short=8 HEAD)

# desktop file name
DESKTOP_NAME = $$AUTHOR_DOMAIN_REVERSED.$$LOWER_DASHED_APP_NAME

#### Dist ####
unix:!macx:!android {
    # Include the-libs build tools
    include(/usr/share/the-libs/pri/buildmaster.pri)

    target.path = /usr/bin

    desktop.path = /usr/share/applications
    desktop.files = $$DESKTOP_NAME.desktop

    icon.path = /usr/share/icons/hicolor/scalable/apps/
    icon.files = icons/$$LOWER_DASHED_APP_NAME.svg

    INSTALLS += target desktop icon
}

#### Checking ####
# error out if APP_NAME is undefined
isEmpty(APP_NAME){
    error(Please set APP_NAME!)
}

# APP_VERSION will also be warned at compile time
isEmpty(APP_VERSION){
    warning(APP_VERSION will be \'undefined\'! Please set it!)
}

#### Add defines ####
# apply the defines
!isEmpty(GIT_VERSION){
    DEFINES += GIT_VERSION=\\\"$$GIT_VERSION\\\"
}
!isEmpty(APP_VERSION){
    DEFINES += APP_VERSION=\\\"$$APP_VERSION\\\"
}
!isEmpty(DESKTOP_NAME){
    DEFINES += DESKTOP_NAME=\\\"$$DESKTOP_NAME\\\"
}

#### Compilation ####
# Qt
QT       += core gui
QT       += svg gamepad
QT       += network websockets
QT       += multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

# the-libs
QT       += thelib entertaining

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

# Linux: support for modfiles
unix:!macx: LIBS += -lmodplug

RESOURCES += \
    resources.qrc
