#include "Operations.h"

static QString typeToString(OperationType type) {
    switch (type) {
        case OperationType::Create: return "create";
        case OperationType::Modify: return "modify";
        case OperationType::Delete: return "delete";
    }
    return "create";
}

bool OperationsModel::addOperation(OperationType type, int fileId, const QString& userId) {
    QSqlQuery query(Database::get());
    query.prepare("INSERT INTO Operations (type, file_id, user_id) "
                  "VALUES (:type, :file_id, :user_id)");
    query.bindValue(":type",    typeToString(type));
    query.bindValue(":file_id", fileId);
    query.bindValue(":user_id", userId);

    return query.exec();
}