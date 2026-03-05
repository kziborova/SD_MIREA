#include <QApplication>
#include <QMessageBox>
#include "qt/mainwindow.h"
#include "qt/logindialog.h"
#include "../models/Auth.h"
#include "../models/Database.h"
 
int main(int argc, char* argv[]) {
    QApplication a(argc, argv);

    if (!Database::init()) {
        qDebug() << "Cannot connect to database";
        return 1;
    }

    LoginDialog login;
    if (login.exec() != QDialog::Accepted)
        return 0;

    MainWindow w;
    w.setWindowIcon(QIcon(":/icons/app_icons.png"));
    w.show();

    return a.exec();
}