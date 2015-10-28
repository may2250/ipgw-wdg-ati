/**************************************************************************
        2007-1-5 11:42 establish by lzh.A cgi program.
        get a file from user's explorer.
***************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEAL_BUF_LEN  1024
#define SIGN_CODE_LEN  100
#define FILE_NAME_LEN 64
#define FILE_SAVE_DIR "/usr/local/upload/upgrade.bin"

enum
{
    STATE_START,
    STATE_GET_SIGN_CODE,
    STATE_GET_FILE_NAME,
    STATE_GET_FILE_START,
    STATE_GET_FILE_CONTENT,
    STATE_CHECK_END,
    STATE_END
};


/* ��������￪ʼ */

int main(void)
{
    FILE *fp; /* �ļ�ָ�룬��������Ҫ��õ��ļ� */
    int getState = STATE_START;
    int contentLength;/*��׼�������ݳ���*/
    int nowReadLen;
    int signCodeLen;
    int tmpLen;
    char *nowReadP;
    char *nowWriteP;
    char dealBuf[DEAL_BUF_LEN];
    char signCode[SIGN_CODE_LEN]; /*�洢���ε�������*/
    char tmpSignCode[SIGN_CODE_LEN];
    char fileName[FILE_NAME_LEN];
	char errorBuf[200];  

	
    memset(dealBuf,0,DEAL_BUF_LEN);
    memset(signCode,0,SIGN_CODE_LEN);
    memset(fileName,0,FILE_NAME_LEN);
	memset(errorBuf, 0, 200);

	printf("%s","Content-type:text/html \r\n\r\n");  
	
    nowReadLen = 0;
    if((char *)getenv("CONTENT_LENGTH")!=NULL)
    {
        contentLength = atoi((char *)getenv("CONTENT_LENGTH"));
    }
    else
    {
        //ShowErrorInfo("û�лָ�����!");
        //exit(1);
        sprintf(errorBuf, "NULL data!");
		goto error;
    }
 
    while(contentLength > 0)
    {
        if(contentLength >= DEAL_BUF_LEN)
        {
            nowReadLen = DEAL_BUF_LEN;
        }
        else
        {
            nowReadLen = contentLength;
        }
        contentLength -= nowReadLen;
        if(fread(dealBuf,sizeof(char),nowReadLen,stdin) != nowReadLen)
        {
           // ShowErrorInfo("��ȡ�ָ�����ʧ�ܣ������ԣ�");
            sprintf(errorBuf, "Receive data failed!");
			goto error;
        }
        nowReadP = dealBuf;
        while(nowReadLen > 0)
        {
            switch (getState)
            {
                case STATE_START:
                    nowWriteP = signCode;
                    getState = STATE_GET_SIGN_CODE;
                case STATE_GET_SIGN_CODE:
                    if(strncmp(nowReadP,"\r\n",2) == 0)
                    {
                        signCodeLen = nowWriteP - signCode;
                        nowReadP++;
                        nowReadLen--;
                        *nowWriteP = 0;
                        getState = STATE_GET_FILE_NAME;
   					   //ShowErrorInfo(signCode);
                    }
                    else
                    {
                        *nowWriteP = *nowReadP;
                        nowWriteP++;
                    }
                    break;
                case STATE_GET_FILE_NAME:
                    if(strncmp(nowReadP,"filename=",strlen("filename=")) == 0)
                    {
                        nowReadP += strlen("filename=");
                        nowReadLen -= strlen("filename=");
                        nowWriteP = fileName + strlen(FILE_SAVE_DIR);
                        while(*nowReadP != '\r')
                        {
                            if(*nowReadP == '\\' || *nowReadP == '/')
                            {
                                nowWriteP = fileName + strlen(FILE_SAVE_DIR);
                            }
                            else if(*nowReadP != '\"')
                            {
                                *nowWriteP = *nowReadP;
                                nowWriteP++;
                            }
                            nowReadP++;
                            nowReadLen--;
                        }
                        *nowWriteP = 0;
                        nowReadP++;
                        nowReadLen--;
                        getState = STATE_GET_FILE_START;
						#if 0
                        memcpy(fileName,FILE_SAVE_DIR,strlen(FILE_SAVE_DIR));
                        if((fp=fopen(fileName,"w"))==NULL)
						#else
						if((fp=fopen(FILE_SAVE_DIR,"wb"))==NULL)
						#endif
                        {
                            fprintf(stderr,"open file error\n");
                           // exit(1);
                           goto error;
                        }
     					 //ShowErrorInfo(fileName);
                    }
                    break;
                case STATE_GET_FILE_START:
                    if(strncmp(nowReadP,"\r\n\r\n",4) == 0)
                    {
                        nowReadP += 3;
                        nowReadLen -= 3;
                        getState = STATE_GET_FILE_CONTENT;
				      //ShowErrorInfo("get");
                    }
                    break;
                case STATE_GET_FILE_CONTENT:
                    if(*nowReadP != '\r')
                    {
                        fputc(*nowReadP,fp);
                    }
                    else
                    {
                        if(nowReadLen >= (signCodeLen + 2))
                        {
                            if(strncmp(nowReadP + 2,signCode,signCodeLen) == 0)
                            {
                                getState = STATE_END;
                                nowReadLen = 1;
                               // ShowErrorInfo("�����ϴ��ɹ�");
                                /*    if( system( "/var/landog/app/sniff/db_recover.sh" ) == 0 ){
			                                        ShowErrorInfo( "���ݿ�ָ���ɣ�����������landog" );   
			                                    }else{
			                                        ShowErrorInfo( "���ݿ�ָ������г��ִ��󣬴���ԭ�򣺻ָ����ļ��Ѿ���" );
			                                    }
			                                */
                            }
                            else
                            {
                                fputc(*nowReadP,fp);
                            }
                        }
                        else
                        {
                            getState = STATE_CHECK_END;
                            nowWriteP = tmpSignCode;
                            *nowWriteP = *nowReadP;
                            nowWriteP++;
                            tmpLen = 1;
                        }
                    }
                    break;
                case STATE_CHECK_END:
                    if(*nowReadP != '\r')
                    {
                        if(tmpLen < signCodeLen + 2)
                        {
                            *nowWriteP = *nowReadP;
                            nowWriteP++;
                            tmpLen++;
                            if(tmpLen == signCodeLen + 2)
                            {
                                *nowWriteP = 0;
                                if((tmpSignCode[1] == '\n')&&(strncmp(tmpSignCode + 2,signCode,signCodeLen) == 0))
                                {
                                    getState = STATE_END;
                                    nowReadLen = 1;
                                   // ShowErrorInfo("�����ϴ��ɹ�");
                                    /*    if( system( "/var/landog/app/sniff/db_recover.sh" ) == 0 ){
				                                            ShowErrorInfo( "���ݿ�ָ���ɣ�����������landog" );   
				                                        }else{
				                                            ShowErrorInfo( "���ݿ�ָ������г��ִ��󣬴���ԭ�򣺻ָ����ļ��Ѿ���" );
				                                        }
				                                    */
                                }
                                else
                                {
        							 //fprintf(fp,tmpSignCode);
                                    fwrite(tmpSignCode,sizeof(char),tmpLen,fp);
                                    getState = STATE_GET_FILE_CONTENT;
                                }
                            }
                        }
                    }
                    else
                    {
                        *nowWriteP = 0;
      					//fprintf(fp,tmpSignCode);
                        fwrite(tmpSignCode,sizeof(char),tmpLen,fp);
                        nowWriteP = tmpSignCode;
                        *nowWriteP = *nowReadP;
                        nowWriteP++;
                        tmpLen = 1;
                    }
                    break;
                case STATE_END:
                    nowReadLen = 1;
                    break;
                    default:break;
            }
            nowReadLen--;
            nowReadP++;
        }
    }
    if(fp != NULL)
    {
        fclose(fp);
    }

error:
	while (fread(dealBuf, 1, DEAL_BUF_LEN, stdin) > 0) ;
	
	if(errorBuf[0]!='\0')  
    {
    	//��ӡ��Ϣ����ҳ�����ص�iframe��  
		printf("<script type='text/javascript' language='javascript'>alert('%s');parent.location.replace('/cgi-bin/index.cgi?system=1');</script>",errorBuf);  
    }
    else   
    {	//printf("file upload success !<br>");  
		//   printf("<script type='text/javascript' language='javascript'>alert('File upload success!');parent.location.replace('system.html');</script>");    
		//printf("<script type='text/javascript' language='javascript'>alert('�������ϴ����!');parent.location.replace('/cgi-bin/index.cgi?system=1');</script>");	  
		printf("<script type='text/javascript' language='javascript'>alert('File upload success!');parent.location.replace('/cgi-bin/index.cgi?system=1');</script>");	  
    }  
    return 0;
}

