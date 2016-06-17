#pragma once

typedef struct 
{
	HWND   hWnd;
	HANDLE hUART;
	FILE*  fp;
	DWORD  dwLoadAddr;
	BOOL   bAutoBoot;
	BOOL   bAutoErase;
}UPDATEPARAM;

BOOL StartUpdate(UPDATEPARAM* uParam);
void StopUpdate(void);


/*
* ������Ϣ:������־
* WPARAM: 0=�߳̽���,1=׷����Ϣ,2=������Ϣ
* LPARAM: ��Ϣ�ı�
*/
#define WM_UPDATE_EVENT	(WM_USER+1)
