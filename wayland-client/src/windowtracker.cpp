#include "windowtracker.h"
#include <QDBusInterface>
#include <QDBusReply>
#include <QDBusConnection>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QStandardPaths>
#include <iostream>
#include <fstream>

WindowTracker::WindowTracker(QObject *parent) : QObject(parent) {
    loadExcludedApps();

    // Register DBus object
    new WindowTrackerAdaptor(this);
    if (!QDBusConnection::sessionBus().registerService("org.unikey.Wayland")) {
        qWarning() << "Failed to register org.unikey.Wayland DBus service:" << QDBusConnection::sessionBus().lastError().message();
    }
    if (!QDBusConnection::sessionBus().registerObject("/WindowTracker", this)) {
        qWarning() << "Failed to register /WindowTracker object";
    }
}

void WindowTracker::loadExcludedApps() {
    m_excludedApps.clear();
    QString configPath = QDir::homePath() + "/UnikeyWayland/preedit_apps.txt";

    QFile file(configPath);
    if (!file.exists()) {
        QDir().mkpath(QFileInfo(configPath).absolutePath());
        if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream out(&file);
            out << "kitty\n";
            out << "alacritty\n";
            out << "konsole\n";
            out << "gnome-terminal\n";
            out << "xfce4-terminal\n";
            out << "lxterminal\n";
            out << "studio\n";
            out << "java\n";
            file.close();
        }
    }

    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine().trimmed();
            if (!line.isEmpty() && !line.startsWith("#")) {
                m_excludedApps.push_back(line.toLower().toStdString());
            }
        }
        file.close();
    }
}

void WindowTracker::reloadExcludedApps() {
    loadExcludedApps();
    // Re-evaluate current active window
    emit activeWindowChangedSignal(QString::fromStdString(m_activeWindowClass));
}

bool WindowTracker::isAppExcluded(const std::string& appClass) const {
    if (appClass.empty()) return false;
    std::string lowerClass;
    for (char c : appClass) {
        lowerClass += std::tolower(c);
    }
    
    for (const auto& excluded : m_excludedApps) {
        if (lowerClass.find(excluded) != std::string::npos) {
            return true;
        }
    }
    return false;
}

void log_to_file(const std::string& msg);

void WindowTracker::activeWindowChanged(const QString& windowClass) {
    m_activeWindowClass = windowClass.toStdString();
    qDebug() << "ACTIVE WINDOW CHANGED:" << windowClass;
    std::string msg = "DEBUG: WindowTracker received: " + m_activeWindowClass;
    
    QFile f("/tmp/tracker.log");
    if (f.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&f);
        out << QString::fromStdString(msg) << "\n";
        f.close();
    }

    emit activeWindowChangedSignal(windowClass);
}

void WindowTracker::injectKWinScript() {
    // Inject script via KWin DBus
    QDBusInterface kwinScripting("org.kde.KWin", "/Scripting", "org.kde.kwin.Scripting", QDBusConnection::sessionBus());
    if (!kwinScripting.isValid()) {
        qWarning() << "Could not connect to org.kde.KWin /Scripting. Are you running KDE Plasma Wayland?";
        return;
    }

    QString scriptCode = 
        "workspace.windowActivated.connect(function(client) {\n"
        "    if (client) {\n"
        "        callDBus('org.unikey.Wayland', '/WindowTracker', 'org.unikey.Wayland.WindowTracker', 'activeWindowChanged', client.resourceClass.toString());\n"
        "    } else {\n"
        "        callDBus('org.unikey.Wayland', '/WindowTracker', 'org.unikey.Wayland.WindowTracker', 'activeWindowChanged', '');\n"
        "    }\n"
        "});\n"
        "if (workspace.activeWindow) {\n"
        "    callDBus('org.unikey.Wayland', '/WindowTracker', 'org.unikey.Wayland.WindowTracker', 'activeWindowChanged', workspace.activeWindow.resourceClass.toString());\n"
        "}\n";

    // Write to a temporary file
    QString tmpPath = QDir::tempPath() + "/unikey_wayland_tracker.js";
    QFile tmpFile(tmpPath);
    if (tmpFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&tmpFile);
        out << scriptCode;
        tmpFile.close();
    }

    QDBusReply<int> reply = kwinScripting.call("loadScript", tmpPath);
    if (reply.isValid()) {
        int scriptId = reply.value();
        QString scriptPath = QString("/Scripting/Script%1").arg(scriptId);
        QDBusInterface scriptObj("org.kde.KWin", scriptPath, "org.kde.kwin.Script", QDBusConnection::sessionBus());
        scriptObj.call("run");
        qDebug() << "Successfully loaded and started KWin window tracker script at" << scriptPath;
    } else {
        qWarning() << "Failed to load KWin script:" << reply.error().message();
    }
}
