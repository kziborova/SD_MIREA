#pragma once
#include <QString>
#include <QSqlQuery>

#include "Database.h"

class AuthService {
    public:
    static bool login(const QString& username, const QString& password);

    static QString currentUserId() { return s_currentUserId; }

    private:
    static QString s_currentUserId;
};