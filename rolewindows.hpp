#pragma once
#include <QDialog>
#include <QString>

// Simple placeholder windows for Librarian/Admin so the startup form
// "displays the appropriate interface"
class LibrarianWindow : public QDialog {
    Q_OBJECT
public:
    explicit LibrarianWindow(const QString& name, QWidget* parent = nullptr);
};

class AdminWindow : public QDialog {
    Q_OBJECT
public:
    explicit AdminWindow(const QString& name, QWidget* parent = nullptr);
};
