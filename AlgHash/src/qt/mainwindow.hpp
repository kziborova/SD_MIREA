#pragma once
#include <QMainWindow>
#include <QPushButton>
#include <QWidget>
#include <QVBoxLayout>
#include <QDebug>
#include <QShortcut>
#include <QStackedWidget>
#include <QLineEdit>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>
#include <QComboBox>

#define WIDTH 500
#define HEIGHT 300

#include "../core/SHA-1Service.hpp"
#include "../core/MD5Service.hpp"
#include "../core/bcryptService.hpp"
#include "../core/Argon2Service.hpp"
#include <thread>
#include <atomic>

class MainWindow: public QMainWindow {
    Q_OBJECT
    public:
    MainWindow(QWidget* parent=nullptr);
    private:
    void setUpUI();
    void setUpConnections();
    void onAddHash();
    void onStartAlg();
    void addInputRow();

    std::atomic<int> runningTasks{0};

    QStackedWidget* stackedWidget;
    QWidget*        mainWidget;
    QScrollArea*    scrollArea;
    QWidget*        scrollContent;
    QVBoxLayout*    scrollLayout;
    QPushButton*    addHash;
    QPushButton*    startAlg;
};