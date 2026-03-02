#include <QApplication>
#include <QDebug>

#include "tray_manager.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    // Prevent app from quitting when window is closed
    QApplication::setQuitOnLastWindowClosed(false);

    // Optional: Application metadata (recommended)
    QCoreApplication::setOrganizationName("CSG");
    QCoreApplication::setApplicationName("EmsNotify");
    QCoreApplication::setApplicationVersion("1.0.1");

#ifdef NDEBUG
    qDebug() << "Release build";
#else
    qDebug() << "Debug build";
#endif

    TrayManager tray;

    return app.exec();
}