#include <QFile>
#include <QApplication>
#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextEdit>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSlider>
#include <QListWidget>
#include <QTabWidget>
#include <QProcess>
#include <QDateTime>
#include <QKeyEvent>
#include <QInputMethodEvent>
#include <QQueue>
#include <QTimer>
#include <QGroupBox>
#include <cmath>
#include <chrono>

// High-resolution nanosecond clock helper
uint64_t get_gui_nano_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC_RAW, &ts);
    return (uint64_t)ts.tv_sec * 1000000000ULL + ts.tv_nsec;
}

// Structure to store hardware key press details
struct HwKeyPress {
    int keycode;
    uint64_t ns;
};

// Queue to match hardware keys to GUI events
static QQueue<HwKeyPress> g_hw_press_queue;
static uint64_t g_last_hw_press_ns = 0;

// Benchmark metrics
struct BenchStats {
    QList<double> latencies_ms;
    int expected_chars = 0;
    int committed_chars = 0;
    int dropped_keys = 0;
};
static BenchStats g_bench_stats;
static bool g_bench_active = false;

class ImTestEdit : public QTextEdit {
    Q_OBJECT
public:
    ImTestEdit(QWidget* parent = nullptr) : QTextEdit(parent) {}

signals:
    void logMessage(const QString& msg);

protected:
    void inputMethodEvent(QInputMethodEvent *e) override {
        uint64_t gui_ns = get_gui_nano_time();
        QTextEdit::inputMethodEvent(e);

        QString commit = e->commitString();
        QString preedit = e->preeditString();

        if (!commit.isEmpty() || !preedit.isEmpty()) {
            uint64_t hw_ns = g_last_hw_press_ns;
            // Try to pull from queue if available
            if (!g_hw_press_queue.isEmpty()) {
                HwKeyPress hw = g_hw_press_queue.dequeue();
                hw_ns = hw.ns;
            }

            double latency_ms = 0.0;
            if (hw_ns > 0) {
                latency_ms = (double)(gui_ns - hw_ns) / 1000000.0;
            }

            QString eventType = !commit.isEmpty() ? "COMMIT" : "PREEDIT";
            QString content = !commit.isEmpty() ? commit : preedit;

            if (g_bench_active) {
                if (!commit.isEmpty()) {
                    g_bench_stats.committed_chars += commit.length();
                }
                if (latency_ms > 0) {
                    g_bench_stats.latencies_ms.append(latency_ms);
                }
            }

            QString log = QString("[%1] %2: '%3' | HW Time: %4 ns | GUI Time: %5 ns | Latency: %6 ms")
                            .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                            .arg(eventType)
                            .arg(content)
                            .arg(hw_ns)
                            .arg(gui_ns)
                            .arg(QString::number(latency_ms, 'f', 4));
            emit logMessage(log);
        }
    }

    void keyPressEvent(QKeyEvent *e) override {
        uint64_t gui_ns = get_gui_nano_time();
        QTextEdit::keyPressEvent(e);

        // If the key was forwarded (not intercepted by IM)
        uint64_t hw_ns = g_last_hw_press_ns;
        if (!g_hw_press_queue.isEmpty()) {
            HwKeyPress hw = g_hw_press_queue.dequeue();
            hw_ns = hw.ns;
        }

        double latency_ms = 0.0;
        if (hw_ns > 0) {
            latency_ms = (double)(gui_ns - hw_ns) / 1000000.0;
        }

        if (g_bench_active && latency_ms > 0) {
            g_bench_stats.latencies_ms.append(latency_ms);
        }

        QString log = QString("[%1] KEY_PRESS: key=0x%2 ('%3') | HW Time: %4 ns | GUI Time: %5 ns | Latency: %6 ms")
                        .arg(QDateTime::currentDateTime().toString("hh:mm:ss.zzz"))
                        .arg(e->key(), 0, 16)
                        .arg(e->text())
                        .arg(hw_ns)
                        .arg(gui_ns)
                        .arg(QString::number(latency_ms, 'f', 4));
        emit logMessage(log);
    }
};

class MainWindow : public QWidget {
    Q_OBJECT
public:
    MainWindow(QWidget* parent = nullptr) : QWidget(parent) {
        setWindowTitle("Wayland IM Benchmark Tool");
        resize(800, 600);

        // Set dark premium stylesheet
        setStyleSheet(R"(
            QWidget {
                background-color: #121212;
                color: #e0e0e0;
                font-family: 'Outfit', 'Inter', sans-serif;
                font-size: 13px;
            }
            QGroupBox {
                border: 2px solid #292929;
                border-radius: 8px;
                margin-top: 10px;
                font-weight: bold;
            }
            QGroupBox::title {
                subcontrol-origin: margin;
                left: 10px;
                padding: 0 3px 0 3px;
            }
            QLineEdit {
                background-color: #1e1e1e;
                border: 1px solid #333333;
                border-radius: 4px;
                padding: 6px;
                color: #ffffff;
            }
            QPushButton {
                background-color: #3b82f6;
                color: white;
                border: none;
                border-radius: 4px;
                padding: 8px 16px;
                font-weight: bold;
            }
            QPushButton:hover {
                background-color: #2563eb;
            }
            QPushButton:disabled {
                background-color: #4b5563;
                color: #9ca3af;
            }
            QListWidget {
                background-color: #181818;
                border: 1px solid #292929;
                border-radius: 6px;
            }
            QSlider::groove:horizontal {
                border: 1px solid #333;
                height: 8px;
                background: #222;
                margin: 2px 0;
            }
            QSlider::handle:horizontal {
                background: #3b82f6;
                border: 1px solid #555;
                width: 18px;
                margin: -2px 0;
                border-radius: 9px;
            }
            QTabWidget::pane {
                border: 1px solid #292929;
                background: #121212;
            }
            QTabBar::tab {
                background: #1e1e1e;
                border: 1px solid #292929;
                padding: 8px 16px;
                margin-right: 2px;
            }
            QTabBar::tab:selected {
                background: #121212;
                border-bottom-color: #121212;
                color: #3b82f6;
            }
        )");

        QVBoxLayout* mainLayout = new QVBoxLayout(this);

        // Tab Widget
        QTabWidget* tabWidget = new QTabWidget();
        mainLayout->addWidget(tabWidget);

        // TAB 1: Live Monitor
        QWidget* monitorTab = new QWidget();
        QVBoxLayout* monitorLayout = new QVBoxLayout(monitorTab);
        
        QGroupBox* authGroup = new QGroupBox("1. Sniffer Authentication (Required for physical keystroke latency)");
        QHBoxLayout* authLayout = new QHBoxLayout(authGroup);
        authLayout->addWidget(new QLabel("Sudo Password:"));
        sudoPassEdit = new QLineEdit("299210");
        sudoPassEdit->setEchoMode(QLineEdit::Password);
        authLayout->addWidget(sudoPassEdit);
        btnStartSniff = new QPushButton("Start Sniffer");
        authLayout->addWidget(btnStartSniff);
        monitorLayout->addWidget(authGroup);

        QGroupBox* testInputGroup = new QGroupBox("2. Real-Time Typing Test");
        QVBoxLayout* testInputLayout = new QVBoxLayout(testInputGroup);
        textEdit = new ImTestEdit();
        textEdit->setPlaceholderText("Click here and type to measure input method latency...");
        testInputLayout->addWidget(textEdit);
        monitorLayout->addWidget(testInputGroup);

        QGroupBox* logGroup = new QGroupBox("3. Microsecond Latency Log");
        QVBoxLayout* logLayout = new QVBoxLayout(logGroup);
        logWidget = new QListWidget();
        logLayout->addWidget(logWidget);
        monitorLayout->addWidget(logGroup);

        tabWidget->addTab(monitorTab, "Live Monitor");

        // TAB 2: Benchmark
        QWidget* benchTab = new QWidget();
        QVBoxLayout* benchLayout = new QVBoxLayout(benchTab);

        QGroupBox* spamGroup = new QGroupBox("Input Method Stress Testing (Spamming)");
        QVBoxLayout* spamLayout = new QVBoxLayout(spamGroup);

        QHBoxLayout* seqLayout = new QHBoxLayout();
        seqLayout->addWidget(new QLabel("Spam Telex Sequence:"));
        spamSeqEdit = new QLineEdit("tieengs viets booj goox test ");
        seqLayout->addWidget(spamSeqEdit);
        btnLoad1000 = new QPushButton("Load 1000-word Test");
        seqLayout->addWidget(btnLoad1000);
        spamLayout->addLayout(seqLayout);

        QHBoxLayout* delayLayout = new QHBoxLayout();
        delayLayout->addWidget(new QLabel("Typing Speed (Delay per key):"));
        delaySlider = new QSlider(Qt::Horizontal);
        delaySlider->setRange(5, 200);
        delaySlider->setValue(20);
        lblDelayVal = new QLabel("20 ms (50 keys/sec)");
        delayLayout->addWidget(delaySlider);
        delayLayout->addWidget(lblDelayVal);
        spamLayout->addLayout(delayLayout);

        btnStartBench = new QPushButton("Run Stress Benchmark");
        spamLayout->addWidget(btnStartBench);
        benchLayout->addWidget(spamGroup);

        QGroupBox* reportGroup = new QGroupBox("Benchmark Report");
        QVBoxLayout* reportLayout = new QVBoxLayout(reportGroup);
        reportText = new QTextEdit();
        reportText->setReadOnly(true);
        reportLayout->addWidget(reportText);
        benchLayout->addWidget(reportGroup);

        tabWidget->addTab(benchTab, "Stress Benchmark");

        // Connections
        connect(btnStartSniff, &QPushButton::clicked, this, &MainWindow::startSniffing);
        connect(textEdit, &ImTestEdit::logMessage, this, &MainWindow::appendLog);
        connect(delaySlider, &QSlider::valueChanged, this, &MainWindow::updateDelayLabel);
        connect(btnStartBench, &QPushButton::clicked, this, &MainWindow::startBenchmark);
        connect(btnLoad1000, &QPushButton::clicked, this, [this]() {
            QFile file("/home/truonghieu/Downloads/Uk362/stress_test_1000_words.txt");
            if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
                spamSeqEdit->setText(QString::fromUtf8(file.readAll()).trimmed() + " ");
                file.close();
                reportText->setHtml("<b style='color:#10b981;'>Loaded 1000-word stress test sequence (5832 keystrokes). Set speed to 10ms and run!</b>");
            }
        });

        // Helper process setups
        sniffProcess = new QProcess(this);
        spamProcess = new QProcess(this);

        connect(sniffProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readSniffOutput);
        connect(spamProcess, &QProcess::readyReadStandardOutput, this, &MainWindow::readSpamOutput);
        connect(spamProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &MainWindow::benchmarkFinished);
    }

    ~MainWindow() {
        if (sniffProcess->state() == QProcess::Running) {
            sniffProcess->kill();
        }
    }

private slots:
    void startSniffing() {
        if (sniffProcess->state() == QProcess::Running) {
            sniffProcess->kill();
            btnStartSniff->setText("Start Sniffer");
            return;
        }

        logWidget->clear();
        QString program = "sudo";
        QStringList arguments;
        arguments << "-S" << "./im-helper" << "--sniff";

        sniffProcess->setWorkingDirectory(QCoreApplication::applicationDirPath());
        sniffProcess->start(program, arguments);

        // Write sudo password
        QString pass = sudoPassEdit->text() + "\n";
        sniffProcess->write(pass.toUtf8());

        btnStartSniff->setText("Stop Sniffer");
    }

    void readSniffOutput() {
        while (sniffProcess->canReadLine()) {
            QByteArray line = sniffProcess->readLine().trimmed();
            QString str = QString::fromUtf8(line);
            
            if (str.startsWith("HW_KEY ")) {
                QStringList parts = str.split(" ");
                if (parts.size() >= 4) {
                    int code = parts[1].toInt();
                    int val = parts[2].toInt();
                    uint64_t ns = parts[3].toULongLong();

                    if (val == 1) { // Key Press
                        HwKeyPress kp = { code, ns };
                        g_hw_press_queue.enqueue(kp);
                        g_last_hw_press_ns = ns;
                    }
                }
            } else if (str.startsWith("SNIFFER_READY")) {
                logWidget->addItem(">>> SNIFFER READY: Listening for hardware keys.");
            }
        }
    }

    void updateDelayLabel(int val) {
        lblDelayVal->setText(QString("%1 ms (%2 keys/sec)").arg(val).arg(1000 / val));
    }

    void startBenchmark() {
        if (g_bench_active) return;

        textEdit->clear();
        textEdit->setFocus();
        logWidget->clear();

        g_bench_stats.latencies_ms.clear();
        g_bench_stats.committed_chars = 0;
        g_bench_stats.expected_chars = spamSeqEdit->text().length();
        g_bench_stats.dropped_keys = 0;

        g_bench_active = true;
        btnStartBench->setEnabled(false);
        reportText->setHtml("<b style='color:#3b82f6;'>Benchmark running... Please wait for typing to finish.</b>");

        // Spawn spammer
        QString program = "sudo";
        QStringList arguments;
        arguments << "-S" << "./im-helper" << "--spam" << spamSeqEdit->text() << QString::number(delaySlider->value());

        spamProcess->setWorkingDirectory(QCoreApplication::applicationDirPath());
        spamProcess->start(program, arguments);

        // Write password
        QString pass = sudoPassEdit->text() + "\n";
        spamProcess->write(pass.toUtf8());
    }

    void readSpamOutput() {
        while (spamProcess->canReadLine()) {
            QByteArray line = spamProcess->readLine().trimmed();
            QString str = QString::fromUtf8(line);

            if (str.startsWith("SPAM_KEY ")) {
                QStringList parts = str.split(" ");
                if (parts.size() >= 4) {
                    int code = parts[1].toInt();
                    int val = parts[2].toInt();
                    uint64_t ns = parts[3].toULongLong();

                    if (val == 1) { // Press
                        HwKeyPress kp = { code, ns };
                        g_hw_press_queue.enqueue(kp);
                        g_last_hw_press_ns = ns;
                    }
                }
            }
        }
    }

    void benchmarkFinished() {
        // Wait briefly for all Qt events to settle
        QTimer::singleShot(1000, this, [this]() {
            g_bench_active = false;
            btnStartBench->setEnabled(true);

            // Compute statistics
            double sum = 0;
            double max_val = 0;
            double min_val = 999999.0;
            for (double l : g_bench_stats.latencies_ms) {
                sum += l;
                if (l > max_val) max_val = l;
                if (l < min_val) min_val = l;
            }

            double avg = g_bench_stats.latencies_ms.isEmpty() ? 0 : sum / g_bench_stats.latencies_ms.size();

            // Calculate std dev (jitter)
            double variance_sum = 0;
            for (double l : g_bench_stats.latencies_ms) {
                variance_sum += pow(l - avg, 2);
            }
            double jitter = g_bench_stats.latencies_ms.isEmpty() ? 0 : sqrt(variance_sum / g_bench_stats.latencies_ms.size());

            // Corrected Stability Calculation:
            // For direct-commit IMs, multiple backspaces and commits happen per key,
            // which causes incremental double-counting.
            // We measure the final text length remaining in the editor instead.
            int committed = textEdit->toPlainText().length();
            
            // Expected length estimation (approx. 72% of Telex keystroke length if custom, or exact if predefined)
            int expected = g_bench_stats.expected_chars;
            if (spamSeqEdit->text().startsWith("tieengs viets")) {
                expected = 39; // "tiếng việt bộ gõ tự động cực kỳ ổn định" + space
            } else if (spamSeqEdit->text().startsWith("trawm nawm trong coix ngwowif ta")) {
                expected = 4536; // 1000-word preset
            } else if (spamSeqEdit->text().startsWith("chuyeen")) {
                expected = 32; // "chuyên gia truyền thông tin học "
            } else if (spamSeqEdit->text().startsWith("duwowngs")) {
                expected = 32; // "đường đi vận chuyển nhanh chóng "
            } else {
                // Heuristic estimation for custom sequences:
                // Count spaces to guess words, estimate avg 5.5 chars per Vietnamese word
                expected = spamSeqEdit->text().split(" ", Qt::SkipEmptyParts).size() * 5;
                if (expected == 0) expected = committed;
            }

            double stability_rate = expected == 0 ? 100.0 : ((double)committed / (double)expected) * 100.0;
            if (stability_rate > 100.0) stability_rate = 100.0; // Cap at 100%

            QString report = QString(
                "<h3>--- BENCHMARK RESULTS ---</h3>"
                "<table>"
                "<tr><td><b>Total Keys Sent:</b></td><td>%1</td></tr>"
                "<tr><td><b>Final Text Length:</b></td><td>%2 chars</td></tr>"
                "<tr><td><b>Expected Text Length:</b></td><td>%3 chars</td></tr>"
                "<tr><td><b>Stability Rate (Accuracy):</b></td><td><span style='color:%7;'>%4%</span></td></tr>"
                "<tr><td><b>Min Latency:</b></td><td>%5 ms</td></tr>"
                "<tr><td><b>Max Latency:</b></td><td>%6 ms</td></tr>"
                "<tr><td><b>Average Latency:</b></td><td><b>%8 ms</b></td></tr>"
                "<tr><td><b>Jitter (Std Dev):</b></td><td>%9 ms</td></tr>"
                "</table>"
                "<br/><i>Note: Stability Rate below 100% means the Input Method dropped characters or backspaces under heavy load.</i>"
            )
            .arg(g_bench_stats.latencies_ms.size())
            .arg(committed)
            .arg(expected)
            .arg(QString::number(stability_rate, 'f', 2))
            .arg(min_val == 999999.0 ? "0" : QString::number(min_val, 'f', 4))
            .arg(QString::number(max_val, 'f', 4))
            .arg(stability_rate >= 98.0 ? "#10b981" : "#f59e0b")
            .arg(QString::number(avg, 'f', 4))
            .arg(QString::number(jitter, 'f', 4));

            reportText->setHtml(report);
        });
    }

    void appendLog(const QString& msg) {
        logWidget->addItem(msg);
        logWidget->scrollToBottom();
    }

private:
    QLineEdit* sudoPassEdit;
    QPushButton* btnStartSniff;
    ImTestEdit* textEdit;
    QListWidget* logWidget;

    QLineEdit* spamSeqEdit;
    QSlider* delaySlider;
    QLabel* lblDelayVal;
    QPushButton* btnStartBench;
    QPushButton* btnLoad1000;
    QTextEdit* reportText;

    QProcess* sniffProcess;
    QProcess* spamProcess;
};

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    MainWindow w;
    w.show();
    return app.exec();
}

#include "main.moc"
