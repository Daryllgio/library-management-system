#include "rolewindows.hpp"
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>

LibrarianWindow::LibrarianWindow(const QString& name, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString("HinLIBS — Librarian: %1").arg(name));
    auto* lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel("Librarian interface placeholder for D1.\nClose this window to return to Startup."));
    auto* closeBtn = new QPushButton("Close");
    lay->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    resize(480, 180);
}

AdminWindow::AdminWindow(const QString& name, QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle(QString("HinLIBS — Administrator: %1").arg(name));
    auto* lay = new QVBoxLayout(this);
    lay->addWidget(new QLabel("Admin interface placeholder for D1.\nClose this window to return to Startup."));
    auto* closeBtn = new QPushButton("Close");
    lay->addWidget(closeBtn);
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    resize(480, 180);
}
