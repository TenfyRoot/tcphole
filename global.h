/* Filename : global.h */
#ifndef GLOBAL_H
#define GLOBAL_H

#ifndef DLL_INTERNAL
#define DLL_INTERNAL __declspec( dllexport )
#endif

//==========================================================================
// 常用操作宏
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
// 开关标志是否“置位”了
#define SWITCH_IS_FLAG(nConstFlag,nValue) ( ( (nConstFlag) & (nValue) ) == (nConstFlag) )
#define LENGTH(x) sizeof(x)/sizeof(x[0])
#define MIN(x,y) (((DWORD)(x)<(DWORD)(y))?(x):(y))
#define MAX(x,y) (((DWORD)(x)>(DWORD)(y))?(x):(y))
// 句柄是否有效
#define HANDLE_IS_VALID(h) ( HANDLE(h) && HANDLE(h) != INVALID_HANDLE_VALUE )
// 关闭句柄
#define SAFE_CLOSE_HANDLE(h)\
{\
	if ( HANDLE_IS_VALID ( h ) )\
	{\
		CloseHandle ( h );\
		h = NULL;\
	}\
}

// 等待事件的 Sleep() 函数
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

// 删除一个数组指针
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

// 删除所有由 new 申请的内存空间，可以是对象，也可以是普通的数据类型，如int、char等
#define DELETE_HEAP(pp)\
{\
	if ( (pp) && (*(pp)) )\
	{\
		delete (*(pp));\
		(*(pp)) = NULL;\
	}\
}

// 删除所有由 new 申请的数组内存空间，可以是对象，也可以是普通的数据类型，如int、char等
#define DELETE_ARY_HEAP(pp)\
{\
	if ( (pp) && (*(pp)) )\
	{\
		delete[] (*(pp));\
		(*(pp)) = NULL;\
	}\
}

// 用 new 申请一个 基本类型（如：char、int等）或结构体的内存空间，但不能是对象
// 如指针指向有效的空间就将那个空间清零
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
// 注意：如果是从 CString 中查找时 Find 千万不要用 LPCTSTR 或者 char* 变量，一定是要用 CString 变量
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
// 从数组 Ary_Org 中查找，只要 Ary_Find 中任何一个元素在 Ary_Org 中出现过
// 就返回该元素在 Ary_Org 中的位置
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
// 将数组 Ary_Src 中的元素拷贝到 Ary_Dest 中
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
// 将数组 Ary_Src 中的元素拷贝到 Ary_Dest 中，并保证 Ary_Src 中的元素不会重复拷贝
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
// 将数组 Ary 中的元素次序颠倒，T2 是 Ary 里保存的数据类型
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
		《TCP实现P2P通信、TCP穿越NAT的方法、TCP打洞》
	这里假设公网上的服务器S，客户端A在NAT-A后面，客户端B在NAT-B后面，现在客户端A希望和客户端B建立直接的
TCP 连接，客户端A为主动端，客户端B为被动端，客户端B向客户端B进行TCP打洞，打洞成功后客户端A便可以直接与
客户端B建立TCP连接。
*/

// 服务器地址和端口号定义
#define SRV_TCP_MAIN_PORT		4000				// 服务器主连接的端口号
#define SRV_TCP_HOLE_PORT		8000				// 服务器响应客户端打洞申请的端口号
#define NET_BUFFER_SIZE			1024				// 缓冲大小

// 数据包类型
typedef enum _packet_type
{
	PACKET_TYPE_INVALID,
	PACKET_TYPE_NEW_USER_LOGIN,			// 服务器收到新的客户端登录，将登录信息发送给其他客户端
	PACKET_TYPE_WELCOME,				// 客户端登录时服务器发送该欢迎信息给客户端，以告知客户端登录成功
	PACKET_TYPE_REQUEST_CONN_CLIENT,	// 某客户端向服务器申请，要求与另一个客户端建立直接的TCP连接，即需要进行TCP打洞
	PACKET_TYPE_REQUEST_MAKE_HOLE,		// 服务器请求某客户端向另一客户端进行TCP打洞，即向另一客户端指定的外部IP和端口号进行connect尝试
	PACKET_TYPE_REQUEST_DISCONNECT,		// 请求服务器断开连接
	PACKET_TYPE_TCP_DIRECT_CONNECT,		// 服务器要求主动端（客户端A）直接连接被动端（客户端B）的外部IP和端口号
	PACKET_TYPE_HOLE_LISTEN_READY,		// 被动端（客户端B）打洞和侦听均已准备就绪

} PACKET_TYPE;

//
// 新用户登录数据
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
	PACKET_TYPE ePacketType;			// 包类型
	char szClientIP[32];				// 新登录的客户端（客户端B）外部IP地址
	UINT nClientPort;					// 新登录的客户端（客户端B）外部端口号
	DWORD dwID;							// 新登录的客户端（客户端B）的ID号（从1开始编号的一个唯一序号）
} t_NewUserLoginPkt;

//
// 欢迎信息
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
	PACKET_TYPE ePacketType;			// 包类型
	char szClientIP[32];				// 接收欢迎信息的客户端外部IP地址
	UINT nClientPort;					// 接收欢迎信息的客户端外部端口号
	DWORD dwID;							// 接收欢迎信息的客户端的ID号（从1开始编号的一个唯一序号）
	char szWelcomeInfo[64];				// 欢迎信息文本
} t_WelcomePkt;

//
// 客户端A请求服务器协助连接客户端B
//
typedef struct _req_conn_client
{
	_req_conn_client ()
		: ePacketType ( PACKET_TYPE_REQUEST_CONN_CLIENT )
		, dwInviterID ( 0 )
		, dwInvitedID ( 0 )
	{
	}
	PACKET_TYPE ePacketType;			// 包类型
	DWORD dwInviterID;					// 发出邀请方（主动方即客户端A）ID号
	DWORD dwInvitedID;					// 被邀请方（被动方即客户端B）ID号
} t_ReqConnClientPkt;

//
// 服务器请求客户端B打洞
//
typedef struct _srv_req_make_hole
{
	_srv_req_make_hole ()
		: ePacketType ( PACKET_TYPE_REQUEST_MAKE_HOLE )
		, dwInviterID ( 0 )
		, dwInvitedID ( 0 )
		, dwInviterHoleID ( 0 )
		, nClientHolePort ( 0 )
	{
		memset ( szClientHoleIP, 0, sizeof(szClientHoleIP) );
	}
	PACKET_TYPE ePacketType;			// 包类型
	DWORD dwInviterID;					// 发出邀请方（主动方即客户端A）ID号
	DWORD dwInviterHoleID;				// 发出邀请方（主动方即客户端A）打洞ID号
	DWORD dwInvitedID;					// 被邀请方（被动方即客户端B）ID号
	char szClientHoleIP[32];			// 可以向该IP（请求方的外部IP）地址打洞，即发生一次connect尝试
	UINT nClientHolePort;				// 可以向该端口号（请求方的外部端口号）打洞，即发生一次connect尝试
} t_SrvReqMakeHolePkt;

//
// 请求服务器断开连接
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
	PACKET_TYPE ePacketType;			// 包类型
	DWORD dwInviterID;					// 发出邀请方（主动方即客户端A）ID号
	DWORD dwInviterHoleID;				// 发出邀请方（主动方即客户端A）打洞ID号
	DWORD dwInvitedID;					// 被邀请方（被动方即客户端B）ID号
} t_ReqSrvDisconnectPkt;

//
// 服务器要求主动端（客户端A）直接连接被动端（客户端B）的外部IP和端口号
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
	PACKET_TYPE ePacketType;			// 包类型
	DWORD dwInvitedID;					// 被邀请方（被动方即客户端B）ID号
	char szInvitedIP[32];				// 可以与该IP（被邀请方客户端B的外部IP）地址直接建立TCP连接
	UINT nInvitedPort;					// 可以与该端口号（被邀请方客户端B的外部IP）地址直接建立TCP连接
} t_SrvReqDirectConnectPkt;

//
// 被动端（客户端B）打洞和侦听均已准备就绪
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
	PACKET_TYPE ePacketType;			// 包类型
	DWORD dwInviterID;					// 发出邀请方（主动方即客户端A）ID号
	DWORD dwInviterHoleID;				// 发出邀请方（主动方即客户端A）打洞ID号
	DWORD dwInvitedID;					// 被邀请方（被动方即客户端B）ID号
} t_HoleListenReadyPkt;

CString hwFormatMessage ( DWORD dwErrorCode );
BOOL WaitForThreadEnd ( HANDLE *phThread, DWORD dwWaitTime =5000 );
BOOL WaitForThreadEnd ( HANDLE *pEvtTerminate, HANDLE *phThread, DWORD dwWaitTime =5000 );

#endif