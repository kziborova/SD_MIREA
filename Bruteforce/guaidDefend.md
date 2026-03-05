# Запуск сервера с защитой от Bruteforce

## 1. Создание виртуального окружения

Сначала необходимо создать Python‑окружение:

```bash
python3 -m venv venv
````

Активировать окружение:

```bash
source venv/bin/activate
```

## 2. Установка зависимостей

Установить необходимые библиотеки:

```bash
pip install requests flask bcrypt
```

Выйти из окружения

```bash
deactivate
```

## 3. Запуск демонстрационного скрипта

Запуск:

```bash
bash testDefend.sh
```

Не забывайте закрыть окно сервера.

## bruteforceOnDefend

Содержит в себе более подобрый дебаг, чем первый скрипт

myDefend/passwordsDefend.txt - можно редактировать пароли

max_workers - можно редактировать скорость