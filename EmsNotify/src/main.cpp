#include <QApplication>
#include "tray_manager.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QApplication::setQuitOnLastWindowClosed(false);

    TrayManager tray;

    return app.exec();
}