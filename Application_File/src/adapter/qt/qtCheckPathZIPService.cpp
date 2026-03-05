#include "qtCheckPathZIPService.h"

bool QTCheckPathZIPService::checking(const std::string& rawPath, PathAccessModeZIP mode) {
    path = PathValidationResZIP{};

    QString qRaw = QString::fromStdString(rawPath).trimmed();
    if (qRaw.isEmpty()) {
        path.empty = false;
        return false;
    }

    QString cleanPath = QDir::cleanPath(qRaw);
    path.hasTraversal = !cleanPath.contains("..");

    QFileInfo info(cleanPath);
    QFileInfo parentInfo(info.absoluteDir().absolutePath());

    path.exists       = info.exists();
    path.isFile       = info.isFile();
    path.hasExtension = !info.suffix().isEmpty();
    path.readable     = info.isReadable();
    path.writable     = info.isWritable();
    path.userReadable = info.permission(QFile::ReadUser);
    path.dirExecutable = parentInfo.permission(QFile::ExeUser);

    if (info.exists()) {
        QFileInfo canon(info.canonicalFilePath());
        path.canonicalizable = canon.exists();
    }

    QDir appDir(QCoreApplication::applicationDirPath());
    QString canonPath = QFileInfo(cleanPath).canonicalFilePath();
    QString canonApp  = QFileInfo(appDir.absolutePath()).canonicalFilePath();

    if (!canonPath.isEmpty() && !canonApp.isEmpty()) {
        path.insideWorkingDir =
            canonPath == canonApp ||
            canonPath.startsWith(canonApp + QDir::separator());
    }

    if (mode == PathAccessModeZIP::CONTENT ||
        mode == PathAccessModeZIP::EDIT ||
        mode == PathAccessModeZIP::EXTRACT) {

        path.zipExtension =
            info.suffix().compare("zip", Qt::CaseInsensitive) == 0;

        if (info.exists() && info.isFile() && info.isReadable()) {
            QFile file(info.absoluteFilePath());
            if (file.open(QIODevice::ReadOnly)) {
                path.zipReadable = true;
                QByteArray sig = file.read(4);
                path.zipSignature =
                    sig.startsWith("PK\x03\x04") ||
                    sig.startsWith("PK\x05\x06") ||
                    sig.startsWith("PK\x07\x08");
                path.zipNotCorrupted = path.zipSignature;
                file.close();
            }
        }
    }

    switch (mode) {

        case PathAccessModeZIP::CONTENT:
            return
                path.hasTraversal &&
                path.exists &&
                path.isFile &&
                path.readable &&
                path.hasExtension &&
                path.canonicalizable &&
                path.zipExtension &&
                path.zipReadable &&
                path.zipSignature &&
                path.zipNotCorrupted;

        case PathAccessModeZIP::EDIT:
            return
                path.hasTraversal &&
                path.exists &&
                path.isFile &&
                path.readable &&
                path.writable &&
                path.hasExtension &&
                path.canonicalizable &&
                path.zipExtension &&
                path.zipReadable &&
                path.zipSignature &&
                path.zipNotCorrupted;

        case PathAccessModeZIP::COMPRESS:
            path.noDuplicate = !existingEntries.contains(info.fileName());
            return
                path.hasTraversal &&
                path.exists &&
                path.isFile &&
                path.readable &&
                path.noDuplicate;

        case PathAccessModeZIP::EXTRACT:
            return
                path.hasTraversal &&
                path.exists &&
                path.isFile &&
                path.readable &&
                path.zipReadable &&
                path.zipSignature &&
                path.zipNotCorrupted &&
                parentInfo.exists() &&
                parentInfo.permission(QFile::WriteUser);
        case PathAccessModeZIP::EXTRACTPATH:
            if (path.exists) {
                return
                    path.hasTraversal &&
                    !path.isFile &&
                    path.writable &&
                    path.insideWorkingDir;
            } else {
                QDir dir;
                path.dirCreatable = dir.mkpath(cleanPath);
                return
                    path.hasTraversal &&
                    path.dirCreatable &&
                    path.insideWorkingDir;
            }
    }

    return false;
}

std::string QTCheckPathZIPService::error() {
    std::string errorText;

    errorText.append("Error: ");

    for (auto [name, ptr] : path.get_fields())
        if (!*ptr) errorText.append(name).append(", ");

    if (errorText.size() >= 2) errorText.resize(errorText.size() - 2);
    
    return errorText;
}