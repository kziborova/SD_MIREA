#include "Files.h"
#include "Database.h"

bool FilesModel::addFile(const QString& filename, const QString& location, int size, const QString& ownerId){
    QSqlDatabase db = Database::get();
    QSqlQuery query(db);
    query.prepare("INSERT INTO Files (filename, location, size, owner_id) "
                  "VALUES (:filename, :location, :size, :owner_id)");
    query.bindValue(":filename", filename);
    query.bindValue(":location", location);
    query.bindValue(":size", size);
    query.bindValue(":owner_id", ownerId);

    return query.exec();
}

FileRecord FilesModel::getFile(const QString& location, const QString& ownerId) {
    FileRecord record;

    QSqlDatabase db = Database::get();
    QSqlQuery query(db);
    query.prepare("SELECT id, filename, size, owner_id, created_at FROM Files "
                  "WHERE location = :location AND owner_id = :owner_id");
    query.bindValue(":location", location);
    query.bindValue(":owner_id", ownerId);

    if (query.exec() && query.next()) {
        record.id = query.value("id").toInt();
        record.filename = query.value("filename").toString();
        record.location = location;
        record.size = query.value("size").toInt();
        record.ownerId = query.value("owner_id").toString();
        record.createdAt = query.value("created_at").toDateTime();
        return record;
    }

    QFileInfo fi(location);
    if (fi.exists()) {
        record.filename = fi.fileName();
        record.location = fi.absoluteFilePath();
        record.size = static_cast<int>(fi.size());
        record.ownerId = ownerId;
        record.createdAt = QDateTime::currentDateTime();

        if (addFile(record.filename, record.location, record.size, ownerId)) {
            QSqlQuery q(db);
            q.prepare("SELECT id, created_at FROM Files WHERE location = :location AND owner_id = :owner_id");
            q.bindValue(":location", record.location);
            q.bindValue(":owner_id", ownerId);
            if (q.exec() && q.next()) {
                record.id = q.value("id").toInt();
                record.createdAt = q.value("created_at").toDateTime();
            }
        }
    }

    return record;
}

bool FilesModel::removeFile(const QString& location) {
    QSqlDatabase db = Database::get();
    QSqlQuery query(db);
    query.prepare("DELETE FROM Files WHERE location = :location");
    query.bindValue(":location", location);

    return query.exec() && query.numRowsAffected() > 0;
}