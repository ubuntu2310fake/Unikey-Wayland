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
#include "ukengine_wrapper.h"
#include "macrodialog.h"

class MainWindow : public QWidget {
    Q_OBJECT
public:
    explicit MainWindow(UkEngineWrapper* engine, QWidget *parent = nullptr);
    void setVietMode(bool viet);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
    void applySettings();
    void onCloseClicked();
    void onMacroButtonClicked();

private:
    UkEngineWrapper* m_engine;
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
    
    // Gõ tắt
    QCheckBox* m_macroWithConsonantCheck;
    QPushButton* m_macroTableBtn;

    QPushButton* m_closeBtn;
    QPushButton* m_defaultBtn;

    // Config persistence
    void loadConfig();
    void saveConfig();
    QString getConfigPath() const;
    bool m_loadingConfig = false;
    std::map<std::string, std::string> m_macros;
};

#endif // MAINWINDOW_H
