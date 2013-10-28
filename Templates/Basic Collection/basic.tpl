<!DOCTYPE html PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN">
<html>
<head>
	<meta content="text/html; charset=UTF-8" http-equiv="content-type">
	<title>$title$</title>
<script language="javascript">
function doDownload(sMagnet)
{
	try
	{
		window.external.download(sMagnet);
		window.location.reload();
	}
	catch(e)
	{
		window.open("magnet:?xs="+sMagnet);
	}
}
</script>
</head>
<body>
<h1>$title$</h1>
<p>Downloaders: Click the links below to download these files using Shareaza.</p>
<p>Collection creators: A .collection can be edited just like a webpage - this .htm file is just something to get you started. Be creative and have fun!</p>
<ol>
$data$
</ol>
</body>
</html>