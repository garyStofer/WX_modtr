<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<!-- <meta http-equiv="refresh" content="15"> -->

<link href="mx.css" rel="stylesheet" type="text/css">
<script src="lib01.js"></script>
<script type="text/javascript">
function setRTC(n, v, min, max) {
	if ( v > max )
		alert("Value " + v +" entered too high, max is " + max );
	else if ( v < min )
		alert("Value " + v +" entered too low, min is " + min );
//	else
// This will reload the current page with the GET arguments tacked onto the URL 
//		window.location = document.getElementById("frmrtc").action	+ "?" + n + "=" + v;

}
</script>
<style type="text/css">
.style1 {
	text-decoration: underline;
}
</style>
</head>

<body class=ifrmBody>

 <br>
 <script type="text/javascript">  document.write( window.location.href);  </script>
 <hr> 
<form  id= "frmWX" method ="get" action = "EXP1.cgi">
<h3> Weather station related configuration</h3>


<h4 class="style1">Wunderground </h4>
Station ID:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; <input type=text size=16  maxlength=16 value=%l41 name="wi" > <br>
Station PWD:&nbsp;&nbsp;&nbsp;&nbsp; <input type=text size=16  maxlength=16 value=%l42 name="wp" > <br>
Station Elev:&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;  <input type=text size=4  maxlength=4 value=%l47 name="we" > <br>
Wunder IP :&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; 
<input type=text size=3  maxlength=3 value=%l43 name="w0"> :
<input type=text size=3  maxlength=3 value=%l44 name="w1"> :
<input type=text size=3  maxlength=3 value=%l45 name="w2"> :
<input type=text size=3  maxlength=3 value=%l46 name="w3">&nbsp;&nbsp;  <em>Default for Wunderground is 38.102.136.125 </em><br><br>
<input type=submit o value="Set WX Params ">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
<em>New WX parameters become active upon reset of the board</em><br>
</form>

<form  id= "frmrtc" method ="get" action = "EXP1.cgi">
<h4 class="style1"> Real Time Clock </h4>
<p>To set the clock enter the fields and click Set Clock.</p>

	Year-Month-Day&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;Hour:Min:Sec <br>
			 
	<input type=text size=4  maxlength=4 value=%x40 name="xtY" onblur="setRTC(name, value,2000,3000)"> -
	<input type=text size=1  maxlength=2 value=%x41 name="xtM" onblur="setRTC(name, value,1,12)"> -
	<input type=text size=1  maxlength=2 value=%x42 name="xtD" onblur="setRTC(name, value,1,31)">&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
	<input type=text size=1  maxlength=2 value=%x43 name="xtH" onblur ="setRTC(name, value,0,23)"> :
	<input type=text size=1  maxlength=2 value=%x44 name="xtm" onblur ="setRTC(name, value,0,59)"> :
	<input type=text size=1  maxlength=2 value=%x45 name="xts" onblur= "SetRTC(name, value,0,59)"> <br><br>

	<input type=submit o value="Set Clock "> <br>
	
</form>
 <hr>
 <h3> Weather station data read out </h3>

Wind  Dir&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; = %l48 Degrees <br>
Wind Speed = %l49 Mph<br>
Wind Gust&nbsp;&nbsp;&nbsp; = %l50 Mph<br>
Temp&nbsp;&nbsp; &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; = %l52 Deg.	F<br>
Dewpoint&nbsp;&nbsp;&nbsp;&nbsp; = %l55 Deg. F<br>
R Humidity&nbsp;&nbsp;&nbsp; = %l54 Percent<br>
Baro&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp; = %l53 inch Hg<br>
Solar Rad&nbsp;&nbsp;&nbsp;&nbsp; = %l51 W/m2<br>
<br>
 <hr>