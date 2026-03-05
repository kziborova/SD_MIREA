#pragma once

#include <QString>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QFileInfo>
#include <QDateTime>

struct FileRecord {
    int id = 0;
    QString filename;
    QString location;
    int size = 0;
    QString ownerId = 0;
    QDateTime createdAt;
};

class FilesModel {
public:
    static bool addFile(const QString& filename, const QString& location, int size, const QString& ownerId);
    static FileRecord getFile(const QString& location, const QString& ownerId);
    static bool removeFile(const QString& location);
};