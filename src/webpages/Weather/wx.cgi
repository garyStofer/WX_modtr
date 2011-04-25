<html>
<head>
<meta http-equiv="Pragma" content="no-cache"/>
<link href="mx.css" rel="stylesheet" type="text/css"/>
<title>Wunderground settings</title>
<script type="text/javascript">
function doSubmit_wx_form(frm)
{
	frm.submit();
	setTimeout("window.location.replace('status.cgi')",1000); // relaod the status display after the SBC rebooted 
}

</script>
</head>

<body class=ifrmBody>
<h3 >Wunderground Station Settings </h3>
<form  name="frmwx" id= "frmWX" method ="get" action = "wx.cgi">
<table>
<tr>
<td>Station ID:</td>
<td> <input type=text size=15  maxlength=15 value="%l41" name="wi" /> </td></tr>
<tr><td> Station PWD:</td><td> <input type=text size=15  maxlength=15 value="%l42" name="wp" /> </td></tr>
<tr><td>Station Elev:</td><td> <input type=text size=4  maxlength=4 value="%l47" name="we" /> </td> </tr>
<tr>
<td>
Wunderground</td><td>IP Address:</td>
<td><input type=text size=3  maxlength=3 value="%k91" name="k91"/>:</td>
<td><input type=text size=3  maxlength=3 value="%k92" name="k92"/>:</td>
<td><input type=text size=3  maxlength=3 value="%k93" name="k93"/>:</td>
<td><input type=text size=3  maxlength=3 value="%k94" name="k94"/> </td>
</tr>
</table>
<p><em>Default for&quot;www.weatherstation.wunderground.com&quot; is 38.102.136.125 <br/> 
Use all '0' to disable reporting.  </em></p>
<input type="hidden" name="m" value="r"/>
<input type="button" value="Set Station Params & Reset " onclick="doSubmit_wx_form(document.frmwx)"/>
</form>

</body>
</html> 