#ifndef TRAYICON_H
#define TRAYICON_H

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QAction>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <QDBusMessage>
#include "mainwindow.h"

class TrayIcon;

class StatusNotifierItemAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.StatusNotifierItem")
    Q_PROPERTY(QString Category READ category)
    Q_PROPERTY(QString Id READ id)
    Q_PROPERTY(QString Title READ title)
    Q_PROPERTY(QString Status READ status)
    Q_PROPERTY(QString IconName READ iconName)
    Q_PROPERTY(QString IconThemePath READ iconThemePath)
    Q_PROPERTY(bool ItemIsMenu READ itemIsMenu)

public:
    explicit StatusNotifierItemAdaptor(TrayIcon* parent);

    QString category() const { return "ApplicationStatus"; }
    QString id() const { return "unikey-wayland"; }
    QString title() const { return "UniKey"; }
    QString status() const { return "Active"; }
    QString iconName() const;
    QString iconThemePath() const { return "/home/quan/.local/share/icons"; }
    bool itemIsMenu() const { return false; }

public slots:
    void ContextMenu(int x, int y);
    void Activate(int x, int y);
    void SecondaryActivate(int x, int y);
    void Scroll(int delta, const QString& orientation) { Q_UNUSED(delta); Q_UNUSED(orientation); }

signals:
    void NewTitle();
    void NewIcon();
    void NewStatus(const QString& status);
};

class QDBusServiceWatcher;

class TrayIcon : public QObject {
    Q_OBJECT
public:
    TrayIcon(bool* p_viet_mode, MainWindow* mainWindow, bool is_gnome = false, QObject* parent = nullptr);
    ~TrayIcon();

    void updateIcon();
    bool isVietMode() const { return p_viet_mode ? *p_viet_mode : true; }
    void toggleMode();

public slots:
    void registerWithWatcher();
    void onShowControlPanel();
    void onQuit();

private:
    bool* p_viet_mode;
    MainWindow* m_mainWindow;
    bool m_isGnome;
    QMenu* m_trayMenu;
    QAction* m_actionControlPanel;
    QAction* m_actionQuit;

    StatusNotifierItemAdaptor* m_dbusAdaptor = nullptr;
    QDBusServiceWatcher* m_serviceWatcher = nullptr;
};

#endif // TRAYICON_H
