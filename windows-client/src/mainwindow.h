#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QTabWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QCloseEvent>
#include <QNetworkAccessManager>
#include <QNetworkReply>

#include "macrodialog.h"

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(bool* p_viet_mode, bool is_gnome = false, QWidget *parent = nullptr);
    void setVietMode(bool viet);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void applySettings();
    void onCloseClicked();
    void onMacroButtonClicked();
    void checkForUpdates(bool silent = false);
    void onUpdateCheckFinished();
    void onUpdateDownloadFinished();
    void onUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);

private:
    QNetworkAccessManager* m_networkManager;
    QNetworkReply* m_reply;
    QString m_latestVersion;
    QString m_downloadUrl;
    bool m_silentUpdateCheck;
    QPushButton* m_updateBtn;
    class QProgressDialog* m_progressDialog;

    QString getCurrentVersion();
    bool* p_viet_mode;
    QTabWidget* m_tabWidget;

    // Cơ bản
    QComboBox* m_charsetCombo;
    QComboBox* m_methodCombo;
    QCheckBox* m_freeMarkingCheck;
    QCheckBox* m_modernStyleCheck;
    QCheckBox* m_macroCheck;
    QCheckBox* m_autoRestoreCheck;
    QCheckBox* m_spellCheckCheck;

    // Phím tắt
    QComboBox* m_evHotkeyCombo;
    QComboBox* m_restoreHotkeyCombo;

    // Hệ thống
    QCheckBox* m_showOnStartupCheck;
    QCheckBox* m_runAtStartupCheck;
    
    QCheckBox* m_macroWithConsonantCheck;
    QPushButton* m_macroTableBtn;
    QPushButton* m_defaultBtn;

    // Config persistence
    void loadConfig();
    void saveConfig();
    QString getConfigPath() const;
    bool m_loadingConfig = false;
    std::map<std::string, std::string> m_macros;

public:
    const std::map<std::string, std::string>& getMacros() const { return m_macros; }
    bool isMacroEnabled() const { return m_macroCheck->isChecked(); }
    int getSwitchKey() const { return m_evHotkeyCombo->currentIndex(); }
    bool isShowOnStartupEnabled() const { return m_showOnStartupCheck->isChecked(); }

    // Preedit exclude/include apps list
    QPushButton* m_closeBtn;

public:
    void selectTab(const QString& name);
};

#endif // MAINWINDOW_H
