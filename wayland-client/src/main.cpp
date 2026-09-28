#include <QApplication>
#include <iostream>
#include "mainwindow.h"
#include "trayicon.h"

int main(int argc, char **argv) {
    QApplication app(argc, argv);
    
    // Qt settings
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName("UnikeyWayland");
    
    bool viet_mode = true;
    bool is_gnome = false;
    
    MainWindow mainWindow(&viet_mode, is_gnome);
    TrayIcon trayIcon(&viet_mode, &mainWindow, is_gnome);
    
    if (argc > 1 && std::string(argv[1]) == "--setup") {
        mainWindow.show();
    }
    
    return app.exec();
}
