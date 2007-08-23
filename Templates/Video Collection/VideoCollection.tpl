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
	function doPlay(sURN) { window.external.open(sURN); }
	function doEnqueue(sURN) { window.external.enqueue(sURN); }
	function doDownload(sMagnet) { window.external.download(sMagnet); window.location.reload(); }
	function writeFile(sURN)
	{
		if ( window.external && typeof(window.external.detect) != "undefined" )
		{
			var sState = window.external.detect( sURN );
			if ( sState == "Complete" )
			{
				document.writeln( "<a href='javascript:doPlay(\"" + sURN + "\")' title='$1$'>$1$</a>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;" );
				document.writeln( "<a href='javascript:doEnqueue(\"" + sURN + "\")' title='$10$'>$10$</a>" );
			}
			else if ( sState == "0.00%" )
			{
				document.writeln( "$2$" );
			}
			else if ( sState.indexOf( "%" ) >= 0 )
			{
				var nBarSize = 100;
				var nBarUsed = Math.round( parseFloat( sState ) / 100 * nBarSize );
				document.writeln( "<img src='images\/bar_on.gif' width='" + nBarUsed + "' height='12' alt='" + sState + "'/><img src='images\/bar_off.gif' width='" + ( nBarSize - nBarUsed ) + "' height='12' alt='" + sState + "'/>" );
			}
			else
			{
				document.writeln( "<a href='javascript:doDownload(\"" + sURN + "\")' title='$3$'>$3$</a>" );
			}
		}
		else
		{
			document.writeln( "<a href='magnet:?xt=" + sURN + "' title='$3$'>$3$</a>" );
		}
	}
-->
</script>
</head>

<body>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
	<tbody>
		<tr>
			<td style="background-image: url('images/bg.png');" align="left" valign="top"><img src="images/Top.png" width="650" height="84" alt="Shareaza P2P"/></td>
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
					</tbody>
				</table>
				<br />

				<table width="95%" cellpadding="10" cellspacing="0" class="BorderTable" align="center">
					<tbody>
						<tr>
							<td align="left" valign="top">
								<span class="release">
										<a>release information</a>
								</span>
								<a>
									<br />
									<span class="releaseinfo">$8$</span>
								</a>
							</td>
						</tr>
					</tbody>
				</table>

				<br />
			</td>
		</tr>
	</tbody>
</table>

</body>

</html>
