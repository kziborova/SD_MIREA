#include "logindialog.h"

LoginDialog::LoginDialog(QWidget* parent)
    : QDialog(parent)
{
    setWindowTitle("Authorization AppFile");
    setModal(true);
    setFixedSize(WIDTH, HEIGHT);

    auto* layout = new QVBoxLayout(this);
    layout->setContentsMargins(40, 40, 40, 40);
    layout->setSpacing(20);

    loginEdit = new QLineEdit(this);
    loginEdit->setPlaceholderText("Login");

    passwordEdit = new QLineEdit(this);
    passwordEdit->setPlaceholderText("Password");
    passwordEdit->setEchoMode(QLineEdit::Password);

    loginButton = new QPushButton("LOG IN", this);
    loginButton->setDefault(true);

    QFont font;
    font.setFamily("Arial");
    font.setPointSize(30);
    font.setBold(true);
    loginEdit->setFont(font);
    passwordEdit->setFont(font);
    loginButton->setFont(font);

    errorLabel = new QLabel(this);
    errorLabel->setStyleSheet("color: red;");
    errorLabel->hide();

    layout->addWidget(loginEdit);
    layout->addWidget(passwordEdit);
    layout->addWidget(loginButton);
    layout->addWidget(errorLabel);
    
    connect(loginButton, &QPushButton::clicked, this, &LoginDialog::onLoginClicked);
}

void LoginDialog::onLoginClicked() {
    if (!AuthService::login(loginEdit->text(), passwordEdit->text())) {
        errorLabel->setText("Invalid login or password");
        errorLabel->show();

        loginEdit->clear();
        passwordEdit->clear();
        loginEdit->setFocus();
        return;
    }

    accept();
}