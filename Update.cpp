#include "stdafx.h"
#include "isp.h"
#include "Update.h"
#include "stdarg.h"

static HANDLE hThread;

static int GetTimeText(char* pzText)
{
	SYSTEMTIME st;
	GetLocalTime(&st);
	return sprintf(pzText,"[%02d:%02d:%02d]",st.wHour,st.wMinute,st.wSecond);
}

static void AppendText(UPDATEPARAM* uParam, char* fmt,...)
{
	va_list va;
	char szText[MAX_PATH];
	GetTimeText(szText);
	va_start(va,fmt);
	vsnprintf(szText+10,250,fmt,va);
	va_end(va);
	SendMessage(uParam->hWnd,WM_UPDATE_EVENT,1,(LPARAM)szText);
}

static void UpdateText(UPDATEPARAM* uParam, char* fmt,...)
{
	va_list va;
	char szText[MAX_PATH];
	GetTimeText(szText);
	va_start(va,fmt);
	vsnprintf(szText+10,250,fmt,va);
	va_end(va);
	SendMessage(uParam->hWnd,WM_UPDATE_EVENT,2,(LPARAM)szText);
}

static DWORD WINAPI UpdateThreadProc(void* arg)
{
	BYTE biVer;
	WORD wPID;
	int  ret;
	BYTE biBuff[256];
	DWORD dwAddr;
	DWORD dwCount;
	DWORD dwTotal;
	FLOAT fSpeed;
	DWORD dwLastCount;
	DWORD dwLastTime;
	DWORD dwCurrTime;
	UPDATEPARAM* uParam;
	uParam = (UPDATEPARAM*)arg;
	AppendText(uParam,"���������豸...");
	ret=ISP_Connect(uParam->hUART);
	if(ret != 1)
	{
		AppendText(uParam,"�����豸ʧ��!");
		goto THREAD_EXIT;
	}
	AppendText(uParam,"�����豸�ɹ�.");
	ret=ISP_GetVersion(&biVer);
	if(ret != 1)
	{
		AppendText(uParam,"��ȡ��������汾ʧ��!");
		goto THREAD_EXIT;
	}
	AppendText(uParam,"��������汾:%d.%d",biVer>>4,biVer&0xF);
	ret=ISP_GetPID(&wPID);
	if(ret != 1)
	{
		AppendText(uParam,"��ȡоƬPIDʧ��!");
		goto THREAD_EXIT;
	}
	AppendText(uParam,"��ȡоƬPID:0x%04X",wPID);
	if(uParam->bAutoErase)
	{
		AppendText(uParam,"���ڲ���оƬ...");
		if(biVer < 0x30)
		{
			ret = ISP_Erase();
		}
		else
		{
			ret = ISP_EraseEx();
		}
		if(ret != 1)
		{
			AppendText(uParam,"����оƬʧ��!");
			goto THREAD_EXIT;
		}
		AppendText(uParam,"����оƬ�ɹ�.");
	}
	AppendText(uParam,"��ʼ��¼...");
	dwAddr = uParam->dwLoadAddr;
	dwCount = 0;
	fseek(uParam->fp,0,SEEK_END);
	dwTotal = ftell(uParam->fp);
	fseek(uParam->fp,0,SEEK_SET);
	dwLastTime = GetTickCount();
	dwLastCount= 0;
	fSpeed = 0.0f;
	while(!feof(uParam->fp))
	{
		dwCount+=fread(biBuff,1,256,uParam->fp);
		ret=ISP_WriteMemory(dwAddr,biBuff,256);
		if(ret != 1)
		{
			AppendText(uParam,"��д���ַ0x%08Xʱʧ��!",dwAddr);
			goto THREAD_EXIT;
		}
		dwCurrTime = GetTickCount();
		if(dwCurrTime - dwLastTime >= 500)
		{
			fSpeed = (FLOAT)(((dwCount-dwLastCount)*1000)/(dwCurrTime-dwLastTime))/1024;
			dwLastCount = dwCount;
			dwLastTime = dwCurrTime;
		}
		UpdateText(uParam,"������¼:%d/%d(%d%%),�ٶ�:%.2fKB/s",dwCount,dwTotal,dwCount*100/dwTotal,fSpeed);
		dwAddr += 256;
	}
	AppendText(uParam,"��¼���.");
	if(uParam->bAutoBoot)
	{
		ret=ISP_Launch(uParam->dwLoadAddr);
		if(ret != 1)
		{
			AppendText(uParam,"���г���ʧ��!");
		}
		else
		{
			AppendText(uParam,"���г���ɹ�.");
		}
	}
THREAD_EXIT:
	SendMessage(uParam->hWnd,WM_UPDATE_EVENT,0,0);
	hThread = NULL;
	return 0;
}

BOOL StartUpdate(UPDATEPARAM* uParam)
{
	DWORD tid;
	if(hThread == NULL)
	{
		hThread=CreateThread(NULL,0,UpdateThreadProc,uParam,0,&tid);
		return (hThread!=NULL);
	}
	return 0;
}

void StopUpdate(void)
{
	if(hThread != NULL)
	{
		TerminateThread(hThread,0);
		CloseHandle(hThread);
		hThread = NULL;
	}
}
