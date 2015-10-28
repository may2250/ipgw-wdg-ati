#include <stdio.h> 
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>


#include "cgic.h"

#include "dataType.h"
#include "tcpClient.h"
#include "cmd.h"
#include "web.h"




void Script_funOnlyNumber()
{
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "function FunOnlyNumber(e) {\n");
	fprintf(cgiOut, "var  key=e.keyCode|| e.which;\n");
	fprintf(cgiOut, "if(key==8 || key==9) return true;\n");
	fprintf(cgiOut, "if(!e.shiftKey && ((key>=35 && key<=39) || key==46))	return true;\n");
	fprintf(cgiOut, "if(key>=48 && key<=57) return true;\n"); // [0:9]
	fprintf(cgiOut, "return false;\n");
	fprintf(cgiOut, "}\n");
	fprintf(cgiOut, "</script>\n");
}

void Script_funOnlyHexNumber()
{
	fprintf(cgiOut, "<script type=\"text/javascript\">\n");
	fprintf(cgiOut, "function FunOnlyHexNumber(e) {\n");
	fprintf(cgiOut, "var  key=e.keyCode|| e.which;\n");
	fprintf(cgiOut, "if(key==8 || key==9) return true;\n");
	fprintf(cgiOut, "if(!e.shiftKey && ((key>=35 && key<=39) || key==46))	return true;\n");
	fprintf(cgiOut, "if(key>=48 && key<=57) return true;\n"); // [0:9]
	fprintf(cgiOut, "if(key>=65 && key<=70) return true;\n"); // [a:f]
	fprintf(cgiOut, "if(key>=97 && key<=102) return true;\n"); // [A:F]
	fprintf(cgiOut, "return false;\n");
	fprintf(cgiOut, "}\n");
	fprintf(cgiOut, "</script>\n");
}

void WebWaitProcessbar()
{
	fprintf(cgiOut, "<DIV id=\"div0\" style=\"background-color:#DDDDDD;border:0 solid black;height:10px;width:200px;\" align=\"left\">\n");
	fprintf(cgiOut, "<DIV id=\"div2\" style=\"position:relative;top:0px;left:0px;background-color:#9BCDFF;height:10px;width:0px;padding-top:0px;\">\n");
	fprintf(cgiOut, "<DIV id=\"div1\" style=\"position:relative;top:0px;left:0px;color:#0000A0;text-align:center;font-size:20%%;\"></DIV></DIV></DIV>\n");
}

void WebWaitGoto(char *pRef)
{
	fprintf(cgiOut, "<script language=\"javascript\" type=\"text/javascript\">\n");
	fprintf(cgiOut, "var i = 0;  \n");
	fprintf(cgiOut, "var intervalid = setInterval(\"fun()\", 10);  \n");
	fprintf(cgiOut, "function fun() {  \n");
	fprintf(cgiOut, "if (i <= %d) {  \n", 100);
	fprintf(cgiOut, "document.getElementById('div2').style.width=2*i+'px'; i++}\n");
	fprintf(cgiOut, "else{ clearInterval(intervalid); \n");	
	fprintf(cgiOut, "window.location.href =\"%s\";} \n", pRef);
	fprintf(cgiOut, "}  \n");
	fprintf(cgiOut, "</script>\n ");
}

void ButtonResetSubmit(char *pSubmitName)
{
	char *pButtonSize = "style=\"width:80px;height:24px\"";
	
	fprintf(cgiOut, "<tr>\n");
	fprintf(cgiOut, "<td><input name=\"reset\" type=\"reset\" value=\"reset\" %s></td>\n", pButtonSize);
	fprintf(cgiOut, "<td><input name=\"%s\" type=\"submit\" value=\"submit\" %s></td>\n", pSubmitName, pButtonSize);
	fprintf(cgiOut, "</tr>\n");
}

typedef void (*WebHandleCallback_t)(void);

typedef struct StWebHandleItem_s
{
	char *pName;
	WebHandleCallback_t handle;
} StWebHandleItem_t;


int cgiMain() 
{	
	int i;
	
	StWebHandleItem_t webHandle[] = 
	{
		{"SearchChannel",WebSearchChannel},
		//{"SearchChannel",WebSearchInput},
		{"Set_Channel", Set_Channel},
		{"channel", WebGet_channel},
		{"tsOut", WebGet_output},
		{"programe", WebGet_programe},
		{"prgSubmit", WebSubmit_programe},
		{"net", WebGet_net},
		{"netSubmit", WebSubmit_net},
		{"system", WebGet_system},
		{"transmit",WebGet_Tsip},
		{"get_ts_ip",WebGet_Tsip},
		{"set_ts_ip",WebSet_Tsip},
		{"uploadSubmit", WebSubmit_upload},
		{"factorySubmit", WebSubmit_factory},
		{"rebootSubmit", WebSubmit_reboot},
		{"test", WebGet_test},
		{"edit_p",WebEditP_Test},
		{"channel_pro_edit",WebGet_channel_edit},
		{"Status",WebGet_Status},
		{"backup",WebGet_Backup},
		{"Import",WebGet_Import},
		{"Export",WebGet_Export}
		
	};
	for(i = 0; i < sizeof(webHandle)/sizeof(StWebHandleItem_t); i++)
	{
		if(cgiFormSubmitClicked(webHandle[i].pName) == cgiFormSuccess)
		{
			webHandle[i].handle();
			return 0;
		}
	}
	//Html2Cgi_waitGoto("../status.html", "");
	return 0;

}


