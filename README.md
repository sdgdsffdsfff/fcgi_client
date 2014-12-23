# 这是一个PHP fastcgi协议的client

# 使用

## client.php

```
$fp = fcgi_connect('web-dev001.m6', 9000);
$s = fcgi_request(array(
	"SCRIPT_FILENAME" => "/home/meng.jun/deploy/src/main/php/echo.php",
	"REQUEST_METHOD"=>'GET',
), $fp);
echo "request result \n$s\n";
```

## 运行
```
[meng.jun@prism002.m6.momo.com fcgi_client]$ php client.php
request result
Content-type: text/html

hello world
```
