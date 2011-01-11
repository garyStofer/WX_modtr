<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="Pragma" content="no-cache"/>
<link href="mx.css" rel="stylesheet" type="text/css"/>
<title>Sensor Calibration</title>
<script type="text/javascript" language="javascript">

// modifies the value to be displayed for editing to show signed values
function formSet( frm )
{
	frm.kB7.value = "%kB7"-127;
	frm.kB8.value = "%kB8"-127;
	frm.kB9.value = "%kB9"-127;
	frm.kBA.value = "%kBA"-127;
	frm.kBB.value = "%kBB"-127;
	frm.kBC.value = "%kBC"-127;
	frm.kBD.value = "%kBD"-127;

}

//helper function to create a form on the fly
function getNewSubmitForm()
{
	var submitForm = document.createElement("FORM");
	document.body.appendChild(submitForm);
	submitForm.method = "GET";
	submitForm.action= "senscal.cgi";
	return submitForm;
}

//helper function to add <input> elements to the form
function createNewFormElement(inputForm, elementName, elementValue)
{
	var newElement = document.createElement("input");
	
	newElement.name = elementName;
	newElement.value = elementValue;
	newElement.type = "hidden";
	inputForm.appendChild(newElement);
	return newElement;
}

// function that creates the form, adds hidden elements and then submits it
// converts the signed number from the UI back to the unsigned form for tehe SBC
function createFormAndSubmit()
{
	 var submitForm = getNewSubmitForm();
	 createNewFormElement(submitForm, "kB7",Number(document.frm1.kB7.value)+127);
	 createNewFormElement(submitForm, "kB8",Number(document.frm1.kB8.value)+127); 
	 createNewFormElement(submitForm, "kB9",Number(document.frm1.kB9.value)+127); 
	 createNewFormElement(submitForm, "kBA",Number(document.frm1.kBA.value)+127); 
	 createNewFormElement(submitForm, "kBB",Number(document.frm1.kBB.value)+127); 
	 createNewFormElement(submitForm, "kBC",Number(document.frm1.kBC.value)+127); 
	 createNewFormElement(submitForm, "kBD",Number(document.frm1.kBD.value)+127); 
	 createNewFormElement(submitForm, "m", "r");	// the reset command to the SBC
	 submitForm.submit();
	 setTimeout("window.location.replace('status.cgi')",1000);// relaod the status display after the SBC rebooted 
}
</script>
</head>

<body  >
<!-- <hr/>
 <script type="text/javascript">  document.write( window.location.href);  </script>
 <hr/> 
-->
<!--
	 frm1 is only for displaying and editing the values, Submit is done via an on-the-fly created form 
     onbutton click. This is so that  we can display signed calibration values and then  convert 
     convert the signed values back to unsigned values for submittal  
 -->    
 <h3> Sensor Calibration parameters</h3>

<form   name="frm1" >
<table >
<tr>
	<td> Temp Gain</td>
	<td><input   type="text" size="4"  maxlength="4" name="kB7" /></td>
</tr>
<tr > 
	<td> Baro Gain</td>
	<td> <input	type="text" size="4"  maxlength="4"  name="kB8"/></td>
	<td> Offset</td>
	<td> <input	type="text" size="4"  maxlength="4"  name="kBB"/></td>
</tr>
<tr>
	<td>Hyg  Gain</td>
	<td><input	type="text" size="4"  maxlength="4" name="kB9"/></td>
	<td> Offset</td>
	<td><input 	type="text" size="4"  maxlength="4" name="kBC"/> </td>
</tr>
<tr>
	<td>Sol Gain</td>
	<td><input 		type="text" size="4"  maxlength="4" name="kBA"/> </td>
</tr>
<tr>
	<td>Wind Dir</td>
	<td><input 		type="text" size="4"  maxlength="4" name="kBD"/> </td>
	<td>&nbsp;Degrees offset</td>

</tr>
<tr><td></td></tr>
<tr><td colspan="4" align="center">
	<input  type="button" value="Set Params & Reset" onclick="createFormAndSubmit()"/>
</td></tr>
</table>
<script type="text/javascript" > formSet(document.frm1) ;</script>
</form>
</body>
</html>




