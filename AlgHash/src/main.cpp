#include <QApplication>
#include "qt/mainwindow.hpp"
 
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/app_icons.png"));
    w.show();

    return a.exec();
}