#include "stdafx.h"
#include "global.h"

CString hwFormatMessage ( DWORD dwErrorCode )
{
	CString csError;
	LPVOID pv;
    FormatMessage (
		FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
		NULL,
		dwErrorCode,
		MAKELANGID(LANG_NEUTRAL,SUBLANG_DEFAULT),
		(LPTSTR)&pv,
		0,
		NULL);
	if(pv)
	{
		csError = (char*)pv;
		LocalFree ( pv );
	}

	return csError;
}
//
// 等待线程退出
//
BOOL WaitForThreadEnd ( HANDLE *phThread, DWORD dwWaitTime /*=5000*/ )
{
	BOOL bRet = TRUE;
	ASSERT ( phThread );
	if ( !(*phThread) ) return TRUE;
	if ( ::WaitForSingleObject ( *phThread, dwWaitTime ) == WAIT_TIMEOUT )
	{
		bRet = FALSE;
		::TerminateThread ( *phThread, 0 );
	}
	::CloseHandle ( *phThread );
	(*phThread) = NULL;
	return bRet;
}

BOOL WaitForThreadEnd ( HANDLE *pEvtTerminate, HANDLE *phThread, DWORD dwWaitTime /*=5000*/ )
{
	if ( pEvtTerminate && HANDLE_IS_VALID(*pEvtTerminate) )
		::SetEvent ( (*pEvtTerminate) );
	BOOL bRet = WaitForThreadEnd ( phThread, dwWaitTime );
	if ( pEvtTerminate && HANDLE_IS_VALID(*pEvtTerminate) )
	{
		::CloseHandle ( *pEvtTerminate );
		(*pEvtTerminate) = NULL;
	}

	return bRet;
}
