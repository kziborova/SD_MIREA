#include "Database.h"

QSqlDatabase Database::db = QSqlDatabase();

bool Database::init() {
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setHostName("localhost");
    db.setDatabaseName("AppFile");
    db.setUserName("postgres");
    db.setPassword("137985");

    if (!db.open()) {
        qDebug() << "DB error:" << db.lastError().text();
        return false;
    }

    return true;
}

QSqlDatabase& Database::get() {
    return db;
}