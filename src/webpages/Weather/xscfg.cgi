<html>
<head>
<meta http-equiv="Pragma" content="no-cache">
<link href="mx.css" rel="stylesheet" type="text/css">
<script src="lib01.js"></script>
<script src="syscfg65.js"></script>

<style type="text/css">
.tt 
{
visibility:hidden;
position:absolute;
top:0;
left:0;
z-index:2;
font:normal 8pt sans-serif; 
padding:3px;
border:solid 1px #000000;
background-color:#FFFFDD;
width:30em;
}
</style>
<script type="text/javascript">
function doSubmit_Reset_form(frm)
{
	frm.submit();
	setTimeout("window.location.replace('status.cgi')",1000); // relaod the status display after the SBC rebooted 
}

</script>
</head>

<body class=ifrmBody>
<script type="text/javascript">

	if (%l03 == 0)
		sectags();
	else {
		/* scfg("Guest", "V2.04", "V3.00", 3, 60, 0x03); //Uncomment for testing */
		scfg("%l00","%l01","%l02",%k12,%k14,%k15,"%l04",%k87);
		ttInit();	//ToolTip Init
	}
</script>

<form name ="frmRst" method="GET" action="rst.cgi"> 
<input type="hidden" name="m" value="r"/>
<input type="button" value="Reset Board" onclick="doSubmit_Reset_form(document.frmRst)"/>
</form>

</body>
</html>