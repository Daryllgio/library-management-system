#include "startupdialog.hpp"
#include "datastore.hpp"
#include "patronwindow.hpp"
#include "rolewindows.hpp"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>

StartupDialog::StartupDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle("HinLIBS — Startup");
    auto* layout = new QVBoxLayout(this);

    // Simple, friendly startup form (NOT real authentication)
    auto* label = new QLabel("Enter your name (e.g., Alice, Bob, Carmen, Dev, Eve, Librarian, Admin):");
    m_nameEdit  = new QLineEdit();
    m_enterBtn  = new QPushButton("Enter");

    layout->addWidget(label);
    layout->addWidget(m_nameEdit);
    layout->addWidget(m_enterBtn);

    connect(m_enterBtn, &QPushButton::clicked, this, &StartupDialog::handleEnter);

    // Nice defaults for demo
    m_nameEdit->setPlaceholderText("Type your name here...");
    m_nameEdit->setFocus();
    resize(520, 150);
}

void StartupDialog::handleEnter() {
    const QString name = m_nameEdit->text().trimmed();
    if (name.isEmpty()) {
        QMessageBox::warning(this, "Missing name", "Please enter a name.");
        return;
    }
    auto u = DataStore::instance().findUser(name);
    if (!u) {
        QMessageBox::warning(this, "Not found", "No user with that name exists in this demo.");
        return;
    }

    switch (u->type) {
        case UserType::Patron:    openPatronUI(u->name); break;
        case UserType::Librarian: openLibrarianUI(u->name); break;
        case UserType::Admin:     openAdminUI(u->name); break;
    }
}

void StartupDialog::openPatronUI(const QString& name) {
    // Hide startup while patron session is active
    this->hide();
    PatronWindow w(name);
    w.exec(); // modal session
    // When the patron window is closed, return to startup & clear field
    DataStore::instance().clearCurrentUserState();
    m_nameEdit->clear();
    this->show();
    this->activateWindow();
}

void StartupDialog::openLibrarianUI(const QString& name) {
    this->hide();
    LibrarianWindow w(name);
    w.exec();
    DataStore::instance().clearCurrentUserState();
    m_nameEdit->clear();
    this->show();
}

void StartupDialog::openAdminUI(const QString& name) {
    this->hide();
    AdminWindow w(name);
    w.exec();
    DataStore::instance().clearCurrentUserState();
    m_nameEdit->clear();
    this->show();
}
