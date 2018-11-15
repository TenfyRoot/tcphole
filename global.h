/* Filename : global.h */
#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef DLL_INTERNAL
#define DLL_INTERNAL __declspec( dllexport )
#endif

//==========================================================================
// ���ò�����
//==========================================================================
#define GET_VALID_STRING_FROM_TOW(cs1,cs2) ( (cs1.GetLength()>0)?cs1:cs2 )
#define GET_SAFE_STRING(str) ( (str)?(str):"" )
#define NULL_STRING_FOR_DB ""
#define GET_VALID_CSTRING(cs) ( (cs).GetLength()>0?(cs):((cs),(cs)=NULL_STRING_FOR_DB) )
#define GET_VALID_CSTRING_P(csp) ( (csp)?(*(csp)):"" )
#define STRNCPY_CS(sz,cs) strncpy((char*)(sz),(cs).GetBuffer(0),sizeof(sz)-1)
#define STRNCPY_SZ(sz1,sz2) strncpy(((char*)(sz1)),(sz2)?((char*)(sz2)):"",sizeof(sz1)-1)
#define STRNCPY(sz1,sz2,size) \
{\
	strncpy(((char*)(sz1)),(sz2)?((char*)(sz2)):"",(size));\
	((char*)(sz1))[(size)-1] = '\0';\
}
#define STRCPY(sz1,sz2) strcpy ( (char*)(sz1), (char*)((sz2)?(sz2):"") )
#define STRLEN_SZ(sz) ((sz)?strlen((char*)(sz)):0)
#define COPMNC_CS_SZ(cs,sz) ( (sz) && ((cs).CompareNoCase(sz)==0) )
#define STRCMP_SAFE(sz1,sz2) (strcmp((char*)GET_SAFE_STRING(sz1),(char*)GET_SAFE_STRING(sz2)))
#define STRLEN_SAFE(sz) ((sz)?strlen((char*)(sz)):0)
#define ATOI_SAFE(sz) (atoi((const char*)(GET_SAFE_STRING((char*)(sz)))))
#define ASSERT_ADDRESS(p,size) ASSERT((p)!=NULL && AfxIsValidAddress((p),(size),TRUE))
#define VALID_IP_PORT(ip,port) ((STRLEN_SAFE(ip)>0) && (port)>1000)
// ���ر�־�Ƿ���λ����
#define SWITCH_IS_FLAG(nConstFlag,nValue) ( ( (nConstFlag) & (nValue) ) == (nConstFlag) )
#define LENGTH(x) sizeof(x)/sizeof(x[0])
#define MIN(x,y) (((DWORD)(x)<(DWORD)(y))?(x):(y))
#define MAX(x,y) (((DWORD)(x)>(DWORD)(y))?(x):(y))
// ����Ƿ���Ч
#define HANDLE_IS_VALID(h) ( HANDLE(h) && HANDLE(h) != INVALID_HANDLE_VALUE )
// �رվ��
#define SAFE_CLOSE_HANDLE(h)\
{\
	if ( HANDLE_IS_VALID ( h ) )\
	{\
		CloseHandle ( h );\
		h = NULL;\
	}\
}

// �ȴ��¼��� Sleep() ����
#define SLEEP_RETURN(x)\
{\
	if ( ::WaitForSingleObject ( m_hEvtEndModule, x ) == WAIT_OBJECT_0 )\
		return FALSE;\
}
#define SLEEP_BREAK(x)\
{\
	if ( ::WaitForSingleObject ( m_hEvtEndModule, x ) == WAIT_OBJECT_0 )\
		break;\
}

// ɾ��һ������ָ��
#define DELETE_ARRAY(pp)\
{\
	if ( (pp) && (*(pp)) )\
	{\
		(*(pp))->RemoveAll();\
		(*(pp))->FreeExtra();\
		delete (*(pp));\
		(*(pp)) = NULL;\
	}\
}

// ɾ�������� new ������ڴ�ռ䣬�����Ƕ���Ҳ��������ͨ���������ͣ���int��char��
#define DELETE_HEAP(pp)\
{\
	if ( (pp) && (*(pp)) )\
	{\
		delete (*(pp));\
		(*(pp)) = NULL;\
	}\
}

// ɾ�������� new ����������ڴ�ռ䣬�����Ƕ���Ҳ��������ͨ���������ͣ���int��char��
#define DELETE_ARY_HEAP(pp)\
{\
	if ( (pp) && (*(pp)) )\
	{\
		delete[] (*(pp));\
		(*(pp)) = NULL;\
	}\
}

// �� new ����һ�� �������ͣ��磺char��int�ȣ���ṹ����ڴ�ռ䣬�������Ƕ���
// ��ָ��ָ����Ч�Ŀռ�ͽ��Ǹ��ռ�����
#define ALLOC_MEM(pp,size,type,desc)\
{\
	if ( (pp) )\
	{\
		if ( !(*(pp)) )\
		{\
			(*(pp)) = new type[(size)];\
			if ( !(*(pp)) )\
				return OutNewObjectFailed ( desc );\
		}\
		ASSERT_ADDRESS ( (*(pp)), (size)*sizeof(type) );\
		memset ( (*(pp)), 0, (size)*sizeof(type) );\
	}\
}

//
template<class T>
int FindFromStaticArray ( IN T *pAry, IN int nArySize, IN T Find )
{
	if ( !pAry ) return -1;
	for ( int i=0; i<nArySize; i++ )
	{
		if ( pAry[i] == Find )
			return i;
	}
	return -1;
}

//
// ע�⣺����Ǵ� CString �в���ʱ Find ǧ��Ҫ�� LPCTSTR ���� char* ������һ����Ҫ�� CString ����
//
template<class T1, class T2>
int FindFromArray ( IN T1 &Ary, IN T2 Find )
{
	int nCount = Ary.GetSize();
	for ( int i=0; i<nCount; i++ )
	{
		T2 tGetValue = Ary.GetAt(i);
		if ( tGetValue == Find )
			return i;
	}
	return -1;
}

//
// ������ Ary_Org �в��ң�ֻҪ Ary_Find ���κ�һ��Ԫ���� Ary_Org �г��ֹ�
// �ͷ��ظ�Ԫ���� Ary_Org �е�λ��
//
template<class T1, class T2>
int FindFromArray ( IN T1 &Ary_Org, IN T1 &Ary_Find, OUT T2 &Element )
{
	int nCount = Ary_Find.GetSize();
	for ( int i=0; i<nCount; i++ )
	{
		T2 tGetValue = Ary_Find.GetAt(i);
		int nFindPos = FindFromArray ( Ary_Org, tGetValue );
		if ( nFindPos >= 0 )
		{
			Element = Ary_Org.GetAt ( nFindPos );
			return nFindPos;
		}
	}
	return -1;
}

template<class T1, class T2, class T3, class T4>
int FindFromArray ( IN T1 &Ary, IN T2 Find, IN T3 &AppAry, IN T4 AppFind )
{
	int nCount = Ary.GetSize();
	for ( int i=0; i<nCount; i++ )
	{
		if ( Ary.GetAt(i) == Find && 
			AppAry.GetAt(i) == AppFind )
		{
			return i;
		}
	}
	return -1;
}

template<class T1>
int FindFromArray ( IN T1 &Ary_Src, IN T1 &Ary_Find )
{
	int nCount = Ary_Src.GetSize();
	for ( int i=0; i<nCount; i++ )
	{
		if ( FindFromArray ( Ary_Find, Ary_Src.GetAt(i) ) >= 0 )
			return i;
	}
	return -1;
}

//
// ������ Ary_Src �е�Ԫ�ؿ����� Ary_Dest ��
//
template<class T>
int ArrayCopy ( IN T &Ary_Dest, IN T &Ary_Src )
{
	int nCount = Ary_Src.GetSize();
	for ( int i=0; i<nCount; i++ )
	{
		Ary_Dest.Add ( Ary_Src.GetAt(i) );
	}
	return i;
}

//
// ������ Ary_Src �е�Ԫ�ؿ����� Ary_Dest �У�����֤ Ary_Src �е�Ԫ�ز����ظ�����
//
template<class T>
int ArrayCopy_DefferValue ( IN T &Ary_Dest, IN T &Ary_Src )
{
	int nCount = Ary_Src.GetSize();
	for ( int i=0; i<nCount; i++ )
	{
		if ( FindFromArray ( Ary_Dest, Ary_Src.GetAt(i) ) < 0 )
			Ary_Dest.Add ( Ary_Src.GetAt(i) );
	}
	return i;
}

//
// ������ Ary �е�Ԫ�ش���ߵ���T2 �� Ary �ﱣ�����������
//
template<class T1, class T2>
void ReversalArray ( IN T1 &Ary, T2 &Temp )
{
	for ( int i=0; i<Ary.GetSize()/2; i++ )
	{
		Temp = Ary.GetAt(i);
		Ary.SetAt ( i, Ary.GetAt(Ary.GetSize()-1-i) );
		Ary.SetAt ( Ary.GetSize()-1-i, Temp );
	}
}

/*
		��TCPʵ��P2Pͨ�š�TCP��ԽNAT�ķ�����TCP�򶴡�
	������蹫���ϵķ�����S���ͻ���A��NAT-A���棬�ͻ���B��NAT-B���棬���ڿͻ���Aϣ���Ϳͻ���B����ֱ�ӵ�
TCP ���ӣ��ͻ���AΪ�����ˣ��ͻ���BΪ�����ˣ��ͻ���B��ͻ���B����TCP�򶴣��򶴳ɹ���ͻ���A�����ֱ����
�ͻ���B����TCP���ӡ�
*/

// ��������ַ�Ͷ˿ںŶ���
#define SRV_TCP_MAIN_PORT		4000				// �����������ӵĶ˿ں�
#define SRV_TCP_HOLE_PORT		8000				// ��������Ӧ�ͻ��˴�����Ķ˿ں�
#define NET_BUFFER_SIZE			1024				// �����С

// ���ݰ�����
typedef enum _packet_type
{
	PACKET_TYPE_INVALID,
	PACKET_TYPE_NEW_USER_LOGIN,			// �������յ��µĿͻ��˵�¼������¼��Ϣ���͸������ͻ���
	PACKET_TYPE_WELCOME,				// �ͻ��˵�¼ʱ���������͸û�ӭ��Ϣ���ͻ��ˣ��Ը�֪�ͻ��˵�¼�ɹ�
	PACKET_TYPE_REQUEST_CONN_CLIENT,	// ĳ�ͻ�������������룬Ҫ������һ���ͻ��˽���ֱ�ӵ�TCP���ӣ�����Ҫ����TCP��
	PACKET_TYPE_REQUEST_MAKE_HOLE,		// ����������ĳ�ͻ�������һ�ͻ��˽���TCP�򶴣�������һ�ͻ���ָ�����ⲿIP�Ͷ˿ںŽ���connect����
	PACKET_TYPE_REQUEST_DISCONNECT,		// ����������Ͽ�����
	PACKET_TYPE_TCP_DIRECT_CONNECT,		// ������Ҫ�������ˣ��ͻ���A��ֱ�����ӱ����ˣ��ͻ���B�����ⲿIP�Ͷ˿ں�
	PACKET_TYPE_HOLE_LISTEN_READY,		// �����ˣ��ͻ���B���򶴺���������׼������

} PACKET_TYPE;

//
// ���û���¼����
//
typedef struct _new_user_login
{
	_new_user_login ()
		: ePacketType ( PACKET_TYPE_NEW_USER_LOGIN )
		, nClientPort ( 0 )
		, dwID ( 0 )
	{
		memset ( szClientIP, 0, sizeof(szClientIP) );
	}
	PACKET_TYPE ePacketType;			// ������
	char szClientIP[32];				// �µ�¼�Ŀͻ��ˣ��ͻ���B���ⲿIP��ַ
	UINT nClientPort;					// �µ�¼�Ŀͻ��ˣ��ͻ���B���ⲿ�˿ں�
	DWORD dwID;							// �µ�¼�Ŀͻ��ˣ��ͻ���B����ID�ţ���1��ʼ��ŵ�һ��Ψһ��ţ�
} t_NewUserLoginPkt;

//
// ��ӭ��Ϣ
//
typedef struct _welcome
{
	_welcome ()
		: ePacketType ( PACKET_TYPE_WELCOME )
		, nClientPort ( 0 )
		, dwID ( 0 )
	{
		memset ( szClientIP, 0, sizeof(szClientIP) );
		memset ( szWelcomeInfo, 0, sizeof(szWelcomeInfo) );
	}
	PACKET_TYPE ePacketType;			// ������
	char szClientIP[32];				// ���ջ�ӭ��Ϣ�Ŀͻ����ⲿIP��ַ
	UINT nClientPort;					// ���ջ�ӭ��Ϣ�Ŀͻ����ⲿ�˿ں�
	DWORD dwID;							// ���ջ�ӭ��Ϣ�Ŀͻ��˵�ID�ţ���1��ʼ��ŵ�һ��Ψһ��ţ�
	char szWelcomeInfo[64];				// ��ӭ��Ϣ�ı�
} t_WelcomePkt;

//
// �ͻ���A���������Э�����ӿͻ���B
//
typedef struct _req_conn_client
{
	_req_conn_client ()
		: ePacketType ( PACKET_TYPE_REQUEST_CONN_CLIENT )
		, dwInviterID ( 0 )
		, dwInvitedID ( 0 )
	{
	}
	PACKET_TYPE ePacketType;			// ������
	DWORD dwInviterID;					// �������뷽�����������ͻ���A��ID��
	DWORD dwInvitedID;					// �����뷽�����������ͻ���B��ID��
} t_ReqConnClientPkt;

//
// ����������ͻ���B��
//
typedef struct _srv_req_make_hole
{
	_srv_req_make_hole ()
		: ePacketType ( PACKET_TYPE_REQUEST_MAKE_HOLE )
		, dwInviterID ( 0 )
		, dwInviterHoleID ( 0 )
		, dwInvitedID ( 0 )
		, nClientHolePort ( 0 )
		, nBindPort ( 0 )
	{
		memset ( szClientHoleIP, 0, sizeof(szClientHoleIP) );
	}
	PACKET_TYPE ePacketType;			// ������
	DWORD dwInviterID;					// �������뷽�����������ͻ���A��ID��
	DWORD dwInviterHoleID;				// �������뷽�����������ͻ���A����ID��
	DWORD dwInvitedID;					// �����뷽�����������ͻ���B��ID��
	char szClientHoleIP[32];			// �������IP�����󷽵��ⲿIP����ַ�򶴣�������һ��connect����
	UINT nClientHolePort;				// ������ö˿ںţ����󷽵��ⲿ�˿ںţ��򶴣�������һ��connect����
	UINT nBindPort;
} t_SrvReqMakeHolePkt;

//
// ����������Ͽ�����
//
typedef struct _req_srv_disconnect
{
	_req_srv_disconnect ()
		: ePacketType ( PACKET_TYPE_REQUEST_DISCONNECT )
		, dwInviterID ( 0 )
		, dwInviterHoleID ( 0 )
		, dwInvitedID ( 0 )
	{
	}
	PACKET_TYPE ePacketType;			// ������
	DWORD dwInviterID;					// �������뷽�����������ͻ���A��ID��
	DWORD dwInviterHoleID;				// �������뷽�����������ͻ���A����ID��
	DWORD dwInvitedID;					// �����뷽�����������ͻ���B��ID��
} t_ReqSrvDisconnectPkt;

//
// ������Ҫ�������ˣ��ͻ���A��ֱ�����ӱ����ˣ��ͻ���B�����ⲿIP�Ͷ˿ں�
//
typedef struct _srv_req_tcp_direct_connect
{
	_srv_req_tcp_direct_connect ()
		: ePacketType ( PACKET_TYPE_TCP_DIRECT_CONNECT )
		, dwInvitedID ( 0 )
		, nInvitedPort ( 0 )
	{
		memset ( szInvitedIP, 0, sizeof(szInvitedIP) );
	}
	PACKET_TYPE ePacketType;			// ������
	DWORD dwInvitedID;					// �����뷽�����������ͻ���B��ID��
	char szInvitedIP[32];				// �������IP�������뷽�ͻ���B���ⲿIP����ֱַ�ӽ���TCP����
	UINT nInvitedPort;					// ������ö˿ںţ������뷽�ͻ���B���ⲿIP����ֱַ�ӽ���TCP����
} t_SrvReqDirectConnectPkt;

//
// �����ˣ��ͻ���B���򶴺���������׼������
//
typedef struct _hole_listen_ready
{
	_hole_listen_ready ()
		: ePacketType ( PACKET_TYPE_HOLE_LISTEN_READY )
		, dwInviterID ( 0 )
		, dwInviterHoleID ( 0 )
		, dwInvitedID ( 0 )
	{
	}
	PACKET_TYPE ePacketType;			// ������
	DWORD dwInviterID;					// �������뷽�����������ͻ���A��ID��
	DWORD dwInviterHoleID;				// �������뷽�����������ͻ���A����ID��
	DWORD dwInvitedID;					// �����뷽�����������ͻ���B��ID��
} t_HoleListenReadyPkt;

CString hwFormatMessage ( DWORD dwErrorCode );
BOOL WaitForThreadEnd ( HANDLE *phThread, DWORD dwWaitTime =5000 );
BOOL WaitForThreadEnd ( HANDLE *pEvtTerminate, HANDLE *phThread, DWORD dwWaitTime =5000 );

#endif