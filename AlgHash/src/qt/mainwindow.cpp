#include "mainwindow.hpp"

MainWindow::MainWindow(QWidget* parent) :
    QMainWindow(parent),
    addHash(nullptr),
    startAlg(nullptr) {
    setUpUI();
    setUpConnections();
    this->setWindowTitle("Algorithm Hash");
    this->setWindowFlags(this->windowFlags() & ~Qt::WindowMaximizeButtonHint);
    this->setFixedSize(WIDTH, HEIGHT);
}

void MainWindow::setUpUI() {
    stackedWidget = new QStackedWidget(this);
    this->setCentralWidget(stackedWidget);

    mainWidget = new QWidget(this);
    QVBoxLayout* mainLayout = new QVBoxLayout(mainWidget);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(10);

    QFont font;
    font.setFamily("Arial");
    font.setPointSize(30);
    font.setBold(true);

    scrollArea = new QScrollArea(mainWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scrollContent = new QWidget();
    scrollLayout = new QVBoxLayout(scrollContent);
    scrollLayout->setContentsMargins(0, 0, 0, 0);
    scrollLayout->setSpacing(10);

    addHash = new QPushButton("ADD HASH", scrollContent);
    addHash->setFont(font);
    scrollLayout->addWidget(addHash);
    scrollLayout->addStretch();

    addInputRow();

    scrollArea->setWidget(scrollContent);
    mainLayout->addWidget(scrollArea);

    startAlg = new QPushButton("START", mainWidget);
    startAlg->setFont(font);
    mainLayout->addWidget(startAlg);

    stackedWidget->addWidget(mainWidget);
    stackedWidget->setCurrentIndex(0);
}

void MainWindow::setUpConnections() {
    connect(addHash, &QPushButton::clicked, this, &MainWindow::onAddHash);
    connect(startAlg, &QPushButton::clicked, this, &MainWindow::onStartAlg);
}

void MainWindow::onAddHash() {
    addInputRow();
    QTimer::singleShot(0, this, [this]() {
        scrollArea->ensureWidgetVisible(addHash);
    });
}

void MainWindow::onStartAlg() {
    int tasksToStart = 0;

    for (int i = 0; i < scrollLayout->count(); i++) {
        QLayoutItem* item = scrollLayout->itemAt(i);
        if (!item) continue;
        QWidget* rowWidget = item->widget();
        if (!rowWidget || rowWidget == addHash) continue;
        QHBoxLayout* rowLayout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
        if (!rowLayout) continue;
        QComboBox* combo    = qobject_cast<QComboBox*>(rowLayout->itemAt(0)->widget());
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
        if (!combo || !lineEdit) continue;

        QString algorithm    = combo->currentText();
        std::string hashCopy = lineEdit->text().toStdString();

        bool valid = false;
        if      (algorithm == "SHA-1")  { SHA1Service   c; valid = c.checkType(hashCopy); }
        else if (algorithm == "MD5")    { MD5Service    c; valid = c.checkType(hashCopy); }
        else if (algorithm == "bcrypt") { bcryptService c; valid = c.checkType(hashCopy); }
        else if (algorithm == "Argon2") { Argon2Service c; valid = c.checkType(hashCopy); }

        if (valid) tasksToStart++;
    }

    if (tasksToStart == 0) return;

    runningTasks = tasksToStart;
    startAlg->setEnabled(false);

    for (int i = 0; i < scrollLayout->count(); i++) {
        QLayoutItem* item = scrollLayout->itemAt(i);
        if (!item) continue;
        QWidget* rowWidget = item->widget();
        if (!rowWidget || rowWidget == addHash) continue;
        QHBoxLayout* rowLayout = qobject_cast<QHBoxLayout*>(rowWidget->layout());
        if (!rowLayout) continue;
        QComboBox* combo    = qobject_cast<QComboBox*>(rowLayout->itemAt(0)->widget());
        QLineEdit* lineEdit = qobject_cast<QLineEdit*>(rowLayout->itemAt(1)->widget());
        if (!combo || !lineEdit) continue;

        QString algorithm    = combo->currentText();
        std::string hashCopy = lineEdit->text().toStdString();

        auto onDone = [this]() {
            QMetaObject::invokeMethod(this, [this]() {
                if (--runningTasks == 0)
                    startAlg->setEnabled(true);
            }, Qt::QueuedConnection);
        };

        if (algorithm == "SHA-1") {
            SHA1Service check;
            if (!check.checkType(hashCopy)) { --runningTasks; continue; }
            std::thread([hashCopy, onDone]() {
                SHA1Service svc; svc.hashing(hashCopy); onDone();
            }).detach();

        } else if (algorithm == "MD5") {
            MD5Service check;
            if (!check.checkType(hashCopy)) { --runningTasks; continue; }
            std::thread([hashCopy, onDone]() {
                MD5Service svc; svc.hashing(hashCopy); onDone();
            }).detach();

        } else if (algorithm == "bcrypt") {
            bcryptService check;
            if (!check.checkType(hashCopy)) { --runningTasks; continue; }
            std::thread([hashCopy, onDone]() {
                bcryptService svc; svc.hashing(hashCopy); onDone();
            }).detach();

        } else if (algorithm == "Argon2") {
            Argon2Service check;
            if (!check.checkType(hashCopy)) { --runningTasks; continue; }
            std::thread([hashCopy, onDone]() {
                Argon2Service svc; svc.hashing(hashCopy); onDone();
            }).detach();
        }
    }
}

void MainWindow::addInputRow() {
    QFont font;
    font.setFamily("Arial");
    font.setPointSize(30);
    font.setBold(true);

    QWidget* rowWidget = new QWidget(scrollContent);
    QHBoxLayout* rowLayout = new QHBoxLayout(rowWidget);
    rowLayout->setContentsMargins(0, 0, 0, 0);
    rowLayout->setSpacing(10);

    QComboBox* combo = new QComboBox(rowWidget);
    combo->addItems({"SHA-1", "MD5", "bcrypt", "Argon2"});
    combo->setFont(font);

    QLineEdit* lineEdit = new QLineEdit(rowWidget);
    lineEdit->setFont(font);

    rowLayout->addWidget(combo, 1);
    rowLayout->addWidget(lineEdit, 4);

    int addHashIndex = scrollLayout->indexOf(addHash);
    scrollLayout->insertWidget(addHashIndex, rowWidget);
}