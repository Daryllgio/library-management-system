#include <QApplication>
#include "startupdialog.hpp"

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    StartupDialog dlg;
    dlg.show();
    return app.exec();
}
