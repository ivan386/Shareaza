<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
	<title>$1$</title>
	<meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
	<style type="text/css">
		<!--
			body {
				margin: 0px;
				font-family: tahoma;
			}
			
			h1 {
				font-size: 16px;
				text-align: center;
			}
			
			p {
				margin: 0px;
			}
			
			#container {
				margin: 3px;
				border: 1px #c00 dashed;
			}
			
			#list {
				margin: 10px;
				position: inline;
				margin-left: auto;
				margin-right: auto;
				width: 400px;
				border: 1px #00c solid;
			}
			
			div.even {
				background: #fff;
			}
			div.odd {
				background: #eee;
			}
			#footer {
			}
			#footer p {
				font-size: 11px;
				color: #666;
			}
		-->
	</style>
</head>

<script language="javascript">
                <!--
                               function doPlay(sURN) { window.external.open(sURN); }
                               function doEnqueue(sURN) { window.external.enqueue(sURN); }
                               function doDownload(sMagnet) { window.external.download(sMagnet); window.location.reload(); }
                               function writeFile(sURN)
                               {
                                      var sState = window.external.detect( sURN );
                                      if ( sState == "Complete" )
                                      {
                                              document.writeln( "<a href='javascript:doPlay(\"" + sURN + "\")' title='$2$'>$2$</a>" );
                                      }
                                      else if ( sState == "0.00%" )
                                      {
                                              document.writeln( "$3$" );
                                      }
                                      else if ( sState.indexOf( "%" ) >= 0 )
                                      {
                                              var nBarSize = 100;
                                              var nBarUsed = Math.round( parseFloat( sState ) / 100 * nBarSize );
                                              document.writeln( "<img src='bar_on.gif' width='" + nBarUsed + "' height='12' alt='" + sState + "'><img src='bar_off.gif' width='" + ( nBarSize - nBarUsed ) + "' height='12' alt='" + sState + "'>" );
                                      }
                                      else
                                      {
                                              document.writeln( "<a href='javascript:doDownload(\"" + sURN + "\")' title='$4$'>$4$</a>" );
                                      }
                               }
                               -->
        </script>
<body>
<div id="container">
	<div id="header">
		<h1>$1$</h1>
	</div>
	<div id="main">
		<div id="list">
			$data$
		</div>
	</div>
	<div id="footer">
		<p>Collection created with Shareaza Collection wizard.</p>
	</div>
</div>
</body>
</html>