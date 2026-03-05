#pragma once

#include "../../core/extractZIPService.h"

#include "../../../models/Auth.h"
#include "../../../models/Files.h"

#include <quazip/quazip.h>
#include <quazip/quazipfile.h>
#include <quazip/JlCompress.h>
#include <QFile>
#include <QLockFile>
#include <QSet>
#include <QCoreApplication>
#include <QStorageInfo>

class QTExtractZIPService : public ExtractZIPService {
    private:
    QFile file;
    QLockFile lock;
    public:
    std::function<void(int)> onProgress;

    explicit QTExtractZIPService (const QString& path) : file(path), lock(path+".lock") {
        if (!lock.tryLock()) {
            throw std::runtime_error("file is already locked");
        }

        if (!file.open(QIODevice::ReadWrite | QIODevice::Text)) {
            lock.unlock();
            throw std::runtime_error("cannot open file");
        }
    }
    ~QTExtractZIPService() override {
        file.close();
        lock.unlock();
    }

    void extractFile(const std::vector<std::string>& vec, const std::string& place) override;
    bool checkBomb(const std::vector<std::string>& vec, const std::string& place) override; 
};