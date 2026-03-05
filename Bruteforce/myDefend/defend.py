from flask import Flask, request, jsonify
import bcrypt
import time
import secrets
from collections import defaultdict

app = Flask(__name__)

users = {
    "admin": bcrypt.hashpw("superpassword".encode(), bcrypt.gensalt())
}

MAX_ATTEMPTS = 5
BLOCK_TIME = 60
BASE_DELAY = 1

ip_attempts = defaultdict(int)
user_attempts = defaultdict(int)

ip_block_time = {}
user_block_time = {}

tokens = {}

@app.route("/get_token", methods=["GET"])
def get_token():
    username = request.args.get("username")
    if not username:
        return jsonify({"error": "Username required"}), 400
    token = secrets.token_urlsafe(16)
    tokens[username] = token
    return jsonify({"token": token})

@app.route("/login", methods=["GET"])
def login():
    username = request.args.get("username")
    password = request.args.get("password")
    token = request.args.get("token")

    ip = request.remote_addr
    now = time.time()

    if username not in tokens or tokens[username] != token:
        return jsonify({"error": "Invalid or missing token"}), 403

    if ip in ip_block_time and now < ip_block_time[ip]:
        return jsonify({"error": "IP temporarily blocked"}), 429

    if username in user_block_time and now < user_block_time[username]:
        return jsonify({"error": "Account temporarily locked"}), 429

    if username in users:
        if bcrypt.checkpw(password.encode(), users[username]):
            ip_attempts[ip] = 0
            user_attempts[username] = 0
            tokens.pop(username, None)
            return jsonify({"status": "login successful"})

    ip_attempts[ip] += 1
    user_attempts[username] += 1

    delay = BASE_DELAY * ip_attempts[ip]
    time.sleep(delay)

    if ip_attempts[ip] >= MAX_ATTEMPTS:
        ip_block_time[ip] = now + BLOCK_TIME
        ip_attempts[ip] = 0

    if user_attempts[username] >= MAX_ATTEMPTS:
        user_block_time[username] = now + BLOCK_TIME
        user_attempts[username] = 0

    return jsonify({"error": "Username and/or password incorrect."}), 401

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)