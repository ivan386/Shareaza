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
                               .style1 {
                                      font-family: Georgia, "Times New Roman", Times, serif;
                                      font-size: 12px;
                               }
                               .style5 {
                                      color: #666666;
                                      font-family: Georgia, "Times New Roman", Times, serif;
                                      font-size: 18px;
                                      font-weight: bold;
                               }
                               .style6 {
                                      font-family: Verdana, Arial, Helvetica, sans-serif;
                                      font-size: 12px;
                                      color: #000000;
                               }
                               .BorderTable {
                                      border: 1px dotted #FF0000;
                               }
                               .style14 {
                                      font-family: "Courier New", Courier, mono;
                                      font-weight: bold;
                                      font-size: 20px;
                               }
                               .style15 {
                                      font-family: "Courier New", Courier, mono;
                                      font-size: 12px;
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
                                              document.writeln( "<a href='javascript:doPlay(\"" + sURN + "\")' title='$1$'>$1$</a>" );
                                      }
                                      else if ( sState == "0.00%" )
                                      {
                                              document.writeln( "$2$" );
                                      }
                                      else if ( sState.indexOf( "%" ) >= 0 )
                                      {
                                              var nBarSize = 100;
                                              var nBarUsed = Math.round( parseFloat( sState ) / 100 * nBarSize );
                                              document.writeln( "<img src='bar_on.gif' width='" + nBarUsed + "' height='12' alt='" + sState + "'><img src='bar_off.gif' width='" + ( nBarSize - nBarUsed ) + "' height='12' alt='" + sState + "'>" );
                                      }
                                      else
                                      {
                                              document.writeln( "<a href='javascript:doDownload(\"" + sURN + "\")' title='$3$'>$3$</a>" );
                                      }
                               }
                               -->
        </script>
<body>

<table width="100%" border="0" cellspacing="0" cellpadding="0">
       <tbody>
       <tr>
               <td align="left" valign="top" background="images/bg.png">
               <img src="images/Top.png" width="650" height="84" /></td>
       </tr>
       <tr>
               <td align="left" valign="top" bgcolor="#EEEEEE">
               <table width="100%" border="0" cellspacing="5" cellpadding="5">
                       <tbody>
                       <tr>
                               <td align="left" valign="top">
                               <table width="100%" border="0" cellspacing="0" cellpadding="0">
                                       <tbody>
                                       <tr>
                                               <td>
                                               <span class="style1">$4$</span>
                                               </td>
                                       </tr>
                                       <tr>
                                               <td height="26" align="left" valign="bottom">
                                                       <table width="100%" border="0" cellspacing="0" cellpadding="0">
                                                               <tr>
                                                                       <td width="50" align="left" valign="middle">
                                                                               <img src="images/res.gif" width="23" height="23"/>
                                                                       </td>
                                                                       <td align="left" valign="middle">
                                                                               <span class="style13"> $5$ </span>
                                                                       </td>
                                                               </tr>
                                                       </table>
                                               </td>
                                       </tr>
                               </tbody>
                               </table>
                               </td>
                       </tr>
               </tbody>
               </table>
               </td>
       </tr>
       <tr>
               <td height="1" bgcolor="#999999"></td>
       </tr>
       <tr>
               <td height="0" align="center" valign="top">
               <table width="100%" border="0" cellspacing="0" cellpadding="15">
                       <tbody>
                       $data$  <!-- Here go data from oddfile.tpl and evenfile.tpl -->
                       </tbody>
               </table>
               <br />
               <table width="95%" cellpadding="10" cellspacing="0" class="BorderTable">
                       <tbody>
                               <tr>
                                       <td align="left" valign="top">
                                               <span class="style14">
                                                       <a name="releaseinfo" id="releaseinfo">release information</a>
                                               </span>
                                               <a name="releaseinfo" id="releaseinfo">
                                                       <br />
                                                       <span class="style15">$8$</span>
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