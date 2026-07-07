#include "macrodialog.h"
#include <QHeaderView>

MacroDialog::MacroDialog(QWidget *parent) : QDialog(parent) {
    setWindowTitle("Bảng gõ tắt");
    resize(400, 300);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);

    m_table = new QTableWidget(0, 2, this);
    m_table->setHorizontalHeaderLabels({"Từ gõ tắt", "Bởi từ (đầy đủ)"});
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    mainLayout->addWidget(m_table);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    m_keyEdit = new QLineEdit(this);
    m_keyEdit->setPlaceholderText("Gõ tắt");
    m_textEdit = new QLineEdit(this);
    m_textEdit->setPlaceholderText("Thay thế bằng");
    inputLayout->addWidget(m_keyEdit);
    inputLayout->addWidget(m_textEdit);
    mainLayout->addLayout(inputLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    m_addBtn = new QPushButton("Thêm / Sửa", this);
    m_removeBtn = new QPushButton("Xoá", this);
    btnLayout->addWidget(m_addBtn);
    btnLayout->addWidget(m_removeBtn);
    btnLayout->addStretch();
    m_saveBtn = new QPushButton("Lưu", this);
    m_cancelBtn = new QPushButton("Huỷ", this);
    btnLayout->addWidget(m_saveBtn);
    btnLayout->addWidget(m_cancelBtn);
    mainLayout->addLayout(btnLayout);

    connect(m_addBtn, &QPushButton::clicked, this, &MacroDialog::onAddClicked);
    connect(m_removeBtn, &QPushButton::clicked, this, &MacroDialog::onRemoveClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &MacroDialog::onSaveClicked);
    connect(m_cancelBtn, &QPushButton::clicked, this, &MacroDialog::onCancelClicked);
}

std::map<std::string, std::string> MacroDialog::getMacroTable() const {
    std::map<std::string, std::string> macros;
    for (int i = 0; i < m_table->rowCount(); ++i) {
        std::string key = m_table->item(i, 0)->text().toStdString();
        std::string text = m_table->item(i, 1)->text().toStdString();
        if (!key.empty() && !text.empty()) {
            macros[key] = text;
        }
    }
    return macros;
}

void MacroDialog::onAddClicked() {
    QString key = m_keyEdit->text().trimmed();
    QString text = m_textEdit->text().trimmed();
    if (key.isEmpty() || text.isEmpty()) return;

    // Check if key exists
    for (int i = 0; i < m_table->rowCount(); ++i) {
        if (m_table->item(i, 0)->text() == key) {
            m_table->item(i, 1)->setText(text);
            m_keyEdit->clear();
            m_textEdit->clear();
            return;
        }
    }

    int row = m_table->rowCount();
    m_table->insertRow(row);
    m_table->setItem(row, 0, new QTableWidgetItem(key));
    m_table->setItem(row, 1, new QTableWidgetItem(text));
    
    m_keyEdit->clear();
    m_textEdit->clear();
}

void MacroDialog::onRemoveClicked() {
    int row = m_table->currentRow();
    if (row >= 0) {
        m_table->removeRow(row);
    }
}

void MacroDialog::onSaveClicked() {
    accept();
}

void MacroDialog::onCancelClicked() {
    reject();
}

void MacroDialog::setMacroTable(const std::map<std::string, std::string>& macroTable) {
    m_table->setRowCount(0);
    for (const auto& pair : macroTable) {
        int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(pair.first)));
        m_table->setItem(row, 1, new QTableWidgetItem(QString::fromStdString(pair.second)));
    }
}
