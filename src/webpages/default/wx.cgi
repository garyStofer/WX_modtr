<html>
<head>
<meta http-equiv="Pragma" content="no-cache"/>
<link href="mx.css" rel="stylesheet" type="text/css"/>
<title>Wunderground settings</title>
<!--<script src="lib01.js"></script>-->

<script type="text/javascript">
function setRTC(n, v, min, max) {
	if ( v > max )
		alert("Value " + v +" entered too high, max is " + max );
	else if ( v < min )
		alert("Value " + v +" entered too low, min is " + min );
}

function doSubmit_wx_form(frm)
{
	frm.submit();
	setTimeout("window.location.replace('status.cgi')",1000); // relaod the status display after the SBC rebooted 
}

</script>
</head>
<body class=ifrmBody>
<!--

 <hr/>
 <script type="text/javascript">  document.write( window.location.href);  </script>
 <hr/> 
 
<h2> Weather station related configuration</h2>
<form  name="frm1" id= "frmWXConf" method ="get" action = "wx.cgi" >

Temp Cal:<input 		type="text" size="4"  maxlength="4" value="%kB7" name="kB7"/> <br/>
Baro Cal:<input 		type="text" size="4"  maxlength="4" value="%kB8" name="kB8"/>&nbsp;&nbsp;
Baro Cal_offset:<input 	type="text" size="4"  maxlength="4" value="%kBB" name="kBB"/> <br/>
Hyg  Cal:<input 		type="text" size="4"  maxlength="4" value="%kB9" name="kB9"/>&nbsp;&nbsp;&nbsp; 
Hyg  Cal_offset:<input 	type="text" size="4"  maxlength="4" value="%kBC" name="kBC"/> <br/>
Sol  Cal:<input 		type="text" size="4"  maxlength="4" value="%kBA" name="kBA"/> <br/>
<input type="hidden" name="m" value="r"/> 
<input type="submit" value="Set Calibration Params & Reset board"/>
</form>
-->

<form  name="frmwx" id= "frmWX" method ="get" action = "wx.cgi">
<h3 >Wunderground Station Settings </h3>
Station ID:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type=text size=15  maxlength=15 value="%l41" name="wi" /> <br/>
Station PWD:&nbsp;&nbsp;&nbsp;&nbsp; <input type=text size=15  maxlength=15 value="%l42" name="wp" /> <br/>
Station Elev:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  <input type=text size=4  maxlength=4 value="%l47" name="we" /> <br/>
Wunder IP :&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 
<input type=text size=3  maxlength=3 value="%k91" name="k91"/>:
<input type=text size=3  maxlength=3 value="%k92" name="k92"/>:
<input type=text size=3  maxlength=3 value="%k93" name="k93"/>:
<input type=text size=3  maxlength=3 value="%k94" name="k94"/> 
<p>
<em>Default for&quot;www.weatherstation.wunderground.com&quot; is 38.102.136.125 </em></p>
<input type="hidden" name="m" value="r"/> 
<input type="button" value="Set Station Params & Reset " onclick="doSubmit_wx_form(document.frmwx)"/>
</form>


<form  id= "frmrtc" method ="get" action = "wx.cgi">
<h3 > Real Time Clock </h3>
<p>To set the clock enter the fields and click Set Clock.</p>

	Year-Month-Day&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Hour:Min:Sec <br/>
			 
	<input type=text size=4  maxlength=4 value="%x40" name="xtY" onblur="setRTC(name, value,2000,2100)"/> -
	<input type=text size=1  maxlength=2 value="%x41" name="xtM" onblur="setRTC(name, value,1,12)"/> -
	<input type=text size=1  maxlength=2 value="%x42" name="xtD" onblur="setRTC(name, value,1,31)"/>&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	<input type=text size=1  maxlength=2 value="%x43" name="xtH" onblur ="setRTC(name, value,0,23)"/> :
	<input type=text size=1  maxlength=2 value="%x44" name="xtm" onblur ="setRTC(name, value,0,59)"/> :
	<input type=text size=1  maxlength=2 value="%x45" name="xts" onblur= "SetRTC(name, value,0,59)"/> <br/><br/>
	<input type=submit value="Set Clock "/> <br/>
</form>
 