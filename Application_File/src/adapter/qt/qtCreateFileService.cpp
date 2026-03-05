#include "qtCreateFileService.h"

bool QTCreateFileService::createFile(const std::string& rawPath) {
    QString qPath = QString::fromStdString(rawPath);
    QFileInfo info(qPath);
    QFile file(qPath);

    if (!file.open(QIODevice::WriteOnly)) {
        file.close();
        return false;
    }
    file.close();
    info.refresh();

    QString location = info.absoluteFilePath();
    QString ownerId = AuthService::currentUserId();

    qDebug() << "location:" << location;
    qDebug() << "ownerId:" << ownerId;

    FileRecord record = FilesModel::getFile(location, ownerId);

    qDebug() << "record.id:" << record.id;

    if (record.id == 0) {
        qDebug() << "record.id is 0, operation not added";
        return true;
    }

    OperationsModel::addOperation(OperationType::Create, record.id, ownerId);
    return true;
}