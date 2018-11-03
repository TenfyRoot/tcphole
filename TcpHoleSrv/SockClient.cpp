// SockClient.cpp : implementation file
//

#include "stdafx.h"
#include "TcpHoleSrv.h"
#include "SockClient.h"
#include <Afxmt.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

CPtrArray g_PtrAry_SockClient;
CCriticalSection g_CSFor_PtrAry_SockClient;
extern BOOL g_bDeleteNullSockClient;
DWORD g_nSockClientID = 0;

CSockClient* GetNewSockClient ()
{
	CSockClient *pSockClient = new CSockClient;
	if ( !pSockClient )
	{
		printf ( "New SockClient object failed\n" );
		return NULL;
	}
	g_CSFor_PtrAry_SockClient.Lock();
	g_PtrAry_SockClient.Add ( pSockClient );
	pSockClient->m_dwID = ++g_nSockClientID;
	g_CSFor_PtrAry_SockClient.Unlock();
	printf ( "Current SocketClient array count is %d\n", g_PtrAry_SockClient.GetSize() );

	return pSockClient;
}

//
// ���¿ͻ��˵�¼��Ϣ���͸������ѵ�¼�Ŀͻ��ˣ��������͸��Լ�
//
BOOL SendNewUserLoginNotifyToAll ( LPCTSTR lpszClientIP, UINT nClientPort, DWORD dwID )
{
	ASSERT ( lpszClientIP && nClientPort > 0 );
	g_CSFor_PtrAry_SockClient.Lock();
	for ( int i=0; i<g_PtrAry_SockClient.GetSize(); i++ )
	{
		CSockClient *pSockClient = (CSockClient*)g_PtrAry_SockClient.GetAt(i);
		if ( pSockClient && pSockClient->m_bMainConn && pSockClient->m_dwID > 0 && pSockClient->m_dwID != dwID )
		{
			if ( !pSockClient->SendNewUserLoginNotify ( lpszClientIP, nClientPort, dwID ) )
			{
				g_CSFor_PtrAry_SockClient.Unlock();
				return FALSE;
			}
		}
	}

	g_CSFor_PtrAry_SockClient.Unlock ();
	return TRUE;
}

CSockClient* FindSocketClient ( DWORD dwID )
{
	g_CSFor_PtrAry_SockClient.Lock ();
	for ( int i=0; i<g_PtrAry_SockClient.GetSize(); i++ )
	{
		CSockClient *pSockClient = (CSockClient*)g_PtrAry_SockClient.GetAt(i);
		if ( pSockClient && pSockClient->m_dwID == dwID )
		{
			g_CSFor_PtrAry_SockClient.Unlock ();
			return pSockClient;
		}
	}
	printf ( "Can't find ID:%u\n", dwID );
	g_CSFor_PtrAry_SockClient.Unlock ();
	return NULL;
}

void DeleteNullSocketClient ()
{
	g_bDeleteNullSockClient = FALSE;
	g_CSFor_PtrAry_SockClient.Lock ();
	for ( int i=0; i<g_PtrAry_SockClient.GetSize(); i++ )
	{
		CSockClient *pSockClient = (CSockClient*)g_PtrAry_SockClient.GetAt(i);
		if ( pSockClient && pSockClient->m_dwID == 0 )
		{
			delete pSockClient;
			g_PtrAry_SockClient.RemoveAt(i);
			i --;
		}
	}
	printf ( "Current SocketClient array count is %d\n", g_PtrAry_SockClient.GetSize() );
	g_CSFor_PtrAry_SockClient.Unlock ();
}

DWORD WINAPI ThreadProc_SockClient(
  LPVOID lpParameter   // thread data
)
{
	CSockClient *pSockClient = reinterpret_cast<CSockClient*>(lpParameter);
	if ( pSockClient )
		return pSockClient->ThreadProc_SockClient ();
	return TRUE;
}

/////////////////////////////////////////////////////////////////////////////
// CSockClient

CSockClient::CSockClient()
	: m_dwID ( 0 )
	, m_nPeerPort ( 0 )
	, m_hThread ( NULL )
	, m_hEvtEndModule ( NULL )
	, m_hEvtWaitClientBHole ( NULL )
	, m_dwThreadID ( 0 )
	, m_bMainConn ( FALSE )
{
}

CSockClient::~CSockClient()
{
	this->CancelBlockingCall ();
	if ( HANDLE_IS_VALID(m_hEvtEndModule) )
		::SetEvent ( m_hEvtEndModule );
	WaitForThreadEnd ( &m_hThread );
}


// Do not edit the following lines, which are needed by ClassWizard.
#if 0
BEGIN_MESSAGE_MAP(CSockClient, CSocket)
	//{{AFX_MSG_MAP(CSockClient)
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()
#endif	// 0

/////////////////////////////////////////////////////////////////////////////
// CSockClient member functions

//
// �Ϳͻ��˽�������
//
BOOL CSockClient::EstablishConnect(SOCKET hSock, SOCKADDR_IN &sockAddr, BOOL bMainConn)
{
	ASSERT ( HANDLE_IS_VALID(hSock) );
	m_csPeerAddress = inet_ntoa(sockAddr.sin_addr);
	m_nPeerPort = ntohs(sockAddr.sin_port);
	m_hSocket = hSock;
	m_bMainConn = bMainConn;

	if ( m_bMainConn )
	{
		if ( !SendNewUserLoginNotifyToAll ( m_csPeerAddress, m_nPeerPort, m_dwID ) )
			return FALSE;
		printf ( "[MAIN SOCKET] New connection ( %s:%d:%u ) come in\n", m_csPeerAddress, m_nPeerPort, m_dwID );
	}
	else
	{
		printf ( "<HOLE SOCKET> New connection ( %s:%d:%u ) come in\n", m_csPeerAddress, m_nPeerPort, m_dwID );
	}

	// ����һ���߳�������ÿͻ��˵���������ͨ��
	ASSERT ( m_hEvtEndModule == NULL && m_hEvtWaitClientBHole == NULL && m_hThread == NULL );
	m_hEvtEndModule = ::CreateEvent ( NULL, TRUE, FALSE, NULL );
	m_hEvtWaitClientBHole = ::CreateEvent ( NULL, FALSE, FALSE, NULL );
	m_hThread = ::CreateThread ( NULL, 0, ::ThreadProc_SockClient, this, 0, &m_dwThreadID );
	if ( !HANDLE_IS_VALID(m_hThread) || !HANDLE_IS_VALID(m_hEvtEndModule) || !HANDLE_IS_VALID(m_hEvtWaitClientBHole) )
		return FALSE;

	return TRUE;
}

//
// �������µ�¼�Ŀͻ�����Ϣ���͸��ÿͻ���
//
BOOL CSockClient::SendNewUserLoginNotify(LPCTSTR lpszClientIP, UINT nClientPort, DWORD dwID)
{
	ASSERT ( lpszClientIP && nClientPort > 0 );
	if ( !m_bMainConn ) return TRUE;
	t_NewUserLoginPkt NewUserLoginPkt;
	STRNCPY_SZ ( NewUserLoginPkt.szClientIP, lpszClientIP );
	NewUserLoginPkt.nClientPort = nClientPort;
	NewUserLoginPkt.dwID = dwID;
	printf ( "Send new user login notify to (%s:%u:%u)\n", m_csPeerAddress, m_nPeerPort, m_dwID );
	return ( SendChunk ( &NewUserLoginPkt, sizeof(t_NewUserLoginPkt), 0 ) == sizeof(t_NewUserLoginPkt) );
}

BOOL CSockClient::ThreadProc_SockClient()
{
	printf ( "Client.%u thread start.\n", m_dwID );
	BOOL bRet = FALSE;
	char szRecvBuffer[NET_BUFFER_SIZE] = {0};
	int nRecvBytes = 0;

	// WinSocket �ľ���ǲ��ܿ��̷߳��ʵģ������������¸��ӽ���
	if ( !Attach ( m_hSocket ) )
		goto finished;

	// ���ͻ�ӭ��Ϣ��֪ͨ�ͻ������ӳɹ��ˡ�
	SendWelcomeInfo ();
	// ѭ��������������
	while ( TRUE )
	{
		nRecvBytes = Receive ( szRecvBuffer, sizeof(szRecvBuffer) );
		if ( nRecvBytes > 0 )
		{
			if ( !HandleDataReceived ( szRecvBuffer, nRecvBytes ) )
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
	Close ();
	printf ( "Client.%u thread end. result : %s\n", m_dwID, bRet?"SCUESS":"FAILED" );
	if ( HANDLE_IS_VALID(m_hEvtEndModule) )
		::SetEvent ( m_hEvtEndModule );
	m_dwID = 0;
	printf ( "ThreadProc_SockClient end\n" );
	g_bDeleteNullSockClient = TRUE;
	return bRet;
}

BOOL CSockClient::HandleDataReceived(char *data, int size)
{
	if ( !data || size < 4 ) return FALSE;

	PACKET_TYPE *pePacketType = (PACKET_TYPE*)data;
	ASSERT ( pePacketType );
	switch ( *pePacketType )
	{
		// Ҫ������һ���ͻ��˽���ֱ�ӵ�TCP����
	case PACKET_TYPE_REQUEST_CONN_CLIENT:
		{
			ASSERT ( !m_bMainConn );
			ASSERT ( size == sizeof(t_ReqConnClientPkt) );
			t_ReqConnClientPkt *pReqConnClientPkt = (t_ReqConnClientPkt*)data;
			if ( !Handle_ReqConnClientPkt ( pReqConnClientPkt ) )
				return FALSE;
			break;
		}
		// �����ˣ��ͻ���B������������Ͽ����ӣ����ʱ��Ӧ�ý��ͻ���B���ⲿIP�Ͷ˿ںŸ��߿ͻ���A�����ÿͻ���A����
		// ���ӿͻ���B���ⲿIP�Ͷ˿ں�
	case PACKET_TYPE_REQUEST_DISCONNECT:
		{
			ASSERT ( !m_bMainConn );
			ASSERT ( size == sizeof(t_ReqSrvDisconnectPkt) );
			t_ReqSrvDisconnectPkt *pReqSrvDisconnectPkt = (t_ReqSrvDisconnectPkt*)data;
			ASSERT ( pReqSrvDisconnectPkt );
			printf ( "Clinet.%u request disconnect\n", m_dwID );

			CSockClient *pSockClientHole_A = FindSocketClient ( pReqSrvDisconnectPkt->dwInviterHoleID );
			if ( !pSockClientHole_A ) return FALSE;
			pSockClientHole_A->m_SrvReqDirectConnectPkt.dwInvitedID = pReqSrvDisconnectPkt->dwInvitedID;
			STRNCPY_CS ( pSockClientHole_A->m_SrvReqDirectConnectPkt.szInvitedIP, m_csPeerAddress );
			pSockClientHole_A->m_SrvReqDirectConnectPkt.nInvitedPort = m_nPeerPort;

			Close ();

			break;
		}
		// �����ˣ��ͻ���B���򶴺���������׼������
	case PACKET_TYPE_HOLE_LISTEN_READY:
		{
			ASSERT ( m_bMainConn );
			ASSERT ( size == sizeof(t_HoleListenReadyPkt) );
			t_HoleListenReadyPkt *pHoleListenReadyPkt = (t_HoleListenReadyPkt*)data;
			ASSERT ( pHoleListenReadyPkt );
			printf ( "Client.%u hole and listen ready\n", pHoleListenReadyPkt->dwInvitedID );
			// ֪ͨ������ͻ���Aͨ�ŵķ������̣߳��Խ��ͻ���B���ⲿIP��ַ�Ͷ˿ںŸ��߿ͻ���A
			CSockClient *pSockClientHole_A = FindSocketClient ( pHoleListenReadyPkt->dwInviterHoleID );
			if ( !pSockClientHole_A ) return FALSE;
			if ( HANDLE_IS_VALID(pSockClientHole_A->m_hEvtWaitClientBHole) )
				SetEvent ( pSockClientHole_A->m_hEvtWaitClientBHole );
			break;
		}
	}

	return TRUE;
}

//
// �ͻ���A�����ң���������Э�����ӿͻ���B�������Ӧ���ڴ�Socket���յ�
//
BOOL CSockClient::Handle_ReqConnClientPkt(t_ReqConnClientPkt *pReqConnClientPkt)
{
	ASSERT ( !m_bMainConn );
	CSockClient *pSockClient_B = FindSocketClient ( pReqConnClientPkt->dwInvitedID );
	if ( !pSockClient_B ) return FALSE;
	printf ( "%s:%u:%u invite %s:%u:%u connection\n", m_csPeerAddress, m_nPeerPort, m_dwID,
		pSockClient_B->m_csPeerAddress, pSockClient_B->m_nPeerPort, pSockClient_B->m_dwID );

	// �ͻ���A��Ҫ�Ϳͻ���B����ֱ�ӵ�TCP���ӣ�����������A���ⲿIP�Ͷ˿ںŸ��߸�B
	t_SrvReqMakeHolePkt SrvReqMakeHolePkt;
	SrvReqMakeHolePkt.dwInviterID = pReqConnClientPkt->dwInviterID;
	SrvReqMakeHolePkt.dwInviterHoleID = m_dwID;
	SrvReqMakeHolePkt.dwInvitedID = pReqConnClientPkt->dwInvitedID;
	STRNCPY_CS ( SrvReqMakeHolePkt.szClientHoleIP, m_csPeerAddress );
	SrvReqMakeHolePkt.nClientHolePort = m_nPeerPort;
	if ( pSockClient_B->SendChunk ( &SrvReqMakeHolePkt, sizeof(t_SrvReqMakeHolePkt), 0 ) != sizeof(t_SrvReqMakeHolePkt) )
		return FALSE;

	// �ȴ��ͻ���B����ɣ�����Ժ�֪ͨ�ͻ���Aֱ�����ӿͻ����ⲿIP�Ͷ˿ں�
	if ( !HANDLE_IS_VALID(m_hEvtWaitClientBHole) )
		return FALSE;
	if ( WaitForSingleObject ( m_hEvtWaitClientBHole, 6000*1000 ) == WAIT_OBJECT_0 )	//d
	{
		if ( SendChunk ( &m_SrvReqDirectConnectPkt, sizeof(t_SrvReqDirectConnectPkt), 0 ) == sizeof(t_SrvReqDirectConnectPkt) )
			return TRUE;
	}

	return FALSE;
}

BOOL CSockClient::SendWelcomeInfo()
{
	if ( !m_bMainConn ) return TRUE;
	t_WelcomePkt WelcomePkt;
	STRNCPY_CS ( WelcomePkt.szClientIP, m_csPeerAddress );
	WelcomePkt.nClientPort = m_nPeerPort;
	WelcomePkt.dwID = m_dwID;
	_snprintf ( WelcomePkt.szWelcomeInfo, sizeof(WelcomePkt.szWelcomeInfo),
		"Hello, ID.%u, Welcome to login", m_dwID );

	return ( SendChunk ( &WelcomePkt, sizeof(t_WelcomePkt), 0 ) == sizeof(t_WelcomePkt) );
}
