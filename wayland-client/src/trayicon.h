#ifndef TRAYICON_H
#define TRAYICON_H

#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include "mainwindow.h"

class TrayIcon : public QObject {
    Q_OBJECT
public:
    TrayIcon(bool* p_viet_mode, MainWindow* mainWindow, bool is_gnome = false, QObject* parent = nullptr);
    ~TrayIcon();

    void updateIcon();

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void onShowControlPanel();
    void onQuit();

private:
    bool* p_viet_mode;
    MainWindow* m_mainWindow;
    bool m_isGnome;
    QSystemTrayIcon* m_trayIcon;
    QMenu* m_trayMenu;
    QAction* m_actionControlPanel;

    QAction* m_actionQuit;
    
    QIcon m_iconV;
    QIcon m_iconE;
};

#endif // TRAYICON_H
