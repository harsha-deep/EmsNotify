#include <QApplication>
#include <QPushButton>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);

    QPushButton button("EmsNotify - Qt Works!");
    button.resize(300, 120);
    button.show();

    return app.exec();
}
