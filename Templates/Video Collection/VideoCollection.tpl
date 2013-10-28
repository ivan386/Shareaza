<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">

<head>
<title>Shareaza video collection</title>
<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
<style type="text/css">
<!--
	body {
		margin-left: 0px;
		margin-top: 0px;
		margin-right: 0px;
		margin-bottom: 0px;
		}
	.nfo {
		font-family: Georgia, "Times New Roman", Times, serif;
		font-size: 12px;
		}
	.filetitle {
		color: #666666;
		font-family: Georgia, "Times New Roman", Times, serif;
		font-size: 18px;
		font-weight: bold;
		}
	.type {
		font-family: Verdana, Arial, Helvetica, sans-serif;
		font-size: 12px;
		color: #000000;
		}
	.BorderTable {
		border: 1px dotted #FF0000;
		}
	.release {
		font-family: "Courier New", Courier, mono;
		font-weight: bold;
		font-size: 20px;
		}
	.releaseinfo {
		font-family: "Courier New", Courier, mono;
		font-size: 12px;
		}
-->
</style>

<script type="text/javascript" language="javascript">
<!--
	var bIsShareaza = false;
	if ( window.external && typeof(window.external.detect) != "undefined" )	bIsShareaza = true;

	function IsShareaza() { return bIsShareaza; }
	function doPlay(sURN) { window.external.open(sURN); }
	function doEnqueue(sURN) { window.external.enqueue(sURN); }
	function doDownload(sBitprint, sEd2kHash, sMD5, nSize, sName) { window.external.download(sBitprint); doUpdateStatus(); }
	function doMagnetDownload(sBitprint, sEd2kHash, sMD5, nSize, sName) { document.location.href = "magnet:?xt=" + sBitprint + "&xt=" + sEd2kHash + "&xt=" + sMD5 + "&xl=" + nSize + "&dn=" + sName; }
	function writeFile(id, sBitprint, sEd2kHash, sMD5, nSize, sName)
	{
		sName = escape(sName);

		if ( IsShareaza() )
		{
			var span = null;

			if(document.getElementById) span = document.getElementById(id);
			if(!span) { alert("Your version of Internet Explorer is too old, please update it."); document.body.innerHTML = ""; return; }

			var sState = window.external.detect( sBitprint );
			if ( sState == "Complete" )
			{
				span.innerHTML = "<a href='javascript:doPlay(\"" + sBitprint + "\")' title='$1$'>$1$</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;";
				span.innerHTML += "<a href='javascript:doEnqueue(\"" + sBitprint + "\")' title='$10$'>$10$</a>";
			}
			else if ( sState == "0.00%" )
			{
				span.innerHTML = "$2$";
			}
			else if ( sState.indexOf( "%" ) >= 0 )
			{
				var nBarSize = 100;
				var nBarUsed = Math.round( parseFloat( sState ) / 100 * nBarSize );
				span.innerHTML = "<img src='images\/bar_on.gif' width='" + nBarUsed + "' height='12' alt='" + sState + "'/><img src='images\/bar_off.gif' width='" + ( nBarSize - nBarUsed ) + "' height='12' alt='" + sState + "'/>";
			}
			else
			{
				span.innerHTML = "<a class='DLLink' href='javascript:doDownload(\"" + sBitprint + "," + sEd2kHash + "," + sMD5 + "," + nSize + "," + sName + "\")' title='$3$'>$3$</a>";
			}
		}
		else
		{
			document.writeln( "<a class='DLLink' href=\"magnet:?xt=" + sBitprint + "&xt=" + sEd2kHash + "&xt=" + sMD5 + "&xl=" + nSize + "&dn=" + sName + "\" title='$3$'>$3$</a>" );
		}
	}
-->
</script>
</head>

<body>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody>
		<tr>
			<td style="background-image: url('images/bg.png');" align="left" valign="top"><a href="http://shareaza.sourceforge.net/?id=download"><img src="images/Top.png" width="650" height="84" border="0" alt="Shareaza P2P"/></a></td>
			<td style="background: url('images/top-tile.jpg') repeat-x;" width="100%">&nbsp;</td>
		</tr>
		<tr>
			<td align="left" valign="top" bgcolor="#EEEEEE" colspan="2">

				<table width="100%" border="0" cellspacing="0" cellpadding="10" style="border-bottom: 1px solid #999999;">
					<tr>
						<td colspan="2" style="padding: 0px; padding-top: 10px; padding-left: 10px;">
							<span class="nfo">$4$</span>
						</td>
					</tr>
					<tr>
						<td width="50px" align="left" valign="middle" style="padding-top: 2px;">
							<img src="images/res.gif" width="23" height="23" alt="$5$"/>
						</td>
						<td width="100%" style="padding-left: 5px; padding-top: 0px;">$5$</td>
					</tr>
				</table>

			</td>
		</tr>
		<tr>
			<td height="1" bgcolor="#999999" colspan="2"></td>
		</tr>
		<tr>
			<td height="0" align="center" valign="top" colspan="2">

				<table width="100%" border="0" cellspacing="0" cellpadding="15">
					<tbody>
<!-- Start data from oddfile.tpl and evenfile.tpl -->
$data$
<!-- End data from oddfile.tpl and evenfile.tpl -->

						<tr>
							<td width="173"></td>
							<td width="100%" align="left" valign="top">
								<br/>
								<span class="nfo">
									<strong>
										<a href="javascript:doDownloadAll()" title="$11$">$11$</a>
									</strong>
								</span>
							</td>
						</tr>

					</tbody>
				</table>
				<br/>

				<table width="95%" cellpadding="10" cellspacing="0" class="BorderTable" align="center">
					<tbody>
						<tr>
							<td align="left" valign="top">
								<span class="release">
										<a>release information</a>
								</span>
								<a>
									<br/>
									<span class="releaseinfo">$8$</span>
								</a>
							</td>
						</tr>
					</tbody>
				</table>

				<br/>
			</td>
		</tr>
	</tbody>
</table>

<script type="text/javascript" language="javascript">
<!--
	var scripts = null, scripts_count = 0, update_timer = null;
	if( document.getElementsByTagName )
	{
		scripts = document.getElementsByTagName("script");
		scripts_count = scripts.length;
	}

	function doUpdateStatus()
	{
		if( IsShareaza() )
			for( var c = 0; c < scripts_count; c++ )
			{
				if( scripts[c].text.indexOf("writeFile") == 0 )	// If the position of writeFile is 0 (At the start of the script) then...
				{
					try{ eval( scripts[c].text ); }
					catch(e){ alert("Error 105"); if( update_timer ) clearInterval(update_timer); update_timer = null; }
				}
			}
	}

	function setUpdateTimer()
	{
		update_timer = setInterval("doUpdateStatus()", 1000);
	}
	setUpdateTimer();

	/* -_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_-_- */

	var timer = null, i, temp, links, links_count;

	function resetVars()
	{
		i = 0;
		if(timer) clearInterval(timer);
		timer = null;
		temp = null;
	}
	resetVars();

	function doSendLink()
	{
		if( i < links_count )
		{
			if( links[i] && links[i].className == "DLLink" )
			{
				temp = links[i].href;
				if( IsShareaza() ) temp = temp.replace("doDownload", "doMagnetDownload");

				try{ document.location.href = temp; }
				catch(e){ alert("Error, magnet: isn't associated to a p2p application."); resetVars(); setUpdateTimer(); }	// This error is displayed in Firefox when magnet: isn't associated to a p2p application
			}
			i++;
		}
		else
		{
			resetVars();

			setUpdateTimer();							// Resume the auto-update of the page
		}
	}

	function doDownloadAll()
	{
		if( document.links )
			links = document.links;
		else if( document.getElementsByTagName )
			links = document.getElementsByTagName("a");
		else
			{ alert("The version of your browser is too old, please update it."); return; }

		if( update_timer ) clearInterval(update_timer);	// Stop the auto-update of the page (It will be resumed at the end of the process)
		update_timer = null;

		links_count = links.length;
		timer = setInterval("doSendLink()", 25);		// Set the timer to pull magnets
	}
-->
</script>

</body>

</html>
