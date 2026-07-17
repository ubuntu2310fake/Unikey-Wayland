#ifndef WINDOWTRACKER_H
#define WINDOWTRACKER_H

#include <QObject>
#include <QString>
#include <QDBusAbstractAdaptor>
#include <QDBusConnection>
#include <string>
#include <vector>

class WindowTracker : public QObject {
    Q_OBJECT
public:
    explicit WindowTracker(QObject *parent = nullptr);
    void injectKWinScript();
    
    std::string getActiveWindowClass() const { return m_activeWindowClass; }
    bool isAppExcluded(const std::string& appClass) const;
    void loadExcludedApps();
    void reloadExcludedApps();

signals:
    void activeWindowChangedSignal(const QString& windowClass);

public slots:
    void activeWindowChanged(const QString& windowClass);

private:
    std::string m_activeWindowClass;
    std::vector<std::string> m_excludedApps;
};

class WindowTrackerAdaptor : public QDBusAbstractAdaptor {
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "io.github.ubuntu2310fake.UnikeyWayland.WindowTracker")
public:
    explicit WindowTrackerAdaptor(WindowTracker *parent) : QDBusAbstractAdaptor(parent), tracker(parent) {}
public slots:
    void activeWindowChanged(const QString& windowClass) {
        tracker->activeWindowChanged(windowClass);
    }
private:
    WindowTracker *tracker;
};

#endif // WINDOWTRACKER_H
