function elePos(e) {
	var ex = ey = 0;
	if (e.offsetParent) {
		ex = e.offsetLeft
		ey = e.offsetTop
		while (e = e.offsetParent) {
			ex += e.offsetLeft
			ey += e.offsetTop
		}
	}
	return [ex,ey];
}

/* ToolTip Show */
function ttShow(evt)
{
    ev = evt ? evt : window.event; /* IE, Mozilla */
    srcEle = ev.target ? ev.target : ev.srcElement;	/* IE, Mozilla */
    ttEle = document.getElementById(""+srcEle.id+"_");
    
    xy = elePos(srcEle);
    ttEle.style.top = xy[1] + srcEle.offsetHeight;
    ttEle.style.left = xy[0];

    ttEle.style.visibility = 'visible'; 
}

/* ToolTip Hide */
function ttHide(evt)
{
    ev = evt ? evt : window.event; /* IE, Mozilla */
    srcEle = ev.target ? ev.target : ev.srcElement;	/* IE, Mozilla */
    ttEle = document.getElementById(""+srcEle.id+"_");
    ttEle.style.visibility = 'hidden';
}

function ttInit()
{
	for(i=20;i>=0;i--) {
		ele = document.getElementById("tt" + i);
		if (ele != null) {
			ele.onmouseover = ttShow;
			ele.onmouseout = ttHide;
		}
	}
}

function scfg(user, stackVer, appVer, bootDly, serDly, sysFlags, hasBL, xbrdtype)
{
	if (isNaN(serDly)) serDly = 60;
	if (isNaN(bootDly)) bootDly = 5;
	serDlyStr = (serDly * 0.05).toString();
	bootDlyStr = (bootDly * 0.8).toString();

	i = serDlyStr.indexOf(".");
	if (i != -1) serDlyStr = serDlyStr.slice(0, i+2);

	i = bootDlyStr.indexOf(".");
	if (i != -1) bootDlyStr = bootDlyStr.slice(0, i+2);
	
	b = 0;

	//ToolTips	
	document.write(" \
	<div id=tt1_ class=tt>On startup, the SBC65EC will wait in bootloader mode for a while. \
		During this time it will check for a connection request from the bootloader client. If found, \
		it will connect with the client and wait for further instructions. <b>If you have difficulty \
		connecting to the SBC65EC with the bootloader client, increase this time!</b> Setting this time to 0 \
		will disable the bootloader!</div> \
	<div id=tt2_ class=tt>On startup, the SBC65EC will write a message out on its serial port and \
		wait the configured time for input from the user. If any input is detected, it will enter \
		serial configuration mode. Setting this time to 0 will disable serial configuration mode!</div> \
	");

	document.write(" \
	<form id=formScfg method=GET action=\"XSCFG.CGI\" onsubmit=\"scfgSubmit();\"> \
	<table class=bBox cellpadding=3 cellspacing=1> \
		<tr><td class=bHdr colspan=2>System Settings</td></tr> \
			<tr><td class=bLbl>Blink System LED</td> \
			<td class=bCtr><input type=checkbox id=sysFlag1 " + ((sysFlags&0x02)?"checked":"") + "> \
			<i>If checked, System LED is toggled every 500ms</i></td></tr> \
			<tr><td class=bLbl id=tt2>Serial Configuration startup delay</td> \
			<td class=bCtr><input type=text size=5 maxlength=5 id=serDly value="+serDlyStr+">&nbsp;<i>In seconds. Default is 3.2 seconds, maximum of 12 seconds</i></td></tr> \
			<tr><td class=bLbl>Expansion Board Type:</td> \
			<td class=bCtr><select id=selectXdb> \
			<option value=0>None</option> \
			<option value=1>MXD2R</option> \
			<option value=2>IOR5E</option> \
			<option value=3>WX</option> \
			</select></td> </tr> \
			<tr><td class=bBot colspan=2><input type=submit value=Update></td></tr> \
	</table> \
	<!-- Hidden elements - values calculated during submit --> \
	<input type=hidden name=k14> \
	<input type=hidden name=k15> \
	<input type=hidden name=k87> \
	</form> \
	");
	
	/* Select correct options, s because option value is a string */
	selOpt(document.getElementById("selectXdb"), 's', xbrdtype);
}

function scfgSubmit() {
	document.getElementsByName("k14")[0].value = getByte(Math.ceil((document.getElementById("serDly").value/0.05)), 60);
	document.getElementsByName("k15")[0].value = getCbxByte("sysFlag",0);
	document.getElementsByName("k87")[0].value = document.getElementById("selectXdb").value;
}