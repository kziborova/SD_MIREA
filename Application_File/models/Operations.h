#pragma once
#include <QString>
#include <QDateTime>
#include <QSqlQuery>
#include <QSqlError>
#include <QDebug>

#include "Database.h"

enum class OperationType {
    Create,
    Modify,
    Delete
};

struct OperationRecord {
    int id = 0;
    QDateTime createdAt;
    OperationType type;
    int fileId = 0;
    QString userId;
};

class OperationsModel {
public:
    static bool addOperation(OperationType type, int fileId, const QString& userId);
};