<html>
<head>
<meta http-equiv="Pragma" content="no-cache"/>
<link href="mx.css" rel="stylesheet" type="text/css"/>
<title>Real Time Clock</title>


<script type="text/javascript">
function setRTC(n, v, min, max) {
	if ( v > max )
		alert("Value " + v +" entered too high, max is " + max );
	else if ( v < min )
		alert("Value " + v +" entered too low, min is " + min );
}

</script>
</head>
<body class=ifrmBody>

<form  id= "frmrtc" method ="get" action = "rtc.cgi">
<h3 > Real Time Clock </h3>
<p>To set the clock enter the fields and click Set Clock.</p>
<table>
<tr>
<td align="center" width="55px">Year   </td>
<td align="center" width="50px">Month</td>
<td align="center" width="45px">Day</td>
<td align="center" width="40px">Hour</td>
<td align="center" width="40px">Min</td>
<td align="center" width="40px">Sec</td>
</tr>	
<tr>	
<td align="center">	<input type=text size=4  maxlength=4 value="%x40" name="xtY" onblur="setRTC(name, value,2000,2100)"/></td>
<td align="center">	<input type=text size=1  maxlength=2 value="%x41" name="xtM" onblur="setRTC(name, value,1,12)"/></td>
<td align="center">	<input type=text size=1  maxlength=2 value="%x42" name="xtD" onblur="setRTC(name, value,1,31)"/></td>
<td align="center">	<input type=text size=1  maxlength=2 value="%x43" name="xtH" onblur ="setRTC(name, value,0,23)"/></td>
<td align="center">	<input type=text size=1  maxlength=2 value="%x44" name="xtm" onblur ="setRTC(name, value,0,59)"/></td>
<td align="center">	<input type=text size=1  maxlength=2 value="%x45" name="xts" onblur= "SetRTC(name, value,0,59)"/></td>
<tr><td></td></tr>
<tr>
<td  colspan="6" align="center">
	<input type=submit value="Set Clock "/></td>
 </tr>
</table>
</form>
</body>
</html>
 