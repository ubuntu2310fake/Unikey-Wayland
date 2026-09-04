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
    if (!QDBusConnection::sessionBus().registerService("io.github.ubuntu2310fake.UnikeyWayland")) {
        qWarning() << "Failed to register io.github.ubuntu2310fake.UnikeyWayland DBus service:" << QDBusConnection::sessionBus().lastError().message();
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
            out << "google docs\n";
            out << "google tài liệu\n";
            out << "docs.google.com\n";
            out << "google sheets\n";
            out << "google trang tính\n";
            out << "sheets.google.com\n";
            out << "google slides\n";
            out << "google trình bày\n";
            out << "google trang trình bày\n";
            out << "slides.google.com\n";
            out << "google forms\n";
            out << "google biểu mẫu\n";
            out << "forms.google.com\n";
            out << "discord\n";
            out << "kitty\n";
            out << "alacritty\n";
            out << "konsole\n";
            out << "gnome-terminal\n";
            out << "xfce4-terminal\n";
            out << "lxterminal\n";
            out << "android-studio\n";
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
        "function notifyActiveWindow(client) {\n"
        "    if (client) {\n"
        "        var resClass = (client.resourceClass ? client.resourceClass : (client.desktopFileName ? client.desktopFileName : '')).toString();\n"
        "        var title = (client.caption ? client.caption : (client.title ? client.title : '')).toString();\n"
        "        callDBus('io.github.ubuntu2310fake.UnikeyWayland', '/WindowTracker', 'io.github.ubuntu2310fake.UnikeyWayland.WindowTracker', 'activeWindowChanged', resClass + '|||' + title);\n"
        "    } else {\n"
        "        callDBus('io.github.ubuntu2310fake.UnikeyWayland', '/WindowTracker', 'io.github.ubuntu2310fake.UnikeyWayland.WindowTracker', 'activeWindowChanged', '');\n"
        "    }\n"
        "}\n"
        "workspace.windowActivated.connect(function(client) {\n"
        "    notifyActiveWindow(client);\n"
        "    if (client) {\n"
        "        try {\n"
        "            client.captionChanged.connect(function() {\n"
        "                notifyActiveWindow(client);\n"
        "            });\n"
        "        } catch (e) {}\n"
        "    }\n"
        "});\n"
        "var active = workspace.activeWindow ? workspace.activeWindow : workspace.activeClient;\n"
        "if (active) {\n"
        "    notifyActiveWindow(active);\n"
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
