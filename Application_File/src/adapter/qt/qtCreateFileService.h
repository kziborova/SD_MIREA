#pragma once

#include "../../../models/Auth.h"
#include "../../../models/Files.h"
#include "../../../models/Operations.h"
#include "../../core/createFileService.h"

#include <QFile>
#include <QFileInfo>
#include <QString>

class QTCreateFileService : public createFileService {
    public:
    bool createFile(const std::string& rawPath) override;
};