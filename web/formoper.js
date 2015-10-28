
function jsSelectItemByValue(objSelectName, objItemText) {
	var objSelect = document.getElementById(objSelectName);
	if(objSelect != null){
		for(var i=0;i<objSelect.options.length;i++) {
			if(objSelect.options[i].value == objItemText) {
				objSelect.options[i].selected = true;
				break;
			}
		}
	}
}

function jsSetRadioValue(objName, valueStr){
     var objs=document.getElementsByName(objName)
	 if(objs != null){
		for(var i=0;i<objs.length;i++){
			if(objs[i].value == valueStr){
				objs[i].checked = true;
				break;
			}
		}
	 }
}

function jsErrorInputInt(inputControl, minVal, maxVal, errStr){	
	if(inputControl){
		if(inputControl.value != ""){
			if(/^[-]?\d+$/.test(inputControl.value)){
				var num = parseInt(inputControl.value);
				if(num >= minVal && num <= maxVal){
					return false;
				}
			}
		}
		if(errStr != null)		{
			inputControl.focus();
			inputControl.select();
			alert(errStr);
		}
	}
	return true;
}

function jsErrorInputFloat(inputControl, minVal, maxVal, errStr){	
	if(inputControl){
		if(inputControl.value != ""){
			if(/^([-]?\d+)(\.\d+)?$/.test(inputControl.value)){
				var num = parseInt(inputControl.value);
				if(num >= minVal && num <= maxVal){
					return false;
				}
			}
		}
		if(errStr != null)		{
			inputControl.focus();
			inputControl.select();
			alert(errStr);
		}
	}
	return true;
}

function jsErrorInputIp(inputControl, errStr){	
	var exp=/^(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])\.(\d{1,2}|1\d\d|2[0-4]\d|25[0-5])$/;
	if(exp.test(inputControl.value)){
		return false;
	}
	if(errStr != null)	{
		inputControl.focus();
		inputControl.select();
		alert(errStr);
	}
	return true;
}

function jsErrorInputMac(inputControl, errStr){
	var exp = /[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}:[A-Fa-f0-9]{2}/;
	if(exp.test(inputControl.value)){
		return false;
	}
	if(errStr != null)	{
		inputControl.focus();
		inputControl.select();
		alert(errStr);
	}
	return true
}

function ResetStatus(strInfo){
	window.status=""; return true;
}

function WebGoto(strWeb){
	location.href = strWeb;
}

function jsInputGetInt(inputControl){
	if(inputControl){
		if(inputControl.value != ""){
			if(/^[-]?\d+$/.test(inputControl.value)){
				var num = parseInt(inputControl.value);
				return num;
			}
		}
	}
	return 0xcd;
}

function formatNumber(num){   
	if(!/^(\+|-)?(\d+)(\.\d+)?$/.test(num)){return   num;}      
	var   a   =   RegExp.$1,   b   =   RegExp.$2,   c   =   RegExp.$3;      
	var   re   =   new   RegExp().compile("(\\d)(\\d{3})(,|$)");      
	while(re.test(b))   b   =   b.replace(re,"$1,$2$3");      
	return   a +""+ b +""+ c;      
}  
