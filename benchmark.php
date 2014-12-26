<?php
$begin = microtime(true);
for ($n=0; $n<$argv[1]; $n++) {
	$pid = pcntl_fork();
	if ($pid) {
	        continue;
	} else {
		$fp = fcgi_pconnect('web-dev001.m6', 9000);
		for ($i=0;$i<$argv[2];$i++){
			$s = fcgi_request(array(
			        "SCRIPT_FILENAME" => "/home/meng.jun/deploy/src/main/php/echo.php",
			        "REQUEST_METHOD"=>'GET',
			), $fp);
		}
		fclose($fp);
		die;
	}
}
pcntl_wait($status);
$end = microtime(true);
$qps = $argv[1] * $argv[2] / ($end-$begin);
echo "qps $qps".PHP_EOL;
