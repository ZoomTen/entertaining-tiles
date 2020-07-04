// Qt
#include <QDir>
#include <QStandardPaths>

// the-libs
#include <tapplication.h>
#include <tsettings.h>

// libentertaining
#include <entertaining.h>
#include <musicengine.h>

// window
#include "gamewindow.h"

#ifndef APP_VERSION
    // emit compiler warning if app version isn't set from the makefile or qmake
    #warning "No version number set! Version number will be 'undefined' unless you specify it with -DAPP_VERSION=<version number>!"
    #define APP_VERSION "undefined"
#endif

int main(int argc, char* argv[]){
    tApplication a(argc, argv);

    #ifdef T_BLUEPRINT_BUILD
        a.setApplicationName("Entertaining Tiles Blueprint");
        a.setDesktopFileName(DESKTOP_NAME);
    #else
        a.setApplicationName("Entertaining Reversi");
        a.setDesktopFileName(DESKTOP_NAME);
    #endif

    a.setGenericName(QApplication::translate("main","Reversi/Othello Game"));

    a.setApplicationVersion(APP_VERSION);

    a.setApplicationIcon(
                QIcon::fromTheme("entertaining-tiles",
                                 QIcon(":/icons/entertaining-tiles.svg")
                                 ));

    a.setApplicationLicense(tApplication::Gpl3OrLater);

    a.setCopyrightHolder("Zumi Daxuya");
    a.setCopyrightYear("2020");

    #ifdef GIT_VERSION
        // add git commit ID to the about page
        a.addCopyrightLine(
                    QApplication::tr("Git version %1").arg(GIT_VERSION)
                    );
    #endif

    a.addLibraryVersion("Entertaining", "1.0"); // is there a library version defined?

        // set share path, this will be important for translations and such
        QString userSharePath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
        QString localSharePath = QDir::cleanPath(QApplication::applicationDirPath() + "/../share/entertaining-tiles/");
        QString systemSharePath = "/usr/share/entertaining-tiles"; // use global system dir (LINUX)

        if (QDir(userSharePath).exists()) {
            // if user share path exists, use that
            a.setShareDir(userSharePath);
        } else if (QDir(systemSharePath).exists()){
            // if not, use the system share path
            a.setShareDir(systemSharePath);
        } else {
            /* otherwise, use a share dir hack located one level above the
               executable location
            */
            a.setShareDir(localSharePath);
        }

        // set defaults file
        QString defaultsPath = QStandardPaths::writableLocation(
                    QStandardPaths::AppDataLocation);
        defaultsPath += "/defaults.conf";
        tSettings::registerDefaults(defaultsPath);   // use local config dir
        tSettings::registerDefaults("/etc/entertaining-tiles/defaults.conf"); // use global config dir (LINUX)

        // setup i18n (it reads the translation files from <sharedir>/translations
        a.installTranslators();

        // setup Entertaining library
        Entertaining::initialize();


        // run app
        GameWindow w;
        w.show();

        return a.exec();
}
