<?php
/*
   Copyright (c) jlh (jlh at gmx dot ch), 2004 - 2009
   This file is part of the Shareaza Connection Test

   The Shareaza Connection Test is free software; you can redistribute
   it and/or modify it under the terms of the GNU General Public License
   as published by the Free Software Foundation; either version 2 of the
   License, or (at your option) any later version.

   The Shareaza Connection Test is distributed in the hope that it will
   be useful, but WITHOUT ANY WARRANTY; without even the implied warranty
   of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with Shareaza; if not, write to the Free Software Foundation,
   Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

   Also visit http://www.gnu.org/
*/

/*
   Shareaza Connection Test

   this is a connection tester specially written for Shareaza or other
   application using the Gnutella2 network.  this file contains the entire
   test, the language files are (by default) in the 'langs'
   directory.
*/

/* -------------------------------- SETTINGS -------------------------------- */

/* timeout in seconds, this gets doubled for the 'long' timeout */
$opt_timeout = 5;

/* version of the test */
$opt_version = 'v1.12';

/* whether to display (and obey) the IP field; this probably
   shouldn't be enabled for publicly available versions */
$opt_ip_field = FALSE;

/* whether to collect and display stats, and the file for storing them */
$opt_enable_stats = TRUE;
$opt_stats_file = 'var/stats.dat';

/* specifies whether the UDP answer must come from the same IP it has been
   sent to.  disable if you have troubles using the test inside a LAN */
$opt_require_same_ip = TRUE;

/* language to use by default when none given any and the browser didn't ask
   for one that's doable; ISO 639 code */
$opt_default_lang = 'en';

/* path to language files */
$opt_lang_path = 'langs';

/* level of error reporting; this is passed to error_reporting().
   0 for none, E_ALL for all; put this to 0 for public tests */
$opt_error_level = 0;

/* ------------------------------ END SETTINGS ------------------------------ */

/*
   TODO and list of ideas:
      - check if IP in IP field is really an IP.
      - on result page, put a link back to the first page.
*/

/*
   The change log is now in the file ChangeLog.  Don't forget to update it.
*/


/* try to get the client's IP, and if possible take care of available
   informations about proxy servers */

function get_client_ip(&$ip, &$proxy)
{
	if (isset($_SERVER['HTTP_X_FORWARDED_FOR']) && $_SERVER['HTTP_X_FORWARDED_FOR'])
	{
		if (isset($_SERVER['HTTP_CLIENT_IP']) && $_SERVER['HTTP_CLIENT_IP'])
			$proxy = $_SERVER['HTTP_CLIENT_IP'];
		else
			$proxy = $_SERVER['REMOTE_ADDR'];

		$ip = $_SERVER['HTTP_X_FORWARDED_FOR'];

		/* sometimes HTTP_X_FORWARDED_FOR contains more than one ip,
		   for example "165.165.209.243, 198.54.202.18".  take the
		   first one */
		$i = strpos($ip, ',');

		if ($i !== FALSE)
			$ip = substr($ip, 0, $i);
	}
	else
	{
		if (isset($_SERVER['HTTP_CLIENT_IP']) && $_SERVER['HTTP_CLIENT_IP'])
			$ip = $_SERVER['HTTP_CLIENT_IP'];
		else
			$ip = $_SERVER['REMOTE_ADDR'];

		$proxy = '';
	}

	$tmp = explode('.', $ip);

	if ($tmp[0] == 10 ||
		($tmp[0] == 172 && $tmp[1] >= 16 && $tmp[1] < 32) ||
		($tmp[0] == 192 && $tmp[1] == 168))
	{
		/* that's a private IP, probably proxying from inside a LAN */
		if ($proxy != '')
		{
			$ip = $proxy;
			$proxy = '';
		}
	}
}


/* get list of acceptable languages from the browser in preferred order */

function get_acceptable_languages()
{
	if (!isset($_SERVER['HTTP_ACCEPT_LANGUAGE']))
		return(array());

	/* extract information into a pair of arrays */

	$langs = explode(',', $_SERVER['HTTP_ACCEPT_LANGUAGE']);

	for ($i = 0; $i < count($langs); $i++)
	{
		$parts = explode(';', trim($langs[$i]));
		$langs[$i] = array_shift($parts);
		$q[$i] = 1.0;

		foreach ($parts as $param)
		{
			if (preg_match('/q=([0-9.]*)/', $param, $m))
				$q[$i] = floatval($m[1]);
			/* ignore any other parameters */
		}
	}

	/* sort languages by q-value, (simple bubble-sort) */

	for ($i = 0; $i < count($langs) - 1; $i++)
	{
		$ok = 1;

		for ($j = 0; $j < count($langs) - $i - 1; $j++)
		{
			if ($q[$j] < $q[$j + 1])
			{
				$ok = 0;
				$t = $q[$j];
				$q[$j] = $q[$j + 1];
				$q[$j + 1] = $t;
				$t = $langs[$j];
				$langs[$j] = $langs[$j + 1];
				$langs[$j + 1] = $t;
			}
		}

		if ($ok) break;
	}

	return($langs);
}


/* perform the test for TCP on the given ip:port */

function test_tcp($ip, $port)
{
	global $opt_timeout, $opt_version;

	$result = 'UNKNOWN';

	echo "<b>Start connection test for TCP...</b>\n";

	/* this is not a real loop, only used for easy exit with 'break' */
	do {

	echo "Get socket.\n";
	$socket = socket_create(AF_INET, SOCK_STREAM, SOL_TCP);

	if ($socket == FALSE)
	{
		$result = 'IE';
		$error = socket_last_error();
		echo "socket_create() failed, error $error: " . socket_strerror($error) . ".\n";
		break;
	}

	/* the following will cause the connection to time out as specified
	   (much earlier than normal timeout), but note that it will cause the
	   returned error to be 115 (op now in progress) after the timeout has
	   been reached, not 110 (timeout).  the timeout values are set using
	   socket_set_option(), because stream_set_timeout() didn't work (and
	   possibly doesn't even apply here) */

	$t = socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array('sec'=>$opt_timeout, 'usec'=>0));
	$u = socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, array('sec'=>$opt_timeout, 'usec'=>0));

	if ($t == FALSE || $u == FALSE)
	{
		/* consider this fatal, we don't want the script to hang until natural timeout */
		$result = 'IE';
		$error = socket_last_error();
		echo "socket_set_option() failed, error $error: " . socket_strerror($error) . ".\n";
		break;
	}

	echo "Attempt to connect to $ip on port $port.\n";

	/* flush, so the user will see something while we're waiting */
	flush();

	if (socket_connect($socket, $ip, $port) == FALSE)
	{
		$error = socket_last_error();
		echo "socket_connect() failed, error $error: " . socket_strerror($error) . ".\n";

		switch ($error)
		{
			case '115':
				echo "(Ignore the previous error reason, it's a timeout instead.)\n";
				/* fall through */

			case '110':
				$result = 'TIMEOUT';
				break;

			case '111':
				$result = 'REFUSED';
				break;

			default:
				$result = 'ERROR';
				break;
		}

		break;
	}

	$result = 'CONNECTED';
	echo "Connected.\n";

	$in = "HEAD /doesnotexist.html HTTP/1.1\r\n" .
		"Host: $ip:$port\r\n" .
		"User-Agent: Shareaza Connection Test, $opt_version\r\n" .
		"Connection: close\r\n\r\n";

	echo "Send HTTP request: [block, " . strlen($in) . " bytes]\n<i>" . $in . "</i>[/block]\n";

	while ($in != '')
	{
		$bytes = socket_write($socket, $in, strlen($in));
		if ($bytes === FALSE)
			break;
		if ($bytes == 0)
			/* note: usleep() is broken on windows prior to PHP5 */
			usleep(200000);
		else
			$in = substr($in, $bytes);
	}

	if ($bytes === FALSE)
	{
		$result = 'IE';
		$error = socket_last_error();
		echo "socket_write() failed, error $error: " . socket_strerror($error) . ".\n";
		break;
	}

	echo "Read response: ";
	$out = '';
	while ($tmp = socket_read($socket, 1024, PHP_BINARY_READ))
	{
		$out .= $tmp;
		if (strlen($out) >= 1024 || strpos($out, "\r\n\r\n") !== FALSE)
			break;
	}

	/* don't test for $tmp being FALSE here.  it's normal to happen if the
	   peer doesn't close the connection and timeout has been reached */

	if ($out != '')
	{
		echo "[block, " . strlen($out) . " bytes]\n<i>" .
			htmlspecialchars($out) . "</i>[/block]\n";
		$result = 'ANSWERED';
	}
	else
	{
		echo "No data received.\n";
	}

	} while (0);

	echo "Close socket.\n";
	socket_close($socket);

	echo "Done, return code: '$result'.\n";

	return($result);
}


/* perform the test for udp */

function test_udp($ip, $port)
{
	global $opt_timeout, $opt_version, $opt_require_same_ip;

	$result = 'UNKNOWN';
	echo "<b>Start connection test for UDP...</b>\n";

	/* this is not a real loop, only used for easy exit with 'break' */
	do {

	echo "Get socket.\n";
	$socket = socket_create(AF_INET, SOCK_DGRAM, SOL_UDP);

	if ($socket == FALSE)
	{
		$result = 'IE';
		$error = socket_last_error();
		echo "socket_create() failed, error $error: " . socket_strerror($error) . ".\n";
		break;
	}

	/* see comment in test_tcp() */

	$t = socket_set_option($socket, SOL_SOCKET, SO_SNDTIMEO, array('sec'=>$opt_timeout, 'usec'=>0));
	$u = socket_set_option($socket, SOL_SOCKET, SO_RCVTIMEO, array('sec'=>$opt_timeout, 'usec'=>0));

	if ($t == FALSE || $u == FALSE)
	{
		/* consider this fatal, we don't want the script to hang until natural timeout */
		$result = 'IE';
		$error = socket_last_error();
		echo "socket_set_option() failed, error $error: " . socket_strerror($error) . ".\n";
		break;
	}

	echo "Attempt to bind to INADDR_ANY on any port.\n";

	if (!socket_bind($socket, "0.0.0.0"))
	{
		$result = 'IE';
		$error = socket_last_error();
		echo "socket_bind() failed, error $error: " . socket_strerror($error) . ".\n";
		break;
	}

	/* this is the UDP packet to be sent, uppon changing, make sure it
	   matches the ACK packet expected below */
	$in = "GND\002sq\001\001\020JCT";

	echo "Send /JCT: 3x" . strlen($in) . " bytes: " .
		htmlspecialchars(addcslashes($in, "\000..\037\177..\377")) . "\n";

	/* return value not documented on php.net, send the packet three
	   times, to reduce chances of it (or the answer) to get lost */
	socket_sendto($socket, $in, strlen($in), 0, $ip, $port);
	socket_sendto($socket, $in, strlen($in), 0, $ip, $port);
	socket_sendto($socket, $in, strlen($in), 0, $ip, $port);

	$result = 'NOTHING';
	echo "Read response:\n";
	flush();

	$time = time();

	/* arguments and return value of socket_recv() not documented on
	   php.net, guessed from examples.  PHP_BINARY_READ doesn't work
	   as expected */

	while (socket_recvfrom($socket, $tmp, 1024, 0, $the_ip, $the_port))
	{
		echo "$the_ip:$the_port sent " . strlen($tmp) . " byte(s): ";
		echo htmlspecialchars(addcslashes($tmp, "\000..\037\177..\377"));
		if ($the_ip != $ip && $opt_require_same_ip)
			echo " &lt;- FAIL: from wrong host\n";
		elseif ($tmp == "GND\000sq\001\000") {
			echo " &lt;- OK: packet accepted\n";
			$result = 'ANSWERED';
		} elseif (substr($tmp, 0, 3) == 'GND')
			echo " &lt;- FAIL: Unexpected G2 packet\n";
		else
			echo " &lt;- FAIL: Unrecognized packet\n";

		if ($result == 'ANSWERED') break;
		if (time() - $time > $opt_timeout) break;
	}

	if ($result != 'ANSWERED')
		echo "No proper answer received.\n";

	} while (0);

	echo "Close socket.\n";
	socket_close($socket);

	echo "Done, return code: '$result'.\n";

	return($result);
}


/* display fatal error about language files and exit */

function language_error()
{
	/* oooh!  display a little page and abort */

	header('Content-Type: text/html; charset=UTF-8');
	header("Expires: Mon, 26 Jul 1990 05:00:00 GMT");
	header("Cache-Control: no-cache, must-revalidate");
	header("Pragma: no-cache");

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html><head><title>Error</title><meta http-equiv='content-type'
content='text/html; charset=UTF-8'></head><body><h1>Error</h1>
<p>There was a fatal problem with the language files.  If you followed
a link somewhere, then please report this problem.</p></body></html>
<?php

	exit(1);
}


/* ============================ main stuff ============================ */

$scriptname = basename($_SERVER['PHP_SELF']);
$do_display_form = TRUE;
error_reporting($opt_error_level);


/* -------- check what we got in $_REQUEST -------- */

$do_test = isset($_REQUEST['test']) && $_REQUEST['test'] == '1';
$port = isset($_REQUEST['port']) ? $_REQUEST['port'] : '';

$ip = $proxy = '';

if ($opt_ip_field && isset($_REQUEST['ip']))
	$ip = $_REQUEST['ip'];

if ($ip == '')
	get_client_ip($ip, $proxy);

$do_lto = isset($_REQUEST['lto']) && $_REQUEST['lto'] == '1';
if ($do_lto)
	$opt_timeout *= 2;


/* -------- look for and load an appropriate language -------- */

if (!is_readable("$opt_lang_path/languages.php"))
	language_error();

require_once "$opt_lang_path/languages.php";

$lang = '';

if (isset($_REQUEST['lang']) && array_key_exists($_REQUEST['lang'], $lang_list))
{
	$lang = $_REQUEST['lang'];
}
else
{
	/* go through list we got from browser */
	$al = get_acceptable_languages();
	foreach ($al as $l)
	{
		if (array_key_exists($l, $lang_list))
		{
			$lang = $l;
			break;
		}
	}
}

/* use default, if still nothing */
if ($lang == '')
	$lang = $opt_default_lang;

if (is_readable("$opt_lang_path/lang-$lang.php"))
	require_once "$opt_lang_path/lang-$lang.php";
else
	language_error();


/* -------- some output: http header, file head and title -------- */

header('Content-Type: text/html; charset=UTF-8');
header("Expires: Mon, 26 Jul 1990 05:00:00 GMT");
header("Cache-Control: no-cache, must-revalidate");
header("Pragma: no-cache");

?><!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN" "http://www.w3.org/TR/html4/loose.dtd">
<html>
<head>
	<meta http-equiv='content-type' content='text/html; charset=UTF-8'>
	<meta http-equiv='expires' content='0'>
	<link rel="icon" href="favicon.ico" type="image/x-icon">
	<link rel="shortcut icon" href="favicon.ico" type="image/x-icon">
	<link rel='Start' href='<?php echo $scriptname; ?>'>
	<title><?php echo _MSG_TITLE; ?></title>
	<style type='text/css'><!--
	body { margin:2% 18% 0% 18%; font-family:sans-serif; font-size:10pt; }
	table, form { font-size:10pt; /* IE 5 fix */ }
	h3 { margin:0px 0px 5px 0px; }
	p { margin:0px; }
	form { margin:10px 0px; }
	pre { font-size:7pt; margin:0px; }
	form { text-align:center; }
	.small { font-size:x-small; }
	.box { border:1px solid black; padding:10px; margin:10px 0px; }
	.pos { font-weight:bold; color:green; }
	.neg { font-weight:bold; color:red; }
	.spacer { height:10px; }
	.removed { display:none; }
	.footer { text-align:right; font-size:7pt; }
	.nowrap { white-space:nowrap; }
	--></style>
</head>

<?php

if (_MSG_RTL)
	echo "<body dir='rtl'>\n";
else
	echo "<body>\n";

echo "<h1>" . _MSG_TITLE_WITH_LINK . "</h1>";


/* -------- some output: list of available languages -------- */

if (count($lang_list) > 1)
{
	echo "<p>" . _MSG_LANGUAGES . "\n";
	$f = 0;
	foreach ($lang_list as $l => $name)
	{
		if ($lang != $l)
		{
			if ($f)
				echo ' | ';
			else
				$f = 1;

			$tmp = "lang=$l";

			if ($port != '')
				$tmp .= "&amp;port=$port";
			if ($do_lto)
				$tmp .= "&amp;lto=1";
			if ($opt_ip_field)
				$tmp .= "&amp;ip=$ip";

			echo "<a href='$scriptname?$tmp'>$name</a>\n";
		}
	}

	echo "</p>\n";
}


/* -------- check the values we got -------- */

if ($ip == '')
{
	/* coudn't get IP */

	$do_display_form = FALSE;
	$do_test = FALSE;

	echo "<div class='box'><h3>" . _MSG_ERROR_CAPTION . "</h3>\n";
	echo _MSG_ERROR_IP . "</div>\n";
}

if ($do_test && (!is_numeric($port) || $port < 1 || $port > 65535))
{
	/* bogus port number */

	$do_test = FALSE;

	echo "<div class='box'><h3>" . _MSG_ERROR_CAPTION . "</h3>\n";

	if (is_numeric($port) && $port == 0)
	{
		echo _MSG_ERROR_PORT_ZERO . "\n";
		echo "<div class='spacer'></div>\n";
		echo _MSG_WIKI_FR . "\n";
	}
	else
	{
		echo _MSG_ERROR_PORT_INVALID . "\n";
	}

	/* clear the port (for the form) */
	$port = '';
	echo "</div>\n";
}


/* -------- now get to the important part -------- */

if ($do_test)
{
	echo "<div class='box' id='progress'><b>" . _MSG_PROGRESS . "</b></div>\n";
	echo "<div class='box'><div id='detail'><h3>" . _MSG_DETAIL_CAPTION . "</h3>\n";
	echo "<div class='spacer'></div>\n";
	echo "<pre>";

	/* dump some vars */

	echo "<b>Data dump:</b> $opt_version,to=$opt_timeout,ipf=" .
		($opt_ip_field ? "yes" : "no") . ",ip=$ip,proxy=$proxy," .
		"port=$port,lang=$lang,el=$opt_error_level,php=" .
		PHP_VERSION . "\n           host=" . $_SERVER['SERVER_NAME'] .
		',path=' . $_SERVER['PHP_SELF'] . ',t=' . gmdate('Y-m-d\TH:i:s\Z') .
		"\n\n";

	$result_tcp = test_tcp($ip, $port);

	if (connection_aborted() == FALSE)
	{
		/* not aborted, continue with UDP */
		echo "\n";
		flush();
		$result_udp = test_udp($ip, $port);
	}
	else
	{
		$result_udp = 'NOTTESTED';
	}

	?></pre></div><!-- end of detail body -->
	<div class='small'>
		<a href='javascript:detail_show();' id='detailshow' class='removed'>
		<i><?php echo _MSG_DETAIL_SHOW; ?></i></a>
		<div id='detailhide' class='removed'><div class='spacer'></div>
		<a href='javascript:detail_hide();'><i><?php echo _MSG_DETAIL_HIDE; ?></i></a></div>
	</div>

	</div><!-- end of detail box -->

	<div class='box'>
	<h3><?php echo _MSG_RESULTS_CAPTION; ?></h3>
	<table cellpadding='0' cellspacing='0' border='0'>
	<tr valign='top'>
	<td><b>TCP:&nbsp;</b></td>
	<td><?php

	switch ($result_tcp)
	{
		case 'TIMEOUT':
			echo "<span class='neg'>" . _MSG_RESULTS_TCP_TIMEOUT_1 .
				"</span> " . _MSG_RESULTS_TCP_TIMEOUT_2 . "\n";
			break;

		case 'REFUSED':
			echo "<span class='neg'>" . _MSG_RESULTS_TCP_REFUSED_1 .
				"</span> " . _MSG_RESULTS_TCP_REFUSED_2 . "\n";
			break;

		case 'CONNECTED':
			echo "<span class='pos'>" . _MSG_RESULTS_TCP_CONNECTED_1 .
				"</span> " . _MSG_RESULTS_TCP_CONNECTED_2 . "\n";
			break;

		case 'ANSWERED':
			echo "<span class='pos'>" . _MSG_RESULTS_TCP_ANSWERED_1 .
				"</span> " . _MSG_RESULTS_TCP_ANSWERED_2 . "\n";
			break;

		case 'NOTTESTED':
			echo _MSG_RESULTS_NOT_TESTED . "\n";
			break;

		case 'ERROR':
			echo "<span class='neg'>" . _MSG_RESULTS_TCP_ERROR .
				"</span> <b>" . _MSG_RESULTS_REPORT . "</b>\n";
			break;

		case 'UNKNOWN':
		case 'IE':
		default:
			echo "<span class='neg'>" . _MSG_RESULTS_IE .
				"</span> <b>" . _MSG_RESULTS_REPORT . "</b>\n";
			break;
	}

	echo "</td></tr>\n<tr valign='top'><td><b>UDP:&nbsp;</b></td>\n<td>";

	switch ($result_udp)
	{
		case 'NOTHING':
			echo "<span class='neg'>" . _MSG_RESULTS_UDP_NOTHING_1 .
				"</span> " . _MSG_RESULTS_UDP_NOTHING_2 . "\n";
			break;

		case 'ANSWERED':
			echo "<span class='pos'>" . _MSG_RESULTS_UDP_ANSWERED_1 .
				"</span> " . _MSG_RESULTS_UDP_ANSWERED_2 . "\n";
			break;

		case 'NOTTESTED':
			echo _MSG_RESULTS_NOT_TESTED . "\n";
			break;

		case 'UNKNOWN':
		case 'IE':
		default:
			echo "<span class='neg'>" . _MSG_RESULTS_IE .
				"</span> <b>" . _MSG_RESULTS_REPORT . "</b>\n";
			break;
	}

	echo "</td></tr></table>\n<div class='spacer'></div>";

	/* display the result summary */

	if (($result_tcp == 'ANSWERED' || $result_tcp == 'CONNECTED') && $result_udp == 'ANSWERED')
	{
		echo "<span class='pos'><b>" . _MSG_RESULTS_SUMMARY_OK . "</b></span>\n";
	}
	else
	{
		echo "<span class='neg'><b>" . _MSG_RESULTS_SUMMARY_NOT_OK . "</b></span>\n";
		echo _MSG_WIKI_FR . "\n";
	}

	?></div><!-- end of result box -->
	<script type='text/javascript'><!--
	document.getElementById('progress').className = 'removed';
	function detail_show() {
		document.getElementById('detail').className = '';
		document.getElementById('detailshow').className = 'removed';
		document.getElementById('detailhide').className = '';
	}
	function detail_hide() {
		document.getElementById('detail').className = 'removed';
		document.getElementById('detailshow').className = '';
		document.getElementById('detailhide').className = 'removed';
	}
	detail_hide();
	//--></script>
	<?php
}


/* -------- output the form -------- */

if ($do_display_form)
{
	echo "<div class='box'><h3>" . _MSG_FORM_CAPTION . "</h3>\n";

	if (!$do_test)
	{
		echo _MSG_FORM_TEXT . "\n";
		echo sprintf(_MSG_FORM_IP, $ip) . "\n";
		if ($proxy)
			echo sprintf(_MSG_FORM_PROXY, $proxy) . "\n";
	}

	echo "<form action='$scriptname' method='get'>\n";

	if ($opt_ip_field)
		echo "IP: <input type='text' name='ip' value='$ip' size='15'>&nbsp;&nbsp;&nbsp;&nbsp;\n";

	echo _MSG_FORM_PORT . " <input type='text' name='port' value='$port' size='6'>&nbsp;&nbsp;&nbsp;&nbsp;\n";
	echo "<input type='hidden' name='lang' value='$lang'>\n";
	echo "<input type='hidden' name='test' value='1'>\n";
	echo "<input type='submit' value='" . _MSG_FORM_BUTTON . "' style='width:100px;'>&nbsp;&nbsp;&nbsp;&nbsp;\n";
	echo "(<input type='checkbox' name='lto' value='1' " .
		($do_lto ? "checked" : "") . "> " . _MSG_FORM_LTO . ")\n";
	echo "</form>\n";

	if (!$do_test)
	{
		echo "<div class='spacer'></div><h3>" . _MSG_FORM_PORT_HOWTO_CAPTION . "</h3>\n";
		echo _MSG_FORM_PORT_HOWTO . "\n";
		echo "<div class='spacer'></div><h3>" . _MSG_FORM_PROXY_HOWTO_CAPTION . "</h3>\n";
		echo _MSG_FORM_PROXY_HOWTO . "\n";
		echo "<div class='spacer'></div><h3>" . _MSG_FORM_LTO_HOWTO_CAPTION . "</h3>\n";
		echo _MSG_FORM_LTO_HOWTO . "\n";
	}

	echo "</div><!-- end of 'do test' box -->\n";
}

/* -------- output file footer -------- */

echo "<div class='footer'>\n" . _MSG_TITLE_WITH_LINK . ", $opt_version.\n" . _MSG_FOOTER . "\n";

/* display statistics */

if ($opt_enable_stats)
{
	do { # not a loop

	if (!file_exists($opt_stats_file))
	{
		/* try to create empty file */

		$handle = fopen($opt_stats_file, 'w');

		if ($handle)
		{
			fclose($handle);
			chmod($opt_stats_file, 0644);
		}
	}

	$handle = fopen($opt_stats_file, $do_test ? 'r+' : 'r');
	if ($handle === FALSE) break;

	$ret = flock($handle, $do_test ? LOCK_EX : LOCK_SH);
	if ($ret == FALSE)
	{
		fclose($handle);
		break;
	}

	$ret = fscanf($handle, "%d %d %d %d %d", $stat_time, $stat_both, $stat_tcp, $stat_udp, $stat_none);

	if ($ret != 5)
	{
		/* file has not proper format, reset (probably first time use) */
		$stat_time = time();
		$stat_both = $stat_tcp = $stat_udp = $stat_none = 0;
	}

	if ($do_test)
	{
		$result_tcp = ($result_tcp == 'CONNECTED' || $result_tcp == 'ANSWERED');
		$result_udp = ($result_udp == 'ANSWERED');

		if ($result_tcp)
			if ($result_udp) $stat_both++; else $stat_tcp++;
		else
			if ($result_udp) $stat_udp++; else $stat_none++;

		ftruncate($handle, 0);
		rewind($handle);

		fwrite($handle, sprintf("%d %d %d %d %d\n", $stat_time, $stat_both, $stat_tcp, $stat_udp, $stat_none));
	}

	/* implicit unlocking here */
	fclose($handle);

	$stat_total = $stat_both + $stat_tcp + $stat_udp + $stat_none;
	$stat_total1 = $stat_total ? $stat_total : 1;
	$stat_both = round($stat_both * 100 / $stat_total1);
	$stat_tcp = round($stat_tcp * 100 / $stat_total1);
	$stat_udp = round($stat_udp * 100 / $stat_total1);
	$stat_none = round($stat_none * 100 / $stat_total1);

	printf(_MSG_STATS . "\n", $stat_total, date('Y-m-d', $stat_time), $stat_both, $stat_tcp, $stat_udp, $stat_none);

	} while (FALSE);
}

echo "<span class='nowrap'>Copyright &copy; 2004-2009 jlh</span>";

if (_MSG_TRANSLATOR_STRING)
	echo ", <span class='nowrap'>" . _MSG_TRANSLATOR_STRING . '</span>';

echo ".\n";

echo "</div>\n</body>\n</html>\n";

/* -------- that's it, end of file -------- */
/* vi:set nowrap: */
?>