# DVWA Bruteforce Lab Setup

Инструкция по установке и настройке DVWA для выполнения задания Bruteforce на Linux.

# Установка необходимых компонентов

Обновление системы:

```bash
sudo apt update
```

Установка веб-сервера, базы данных и PHP:

```bash
sudo apt install apache2 mariadb-server php php-mysqli php-gd libapache2-mod-php git -y
```

# Проверка работы Apache

Проверить статус сервера:

```bash
sudo systemctl status apache2
```

Если сервер не запущен:

```bash
sudo systemctl start apache2
sudo systemctl enable apache2
```

Проверить открыв в браузере:

```
http://localhost
```

# Установка DVWA

Перейти в директорию веб-сервера:

```bash
cd /var/www/html
```

Скачать DVWA:

```bash
sudo git clone https://github.com/digininja/DVWA.git dvwa
```

Настроить права доступа:

```bash
sudo chown -R www-data:www-data /var/www/html/dvwa
sudo chmod -R 755 /var/www/html/dvwa
```

# Настройка конфигурации DVWA

Перейти в папку конфигурации:

```bash
cd /var/www/html/dvwa/config
```

Создать конфигурационный файл:

```bash
sudo cp config.inc.php.dist config.inc.php
```

Открыть файл:

```bash
sudo nano config.inc.php
```

Настроить подключение к базе данных:

```php
$_DVWA['db_server'] = '127.0.0.1';
$_DVWA['db_database'] = 'dvwa';
$_DVWA['db_user'] = 'dvwa';
$_DVWA['db_password'] = 'password';
```

Сохранить изменения.

# Настройка базы данных

Запустить MariaDB:

```bash
sudo systemctl start mariadb
sudo systemctl enable mariadb
```

Открыть консоль базы данных:

```bash
sudo mysql
```

Создать базу данных и пользователя:

```sql
CREATE DATABASE dvwa;

CREATE USER 'dvwa'@'localhost' IDENTIFIED BY 'password';

GRANT ALL PRIVILEGES ON dvwa.* TO 'dvwa'@'localhost';

FLUSH PRIVILEGES;

exit;
```

# Перезапуск веб-сервера

```bash
sudo systemctl restart apache2
```

# Инициализация базы данных DVWA

Открыть страницу настройки:

```
http://localhost/dvwa/setup.php
```

Нажать кнопку:

```
Create / Reset Database
```

После успешного создания базы откроется страница входа.

# Данные для входа

```
username: admin
password: password
```

# Настройка уровня безопасности

После входа в систему необходимо изменить уровень безопасности.

Перейти в раздел:

```
DVWA Security
```

Выбрать уровень:

```
Low
```

Нажать:

```
Submit
```

# Переход к заданию Bruteforce

После настройки безопасности перейти в раздел:

```
Vulnerabilities → Brute Force
```

URL страницы:

```
http://localhost/dvwa/vulnerabilities/brute/
```

После этого приложение готово для выполнения задания по перебору паролей (Bruteforce).