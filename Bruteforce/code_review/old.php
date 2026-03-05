<?php

// Проверка параметра Login через GET.
// Проблема: для логина нельзя использовать GET (пароль попадает в URL и логи сервера).
if( isset( $_GET[ 'Login' ] ) ) {

	// Получение username из GET без какой-либо валидации
	$user = $_GET[ 'username' ];

	// Получение password из GET
	$pass = $_GET[ 'password' ];

	// Используется MD5 для хеширования
	// Проблема: MD5 считается криптографически небезопасным
	// Нужно использовать password_hash() / password_verify()
	$pass = md5( $pass );

	// Формирование SQL запроса через конкатенацию строки
	// Критическая проблема: SQL Injection
	// Пользователь может передать например ' OR 1=1 --
	$query  = "SELECT * FROM `users` WHERE user = '$user' AND password = '$pass';";

	// Выполнение запроса
	// Проблема: используется глобальная переменная соединения
	$result = mysqli_query($GLOBALS["___mysqli_ston"],  $query ) 
		or die(
			'<pre>' .
			((is_object($GLOBALS["___mysqli_ston"])) 
				? mysqli_error($GLOBALS["___mysqli_ston"]) 
				: (($___mysqli_res = mysqli_connect_error()) ? $___mysqli_res : false)
			)
			. '</pre>'
		);

	// Проверка что найден ровно 1 пользователь
	if( $result && mysqli_num_rows( $result ) == 1 ) {

		// Получение строки результата
		$row = mysqli_fetch_assoc( $result );

		// Получение пути к аватару
		// Проблема: если данные из БД не фильтровать — возможен XSS
		$avatar = $row["avatar"];

		// Сообщение о входе
		// Проблема: переменная $user выводится без htmlspecialchars()
		$html .= "<p>Welcome to the password protected area {$user}</p>";

		// Вывод изображения
		// Проблема: потенциальный XSS если avatar содержит вредоносный URL
		$html .= "<img src=\"{$avatar}\" />";
	}
	else {

		// Сообщение об ошибке авторизации
		$html .= "<pre><br />Username and/or password incorrect.</pre>";
	}

	// Закрытие соединения
	// Код перегружен и плохо читаем
	((is_null($___mysqli_res = mysqli_close($GLOBALS["___mysqli_ston"]))) ? false : $___mysqli_res);
}
?>