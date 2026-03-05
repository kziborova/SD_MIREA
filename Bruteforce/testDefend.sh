#!/bin/bash

echo "---"
echo "Запуск окружения venv"

source venv/bin/activate

echo "---"
echo "Запуск сервера в отдельном терминале"
echo "---"

gnome-terminal -- bash -c "source venv/bin/activate && python3 myDefend/defend.py"

echo "Ждем запуск сервера..."
sleep 3

echo "---"
echo "Получаем токен для пользователя admin"
echo "---"

TOKEN=$(curl -s "http://localhost:5000/get_token?username=admin" | jq -r '.token')

echo "Полученный токен:"
echo $TOKEN

echo
echo "---"
echo "Отправляем правильный запрос авторизации"
echo "---"

curl -s "http://localhost:5000/login?username=admin&password=superpassword&token=$TOKEN"
echo

echo "---"
echo "Отправляем запрос с неправильным паролем"
echo "---"

curl -s "http://localhost:5000/login?username=admin&password=wrongpassword&token=$TOKEN"
echo

echo "---"
echo "Отправляем запрос БЕЗ токена"
echo "---"

curl -s "http://localhost:5000/login?username=admin&password=superpassword"
echo

echo "---"
echo "Запускаем аналогичный bruteforce из первого задания"
echo "---"

python3 myDefend/bruteforceOnDefend.py

echo "---"
echo "Выходим из окружения venv"

deactivate

echo "---"
echo "Сервер работает в отдельном терминале"
echo "Закрой окно сервера вручную или нажми CTRL+C"
echo "---"