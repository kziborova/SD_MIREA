#pragma once

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QFont>

#include "../../models/Auth.h"

#define WIDTH 500
#define HEIGHT 300

class LoginDialog : public QDialog {
    Q_OBJECT
    public:
    explicit LoginDialog(QWidget* parent = nullptr);

    QString login() const;
    QString password() const;

    private:
    QLineEdit* loginEdit;
    QLineEdit* passwordEdit;
    QPushButton* loginButton;
    QLabel* errorLabel;

    void onLoginClicked();
};