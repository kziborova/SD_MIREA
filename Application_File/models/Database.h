#pragma once

#include <QSqlDatabase>
#include <QSqlError>
#include <QDebug>

class Database {
public:
    static bool init();
    static QSqlDatabase& get();
private:
    static QSqlDatabase db;
};