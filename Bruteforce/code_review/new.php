<?php

if (isset($_POST['Login'])) {

    $user = $_POST['username'] ?? '';
    $pass = $_POST['password'] ?? '';

    $stmt = mysqli_prepare($GLOBALS["___mysqli_ston"], "SELECT user, password, avatar FROM users WHERE user = ? LIMIT 1");
    mysqli_stmt_bind_param($stmt, "s", $user);
    mysqli_stmt_execute($stmt);

    $result = mysqli_stmt_get_result($stmt);

    if ($row = mysqli_fetch_assoc($result)) {

        if (password_verify($pass, $row['password'])) {

            $username = htmlspecialchars($row['user'], ENT_QUOTES, 'UTF-8');
            $avatar = htmlspecialchars($row['avatar'], ENT_QUOTES, 'UTF-8');

            $html .= "<p>Welcome to the password protected area {$username}</p>";
            $html .= "<img src=\"{$avatar}\" />";
        } else {
            $html .= "<pre><br />Username and/or password incorrect.</pre>";
        }

    } else {
        $html .= "<pre><br />Username and/or password incorrect.</pre>";
    }

    mysqli_stmt_close($stmt);
    mysqli_close($GLOBALS["___mysqli_ston"]);
}
?>