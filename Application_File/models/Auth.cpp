#include "Auth.h"

QString AuthService::s_currentUserId = "";

bool AuthService::login(const QString& username, const QString& password) {
    QSqlQuery q;
    q.prepare(R"(
        SELECT id
        FROM users
        WHERE username = :u
          AND password = crypt(:p, password)
    )");

    q.bindValue(":u", username);
    q.bindValue(":p", password);

    if (q.exec() && q.next()) {
        s_currentUserId = q.value(0).toString();
        return true;
    }

    return false;
}