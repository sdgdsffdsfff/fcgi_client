# 这是一个PHP fastcgi协议的client

# [进入ext目录下编译一下](https://git.wemomo.com/meng.jun/fcgi_client/tree/master/ext) 

## client.php

```
$fp = fcgi_connect('web-dev001.m6', 9000);
$s = fcgi_request(array(
	"SCRIPT_FILENAME" => "/home/meng.jun/deploy/src/main/php/echo.php",
	"REQUEST_METHOD"=>'GET',
), $fp);
echo "request result \n$s\n";

```

## 运行效果

```
[meng.jun@prism002.m6.momo.com fcgi_client]$ php client.php
request result
Content-type: text/html

hello world
```

## 性能测试

```
[meng.jun@prism002.m6.momo.com fcgi_client]$ php benchmark.php 100 1000
```

* 第一个参数是进程数量，第二个参数是每个进程的请求次数