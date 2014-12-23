<?php
$fp = fcgi_connect('web-dev001.m6', 9000);
$s = fcgi_request(array(
	"SCRIPT_FILENAME" => "/home/meng.jun/deploy/src/main/php/echo.php",
	"REQUEST_METHOD"=>'GET',
), $fp);
echo "request result \n$s\n";
