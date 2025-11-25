#pragma once
#include <QDialog>

class QLineEdit;
class QPushButton;

class StartupDialog : public QDialog {
    Q_OBJECT
public:
    explicit StartupDialog(QWidget* parent = nullptr);

private slots:
    void handleEnter();

private:
    QLineEdit*   m_nameEdit;
    QPushButton* m_enterBtn;
    void openPatronUI(const QString& name);
    void openLibrarianUI(const QString& name);
    void openAdminUI(const QString& name);
};
