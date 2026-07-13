#include "trayicon.h"
#include <QApplication>
#include <QPainter>
#include <QPixmap>
#include <QColor>
#include <QFont>

static QIcon createIconWithText(const QString& text, QColor bgColor, QColor textColor) {
    QPixmap pixmap(64, 64);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    // Draw rounded rect background
    painter.setBrush(bgColor);
    painter.setPen(Qt::NoPen);
    painter.drawRoundedRect(4, 4, 56, 56, 12, 12);

    // Draw text
    painter.setPen(textColor);
    QFont font = painter.font();
    font.setPixelSize(48);
    font.setBold(true);
    painter.setFont(font);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

    return QIcon(pixmap);
}

#include <QTimer>

TrayIcon::TrayIcon(bool* p_viet_mode, MainWindow* mainWindow, bool is_gnome, QObject* parent)
    : QObject(parent), p_viet_mode(p_viet_mode), m_mainWindow(mainWindow), m_isGnome(is_gnome) {
    
    // Create icons dynamically
    m_iconV = createIconWithText("V", QColor(220, 53, 69), Qt::white); // Red background for V
    m_iconE = createIconWithText("E", QColor(0, 123, 255), Qt::white); // Blue background for E

    m_trayIcon = new QSystemTrayIcon(this);
    
    m_trayMenu = new QMenu();
    m_actionControlPanel = m_trayMenu->addAction("Bảng điều khiển... [CS+F5]");
    m_trayMenu->addSeparator();
    m_actionQuit = m_trayMenu->addAction("Kết thúc");

    m_trayIcon->setContextMenu(m_trayMenu);

    connect(m_trayIcon, &QSystemTrayIcon::activated, this, &TrayIcon::onTrayIconActivated);
    connect(m_actionControlPanel, &QAction::triggered, this, &TrayIcon::onShowControlPanel);
    connect(m_actionQuit, &QAction::triggered, this, &TrayIcon::onQuit);

    updateIcon();
    m_trayIcon->show();

    // Start timer to poll status for hotkey updates
    QTimer* timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &TrayIcon::updateIcon);
    timer->start(100);
}

TrayIcon::~TrayIcon() {
    delete m_trayIcon;
    delete m_trayMenu;
}

void TrayIcon::updateIcon() {
    static bool lastMode = !(*p_viet_mode); // Force initial update
    bool currentMode = *p_viet_mode;
    if (currentMode == lastMode) return; // Avoid setting icon unnecessarily
    lastMode = currentMode;

    bool isViet = p_viet_mode ? *p_viet_mode : true;
    if (m_isGnome) {
        if (isViet) {
            m_trayIcon->setIcon(QIcon::fromTheme("unikey-vietnamese", QIcon(":/icons/unikey-vietnamese.png")));
        } else {
            m_trayIcon->setIcon(QIcon::fromTheme("unikey-english", QIcon(":/icons/unikey-english.png")));
        }
    } else {
        if (isViet) {
            m_trayIcon->setIcon(m_iconV);
            m_trayIcon->setToolTip("UniKey - Vietnamese");
        } else {
            m_trayIcon->setIcon(m_iconE);
            m_trayIcon->setToolTip("UniKey - English");
        }
    }
}

#include <QElapsedTimer>

void TrayIcon::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason) {
    static QElapsedTimer clickTimer;
    
    if (reason == QSystemTrayIcon::Trigger) {
        if (m_isGnome) {
            onShowControlPanel();
            return;
        }

        if (clickTimer.isValid() && clickTimer.elapsed() < 400) {
            // Double click detected via timing!
            // Undo the first click's toggle by toggling again
            if (p_viet_mode) {
                *p_viet_mode = !(*p_viet_mode);
                m_mainWindow->setVietMode(*p_viet_mode);
            }
            updateIcon();
            onShowControlPanel();
        } else {
            // Single click: toggle E/V
            if (p_viet_mode) {
                *p_viet_mode = !(*p_viet_mode);
                m_mainWindow->setVietMode(*p_viet_mode);
            }
            updateIcon();
        }
        clickTimer.restart();
    } else if (reason == QSystemTrayIcon::DoubleClick || reason == QSystemTrayIcon::MiddleClick) {
        onShowControlPanel();
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
