#ifndef MACRODIALOG_H
#define MACRODIALOG_H

#include <QDialog>
#include <QTableWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <map>
#include <string>

class MacroDialog : public QDialog {
    Q_OBJECT
public:
    explicit MacroDialog(QWidget *parent = nullptr);
    std::map<std::string, std::string> getMacroTable() const;
    void setMacroTable(const std::map<std::string, std::string>& macroTable);

private slots:
    void onAddClicked();
    void onRemoveClicked();
    void onSaveClicked();
    void onCancelClicked();

private:
    QTableWidget* m_table;
    QLineEdit* m_keyEdit;
    QLineEdit* m_textEdit;
    QPushButton* m_addBtn;
    QPushButton* m_removeBtn;
    QPushButton* m_saveBtn;
    QPushButton* m_cancelBtn;
};

#endif // MACRODIALOG_H
