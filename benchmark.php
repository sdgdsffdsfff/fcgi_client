<?php
ini_set('default_socket_timeout',1);
$begin = microtime(true);
$pids = array();
for ($n=0; $n<$argv[1]; $n++) {
	$pid = pcntl_fork();
	if ($pid) {
		$pids[] = $pid;
	        continue;
	} else {
		$fp = fcgi_pconnect('web-dev001.m6', 9000);
		$reconnect = 0;
		$success = 0;
		for ($i=0;$i<$argv[2];$i++){
			$s = fcgi_request(array(
			        "SCRIPT_FILENAME" => "/home/meng.jun/deploy/src/main/php/echo.php",
			        "REQUEST_METHOD"=>'GET',
			), $fp);
			if ($s == false) {
				$reconnect++;
				$fp = fcgi_pconnect('web-dev001.m6', 9000);
			} else {
				$success++;
			}
		}
		fclose($fp);
		echo "RECONNECT $reconnect times $success success".PHP_EOL;
		die;
	}
}
foreach ($pids as $pid) {
	pcntl_waitpid($pid, $status);
}
$end = microtime(true);
$qps = $argv[1] * $argv[2] / ($end-$begin);
echo "qps $qps".PHP_EOL;