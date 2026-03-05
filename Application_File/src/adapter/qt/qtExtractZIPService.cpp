#include "qtExtractZIPService.h"

void QTExtractZIPService::extractFile(const std::vector<std::string>& vec, const std::string& place) {
    if (vec.empty()) return;

    QString filePath = file.fileName();
    QString destPath = QString::fromStdString(place);
    QString ownerId = AuthService::currentUserId(); 

    file.close();

    QuaZip zip(filePath);
    if (!zip.open(QuaZip::mdUnzip))
        throw std::runtime_error("cannot open zip");

    int total = vec.size();
    int current = 0;

    for (const auto& entry : vec) {
        QString entryName = QString::fromStdString(entry);
        if (!zip.setCurrentFile(entryName))
            throw std::runtime_error("entry not found: " + entry);

        QuaZipFile zipFile(&zip);
        if (!zipFile.open(QIODevice::ReadOnly))
            throw std::runtime_error("cannot open entry: " + entry);

        QString fileName = QFileInfo(entryName).fileName();
        QString destFilePath = QFileInfo(destPath + QDir::separator() + fileName).absoluteFilePath();

        QFile outFile(destFilePath);
        if (!outFile.open(QIODevice::WriteOnly))
            throw std::runtime_error("cannot create file: " + destFilePath.toStdString());

        const qint64 chunkSize = 64 * 1024;
        while (!zipFile.atEnd())
            outFile.write(zipFile.read(chunkSize));

        outFile.close();
        zipFile.close();

        FilesModel::getFile(destFilePath, ownerId);

        if (onProgress && total > 0)
            onProgress(++current * 100 / total);
    }
    zip.close();

    if (!file.open(QIODevice::ReadWrite))
        throw std::runtime_error("cannot reopen file after extract");
}

bool QTExtractZIPService::checkBomb(const std::vector<std::string>& vec, const std::string& place) {
    if (vec.empty()) return false;

    QString zipPath = file.fileName();
    QString destPath = QString::fromStdString(place);

    QuaZip zip(zipPath);
    if (!zip.open(QuaZip::mdUnzip))
        throw std::runtime_error("cannot open zip for bomb check");

    qint64 totalUncompressedSize = 0;

    for (const auto& entry : vec) {
        QString entryName = QString::fromStdString(entry);
        if (!zip.setCurrentFile(entryName))
            throw std::runtime_error("entry not found during bomb check: " + entry);

        QuaZipFile zipFile(&zip);
        if (!zipFile.open(QIODevice::ReadOnly))
            throw std::runtime_error("cannot open entry during bomb check: " + entry);

        totalUncompressedSize += zipFile.size();
        zipFile.close();
    }

    zip.close();

    QFileInfo fi(zipPath);
    qint64 archiveSize = fi.size();
    if (totalUncompressedSize > archiveSize * 100) {
        throw std::runtime_error("ZIP bomb detected: total uncompressed size is too large");
    }

    QDir dir(destPath);
    QStorageInfo storage(dir.rootPath());
    qint64 freeSpace = storage.bytesAvailable();

    if (totalUncompressedSize > freeSpace) {
        throw std::runtime_error("Not enough disk space for extraction");
    }
    return false;
}