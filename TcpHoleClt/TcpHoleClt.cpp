// TcpHoleClt.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "TcpHoleClt.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

HANDLE m_hEvtEndModule = NULL;

HANDLE g_hThread_Main = NULL;
CSocket *g_pSock_Main = NULL;

HANDLE g_hThread_MakeHole = NULL;
CSocket *g_pSock_MakeHole = NULL;

HANDLE g_hThread_Listen = NULL;
CSocket *g_pSock_Listen = NULL;

char *g_pServerAddess = "callgle.xicp.net";	// ��������ַ

// ���Լ��Ŀͻ�����Ϣ
t_WelcomePkt g_WelcomePkt;
UINT g_nHolePort = 0;

HANDLE g_hEvt_MakeHoleFinished = NULL;		// �򶴲����Ѿ���ɣ������������ˣ��ͻ���A����������
HANDLE g_hEvt_ListenFinished = NULL;		// ��������������
HANDLE g_hEvt_ConnectOK = NULL;				// ���ӽ����ˣ�����¼���֪ͨ�����߳�ֹͣ���ӳ���

//
// ִ���ߣ��ͻ���A
// ������Ҫ�������ˣ��ͻ���A��ֱ�����ӱ����ˣ��ͻ���B�����ⲿIP�Ͷ˿ں�
//
BOOL Handle_SrvReqDirectConnect ( t_SrvReqDirectConnectPkt *pSrvReqDirectConnectPkt )
{
	ASSERT ( pSrvReqDirectConnectPkt );
	printf ( "You can connect direct to ( IP:%s  PORT:%d  ID:%u )\n", pSrvReqDirectConnectPkt->szInvitedIP,
		pSrvReqDirectConnectPkt->nInvitedPort, pSrvReqDirectConnectPkt->dwInvitedID );

	// ֱ����ͻ���B����TCP���ӣ�������ӳɹ�˵��TCP���Ѿ��ɹ��ˡ�
	CSocket Sock;
	try
	{
		if ( !Sock.Socket () )
		{
			printf ( "Create socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		UINT nOptValue = 1;
		if ( !Sock.SetSockOpt ( SO_REUSEADDR, &nOptValue , sizeof(UINT) ) )
		{
			printf ( "SetSockOpt socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		if ( !Sock.Bind ( g_nHolePort ) )
		{
			printf ( "Bind socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		for ( int ii=0; ii<100; ii++ )
		{
			if ( WaitForSingleObject ( g_hEvt_ConnectOK, 0 ) == WAIT_OBJECT_0 )
				break;
			DWORD dwArg = 1;
			if ( !Sock.IOCtl ( FIONBIO, &dwArg ) )
			{
				printf ( "IOCtl failed : %s\n", hwFormatMessage(GetLastError()) );
			}
			if ( !Sock.Connect ( pSrvReqDirectConnectPkt->szInvitedIP, pSrvReqDirectConnectPkt->nInvitedPort ) )
			{
				printf ( "Connect to [%s:%d] failed : %s\n", pSrvReqDirectConnectPkt->szInvitedIP, pSrvReqDirectConnectPkt->nInvitedPort, hwFormatMessage(GetLastError()) );
				Sleep (100);
			}
			else break;
		}
		if ( WaitForSingleObject ( g_hEvt_ConnectOK, 0 ) != WAIT_OBJECT_0 )
		{
			if ( HANDLE_IS_VALID ( g_hEvt_ConnectOK ) ) SetEvent ( g_hEvt_ConnectOK );
			printf ( "Connect to [%s:%d] successfully !!!\n", pSrvReqDirectConnectPkt->szInvitedIP, pSrvReqDirectConnectPkt->nInvitedPort );
			
			// ���ղ�������
			printf ( "Receiving data ...\n" );
			char szRecvBuffer[NET_BUFFER_SIZE] = {0};
			int nRecvBytes = 0;
			for ( int i=0; i<1000; i++ )
			{
				nRecvBytes = Sock.Receive ( szRecvBuffer, sizeof(szRecvBuffer) );
				if ( nRecvBytes > 0 )
				{
					printf ( "-->>> Received Data : %s\n", szRecvBuffer );
					memset ( szRecvBuffer, 0, sizeof(szRecvBuffer) );
					SLEEP_BREAK ( 1 );
				}
				else
				{
					SLEEP_BREAK ( 300 );
				}
			}
		}
	}
	catch ( CException *e )
	{
		char szError[255] = {0};
		e->GetErrorMessage( szError, sizeof(szError) );
		printf ( "Exception occur, %s\n", szError );
		return FALSE;
	}

	return TRUE;
}

//
// ִ���ߣ��ͻ���A���ͻ���B
// �����̺߳�����
// �򶴿�ʼ�󣬿ͻ��˻�ͬʱ����һ���������������Զ˿� g_nHolePort ����������
//
DWORD WINAPI ThreadProc_Listen(
  LPVOID lpParameter   // thread data
)
{
	ASSERT ( HANDLE_IS_VALID(g_hEvt_ListenFinished) && HANDLE_IS_VALID(g_hEvt_MakeHoleFinished) );
	printf ( "Client.%u will listen at port %u\n", g_WelcomePkt.dwID, g_nHolePort );

	BOOL bRet = FALSE;
	CSocket Sock;
	// ����Socket���������Զ˿� g_nHolePort ����������
	try
	{
		if ( !Sock.Socket () )
		{
			printf ( "Create socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		UINT nOptValue = 1;
		if ( !Sock.SetSockOpt ( SO_REUSEADDR, &nOptValue , sizeof(UINT) ) )
		{
			printf ( "SetSockOpt socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		if ( !Sock.Bind ( g_nHolePort ) )
		{
			printf ( "Bind socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		if ( !Sock.Listen () )
		{
			printf ( "Listen failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		printf ( "Start TCP server listen port : %u\n", g_nHolePort );
		g_pSock_Listen = &Sock;
		if ( HANDLE_IS_VALID(g_hEvt_ListenFinished) )
			SetEvent ( g_hEvt_ListenFinished );

		CSocket sockAct;
		if ( Sock.Accept ( sockAct ) )
		{
			CString csSocketAddress;
			UINT nPort = 0;
			if ( !sockAct.GetPeerName ( csSocketAddress, nPort ) )
			{
				printf ( "GetPeerName failed : %s\n", hwFormatMessage(GetLastError()) );
			}
			else
			{
				if ( HANDLE_IS_VALID ( g_hEvt_ConnectOK ) ) SetEvent ( g_hEvt_ConnectOK );
				printf ( "Client.%u accept %s:%u\n", g_WelcomePkt.dwID, csSocketAddress, nPort );
				// ���Ͳ�������
				printf ( "Sending data ...\n" );
				char szBuf[1024] = {0};
				for ( int i=0; i<10; i++ )
				{
					int nLen = _snprintf ( szBuf, sizeof(szBuf), "Line.%04d - Test Data", i );
					if ( sockAct.Send ( szBuf, nLen ) != nLen )
					{
						printf ( "Send data failed : %s\n", hwFormatMessage(GetLastError()) );
						break;
					}
					else
					{
						printf ( "Sent Data : %s -->>>\n", szBuf );
						SLEEP_BREAK ( 300 );
					}
				}
			}
		}
	}
	catch ( CException *e )
	{
		char szError[255] = {0};
		e->GetErrorMessage( szError, sizeof(szError) );
		printf ( "Exception occur, %s\n", szError );
		goto finished;
	}
	
	bRet = TRUE;

finished:
	printf ( "ThreadProc_Listen end\n" );
	return bRet;
}

//
// ִ���ߣ��ͻ���A
// ���¿ͻ���B��¼�ˣ��ң��ͻ���A�����ӷ������˿� SRV_TCP_HOLE_PORT ��������ͻ���B����ֱ�ӵ�TCP����
//
BOOL Handle_NewUserLogin ( CSocket &MainSock, t_NewUserLoginPkt *pNewUserLoginPkt )
{
	printf ( "New user ( %s:%u:%u ) login server\n", pNewUserLoginPkt->szClientIP,
		pNewUserLoginPkt->nClientPort, pNewUserLoginPkt->dwID );

	BOOL bRet = FALSE;
	DWORD dwThreadID = 0;
	t_ReqConnClientPkt ReqConnClientPkt;
	CSocket Sock;
	CString csSocketAddress;
	char szRecvBuffer[NET_BUFFER_SIZE] = {0};
	int nRecvBytes = 0;
	// ������Socket�����ӷ�����Э���򶴵Ķ˿ں� SRV_TCP_HOLE_PORT
	try
	{
		if ( !Sock.Socket () )
		{
			printf ( "Create socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		UINT nOptValue = 1;
		if ( !Sock.SetSockOpt ( SO_REUSEADDR, &nOptValue , sizeof(UINT) ) )
		{
			printf ( "SetSockOpt socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		if ( !Sock.Bind ( 0 ) )
		{
			printf ( "Bind socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		if ( !Sock.Connect ( g_pServerAddess, SRV_TCP_HOLE_PORT ) )
		{
			printf ( "Connect to [%s:%d] failed : %s\n", g_pServerAddess, SRV_TCP_HOLE_PORT, hwFormatMessage(GetLastError()) );
			goto finished;
		}
	}
	catch ( CException *e )
	{
		char szError[255] = {0};
		e->GetErrorMessage( szError, sizeof(szError) );
		printf ( "Exception occur, %s\n", szError );
		goto finished;
	}
	g_pSock_MakeHole = &Sock;
	ASSERT ( g_nHolePort == 0 );
	VERIFY ( Sock.GetSockName ( csSocketAddress, g_nHolePort ) );

	// ����һ���߳��������˿� g_nHolePort ����������
	dwThreadID = 0;
	g_hThread_Listen = ::CreateThread ( NULL, 0, ::ThreadProc_Listen, LPVOID(NULL), 0, &dwThreadID );
	if (!HANDLE_IS_VALID(g_hThread_Listen) ) return FALSE;
	Sleep ( 3000 );

	// �ң��ͻ���A���������Э���򶴵Ķ˿ں� SRV_TCP_HOLE_PORT �������룬ϣ�����µ�¼�Ŀͻ���B��������
	// �������Ὣ�ҵĴ��õ��ⲿIP�Ͷ˿ںŸ��߿ͻ���B
	ASSERT ( g_WelcomePkt.dwID > 0 );
	ReqConnClientPkt.dwInviterID = g_WelcomePkt.dwID;
	ReqConnClientPkt.dwInvitedID = pNewUserLoginPkt->dwID;
	if ( Sock.Send ( &ReqConnClientPkt, sizeof(t_ReqConnClientPkt) ) != sizeof(t_ReqConnClientPkt) )
		goto finished;

	// �ȴ���������Ӧ�����ͻ���B���ⲿIP��ַ�Ͷ˿ںŸ����ң��ͻ���A��
	nRecvBytes = Sock.Receive ( szRecvBuffer, sizeof(szRecvBuffer) );
	if ( nRecvBytes > 0 )
	{
		ASSERT ( nRecvBytes == sizeof(t_SrvReqDirectConnectPkt) );
		PACKET_TYPE *pePacketType = (PACKET_TYPE*)szRecvBuffer;
		ASSERT ( pePacketType && *pePacketType == PACKET_TYPE_TCP_DIRECT_CONNECT );
		Sleep ( 1000 );
		Handle_SrvReqDirectConnect ( (t_SrvReqDirectConnectPkt*)szRecvBuffer );
		printf ( "Handle_SrvReqDirectConnect end\n" );
	}
	// �Է��Ͽ�������
	else
	{
		goto finished;
	}
	
	bRet = TRUE;
finished:
	g_pSock_MakeHole = NULL;
	return bRet;

}

//
// ִ���ߣ��ͻ���B
// �򶴴����̺߳�����
// ������Ҫ�ң��ͻ���B����ͻ���A�򶴣��ң��ͻ���B����������ͻ���A���ⲿIP�Ͷ˿ں�connect
//
DWORD WINAPI ThreadProc_MakeHole(
  LPVOID lpParameter   // thread data
)
{
/*	{	//d
		if ( HANDLE_IS_VALID(g_hEvt_MakeHoleFinished) )
			SetEvent ( g_hEvt_MakeHoleFinished );
		return 0;
	}	//d */
	ASSERT ( HANDLE_IS_VALID(g_hEvt_ListenFinished) && HANDLE_IS_VALID(g_hEvt_MakeHoleFinished) );
	t_SrvReqMakeHolePkt *pSrvReqMakeHolePkt = (t_SrvReqMakeHolePkt*)lpParameter;
	ASSERT ( pSrvReqMakeHolePkt );
	t_SrvReqMakeHolePkt SrvReqMakeHolePkt;
	memcpy ( &SrvReqMakeHolePkt, pSrvReqMakeHolePkt, sizeof(t_SrvReqMakeHolePkt) );
	delete pSrvReqMakeHolePkt; pSrvReqMakeHolePkt = NULL;
	
	printf ( "Server request make hole to ( IP:%s  PORT:%d  ID:%u )\n", SrvReqMakeHolePkt.szClientHoleIP,
		SrvReqMakeHolePkt.nClientHolePort, SrvReqMakeHolePkt.dwInviterID );

	BOOL bRet = FALSE;
	CSocket Sock;
	// ����Socket�����ض˿ڰ󶨵� g_nHolePort�����ӿͻ���A���ⲿIP�Ͷ˿ںţ��������������ʧ�ܣ�
	try
	{
		if ( !Sock.Socket () )
		{
			printf ( "Create socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		UINT nOptValue = 1;
		if ( !Sock.SetSockOpt ( SO_REUSEADDR, &nOptValue , sizeof(UINT) ) )
		{
			printf ( "SetSockOpt socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		if ( !Sock.Bind ( g_nHolePort ) )
		{
			printf ( "Bind socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		if ( HANDLE_IS_VALID(g_hEvt_MakeHoleFinished) )
			SetEvent ( g_hEvt_MakeHoleFinished );

		DWORD dwArg = 1;
		if ( !Sock.IOCtl ( FIONBIO, &dwArg ) )
		{
			printf ( "IOCtl failed : %s\n", hwFormatMessage(GetLastError()) );
		}
		for ( int i=0; i<100; i++ )
		{
			if ( WaitForSingleObject ( g_hEvt_ConnectOK, 0 ) == WAIT_OBJECT_0 )
				break;
			if ( !Sock.Connect ( SrvReqMakeHolePkt.szClientHoleIP, SrvReqMakeHolePkt.nClientHolePort ) )
			{
				printf ( "Connect to [%s:%d] failed : %s\n", SrvReqMakeHolePkt.szClientHoleIP, SrvReqMakeHolePkt.nClientHolePort, hwFormatMessage(GetLastError()) );
				Sleep ( 100 );
			}
			else
			{
				if ( HANDLE_IS_VALID ( g_hEvt_ConnectOK ) ) SetEvent ( g_hEvt_ConnectOK );
				// ��Щ·��������TPLink R402�����ô򶴾���ֱ�����ӽ�ȥ
				// ���ղ�������
				printf ( "Connect success when make hole. Receiving data ...\n" );
				char szRecvBuffer[NET_BUFFER_SIZE] = {0};
				int nRecvBytes = 0;
				for ( int i=0; i<1000; i++ )
				{
					nRecvBytes = Sock.Receive ( szRecvBuffer, sizeof(szRecvBuffer) );
					if ( nRecvBytes > 0 )
					{
						printf ( "-->>> Received Data : %s\n", szRecvBuffer );
						memset ( szRecvBuffer, 0, sizeof(szRecvBuffer) );
						SLEEP_BREAK ( 1 );
					}
					else
					{
						SLEEP_BREAK ( 300 );
					}
				}
				goto finished;
			}
		}
	}
	catch ( CException *e )
	{
		char szError[255] = {0};
		e->GetErrorMessage( szError, sizeof(szError) );
		printf ( "Exception occur, %s\n", szError );
		goto finished;
	}
	
	bRet = TRUE;

finished:
	printf ( "ThreadProc_MakeHole end\n" );
	return bRet;
}

//
// ִ���ߣ��ͻ���B
// ���������Ҫ�ң��ͻ���B��������һ���ͻ��ˣ�A���򶴣��򶴲������߳��н��С�
// �����ӷ�����Э���򶴵Ķ˿ں� SRV_TCP_HOLE_PORT ��ͨ�����������߿ͻ���A�ң��ͻ���B�����ⲿIP��ַ�Ͷ˿ںţ�Ȼ�������߳̽��д򶴣�
// �ͻ���A���յ���Щ��Ϣ�Ժ�ᷢ����ң��ͻ���B�����ⲿIP��ַ�Ͷ˿ںŵ����ӣ���������ڿͻ���B������Ժ���У�����
// �ͻ���B��NAT���ᶪ�����SYN�����Ӷ������ܽ�����
//
BOOL Handle_SrvReqMakeHole ( CSocket &MainSock, t_SrvReqMakeHolePkt *pSrvReqMakeHolePkt )
{
	ASSERT ( pSrvReqMakeHolePkt );
	// ����Socket�����ӷ�����Э���򶴵Ķ˿ں� SRV_TCP_HOLE_PORT�����ӽ����Ժ���һ���Ͽ����ӵ��������������Ȼ�����ӶϿ�
	// �������ӵ�Ŀ�����÷�����֪���ң��ͻ���B�����ⲿIP��ַ�Ͷ˿ںţ���֪ͨ�ͻ���A
	CSocket Sock;
	try
	{
		if ( !Sock.Create () )
		{
			printf ( "Create socket failed : %s\n", hwFormatMessage(GetLastError()) );
			return FALSE;
		}
		if ( !Sock.Connect ( g_pServerAddess, SRV_TCP_HOLE_PORT ) )
		{
			printf ( "Connect to [%s:%d] failed : %s\n", g_pServerAddess, SRV_TCP_HOLE_PORT, hwFormatMessage(GetLastError()) );
			return FALSE;
		}
	}
	catch ( CException *e )
	{
		char szError[255] = {0};
		e->GetErrorMessage( szError, sizeof(szError) );
		printf ( "Exception occur, %s\n", szError );
		return FALSE;
	}

	CString csSocketAddress;
	ASSERT ( g_nHolePort == 0 );
	VERIFY ( Sock.GetSockName ( csSocketAddress, g_nHolePort ) );

	// ���ӷ�����Э���򶴵Ķ˿ں� SRV_TCP_HOLE_PORT������һ���Ͽ����ӵ�����Ȼ�����ӶϿ������������յ��������ʱ��Ҳ�Ὣ
	// ���ӶϿ�
	t_ReqSrvDisconnectPkt ReqSrvDisconnectPkt;
	ReqSrvDisconnectPkt.dwInviterID = pSrvReqMakeHolePkt->dwInvitedID;
	ReqSrvDisconnectPkt.dwInviterHoleID = pSrvReqMakeHolePkt->dwInviterHoleID;
	ReqSrvDisconnectPkt.dwInvitedID = pSrvReqMakeHolePkt->dwInvitedID;
	ASSERT ( ReqSrvDisconnectPkt.dwInvitedID == g_WelcomePkt.dwID );
	if ( Sock.Send ( &ReqSrvDisconnectPkt, sizeof(t_ReqSrvDisconnectPkt) ) != sizeof(t_ReqSrvDisconnectPkt) )
		return FALSE;
	Sleep ( 100 );
	Sock.Close ();

	// ����һ���߳�����ͻ���A���ⲿIP��ַ���˿ںŴ�
	t_SrvReqMakeHolePkt *pSrvReqMakeHolePkt_New = new t_SrvReqMakeHolePkt;
	if ( !pSrvReqMakeHolePkt_New ) return FALSE;
	memcpy ( pSrvReqMakeHolePkt_New, pSrvReqMakeHolePkt, sizeof(t_SrvReqMakeHolePkt) );
	DWORD dwThreadID = 0;
	g_hThread_MakeHole = ::CreateThread ( NULL, 0, ::ThreadProc_MakeHole, LPVOID(pSrvReqMakeHolePkt_New), 0, &dwThreadID );
	if (!HANDLE_IS_VALID(g_hThread_MakeHole) ) return FALSE;

	// ����һ���߳��������˿� g_nHolePort ����������
	dwThreadID = 0;
	g_hThread_Listen = ::CreateThread ( NULL, 0, ::ThreadProc_Listen, LPVOID(NULL), 0, &dwThreadID );
	if (!HANDLE_IS_VALID(g_hThread_Listen) ) return FALSE;

	// �ȴ��򶴺��������
	HANDLE hEvtAry[] = { g_hEvt_ListenFinished, g_hEvt_MakeHoleFinished };
	if ( ::WaitForMultipleObjects ( LENGTH(hEvtAry), hEvtAry, TRUE, 30*1000 ) == WAIT_TIMEOUT )
		return FALSE;
	t_HoleListenReadyPkt HoleListenReadyPkt;
	HoleListenReadyPkt.dwInvitedID = pSrvReqMakeHolePkt->dwInvitedID;
	HoleListenReadyPkt.dwInviterHoleID = pSrvReqMakeHolePkt->dwInviterHoleID;
	HoleListenReadyPkt.dwInvitedID = pSrvReqMakeHolePkt->dwInvitedID;
	if ( MainSock.Send ( &HoleListenReadyPkt, sizeof(t_HoleListenReadyPkt) ) != sizeof(t_HoleListenReadyPkt) )
	{
		printf ( "Send HoleListenReadyPkt to %s:%u failed : %s\n", g_WelcomePkt.szClientIP, g_WelcomePkt.nClientPort,
			hwFormatMessage(GetLastError()) );
		return FALSE;
	}
	
	return TRUE;
}

//
// ִ���ߣ��ͻ���A���ͻ���B
// ����ӷ��������������յ�������
//
BOOL HandleDataMainSocket(CSocket &MainSock, char *data, int size)
{
	if ( !data || size < 4 ) return FALSE;

	PACKET_TYPE *pePacketType = (PACKET_TYPE*)data;
	ASSERT ( pePacketType );
	switch ( *pePacketType )
	{
		// �յ��������Ļ�ӭ��Ϣ��˵����¼�Ѿ��ɹ�
	case PACKET_TYPE_WELCOME:
		{
			ASSERT ( sizeof(t_WelcomePkt) == size );
			t_WelcomePkt *pWelcomePkt = (t_WelcomePkt*)data;
			printf ( "%s:%u:%u >>> %s\n", pWelcomePkt->szClientIP, pWelcomePkt->nClientPort,
				pWelcomePkt->dwID, pWelcomePkt->szWelcomeInfo );
			memcpy ( &g_WelcomePkt, pWelcomePkt, sizeof(t_WelcomePkt) );
			ASSERT ( g_WelcomePkt.dwID > 0 );
			break;
		}
		// �����ͻ��ˣ��ͻ���B����¼����������
	case PACKET_TYPE_NEW_USER_LOGIN:
		{
			ASSERT ( size == sizeof(t_NewUserLoginPkt) );
			Handle_NewUserLogin ( MainSock, (t_NewUserLoginPkt*)data );
			break;
		}
		// ������Ҫ�ң��ͻ���B��������һ���ͻ��ˣ��ͻ���A����
	case PACKET_TYPE_REQUEST_MAKE_HOLE:
		{
			ASSERT ( size == sizeof(t_SrvReqMakeHolePkt) );
			Handle_SrvReqMakeHole ( MainSock, (t_SrvReqMakeHolePkt*)data );
			break;
		}
	}

	return TRUE;
}

//
// ִ���ߣ��ͻ���A���ͻ���B
// ���̺߳���
//
DWORD WINAPI ThreadProc_MainTCPClient(
  LPVOID lpParameter   // thread data
)
{
	BOOL bRet = FALSE;
	UINT nPort = (UINT)lpParameter;
	CSocket MainSock;
	char szRecvBuffer[NET_BUFFER_SIZE] = {0};
	int nRecvBytes = 0;
	// ���������ӵ�Socket�������ͷ�������Socket����������
	try
	{
		if ( !MainSock.Socket () )
		{
			printf ( "Create socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		UINT nOptValue = 1;
		if ( !MainSock.SetSockOpt ( SO_REUSEADDR, &nOptValue , sizeof(UINT) ) )
		{
			printf ( "SetSockOpt socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		if ( !MainSock.Bind ( 0 ) )
		{
			printf ( "Bind socket failed : %s\n", hwFormatMessage(GetLastError()) );
			goto finished;
		}
		if ( !MainSock.Connect ( g_pServerAddess, nPort ) )
		{
			printf ( "Connect to [%s:%d] failed : %s\n", g_pServerAddess, nPort, hwFormatMessage(GetLastError()) );
			goto finished;
		}
		CString csSocketAddress;
		UINT nMyPort = 0;
		VERIFY ( MainSock.GetSockName ( csSocketAddress, nMyPort ) );
		printf ( "Connect to [%s:%d] success, My Info is [%s:%d]\n", g_pServerAddess, nPort, csSocketAddress, nMyPort );
	}
	catch ( CException *e )
	{
		char szError[255] = {0};
		e->GetErrorMessage( szError, sizeof(szError) );
		printf ( "Exception occur, %s\n", szError );
		goto finished;
	}

	g_pSock_Main = &MainSock;

	// ѭ��������������
	while ( TRUE )
	{
		nRecvBytes = MainSock.Receive ( szRecvBuffer, sizeof(szRecvBuffer) );
		if ( nRecvBytes > 0 )
		{
			if ( !HandleDataMainSocket ( MainSock, szRecvBuffer, nRecvBytes ) )
				goto finished;
		}
		else if ( (nRecvBytes == 0 && GetLastError() != NO_ERROR) || (SOCKET_ERROR == nRecvBytes && GetLastError() == WSAEWOULDBLOCK) )
		{
			SLEEP_BREAK ( 10 );
		}
		// �Է��Ͽ�������
		else
		{
			goto finished;
		}
		SLEEP_BREAK ( 1 );
	}
	bRet = TRUE;

finished:
	g_pSock_Main = NULL;
	printf ( "ThreadProc_MainTCPClient end\n" );
	return bRet;
}

BOOL StartMainTCPClient ( UINT nPort, HANDLE *phThread )
{
	ASSERT ( phThread );
	DWORD dwThreadID = 0;
	*phThread = ::CreateThread ( NULL, 0, ::ThreadProc_MainTCPClient, LPVOID(nPort), 0, &dwThreadID );

	return HANDLE_IS_VALID(*phThread);
}

//
// ���г���
//
int Run ()
{
	if ( !AfxSocketInit() )
		return End( FALSE );

	m_hEvtEndModule = ::CreateEvent ( NULL, TRUE, FALSE, NULL );
	g_hEvt_MakeHoleFinished = ::CreateEvent ( NULL, FALSE, FALSE, NULL );
	g_hEvt_ListenFinished = ::CreateEvent ( NULL, FALSE, FALSE, NULL );
	g_hEvt_ConnectOK = ::CreateEvent ( NULL, TRUE, FALSE, NULL );

	if ( !HANDLE_IS_VALID(m_hEvtEndModule) || !HANDLE_IS_VALID(g_hEvt_MakeHoleFinished) ||
		!HANDLE_IS_VALID(g_hEvt_ListenFinished) || !HANDLE_IS_VALID(g_hEvt_ConnectOK) )
		return End( FALSE );
	
	if ( !StartMainTCPClient ( SRV_TCP_MAIN_PORT, &g_hThread_Main ) )
		return End( FALSE );

	printf ( "Press any key to terminate program ...\n" );
	::getchar ();
	return End( TRUE );
}

//
// ��������
//
int End ( BOOL bSuccess )
{
	if ( HANDLE_IS_VALID(m_hEvtEndModule) )
		::SetEvent ( m_hEvtEndModule );

	if ( g_pSock_Main ) g_pSock_Main->CancelBlockingCall ();
	if ( g_pSock_MakeHole ) g_pSock_MakeHole->CancelBlockingCall ();

	WSACleanup ();

	printf ( "End programe\n" );
	if ( bSuccess ) return 0;

	printf ( "Last error is : %s\n", hwFormatMessage(GetLastError()) );
	return 1;
}

/////////////////////////////////////////////////////////////////////////////
// The one and only application object

CWinApp theApp;

using namespace std;

int _tmain(int argc, TCHAR* argv[], TCHAR* envp[])
{
	int nRetCode = 0;

	if ( argc >= 2 )
	{
		g_pServerAddess = argv[1];
	}

	// initialize MFC and print and error on failure
	if (!AfxWinInit(::GetModuleHandle(NULL), NULL, ::GetCommandLine(), 0))
	{
		// TODO: change error code to suit your needs
		cerr << _T("Fatal Error: MFC initialization failed") << endl;
		nRetCode = 1;
		return nRetCode;
	}
	else
	{
		nRetCode = Run ();
	}

	return nRetCode;
}


