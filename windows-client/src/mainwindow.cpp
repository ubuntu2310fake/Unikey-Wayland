#include "mainwindow.h"
#include "keycons.h"
#include <QStandardPaths>
#include <QDir>
#include <vector>
#include <map>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QFileInfo>
#include <QPlainTextEdit>
#include <QSettings>
#include <QCoreApplication>
#include <QNetworkRequest>
#include <QMessageBox>
#include <QProcess>
#include <QProgressDialog>
#include <QTimer>
#include <QApplication>

MainWindow::MainWindow(bool* p_viet_mode, bool is_gnome, QWidget *parent)
    : QWidget(parent), p_viet_mode(p_viet_mode) {
    setWindowTitle("Unikey-Wayland");
    setFixedSize(450, 360);

    m_networkManager = new QNetworkAccessManager(this);
    m_reply = nullptr;
    m_progressDialog = nullptr;

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    m_tabWidget = new QTabWidget(this);
    mainLayout->addWidget(m_tabWidget);

    // --- Tab Cơ bản ---
    QWidget* tabBasic = new QWidget();
    QVBoxLayout* basicLayout = new QVBoxLayout(tabBasic);

    QGridLayout* comboLayout = new QGridLayout();
    comboLayout->addWidget(new QLabel("Bảng mã:"), 0, 0);
    m_charsetCombo = new QComboBox();
    m_charsetCombo->addItem("Unicode", QVariant(UNICODE_CHARSET));
    m_charsetCombo->addItem("TCVN3 (ABC)", QVariant(TCVN3_CHARSET));
    m_charsetCombo->addItem("VNI Windows", QVariant(VNI_CHARSET));
    m_charsetCombo->addItem("VIQR", QVariant(VIQR_CHARSET));
    comboLayout->addWidget(m_charsetCombo, 0, 1);

    comboLayout->addWidget(new QLabel("Kiểu gõ:"), 1, 0);
    m_methodCombo = new QComboBox();
    m_methodCombo->addItem("Telex", QVariant(TELEX_INPUT));
    m_methodCombo->addItem("Telex giản lược", QVariant(TELEX_SIMPLIFIED_INPUT));
    m_methodCombo->addItem("VNI", QVariant(VNI_INPUT));
    m_methodCombo->addItem("VIQR", QVariant(VIQR_INPUT));
    comboLayout->addWidget(m_methodCombo, 1, 1);
    basicLayout->addLayout(comboLayout);

    QFrame* line1 = new QFrame();
    line1->setFrameShape(QFrame::HLine);
    line1->setFrameShadow(QFrame::Sunken);
    basicLayout->addWidget(line1);

    m_freeMarkingCheck = new QCheckBox("Cho phép gõ tự do (free marking)");
    m_freeMarkingCheck->setChecked(true);
    m_modernStyleCheck = new QCheckBox("Đặt dấu oà, uý (thay vì òa, úy)");
    m_autoRestoreCheck = new QCheckBox("Tự động khôi phục phím với từ sai");
    m_spellCheckCheck = new QCheckBox("Kiểm tra chính tả");
    m_macroCheck = new QCheckBox("Cho phép gõ tắt");

    basicLayout->addWidget(m_freeMarkingCheck);
    basicLayout->addWidget(m_modernStyleCheck);
    basicLayout->addWidget(m_autoRestoreCheck);
    basicLayout->addWidget(m_spellCheckCheck);
    basicLayout->addWidget(m_macroCheck);
    basicLayout->addStretch();

    m_tabWidget->addTab(tabBasic, "Cơ bản");

    // --- Tab Phím tắt ---
    m_evHotkeyCombo = new QComboBox();
    m_evHotkeyCombo->addItems({"Ctrl + Shift", "Alt + Z"});
    m_restoreHotkeyCombo = new QComboBox();
    m_restoreHotkeyCombo->addItems({"Shift + Backspace", "Alt + Backspace"});

    if (!is_gnome) {
        QWidget* tabHotkey = new QWidget();
        QVBoxLayout* hotkeyLayout = new QVBoxLayout(tabHotkey);
        
        QGridLayout* hkGrid = new QGridLayout();
        hkGrid->addWidget(new QLabel("Phím chuyển E/V:"), 0, 0);
        hkGrid->addWidget(m_evHotkeyCombo, 0, 1);

        hkGrid->addWidget(new QLabel("Phục hồi từ sai:"), 1, 0);
        hkGrid->addWidget(m_restoreHotkeyCombo, 1, 1);
        hotkeyLayout->addLayout(hkGrid);
        hotkeyLayout->addStretch();
        m_tabWidget->addTab(tabHotkey, "Phím tắt");
    }

    // --- Tab Hệ thống ---
    QWidget* tabSystem = new QWidget();
    QVBoxLayout* sysLayout = new QVBoxLayout(tabSystem);
    m_showOnStartupCheck = new QCheckBox("Bật hội thoại này khi khởi động");
    m_runAtStartupCheck = new QCheckBox("Khởi động cùng Windows");
    m_updateBtn = new QPushButton("Kiểm tra cập nhật (OTA)");
    
    sysLayout->addWidget(m_showOnStartupCheck);
    sysLayout->addWidget(m_runAtStartupCheck);
    sysLayout->addWidget(m_updateBtn);
    sysLayout->addStretch();
    m_tabWidget->addTab(tabSystem, "Hệ thống");

    // --- Tab Gõ tắt ---
    QWidget* tabMacro = new QWidget();
    QVBoxLayout* macroLayout = new QVBoxLayout(tabMacro);
    m_macroWithConsonantCheck = new QCheckBox("Gõ tắt kể cả khi gõ tắt cùng phụ âm");
    m_macroTableBtn = new QPushButton("Bảng gõ tắt...");
    macroLayout->addWidget(m_macroWithConsonantCheck);
    macroLayout->addWidget(m_macroTableBtn);
    macroLayout->addStretch();
    m_tabWidget->addTab(tabMacro, "Gõ tắt");

    // --- Tab Thông tin ---
    QWidget* tabAbout = new QWidget();
    QVBoxLayout* aboutLayout = new QVBoxLayout(tabAbout);
    QLabel* titleLabel = new QLabel("<h2>Unikey-Wayland (Windows Edition)</h2>");
    titleLabel->setAlignment(Qt::AlignCenter);
    QLabel* infoLabel = new QLabel("Dựa trên mã nguồn UniKey\nKiến trúc: Win32 Global Hook + Tiêm phím");
    infoLabel->setAlignment(Qt::AlignCenter);
    aboutLayout->addStretch();
    aboutLayout->addWidget(titleLabel);
    aboutLayout->addWidget(infoLabel);
    aboutLayout->addStretch();
    m_tabWidget->addTab(tabAbout, "Thông tin");

    // --- Bottom Buttons ---
    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_defaultBtn = new QPushButton("Mặc định");
    m_closeBtn = new QPushButton("Đóng");
    btnLayout->addWidget(m_defaultBtn);
    btnLayout->addStretch();
    btnLayout->addWidget(m_closeBtn);
    mainLayout->addLayout(btnLayout);

    // Connections
    connect(m_charsetCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applySettings);
    connect(m_methodCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applySettings);
    connect(m_freeMarkingCheck, &QCheckBox::stateChanged, this, &MainWindow::applySettings);
    connect(m_modernStyleCheck, &QCheckBox::stateChanged, this, &MainWindow::applySettings);
    connect(m_autoRestoreCheck, &QCheckBox::stateChanged, this, &MainWindow::applySettings);
    connect(m_spellCheckCheck, &QCheckBox::stateChanged, this, &MainWindow::applySettings);
    connect(m_macroCheck, &QCheckBox::stateChanged, this, &MainWindow::applySettings);
    connect(m_evHotkeyCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &MainWindow::applySettings);
    connect(m_runAtStartupCheck, &QCheckBox::stateChanged, this, &MainWindow::applySettings);
    
    connect(m_macroTableBtn, &QPushButton::clicked, this, &MainWindow::onMacroButtonClicked);
    connect(m_closeBtn, &QPushButton::clicked, this, &MainWindow::onCloseClicked);
    connect(m_updateBtn, &QPushButton::clicked, this, [this](){ checkForUpdates(false); });
    
    // Load persisted configuration
    loadConfig();

    // Default config application to engine
    applySettings();

    // Check update on startup silently after 3 seconds
    QTimer::singleShot(3000, this, [this]() { checkForUpdates(true); });
}

void MainWindow::applySettings() {
    int method = m_methodCombo->currentData().toInt();
    int charset = m_charsetCombo->currentData().toInt();
    
    // Toàn bộ logic tùy chọn phức tạp đã được thay bằng Bamboo CGO hiện đại.
    // Các UI này giữ lại để không phá vỡ layout, nhưng không cần làm gì ở đây.

    saveConfig();
}

void MainWindow::onMacroButtonClicked() {
    MacroDialog dialog(this);
    dialog.setMacroTable(m_macros);
    if (dialog.exec() == QDialog::Accepted) {
        m_macros = dialog.getMacroTable();
        // Không cần truyền xuống engine nữa
        saveConfig();
    }
}

void MainWindow::onCloseClicked() {
    hide();
}

void MainWindow::closeEvent(QCloseEvent *event) {
    event->ignore();
    hide();
}

void MainWindow::setVietMode(bool viet) {
    if (p_viet_mode) *p_viet_mode = viet;
    saveConfig();
}

QString MainWindow::getConfigPath() const {
    QString path = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/Unikey/global.json";
    QFileInfo fi(path);
    QDir().mkpath(fi.absolutePath());
    return path;
}

void MainWindow::loadConfig() {
    m_loadingConfig = true;
    
    QFile file(getConfigPath());
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        file.close();
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            
            // Set comboboxes
            int method = obj.value("method").toInt(0);
            int charset = obj.value("charset").toInt(0);
            
            int methodIdx = m_methodCombo->findData(method);
            if (methodIdx != -1) m_methodCombo->setCurrentIndex(methodIdx);
            
            int charsetIdx = m_charsetCombo->findData(charset);
            if (charsetIdx != -1) m_charsetCombo->setCurrentIndex(charsetIdx);
            
            // Set checkboxes
            m_freeMarkingCheck->setChecked(obj.value("freeMarking").toBool(true));
            m_modernStyleCheck->setChecked(obj.value("modernStyle").toBool(false));
            m_autoRestoreCheck->setChecked(obj.value("autoRestore").toBool(false));
            m_spellCheckCheck->setChecked(obj.value("spellCheck").toBool(false));
            m_macroCheck->setChecked(obj.value("macroEnabled").toBool(false));
            m_showOnStartupCheck->setChecked(obj.value("showOnStartup").toBool(true));
            m_macroWithConsonantCheck->setChecked(obj.value("macroWithConsonant").toBool(false));
            
            // Hotkeys
            int switchKey = obj.value("switchKey").toInt(0);
            m_evHotkeyCombo->setCurrentIndex(switchKey);
            
            // E/V Mode
            bool vietKey = obj.value("vietKey").toBool(true);
            if (p_viet_mode) *p_viet_mode = vietKey;

            // Load macros
            m_macros.clear();
            QJsonObject macroObj = obj.value("macros").toObject();
            for (auto it = macroObj.begin(); it != macroObj.end(); ++it) {
                m_macros[it.key().toStdString()] = it.value().toString().toStdString();
            }
        }
    }

    QSettings bootUpSettings("HKEY_CURRENT_USER\\\\Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run", QSettings::NativeFormat);
    m_runAtStartupCheck->setChecked(bootUpSettings.contains("UnikeyWayland"));

    m_loadingConfig = false;
}

void MainWindow::saveConfig() {
    if (m_loadingConfig) return;

    QString configPath = getConfigPath();
    QDir().mkpath(QFileInfo(configPath).absolutePath());

    QFile file(configPath);
    if (file.open(QIODevice::WriteOnly)) {
        QJsonObject obj;
        obj["method"] = m_methodCombo->currentData().toInt();
        obj["charset"] = m_charsetCombo->currentData().toInt();
        obj["freeMarking"] = m_freeMarkingCheck->isChecked();
        obj["modernStyle"] = m_modernStyleCheck->isChecked();
        obj["autoRestore"] = m_autoRestoreCheck->isChecked();
        obj["spellCheck"] = m_spellCheckCheck->isChecked();
        obj["macroEnabled"] = m_macroCheck->isChecked();
        obj["showOnStartup"] = m_showOnStartupCheck->isChecked();
        obj["macroWithConsonant"] = m_macroWithConsonantCheck->isChecked();
        obj["switchKey"] = m_evHotkeyCombo->currentIndex();
        obj["vietKey"] = p_viet_mode ? *p_viet_mode : true;
        
        QJsonObject macroObj;
        for (const auto& pair : m_macros) {
            macroObj[QString::fromStdString(pair.first)] = QString::fromStdString(pair.second);
        }
        obj["macros"] = macroObj;
        
        QJsonDocument doc(obj);
        file.write(doc.toJson());
        file.close();
    }

    QSettings bootUpSettings("HKEY_CURRENT_USER\\\\Software\\\\Microsoft\\\\Windows\\\\CurrentVersion\\\\Run", QSettings::NativeFormat);
    if (m_runAtStartupCheck->isChecked()) {
        bootUpSettings.setValue("UnikeyWayland", QDir::toNativeSeparators(QCoreApplication::applicationFilePath()));
    } else {
        bootUpSettings.remove("UnikeyWayland");
    }
}

void MainWindow::selectTab(const QString& name) {
    for (int i = 0; i < m_tabWidget->count(); ++i) {
        if (m_tabWidget->tabText(i) == name) {
            m_tabWidget->setCurrentIndex(i);
            break;
        }
    }
}

QString MainWindow::getCurrentVersion() {
    QFile file(QCoreApplication::applicationFilePath());
    if (file.open(QIODevice::ReadOnly)) {
        qint64 size = file.size();
        qint64 readSize = qMin(size, (qint64)1024);
        file.seek(size - readSize);
        QByteArray data = file.read(readSize);
        int startIdx = data.lastIndexOf("[UKW_VERSION]");
        int endIdx = data.lastIndexOf("[/UKW_VERSION]");
        if (startIdx != -1 && endIdx != -1 && startIdx < endIdx) {
            startIdx += 13; // length of [UKW_VERSION]
            return QString(data.mid(startIdx, endIdx - startIdx));
        }
    }
    return "2.0.0"; // default fallback
}

void MainWindow::checkForUpdates(bool silent) {
    m_silentUpdateCheck = silent;
    if (!silent) {
        m_updateBtn->setEnabled(false);
        m_updateBtn->setText("Đang kiểm tra...");
    }

    QUrl url("https://api.github.com/repos/ubuntu2310fake/Unikey-Wayland/releases/latest");
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "UnikeyWayland-Updater");

    QNetworkReply* reply = m_networkManager->get(request);
    connect(reply, &QNetworkReply::finished, this, &MainWindow::onUpdateCheckFinished);
}

void MainWindow::onUpdateCheckFinished() {
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    reply->deleteLater();

    if (!m_silentUpdateCheck) {
        m_updateBtn->setEnabled(true);
        m_updateBtn->setText("Kiểm tra cập nhật (OTA)");
    }

    if (reply->error() != QNetworkReply::NoError) {
        if (!m_silentUpdateCheck) {
            QMessageBox::warning(this, "Lỗi", "Không thể kiểm tra bản cập nhật:\\n" + reply->errorString());
        }
        return;
    }

    QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    QJsonObject obj = doc.object();
    QString latestVersion = obj.value("tag_name").toString();
    QString currentVersion = getCurrentVersion();

    if (latestVersion.isEmpty() || latestVersion == currentVersion) {
        if (!m_silentUpdateCheck) {
            QMessageBox::information(this, "Cập nhật", "Bạn đang sử dụng phiên bản mới nhất (" + currentVersion + ").");
        }
        return;
    }

    m_latestVersion = latestVersion;
    QJsonArray assets = obj.value("assets").toArray();
    
#if defined(__aarch64__) || defined(_M_ARM64)
    QString targetAsset = "UnikeyWayland-Windows-ARM64.zip";
#else
    QString targetAsset = "UnikeyWayland-Windows-x64.zip";
#endif

    m_downloadUrl.clear();
    for (int i = 0; i < assets.size(); ++i) {
        QJsonObject asset = assets[i].toObject();
        if (asset.value("name").toString() == targetAsset) {
            m_downloadUrl = asset.value("browser_download_url").toString();
            break;
        }
    }

    if (m_downloadUrl.isEmpty()) {
        if (!m_silentUpdateCheck) {
            QMessageBox::warning(this, "Lỗi", "Không tìm thấy file cài đặt cho nền tảng này.");
        }
        return;
    }

    QMessageBox::StandardButton btn = QMessageBox::question(this, "Có bản cập nhật mới",
        "Phiên bản mới " + latestVersion + " đã sẵn sàng (Hiện tại: " + currentVersion + ").\\n\\nBạn có muốn tải xuống và cài đặt ngay không?");

    if (btn == QMessageBox::Yes) {
        QNetworkRequest request;
        request.setUrl(QUrl(m_downloadUrl));
        request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        request.setRawHeader("User-Agent", "UnikeyWayland-Updater");
        
        m_reply = m_networkManager->get(request);
        connect(m_reply, &QNetworkReply::finished, this, &MainWindow::onUpdateDownloadFinished);
        connect(m_reply, &QNetworkReply::downloadProgress, this, &MainWindow::onUpdateDownloadProgress);
        
        m_progressDialog = new QProgressDialog("Đang tải xuống bản cập nhật...", "Hủy", 0, 100, this);
        m_progressDialog->setWindowTitle("Cập nhật OTA");
        m_progressDialog->setWindowModality(Qt::WindowModal);
        connect(m_progressDialog, &QProgressDialog::canceled, m_reply, &QNetworkReply::abort);
        
        this->setEnabled(false);
    }
}

void MainWindow::onUpdateDownloadProgress(qint64 bytesReceived, qint64 bytesTotal) {
    if (m_progressDialog && bytesTotal > 0) {
        m_progressDialog->setMaximum(bytesTotal);
        m_progressDialog->setValue(bytesReceived);
    }
}

void MainWindow::onUpdateDownloadFinished() {
    this->setEnabled(true);
    if (m_progressDialog) {
        m_progressDialog->deleteLater();
        m_progressDialog = nullptr;
    }

    QNetworkReply* reply = m_reply;
    m_reply = nullptr;
    if (!reply) return;
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        if (reply->error() != QNetworkReply::OperationCanceledError) {
            QMessageBox::warning(this, "Lỗi tải xuống", reply->errorString());
        }
        return;
    }

    QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/UnikeyWayland-Update.zip";
    QFile file(tempPath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
    } else {
        QMessageBox::warning(this, "Lỗi", "Không thể ghi file cập nhật vào thư mục Temp.");
        return;
    }

    QString extractDir = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/UnikeyWayland-Update";
    QDir dir(extractDir);
    if (dir.exists()) dir.removeRecursively();
    QDir().mkpath(extractDir);

    QString psCmd = QString("Expand-Archive -Path '%1' -DestinationPath '%2' -Force").arg(tempPath).arg(extractDir);
    
    QProcess ps;
    ps.start("powershell", QStringList() << "-Command" << psCmd);
    ps.waitForFinished(-1);

    if (ps.exitCode() != 0) {
        QMessageBox::warning(this, "Lỗi giải nén", "Không thể giải nén file cập nhật.");
        return;
    }

    QString setupBat = extractDir + "/setup.bat";
    if (QFile::exists(setupBat)) {
        // Run setup.bat asynchronously and exit
        QProcess::startDetached("cmd.exe", QStringList() << "/c" << "start" << "\"\"" << QDir::toNativeSeparators(setupBat));
        QApplication::quit();
    } else {
        QMessageBox::warning(this, "Lỗi cài đặt", "Không tìm thấy file setup.bat trong bản cập nhật.");
    }
}
