<html>
<head>
	<meta http-equiv="refresh" content="60">
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8">
	<title>$title$</title>
	<style>
	p,div,td
	{
		font-family: Verdana;
		font-size: 12px;
	}
	a
	{
		text-decoration: none;
		font-weight: bold;
		color: #FFD0E0;
	}
	a:hover
	{
		text-decoration: underline;
	}
	.info
	{
		font-size: 12px;
		color: #C0A0A0;
	}
	.titlebar
	{
		background-color: #600030;
		border-style: solid;
		border-width: 0px 0px 1px 0px;
		border-color: #A00080;
		color: #FFD0E0;
	}
	.track
	{
		border: #8000A0 1px solid;
		border-collapse: collapse;
		padding: 3px;
	}
	.trackhdr
	{
		border: #A00080 1px solid;
		border-collapse: collapse;
		background: #600030;
		font-weight: bold;
		color: #FFD0E0;
		padding: 3px;
	}
	.picframe
	{
		border: #A00080 1px solid;
		padding: 1px;
	}
	</style>
</head>
<script language="javascript">
function doPlay(sURN)
{
window.external.open(sURN);
}
function doDownload(sURN)
{
try {
	window.external.download(sURN);
	window.location.reload();
} catch(e) {
	window.open("magnet:?xs="+sURN);
}
}
function writeFile(sURN, sTitle,nFileSize)
{
var sState = "";
try { sState = window.external.detect( sURN ); } catch(e) {}
document.writeln( "<tr onmouseover='onMouseIn(this,\"" + sURN + "\")' onmouseout='onMouseOut(this)'>" );
document.writeln( "<td class='track' align='left'><b>" + sTitle + "</b></td>" );
document.writeln( "<td class='track' align='right'><b>" + nFileSize + "</b></td>" );
if ( sState == "Complete" )
{
	document.writeln( "<td class='track' align='center' nowrap><a href='javascript:doPlay(\"" + sURN + "\")' title='Open File'>Open File</a>");
}
else if ( sState == "0.00%" )
{
	document.writeln( "<td class='track' align='center'>Downloading</td>" );
}
else if ( sState.indexOf( "%" ) >= 0 )
{
	var nBarSize = 100;
	var nBarUsed = Math.round( parseFloat( sState ) / 100 * nBarSize );
	document.writeln( "<td class='track' align='center'><img src='bar_on.gif' width='" + nBarUsed + "' height='12' alt='" + sState + "'><img src='bar_off.gif' width='" + ( nBarSize - nBarUsed ) + "' height='12' alt='" + sState + "'></td>" );
}
else
{
	//var sMagnet = "magnet:?xt=" + sURN + "&dn=" + escape(sFilename);
	document.writeln( "<td class='track' align='center'><a href='javascript:doDownload(\"" + sURN + "\")' title='Download Now'>Download Now</a></td>" );
}
document.writeln( "</tr>" );
}
function onMouseIn(el,sURN)
{
el.style.backgroundColor = "#300060";
try { window.external.hover(sURN); } catch(e) {}
}
function onMouseOut(el)
{
el.style.backgroundColor = "";
try { window.external.hover(""); } catch(e) {}
}
</script>
<body text="white" bgColor="#300000" leftMargin="0" topMargin="0">
	<table width="100%" cellpadding="0" cellspacing="0" height="56" background="title_back.gif">
		<tr>
			<td width="78"><a href="http://shareaza.sourceforge.net/?id=download"><img src="icon.gif" width="72" height="72" border="0" alt="Download Shareaza"></a></td>
			<td align="center" valign="top" style="padding: 3px"><b>$1$</b></td>
		</tr>
	</table>
	<table width="100%" cellpadding="0" cellspacing="0" ID="Table1">
		<tr>
			<div class="info">
			$2$
			</div>
		</tr>
		<tr>
			<td width="100%" valign="top">
				<table width="100%" cellPadding="4" class="track">
					<tr>
						<td class="trackhdr" align="center" width="*">Title</td>
						<td class="trackhdr" align="center" width="110">Size</td>
						<td class="trackhdr" align="center" width="30%">Status</td>
					</tr>
					<script language="javascript">
$data$
					</script>
				</table>
			</td>
		</tr> 
	</table>
	$3$
</body>
</html>
