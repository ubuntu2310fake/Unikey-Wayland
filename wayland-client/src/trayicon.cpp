#include "trayicon.h"
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QFont>
#include <QTimer>
#include <QElapsedTimer>
#include <QDBusConnection>
#include <QDBusMessage>

StatusNotifierItemAdaptor::StatusNotifierItemAdaptor(TrayIcon* parent)
    : QDBusAbstractAdaptor(parent) {
}

QString StatusNotifierItemAdaptor::iconName() const {
    TrayIcon* tray = static_cast<TrayIcon*>(parent());
    return tray->isVietMode() ? "unikey-wayland-v" : "unikey-wayland-e";
}

void StatusNotifierItemAdaptor::Activate(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
    TrayIcon* tray = static_cast<TrayIcon*>(parent());
    tray->toggleMode();
}

void StatusNotifierItemAdaptor::SecondaryActivate(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
    TrayIcon* tray = static_cast<TrayIcon*>(parent());
    tray->onShowControlPanel();
}

void StatusNotifierItemAdaptor::ContextMenu(int x, int y) {
    Q_UNUSED(x);
    Q_UNUSED(y);
    TrayIcon* tray = static_cast<TrayIcon*>(parent());
    tray->onShowControlPanel();
}

#include <QDBusServiceWatcher>

TrayIcon::TrayIcon(bool* p_viet_mode, MainWindow* mainWindow, bool is_gnome, QObject* parent)
    : QObject(parent), p_viet_mode(p_viet_mode), m_mainWindow(mainWindow), m_isGnome(is_gnome), m_dbusAdaptor(nullptr), m_serviceWatcher(nullptr) {

    m_trayMenu = new QMenu();
    m_actionControlPanel = m_trayMenu->addAction("Bảng điều khiển... [CS+F5]");
    m_trayMenu->addSeparator();
    m_actionQuit = m_trayMenu->addAction("Kết thúc");

    connect(m_actionControlPanel, &QAction::triggered, this, &TrayIcon::onShowControlPanel);
    connect(m_actionQuit, &QAction::triggered, this, &TrayIcon::onQuit);

    // Register DBus StatusNotifierItem directly for KDE Plasma Wayland taskbar
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (bus.isConnected()) {
        m_dbusAdaptor = new StatusNotifierItemAdaptor(this);
        bus.registerObject("/StatusNotifierItem", this, QDBusConnection::ExportAdaptors);

        // Watch for StatusNotifierWatcher (in case Plasma/Watcher starts after Unikey or restarts)
        m_serviceWatcher = new QDBusServiceWatcher(
            "org.kde.StatusNotifierWatcher",
            bus,
            QDBusServiceWatcher::WatchForOwnerChange | QDBusServiceWatcher::WatchForRegistration,
            this
        );
        connect(m_serviceWatcher, &QDBusServiceWatcher::serviceRegistered, this, &TrayIcon::registerWithWatcher);
        connect(m_serviceWatcher, &QDBusServiceWatcher::serviceOwnerChanged, this, [this](const QString&, const QString&, const QString& newOwner) {
            if (!newOwner.isEmpty()) {
                registerWithWatcher();
            }
        });

        // Listen for new tray hosts registering (e.g. system tray reloaded/restarted)
        bus.connect(
            "org.kde.StatusNotifierWatcher",
            "/StatusNotifierWatcher",
            "org.kde.StatusNotifierWatcher",
            "StatusNotifierHostRegistered",
            this,
            SLOT(registerWithWatcher())
        );

        // Register immediately
        registerWithWatcher();

        // Extra retry shots for slow startup race conditions
        QTimer::singleShot(500, this, &TrayIcon::registerWithWatcher);
        QTimer::singleShot(1500, this, &TrayIcon::registerWithWatcher);
    }

    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TrayIcon::updateIcon);
    timer->start(100);
}

TrayIcon::~TrayIcon() {
    delete m_trayMenu;
}

void TrayIcon::registerWithWatcher() {
    QDBusConnection bus = QDBusConnection::sessionBus();
    if (!bus.isConnected()) return;

    QDBusMessage msg = QDBusMessage::createMethodCall(
        "org.kde.StatusNotifierWatcher",
        "/StatusNotifierWatcher",
        "org.kde.StatusNotifierWatcher",
        "RegisterStatusNotifierItem"
    );
    msg << "/StatusNotifierItem";
    bus.call(msg);

    if (m_dbusAdaptor) {
        emit m_dbusAdaptor->NewIcon();
        emit m_dbusAdaptor->NewTitle();
        emit m_dbusAdaptor->NewStatus(QString("Active"));
    }
}

void TrayIcon::toggleMode() {
    if (p_viet_mode) {
        *p_viet_mode = !(*p_viet_mode);
        if (m_mainWindow) {
            m_mainWindow->setVietMode(*p_viet_mode);
        }
    }
    updateIcon();
}

void TrayIcon::updateIcon() {
    static bool lastMode = !(*p_viet_mode);
    bool currentMode = *p_viet_mode;
    if (currentMode == lastMode) return;
    lastMode = currentMode;

    if (m_dbusAdaptor) {
        emit m_dbusAdaptor->NewIcon();
        emit m_dbusAdaptor->NewTitle();
    }
}

void TrayIcon::onShowControlPanel() {
    if (m_mainWindow) {
        m_mainWindow->show();
        m_mainWindow->raise();
        m_mainWindow->activateWindow();
    }
}

void TrayIcon::onQuit() {
    QApplication::quit();
}
