/*CAsyncSocketEx by Tim Kosse (Tim.Kosse@gmx.de)
            Version 1.3 (2003-04-26)
--------------------------------------------------------

Introduction:
-------------

CAsyncSocketEx is a replacement for the MFC class CAsyncSocket.
This class was written because CAsyncSocket is not the fastest WinSock
wrapper and it's very hard to add new functionality to CAsyncSocket
derived classes. This class offers the same functionality as CAsyncSocket.
Also, CAsyncSocketEx offers some enhancements which were not possible with
CAsyncSocket without some tricks.

How do I use it?
----------------
Basically exactly like CAsyncSocket.
To use CAsyncSocketEx, just replace all occurrences of CAsyncSocket in your
code with CAsyncSocketEx, if you did not enhance CAsyncSocket yourself in
any way, you won't have to change anything else in your code.

Why is CAsyncSocketEx faster?
-----------------------------

CAsyncSocketEx is slightly faster when dispatching notification event messages.
First have a look at the way CAsyncSocket works. For each thread that uses
CAsyncSocket, a window is created. CAsyncSocket calls WSAAsyncSelect with
the handle of that window. Until here, CAsyncSocketEx works the same way.
But CAsyncSocket uses only one window message (WM_SOCKET_NOTIFY) for all
sockets within one thread. When the window recieve WM_SOCKET_NOTIFY, wParam
contains the socket handle and the window looks up an CAsyncSocket instance
using a map. CAsyncSocketEx works differently. It's helper window uses a
wide range of different window messages (WM_USER through 0xBFFF) and passes
a different message to WSAAsyncSelect for each socket. When a message in
the specified range is received, CAsyncSocketEx looks up the pointer to a
CAsyncSocketEx instance in an Array using the index of message - WM_USER.
As you can see, CAsyncSocketEx uses the helper window in a more efficient
way, as it don't have to use the slow maps to lookup it's own instance.
Still, speed increase is not very much, but it may be noticeable when using
a lot of sockets at the same time.
Please note that the changes do not affect the raw data throughput rate,
CAsyncSocketEx only dispatches the notification messages faster.

What else does CAsyncSocketEx offer?
------------------------------------

CAsyncSocketEx offers a flexible layer system. One example is the proxy layer.
Just create an instance of the proxy layer, configure it and add it to the layer
chain of your CAsyncSocketEx instance. After that, you can connect through
proxies.
Benefit: You don't have to change much to use the layer system.
Another layer that is currently in development is the SSL layer to establish
SSL encrypted connections.

License
-------

Feel free to use this class, as long as you don't claim that you wrote it
and this copyright notice stays intact in the source files.
If you use this class in commercial applications, please send a short message
to tim.kosse@gmx.de
*/

#include "stdafx.h"
#include "AsyncSocketEx.h"
#include "wtypes.h"
#include "oleauto.h"
#include "atlconv.h"

#ifndef NOLAYERS
#include "AsyncSocketExLayer.h"
#endif //NOLAYERS

#ifdef MPEXT
#pragma warn -inl
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#endif

#ifndef CCRITICALSECTIONWRAPPERINCLUDED
class CCriticalSectionWrapper
{
public:
	CCriticalSectionWrapper()
	{
		m_bInitialized = TRUE;
		InitializeCriticalSection(&m_criticalSection);
	}

	~CCriticalSectionWrapper()
	{
		if (m_bInitialized)
			DeleteCriticalSection(&m_criticalSection);
		m_bInitialized = FALSE;
	}

	void Lock()
	{
		if (m_bInitialized)
			EnterCriticalSection(&m_criticalSection);
	}
	void Unlock()
	{
		if (m_bInitialized)
			LeaveCriticalSection(&m_criticalSection);
	}
protected:
	CRITICAL_SECTION m_criticalSection;
	BOOL m_bInitialized;
};
#define CCRITICALSECTIONWRAPPERINCLUDED
#endif

CCriticalSectionWrapper CAsyncSocketEx::m_sGlobalCriticalSection;
CAsyncSocketEx::t_AsyncSocketExThreadDataList *CAsyncSocketEx::m_spAsyncSocketExThreadDataList = 0;


#ifndef _AFX
#ifndef VERIFY
#define VERIFY(x) (void(x))
#endif //VERIFY
#ifndef ASSERT
#define ASSERT(x)
#endif //ASSERT
#endif //_AFX

/////////////////////////////
//Helper Window class
#define WM_SOCKETEX_NOTIFY (WM_USER+3)
#define MAX_SOCKETS (0xBFFF-WM_SOCKETEX_NOTIFY+1)

class CAsyncSocketExHelperWindow
{
public:
	CAsyncSocketExHelperWindow(CAsyncSocketEx::t_AsyncSocketExThreadData* pThreadData)
	{
		//Initialize data
		m_pAsyncSocketExWindowData = new t_AsyncSocketExWindowData[512]; //Reserve space for 512 active sockets
		memset(m_pAsyncSocketExWindowData, 0, 512*sizeof(t_AsyncSocketExWindowData));
		m_nWindowDataSize=512;
		m_nSocketCount=0;
		m_nWindowDataPos=0;
		m_pThreadData = pThreadData;

		//Create window
		WNDCLASSEX wndclass;
		wndclass.cbSize=sizeof wndclass;
		wndclass.style=0;
		wndclass.lpfnWndProc=WindowProc;
		wndclass.cbClsExtra=0;
		wndclass.cbWndExtra=0;
		wndclass.hInstance=GetModuleHandle(0);
		wndclass.hIcon=0;
		wndclass.hCursor=0;
		wndclass.hbrBackground=0;
		wndclass.lpszMenuName=0;
		wndclass.lpszClassName=_T("CAsyncSocketEx Helper Window");
		wndclass.hIconSm=0;

		#ifdef _DEBUG
		ATOM ClassAtom =
		#endif
		RegisterClassEx(&wndclass);

		m_hWnd=CreateWindow(_T("CAsyncSocketEx Helper Window"), _T("CAsyncSocketEx Helper Window"), 0, 0, 0, 0, 0, 0, 0, GetModuleHandle(0), 0);
		ASSERT(m_hWnd);
		SetWindowLongPtr(m_hWnd, GWL_USERDATA, (LONG)this);
	};

	virtual ~CAsyncSocketExHelperWindow()
	{
		//Clean up socket storage
		delete [] m_pAsyncSocketExWindowData;
		m_pAsyncSocketExWindowData=0;
		m_nWindowDataSize=0;
		m_nSocketCount=0;

		//Destroy window
		if (m_hWnd)
		{
			DestroyWindow(m_hWnd);
			m_hWnd=0;
		}
	}

	//Adds a socket to the list of attached sockets
	BOOL AddSocket(CAsyncSocketEx *pSocket, int &nSocketIndex)
	{
		ASSERT(pSocket);
		if (!m_nWindowDataSize)
		{
			ASSERT(!m_nSocketCount);
			m_nWindowDataSize=512;
			m_pAsyncSocketExWindowData=new t_AsyncSocketExWindowData[512]; //Reserve space for 512 active sockets
			memset(m_pAsyncSocketExWindowData, 0, 512*sizeof(t_AsyncSocketExWindowData));
		}

		if (nSocketIndex!=-1)
		{
			ASSERT(m_pAsyncSocketExWindowData);
			ASSERT(m_nWindowDataSize>nSocketIndex);
			ASSERT(m_pAsyncSocketExWindowData[nSocketIndex].m_pSocket==pSocket);
			ASSERT(m_nSocketCount);
			return TRUE;
		}

		//Increase socket storage if too small
		if (m_nSocketCount>=(m_nWindowDataSize-10))
		{
			int nOldWindowDataSize=m_nWindowDataSize;
			ASSERT(m_nWindowDataSize<MAX_SOCKETS);
			m_nWindowDataSize+=512;
			if (m_nWindowDataSize>MAX_SOCKETS)
				m_nWindowDataSize=MAX_SOCKETS;
			t_AsyncSocketExWindowData *tmp=m_pAsyncSocketExWindowData;
			m_pAsyncSocketExWindowData = new t_AsyncSocketExWindowData[m_nWindowDataSize];
			memcpy(m_pAsyncSocketExWindowData, tmp, nOldWindowDataSize * sizeof(t_AsyncSocketExWindowData));
			memset(m_pAsyncSocketExWindowData+nOldWindowDataSize, 0, (m_nWindowDataSize-nOldWindowDataSize)*sizeof(t_AsyncSocketExWindowData));
			delete [] tmp;
		}

		//Search for free slot
		for (int i=m_nWindowDataPos;i<(m_nWindowDataSize+m_nWindowDataPos);i++)
		{
			if (!m_pAsyncSocketExWindowData[i%m_nWindowDataSize].m_pSocket)
			{
				m_pAsyncSocketExWindowData[i%m_nWindowDataSize].m_pSocket=pSocket;
				nSocketIndex=i%m_nWindowDataSize;
				m_nWindowDataPos=(i+1)%m_nWindowDataSize;
				m_nSocketCount++;
				return TRUE;
			}
		}

		//No slot found, maybe there are too much sockets!
		return FALSE;
	}

	//Removes a socket from the socket storage
	BOOL RemoveSocket(CAsyncSocketEx *pSocket, int &nSocketIndex)
	{
		ASSERT(pSocket);
		if (nSocketIndex==-1)
			return TRUE;

		// Remove additional messages from queue
		MSG msg;
		while (PeekMessage(&msg, m_hWnd, WM_SOCKETEX_NOTIFY + nSocketIndex, WM_SOCKETEX_NOTIFY + nSocketIndex, PM_REMOVE));

		ASSERT(m_pAsyncSocketExWindowData);
		ASSERT(m_nWindowDataSize>0);
		ASSERT(m_nSocketCount>0);
		ASSERT(m_pAsyncSocketExWindowData[nSocketIndex].m_pSocket==pSocket);
		m_pAsyncSocketExWindowData[nSocketIndex].m_pSocket=0;
		nSocketIndex=-1;
		m_nSocketCount--;

		return TRUE;
	}

	void RemoveLayers(CAsyncSocketEx *pOrigSocket)
	{
		// Remove all layer messages from old socket
		std::list<MSG> msgList;
		MSG msg;
		while (PeekMessage(&msg, m_hWnd, WM_USER, WM_USER, PM_REMOVE))
		{
			//Verify parameters, lookup socket and notification message
			//Verify parameters
			if (msg.wParam >= static_cast<UINT>(m_nWindowDataSize)) //Index is within socket storage
				continue;

			CAsyncSocketEx *pSocket = m_pAsyncSocketExWindowData[msg.wParam].m_pSocket;
			CAsyncSocketExLayer::t_LayerNotifyMsg *pMsg=(CAsyncSocketExLayer::t_LayerNotifyMsg *)msg.lParam;
			if (!pMsg || !pSocket || pSocket == pOrigSocket || pSocket->m_SocketData.hSocket != pMsg->hSocket)
			{
				delete pMsg;
				continue;
			}

			msgList.push_back(msg);
		}

		for (std::list<MSG>::iterator iter = msgList.begin(); iter != msgList.end(); iter++)
			PostMessage(m_hWnd, iter->message, iter->wParam, iter->lParam);
	}

	//Processes event notifications sent by the sockets or the layers
	static LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		//Verify parameters
		ASSERT(hWnd);
		if (!hWnd)
			return 0;
		CAsyncSocketExHelperWindow *pWnd=(CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWL_USERDATA);
		if (message>=WM_SOCKETEX_NOTIFY)
		{
			ASSERT(pWnd);
			
			if (message<static_cast<UINT>(WM_SOCKETEX_NOTIFY+pWnd->m_nWindowDataSize)) //Index is within socket storage
			{
				//Lookup socket and verify if it's valid
				CAsyncSocketEx *pSocket=pWnd->m_pAsyncSocketExWindowData[message-WM_SOCKETEX_NOTIFY].m_pSocket;
				SOCKET hSocket=wParam;
				if (!pSocket)
					return 0;
				if (hSocket==INVALID_SOCKET)
					return 0;
				if (pSocket->m_SocketData.hSocket != hSocket)
					return 0;
				
				int nEvent=lParam&0xFFFF;
				int nErrorCode=lParam>>16;

				//Dispatch notification
#ifndef NOLAYERS
				if (!pSocket->m_pFirstLayer)
				{
#endif //NOLAYERS
					//Dispatch to CAsyncSocketEx instance
					switch (nEvent)
					{
					case FD_READ:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() == connecting && !nErrorCode)
						{
							pSocket->m_nPendingEvents |= FD_READ;
							break;
						}
						else if (pSocket->GetState() == attached)
							pSocket->SetState(connected);
						if (pSocket->GetState() != connected)
							break;

						// Ignore further FD_READ events after FD_CLOSE has been received
						if (pSocket->m_SocketData.onCloseCalled)
							break;
#endif //NOSOCKETSTATES
						
						if (pSocket->m_lEvent & FD_READ)
						{
							DWORD nBytes = 0;
							if (!nErrorCode)
								if (!pSocket->IOCtl(FIONREAD, &nBytes))
									nErrorCode = WSAGetLastError();
#ifndef NOSOCKETSTATES
							if (nErrorCode)
								pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
							if (nBytes != 0 || nErrorCode != 0)
								pSocket->OnReceive(nErrorCode);
						}
						break;
					case FD_FORCEREAD: //Forceread does not check if there's data waiting
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() == connecting && !nErrorCode)
						{
							pSocket->m_nPendingEvents |= FD_FORCEREAD;
							break;
						}
						else if (pSocket->GetState() == attached)
							pSocket->SetState(connected);
						if (pSocket->GetState() != connected)
							break;
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_READ)
						{
#ifndef NOSOCKETSTATES
							if (nErrorCode)
								pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
							pSocket->OnReceive(nErrorCode);
						}
						break;
					case FD_WRITE:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() == connecting && !nErrorCode)
						{
							pSocket->m_nPendingEvents |= FD_WRITE;
							break;
						}
						else if (pSocket->GetState() == attached && !nErrorCode)
							pSocket->SetState(connected);
						if (pSocket->GetState() != connected)
							break;
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_WRITE)
						{
#ifndef NOSOCKETSTATES
							if (nErrorCode)
								pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
							pSocket->OnSend(nErrorCode);
						}
						break;
					case FD_CONNECT:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() == connecting)
						{
							if (nErrorCode && pSocket->m_SocketData.nextAddr)
							{
								if (pSocket->TryNextProtocol())
                                    break;
							}
							pSocket->SetState(connected);
						}
						else if (pSocket->GetState() == attached && !nErrorCode)
							pSocket->SetState(connected);
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_CONNECT)
							pSocket->OnConnect(nErrorCode);
#ifndef NOSOCKETSTATES
						if (!nErrorCode)
						{
							if ((pSocket->m_nPendingEvents&FD_READ) && pSocket->GetState() == connected)
								pSocket->OnReceive(0);
							if ((pSocket->m_nPendingEvents&FD_FORCEREAD) && pSocket->GetState() == connected)
								pSocket->OnReceive(0);
							if ((pSocket->m_nPendingEvents&FD_WRITE) && pSocket->GetState() == connected)
								pSocket->OnSend(0);
						}
						pSocket->m_nPendingEvents = 0;
#endif
						break;
					case FD_ACCEPT:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() != listening && pSocket->GetState() != attached)
							break;
#endif //NOSOCKETSTATES
						if (pSocket->m_lEvent & FD_ACCEPT)
							pSocket->OnAccept(nErrorCode);
						break;
					case FD_CLOSE:
#ifndef NOSOCKETSTATES
						if (pSocket->GetState() != connected && pSocket->GetState() != attached)
							break;

						// If there are still bytes left to read, call OnReceive instead of 
						// OnClose and trigger a new OnClose
						DWORD nBytes = 0;
						if (!nErrorCode && pSocket->IOCtl(FIONREAD, &nBytes))
						{
							if (nBytes > 0)
							{
								// Just repeat message.
								PostMessage(hWnd, message, wParam, lParam);
								pSocket->m_SocketData.onCloseCalled = true;								
								pSocket->OnReceive(WSAESHUTDOWN);
								break;
							}
						}

						pSocket->SetState(nErrorCode?aborted:closed);
#endif //NOSOCKETSTATES
						pSocket->OnClose(nErrorCode);
						break;
					}
				}
#ifndef NOLAYERS
				else //Dispatch notification to the lowest layer
				{
					if (nEvent == FD_READ)
					{
						// Ignore further FD_READ events after FD_CLOSE has been received
						if (pSocket->m_SocketData.onCloseCalled)
							return 0;

						DWORD nBytes;
						if (!pSocket->IOCtl(FIONREAD, &nBytes))
							nErrorCode = WSAGetLastError();
						if (nBytes != 0 || nErrorCode != 0)
							pSocket->m_pLastLayer->CallEvent(nEvent, nErrorCode);
					}
					else if (nEvent == FD_CLOSE)
					{
						// If there are still bytes left to read, call OnReceive instead of 
						// OnClose and trigger a new OnClose
						DWORD nBytes = 0;
						if (!nErrorCode && pSocket->IOCtl(FIONREAD, &nBytes))
						{
							if (nBytes > 0)
							{
								// Just repeat message.
								pSocket->ResendCloseNotify();
								pSocket->m_pLastLayer->CallEvent(FD_READ, 0);
								return 0;
							}
						}
						pSocket->m_SocketData.onCloseCalled = true;
						pSocket->m_pLastLayer->CallEvent(nEvent, nErrorCode);
					}
					else
					{
						pSocket->m_pLastLayer->CallEvent(nEvent, nErrorCode);
					}
				}
			}
#endif //NOLAYERS
			return 0;
		}
#ifndef NOLAYERS
		else if (message == WM_USER) //Notification event sent by a layer
		{
			ASSERT(pWnd);
			
			if (wParam >= static_cast<UINT>(pWnd->m_nWindowDataSize)) //Index is within socket storage
			{
				return 0;
			}
			
			CAsyncSocketEx *pSocket=pWnd->m_pAsyncSocketExWindowData[wParam].m_pSocket;
			CAsyncSocketExLayer::t_LayerNotifyMsg *pMsg=(CAsyncSocketExLayer::t_LayerNotifyMsg *)lParam;
			if (!pMsg || !pSocket || pSocket->m_SocketData.hSocket != pMsg->hSocket)
			{
				delete pMsg;
				return 0;
			}
			int nEvent=pMsg->lEvent&0xFFFF;
			int nErrorCode=pMsg->lEvent>>16;
			
			//Dispatch to layer
			if (pMsg->pLayer)
			{
				pMsg->pLayer->CallEvent(nEvent, nErrorCode);
			}
			else
			{
				//Dispatch to CAsyncSocketEx instance
				switch (nEvent)
				{
				case FD_READ:
#ifndef NOSOCKETSTATES
					if (pSocket->GetState() == connecting && !nErrorCode)
					{
						pSocket->m_nPendingEvents |= FD_READ;
						break;
					}
					else if (pSocket->GetState() == attached && !nErrorCode)
						pSocket->SetState(connected);
					if (pSocket->GetState() != connected)
						break;
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_READ)
					{
#ifndef NOSOCKETSTATES
						if (nErrorCode)
							pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
						pSocket->OnReceive(nErrorCode);
					}
					break;
				case FD_FORCEREAD: //Forceread does not check if there's data waiting
#ifndef NOSOCKETSTATES
					if (pSocket->GetState() == connecting && !nErrorCode)
					{
						pSocket->m_nPendingEvents |= FD_FORCEREAD;
						break;
					}
					else if (pSocket->GetState() == attached && !nErrorCode)
						pSocket->SetState(connected);
					if (pSocket->GetState() != connected)
						break;
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_READ)
					{
#ifndef NOSOCKETSTATES
						if (nErrorCode)
							pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
						pSocket->OnReceive(nErrorCode);
					}
					break;
				case FD_WRITE:
#ifndef NOSOCKETSTATES
					if (pSocket->GetState() == connecting && !nErrorCode)
					{
						pSocket->m_nPendingEvents |= FD_WRITE;
						break;
					}
					else if (pSocket->GetState() == attached && !nErrorCode)
						pSocket->SetState(connected);
					if (pSocket->GetState() != connected)
						break;
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_WRITE)
					{
#ifndef NOSOCKETSTATES
						if (nErrorCode)
							pSocket->SetState(aborted);
#endif //NOSOCKETSTATES
						pSocket->OnSend(nErrorCode);
					}
					break;
				case FD_CONNECT:
#ifndef NOSOCKETSTATES
					if (pSocket->GetState() == connecting)
						pSocket->SetState(connected);
					else if (pSocket->GetState() == attached && !nErrorCode)
						pSocket->SetState(connected);
#endif //NOSOCKETSTATES
					if (pSocket->m_lEvent & FD_CONNECT)
						pSocket->OnConnect(nErrorCode);
#ifndef NOSOCKETSTATES
					if (!nErrorCode)
					{
						if (((pSocket->m_nPendingEvents&FD_READ) && pSocket->GetState() == connected) && (pSocket->m_lEvent & FD_READ))
							pSocket->OnReceive(0);
						if (((pSocket->m_nPendingEvents&FD_FORCEREAD) && pSocket->GetState() == connected) && (pSocket->m_lEvent & FD_READ))
							pSocket->OnReceive(0);
						if (((pSocket->m_nPendingEvents&FD_WRITE) && pSocket->GetState() == connected) && (pSocket->m_lEvent & FD_WRITE))
							pSocket->OnSend(0);
					}
					pSocket->m_nPendingEvents = 0;
#endif //NOSOCKETSTATES
					break;
				case FD_ACCEPT:
#ifndef NOSOCKETSTATES
					if ((pSocket->GetState() == listening || pSocket->GetState() == attached) && (pSocket->m_lEvent & FD_ACCEPT))
#endif //NOSOCKETSTATES
					{
						pSocket->OnAccept(nErrorCode);
					}
					break;
				case FD_CLOSE:
#ifndef NOSOCKETSTATES
					if ((pSocket->GetState() == connected || pSocket->GetState() == attached) && (pSocket->m_lEvent & FD_CLOSE))
					{
						pSocket->SetState(nErrorCode?aborted:closed);
#else
					{
#endif //NOSOCKETSTATES
						pSocket->OnClose(nErrorCode);
					}
					break;
				}
			}
			delete pMsg;
			return 0;
		}
#endif //NOLAYERS
		else if (message == WM_USER+1)
		{
			ASSERT(pWnd);
			// WSAAsyncGetHostByName reply

			// Verify parameters
			ASSERT(hWnd);
			CAsyncSocketExHelperWindow *pWnd = (CAsyncSocketExHelperWindow *)GetWindowLongPtr(hWnd, GWL_USERDATA);
			ASSERT(pWnd);

			CAsyncSocketEx *pSocket = NULL;
			for (int i = 0; i < pWnd->m_nWindowDataSize; i++)
			{
				pSocket = pWnd->m_pAsyncSocketExWindowData[i].m_pSocket;
				if (pSocket && pSocket->m_hAsyncGetHostByNameHandle &&
					pSocket->m_hAsyncGetHostByNameHandle == (HANDLE)wParam)
					break;
			}
			if (!pSocket)
				return 0;

			int nErrorCode = lParam >> 16;
			if (nErrorCode)
			{
				pSocket->OnConnect(nErrorCode);
				return 0;
			}

			SOCKADDR_IN sockAddr;
			memset(&sockAddr,0,sizeof(sockAddr));
			sockAddr.sin_family=AF_INET;
			sockAddr.sin_addr.s_addr = ((LPIN_ADDR)((LPHOSTENT)pSocket->m_pAsyncGetHostByNameBuffer)->h_addr)->s_addr;

			sockAddr.sin_port = htons(pSocket->m_nAsyncGetHostByNamePort);

			BOOL res = pSocket->Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
			delete [] pSocket->m_pAsyncGetHostByNameBuffer;
			pSocket->m_pAsyncGetHostByNameBuffer=0;
			pSocket->m_hAsyncGetHostByNameHandle=0;

			if (!res)
				if (GetLastError()!=WSAEWOULDBLOCK)
					pSocket->OnConnect(GetLastError());
			return 0;
		}
		else if (message == WM_USER + 2)
		{
			ASSERT(pWnd);
			if (wParam >= static_cast<UINT>(pWnd->m_nWindowDataSize)) //Index is within socket storage
				return 0;
			
			CAsyncSocketEx *pSocket = pWnd->m_pAsyncSocketExWindowData[wParam].m_pSocket;
			if (!pSocket)
				return 0;
			
			// Process pending callbacks
			std::list<t_callbackMsg> tmp;
			tmp.swap(pSocket->m_pendingCallbacks);
			pSocket->OnLayerCallback(tmp);
		}
		else if (message == WM_TIMER)
		{
			ASSERT(pWnd);
			if (wParam != 1)
				return 0;

			if (pWnd->m_pThreadData->layerCloseNotify.empty())
			{
				KillTimer(hWnd, 1);
				return 0;
			}
			CAsyncSocketEx* socket = pWnd->m_pThreadData->layerCloseNotify.front();
			pWnd->m_pThreadData->layerCloseNotify.pop_front();
			if (pWnd->m_pThreadData->layerCloseNotify.empty())
				KillTimer(hWnd, 1);

			PostMessage(hWnd, socket->m_SocketData.nSocketIndex + WM_SOCKETEX_NOTIFY, socket->m_SocketData.hSocket, FD_CLOSE);
			return 0;
		}
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	HWND GetHwnd()
	{
		return m_hWnd;
	}

private:
	HWND m_hWnd;
	struct t_AsyncSocketExWindowData
	{
		CAsyncSocketEx *m_pSocket;
	} *m_pAsyncSocketExWindowData;
	int m_nWindowDataSize;
	int m_nWindowDataPos;
	int m_nSocketCount;
	CAsyncSocketEx::t_AsyncSocketExThreadData* m_pThreadData;
};

//////////////////////////////////////////////////////////////////////
// Konstruktion/Destruktion
//////////////////////////////////////////////////////////////////////

CAsyncSocketEx::CAsyncSocketEx()
{
	m_SocketData.hSocket = INVALID_SOCKET;
	m_SocketData.nSocketIndex = -1;
	m_SocketData.nFamily = AF_UNSPEC;
	m_SocketData.onCloseCalled = false;
	m_pLocalAsyncSocketExThreadData = 0;

#ifndef NOSOCKETSTATES
	m_nPendingEvents = 0;
	m_nState = notsock;
#endif //NOSOCKETSTATES

#ifndef NOLAYERS
	m_pFirstLayer = 0;
	m_pLastLayer = 0;
#endif //NOLAYERS
	m_lEvent = 0;
	m_pAsyncGetHostByNameBuffer = NULL;
	m_hAsyncGetHostByNameHandle = NULL;
	m_nAsyncGetHostByNamePort = 0;

	m_nSocketPort = 0;
	m_lpszSocketAddress = 0;

	m_SocketData.addrInfo = 0;
	m_SocketData.nextAddr = 0;
}

CAsyncSocketEx::~CAsyncSocketEx()
{
	Close();
	FreeAsyncSocketExInstance();
}

BOOL CAsyncSocketEx::Create(UINT nSocketPort /*=0*/, int nSocketType /*=SOCK_STREAM*/, long lEvent /*=FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/, LPCTSTR lpszSocketAddress /*=NULL*/, int nFamily /*=AF_INET*/)
{
	ASSERT(GetSocketHandle() == INVALID_SOCKET);

	//Close the socket, although this should not happen
	if (GetSocketHandle() != INVALID_SOCKET)
	{
		WSASetLastError(WSAEALREADY);
		return FALSE;
	}

	BOOL res = InitAsyncSocketExInstance();
	ASSERT(res);
	if (!res)
	{
		WSASetLastError(WSANOTINITIALISED);
		return FALSE;
	}

	m_SocketData.nFamily = nFamily;

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		BOOL res = m_pFirstLayer->Create(nSocketPort, nSocketType, lEvent, lpszSocketAddress, nFamily);
#ifndef NOSOCKETSTATES
		if (res)
			SetState(unconnected);
#endif //NOSOCKETSTATES
		return res;
	}
	else
#endif //NOLAYERS
	{
		if (m_SocketData.nFamily == AF_UNSPEC)
		{
#ifndef NOSOCKETSTATES
			SetState(unconnected);
#endif //NOSOCKETSTATES
			m_lEvent = lEvent;

			m_nSocketPort = nSocketPort;
		
			delete [] m_lpszSocketAddress;
			if (lpszSocketAddress && *lpszSocketAddress)
			{
				m_lpszSocketAddress = new TCHAR[_tcslen(lpszSocketAddress) + 1];
				_tcscpy(m_lpszSocketAddress, lpszSocketAddress);
			}
			else
				m_lpszSocketAddress = 0;

			return TRUE;
		}
		else
		{
			SOCKET hSocket = socket(m_SocketData.nFamily, nSocketType, 0);
			if (hSocket == INVALID_SOCKET)
				return FALSE;
			m_SocketData.hSocket = hSocket;
			AttachHandle(hSocket);
		
#ifndef NOLAYERS
			if (m_pFirstLayer)
			{
				m_lEvent = lEvent;
				if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE) )
				{
					Close();
					return FALSE;
				}
			}
			else
#endif //NOLAYERS
			{
				if (!AsyncSelect(lEvent))
				{
					Close();
					return FALSE;
				}
			}

			if (!Bind(nSocketPort, lpszSocketAddress))
			{
				Close();
				return FALSE;
			}

#ifndef NOSOCKETSTATES
			SetState(unconnected);
#endif //NOSOCKETSTATES
		
			return TRUE;
		}
	}
}

void CAsyncSocketEx::OnReceive(int nErrorCode)
{
}

void CAsyncSocketEx::OnSend(int nErrorCode)
{
}

void CAsyncSocketEx::OnConnect(int nErrorCode)
{
}

void CAsyncSocketEx::OnAccept(int nErrorCode)
{
}

void CAsyncSocketEx::OnClose(int nErrorCode)
{
}

BOOL CAsyncSocketEx::Bind(UINT nSocketPort, LPCTSTR lpszSocketAddress)
{
	delete [] m_lpszSocketAddress;
	if (lpszSocketAddress && *lpszSocketAddress)
	{
		m_lpszSocketAddress = new TCHAR[_tcslen(lpszSocketAddress) + 1];
		_tcscpy(m_lpszSocketAddress, lpszSocketAddress);
	}
	else
		m_lpszSocketAddress = 0;
	m_nSocketPort = nSocketPort;

	if (m_SocketData.nFamily == AF_UNSPEC)
		return TRUE;

	USES_CONVERSION;
	
	LPSTR lpszAscii = (lpszSocketAddress && *lpszSocketAddress) ? T2A((LPTSTR)lpszSocketAddress) : 0;
	
	if ((m_SocketData.nFamily == AF_INET6 || m_SocketData.nFamily == AF_INET) && lpszAscii)
	{
		addrinfo hints, *res0, *res;
		int error;
		char port[10];
		BOOL ret = FALSE;

		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = m_SocketData.nFamily;
		hints.ai_socktype = SOCK_STREAM;
		_snprintf(port, 9, "%lu", nSocketPort);
		error = getaddrinfo(lpszAscii, port, &hints, &res0);
		if (error)
			return FALSE;

		for (res = res0; res; res = res->ai_next)
			if (Bind(res->ai_addr, res->ai_addrlen))
			{
				ret = TRUE;
				break;
			}
			else
				continue ;

			freeaddrinfo(res0);

			return ret ;
	}
	else if (!lpszAscii && m_SocketData.nFamily == AF_INET6)
	{
		SOCKADDR_IN6 sockAddr6;

		memset(&sockAddr6, 0, sizeof(sockAddr6));
		sockAddr6.sin6_family = AF_INET6 ;
		sockAddr6.sin6_addr = in6addr_any ;
		sockAddr6.sin6_port = htons((u_short)nSocketPort);

		return Bind((SOCKADDR*)&sockAddr6, sizeof(sockAddr6));
	}
	else if (!lpszAscii && m_SocketData.nFamily == AF_INET)
	{
		SOCKADDR_IN sockAddr;

		memset(&sockAddr, 0, sizeof(sockAddr));
		sockAddr.sin_family = AF_INET ;
		sockAddr.sin_addr.s_addr = INADDR_ANY ;
		sockAddr.sin_port = htons((u_short)nSocketPort);

		return Bind((SOCKADDR*)&sockAddr, sizeof(sockAddr));
	}
	else
		return FALSE ;
}

BOOL CAsyncSocketEx::Bind(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	if (!bind(m_SocketData.hSocket, lpSockAddr, nSockAddrLen))
		return TRUE;
	else
		return FALSE;
}

void CAsyncSocketEx::AttachHandle(SOCKET hSocket)
{
	ASSERT(m_pLocalAsyncSocketExThreadData);
	VERIFY(m_pLocalAsyncSocketExThreadData->m_pHelperWindow->AddSocket(this, m_SocketData.nSocketIndex));
#ifndef NOSOCKETSTATES
	SetState(attached);
#endif //NOSOCKETSTATES
}

void CAsyncSocketEx::DetachHandle(SOCKET hSocket)
{
	ASSERT(m_pLocalAsyncSocketExThreadData);
	if (!m_pLocalAsyncSocketExThreadData)
		return;
	ASSERT(m_pLocalAsyncSocketExThreadData->m_pHelperWindow);
	if (!m_pLocalAsyncSocketExThreadData->m_pHelperWindow)
		return;
	VERIFY(m_pLocalAsyncSocketExThreadData->m_pHelperWindow->RemoveSocket(this, m_SocketData.nSocketIndex));
#ifndef NOSOCKETSTATES
	SetState(notsock);
#endif //NOSOCKETSTATES
}

void CAsyncSocketEx::Close()
{
#ifndef NOSOCKETSTATES
	m_nPendingEvents = 0;
#endif //NOSOCKETSTATES
#ifndef NOLAYERS
	if (m_pFirstLayer)
		m_pFirstLayer->Close();
#endif //NOLAYERS
	if (m_SocketData.hSocket != INVALID_SOCKET)
	{
		VERIFY((closesocket(m_SocketData.hSocket) != SOCKET_ERROR));
		DetachHandle(m_SocketData.hSocket);
		m_SocketData.hSocket = INVALID_SOCKET;
	}
	if (m_SocketData.addrInfo)
	{
		freeaddrinfo(m_SocketData.addrInfo);
		m_SocketData.addrInfo = 0;
		m_SocketData.nextAddr = 0;
	}
	m_SocketData.nFamily = AF_UNSPEC;
	delete [] m_lpszSocketAddress;
	m_lpszSocketAddress = 0;
	m_nSocketPort = 0;
#ifndef NOLAYERS
	RemoveAllLayers();
#endif //NOLAYERS
	delete [] m_pAsyncGetHostByNameBuffer;
	m_pAsyncGetHostByNameBuffer = NULL;
	if (m_hAsyncGetHostByNameHandle)
		WSACancelAsyncRequest(m_hAsyncGetHostByNameHandle);
	m_hAsyncGetHostByNameHandle = NULL;
	m_SocketData.onCloseCalled = false;
}

BOOL CAsyncSocketEx::InitAsyncSocketExInstance()
{
	//Check if already initialized
	if (m_pLocalAsyncSocketExThreadData)
		return TRUE;

	DWORD id=GetCurrentThreadId();

	m_sGlobalCriticalSection.Lock();

	//Get thread specific data
	if (m_spAsyncSocketExThreadDataList)
	{
		t_AsyncSocketExThreadDataList *pList=m_spAsyncSocketExThreadDataList;
		while (pList)
		{
			ASSERT(pList->pThreadData);
			ASSERT(pList->pThreadData->nInstanceCount>0);

			if (pList->pThreadData->nThreadId==id)
			{
				m_pLocalAsyncSocketExThreadData=pList->pThreadData;
				m_pLocalAsyncSocketExThreadData->nInstanceCount++;
				break;
			}
			pList=pList->pNext;
		}
		//Current thread yet has no sockets
		if (!pList)
		{
			//Initialize data for current thread
			pList=new t_AsyncSocketExThreadDataList;
			pList->pNext=m_spAsyncSocketExThreadDataList;
			m_spAsyncSocketExThreadDataList=pList;
			m_pLocalAsyncSocketExThreadData=new t_AsyncSocketExThreadData;
			m_pLocalAsyncSocketExThreadData->nInstanceCount=1;
			m_pLocalAsyncSocketExThreadData->nThreadId=id;
			m_pLocalAsyncSocketExThreadData->m_pHelperWindow=new CAsyncSocketExHelperWindow(m_pLocalAsyncSocketExThreadData);
			m_spAsyncSocketExThreadDataList->pThreadData=m_pLocalAsyncSocketExThreadData;
		}
	}
	else
	{	//No thread has instances of CAsyncSocketEx; Initialize data
		m_spAsyncSocketExThreadDataList=new t_AsyncSocketExThreadDataList;
		m_spAsyncSocketExThreadDataList->pNext=0;
		m_pLocalAsyncSocketExThreadData=new t_AsyncSocketExThreadData;
		m_pLocalAsyncSocketExThreadData->nInstanceCount=1;
		m_pLocalAsyncSocketExThreadData->nThreadId=id;
		m_pLocalAsyncSocketExThreadData->m_pHelperWindow=new CAsyncSocketExHelperWindow(m_pLocalAsyncSocketExThreadData);
		m_spAsyncSocketExThreadDataList->pThreadData=m_pLocalAsyncSocketExThreadData;
	}
	m_sGlobalCriticalSection.Unlock();
	return TRUE;
}

void CAsyncSocketEx::FreeAsyncSocketExInstance()
{
	//Check if already freed
	if (!m_pLocalAsyncSocketExThreadData)
		return;

	for (std::list<CAsyncSocketEx*>::iterator iter = m_pLocalAsyncSocketExThreadData->layerCloseNotify.begin(); iter != m_pLocalAsyncSocketExThreadData->layerCloseNotify.end(); iter++)
	{
		if (*iter != this)
			continue;

		m_pLocalAsyncSocketExThreadData->layerCloseNotify.erase(iter);
		if (m_pLocalAsyncSocketExThreadData->layerCloseNotify.empty())
			KillTimer(m_pLocalAsyncSocketExThreadData->m_pHelperWindow->GetHwnd(), 1);
		break;
	}

	DWORD id=m_pLocalAsyncSocketExThreadData->nThreadId;
	m_sGlobalCriticalSection.Lock();

	ASSERT(m_spAsyncSocketExThreadDataList);
	t_AsyncSocketExThreadDataList *pList=m_spAsyncSocketExThreadDataList;
	t_AsyncSocketExThreadDataList *pPrev=0;

	//Serach for data for current thread and decrease instance count
	while (pList)
	{
		ASSERT(pList->pThreadData);
		ASSERT(pList->pThreadData->nInstanceCount>0);

		if (pList->pThreadData->nThreadId==id)
		{
			ASSERT(m_pLocalAsyncSocketExThreadData==pList->pThreadData);
			m_pLocalAsyncSocketExThreadData->nInstanceCount--;

			//Freeing last instance?
			//If so, destroy helper window
			if (!m_pLocalAsyncSocketExThreadData->nInstanceCount)
			{
				delete m_pLocalAsyncSocketExThreadData->m_pHelperWindow;
				delete m_pLocalAsyncSocketExThreadData;
				if (pPrev)
					pPrev->pNext=pList->pNext;
				else
					m_spAsyncSocketExThreadDataList=pList->pNext;
				delete pList;
				break;
			}

			break;
		}
		pPrev=pList;
		pList=pList->pNext;
		ASSERT(pList);
	}

	m_sGlobalCriticalSection.Unlock();
}

int CAsyncSocketEx::Receive(void* lpBuf, int nBufLen, int nFlags /*=0*/)
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->Receive(lpBuf, nBufLen, nFlags);
	else
#endif //NOLAYERS
		return recv(m_SocketData.hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}


int CAsyncSocketEx::Send(const void* lpBuf, int nBufLen, int nFlags /*=0*/)
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->Send(lpBuf, nBufLen, nFlags);
	else
#endif //NOLAYERS
		return send(m_SocketData.hSocket, (LPSTR)lpBuf, nBufLen, nFlags);
}

BOOL CAsyncSocketEx::Connect(LPCTSTR lpszHostAddress, UINT nHostPort)
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		BOOL res = m_pFirstLayer->Connect(lpszHostAddress, nHostPort);
#ifndef NOSOCKETSTATES
		if (res || GetLastError()==WSAEWOULDBLOCK)
			SetState(connecting);
#endif //NOSOCKETSTATES
		return res;
	} else
#endif //NOLAYERS
	if (m_SocketData.nFamily == AF_INET)
	{
		USES_CONVERSION;

		ASSERT(lpszHostAddress != NULL);

		SOCKADDR_IN sockAddr;
		memset(&sockAddr,0,sizeof(sockAddr));

		LPSTR lpszAscii = T2A((LPTSTR)lpszHostAddress);
		sockAddr.sin_family = AF_INET;
		sockAddr.sin_addr.s_addr = inet_addr(lpszAscii);

		if (sockAddr.sin_addr.s_addr == INADDR_NONE)
		{
			if (m_pAsyncGetHostByNameBuffer)
				delete [] m_pAsyncGetHostByNameBuffer;
			m_pAsyncGetHostByNameBuffer=new char[MAXGETHOSTSTRUCT];

			m_nAsyncGetHostByNamePort=nHostPort;

			m_hAsyncGetHostByNameHandle=WSAAsyncGetHostByName(GetHelperWindowHandle(), WM_USER+1, lpszAscii, m_pAsyncGetHostByNameBuffer, MAXGETHOSTSTRUCT);
			if (!m_hAsyncGetHostByNameHandle)
				return FALSE;

			WSASetLastError(WSAEWOULDBLOCK);
#ifndef NOSOCKETSTATES
			SetState(connecting);
#endif //NOSOCKETSTATES
			return FALSE;
		}

		sockAddr.sin_port = htons((u_short)nHostPort);

		return CAsyncSocketEx::Connect((SOCKADDR*)&sockAddr, sizeof(sockAddr));
	}
	else
	{
		USES_CONVERSION;

		ASSERT( lpszHostAddress != NULL );

		if (m_SocketData.addrInfo)
		{
			freeaddrinfo(m_SocketData.addrInfo);
			m_SocketData.addrInfo = 0;
			m_SocketData.nextAddr = 0;
		}

		addrinfo hints;
		int error;
		BOOL ret;
		char port[10];

		memset(&hints, 0, sizeof(addrinfo));
		hints.ai_family = m_SocketData.nFamily;
		hints.ai_socktype = SOCK_STREAM;
		_snprintf(port, 9, "%lu", nHostPort);
		error = getaddrinfo(T2CA(lpszHostAddress), port, &hints, &m_SocketData.addrInfo);
		if (error)
			return FALSE;

		for (m_SocketData.nextAddr = m_SocketData.addrInfo; m_SocketData.nextAddr; m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next)
		{
			bool newSocket = false;
			if (m_SocketData.nFamily == AF_UNSPEC)
			{
				newSocket = true;
				m_SocketData.hSocket = socket(m_SocketData.nextAddr->ai_family, m_SocketData.nextAddr->ai_socktype, m_SocketData.nextAddr->ai_protocol);

				if (m_SocketData.hSocket == INVALID_SOCKET)
					continue;

				m_SocketData.nFamily = m_SocketData.nextAddr->ai_family;
				AttachHandle(m_SocketData.hSocket);
				if (!AsyncSelect(m_lEvent))
				{
					if (newSocket)
					{
						DetachHandle(m_SocketData.hSocket);
						closesocket(m_SocketData.hSocket);
						m_SocketData.hSocket = INVALID_SOCKET;
					}
					continue;
				}
			}
			else if (m_SocketData.hSocket == INVALID_SOCKET)
				continue;

#ifndef NOLAYERS
			if (m_pFirstLayer)
			{
				if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE))
				{
					if (newSocket)
					{
						m_SocketData.nFamily = AF_UNSPEC;
						DetachHandle(m_SocketData.hSocket);
						closesocket(m_SocketData.hSocket);
						m_SocketData.hSocket = INVALID_SOCKET;
					}
					continue;
				}
			}
#endif //NOLAYERS

			if (newSocket)
			{
				m_SocketData.nFamily = m_SocketData.nextAddr->ai_family;
				if (!Bind(m_nSocketPort, m_lpszSocketAddress))
				{ 
					m_SocketData.nFamily = AF_UNSPEC;
					DetachHandle(m_SocketData.hSocket);
					closesocket(m_SocketData.hSocket);
					m_SocketData.hSocket = INVALID_SOCKET;
					continue; 
				}
			}

			if (!(ret = CAsyncSocketEx::Connect(m_SocketData.nextAddr->ai_addr, m_SocketData.nextAddr->ai_addrlen)) && GetLastError() != WSAEWOULDBLOCK)
			{
				if (newSocket)
				{
					m_SocketData.nFamily = AF_UNSPEC;
					DetachHandle(m_SocketData.hSocket);
					closesocket(m_SocketData.hSocket);
					m_SocketData.hSocket = INVALID_SOCKET;
				}
				continue;
			}

			break;
		}

		if (m_SocketData.nextAddr)
			m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next;

		if (!m_SocketData.nextAddr)
		{
			freeaddrinfo(m_SocketData.addrInfo);
			m_SocketData.nextAddr = 0;
			m_SocketData.addrInfo = 0;
		}

		if (m_SocketData.hSocket == INVALID_SOCKET || !ret)
			return FALSE;
		else
			return TRUE;
	}
}

BOOL CAsyncSocketEx::Connect(const SOCKADDR* lpSockAddr, int nSockAddrLen)
{
	BOOL res;
#ifndef NOLAYERS
	if (m_pFirstLayer)
		res = SOCKET_ERROR!=m_pFirstLayer->Connect(lpSockAddr, nSockAddrLen);
	else
#endif //NOLAYERS
		res = SOCKET_ERROR!=connect(m_SocketData.hSocket, lpSockAddr, nSockAddrLen);

#ifndef NOSOCKETSTATES
	if (res || GetLastError()==WSAEWOULDBLOCK)
		SetState(connecting);
#endif //NOSOCKETSTATES
	return res;
}

#ifdef _AFX
BOOL CAsyncSocketEx::GetPeerName( CString& rPeerAddress, UINT& rPeerPort )
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->GetPeerName(rPeerAddress, rPeerPort);
#endif NOLAYERS

	SOCKADDR* sockAddr;
	int nSockAddrLen;

	if (m_SocketData.nFamily == AF_INET6)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN6;
		nSockAddrLen = sizeof(SOCKADDR_IN6);
	} 
	else if (m_SocketData.nFamily == AF_INET)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN;
		nSockAddrLen = sizeof(SOCKADDR_IN);
	}

	memset(sockAddr, 0, nSockAddrLen);

	BOOL bResult = GetPeerName(sockAddr, &nSockAddrLen);

	if (bResult)
	{
		if (m_SocketData.nFamily == AF_INET6)
		{
			rPeerPort = ntohs(((SOCKADDR_IN6*)sockAddr)->sin6_port);
			LPTSTR buf = Inet6AddrToString(((SOCKADDR_IN6*)sockAddr)->sin6_addr);
			rPeerAddress = buf;
			delete [] buf;
		}
		else if (m_SocketData.nFamily == AF_INET)
		{
			rPeerPort = ntohs(((SOCKADDR_IN*)sockAddr)->sin_port);
			rPeerAddress = inet_ntoa(((SOCKADDR_IN*)sockAddr)->sin_addr);
		}
		else
		{
			delete sockAddr;
			return FALSE;
		}
	}
	delete sockAddr;

	return bResult;
}
#endif //_AFX

BOOL CAsyncSocketEx::GetPeerName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->GetPeerName(lpSockAddr, lpSockAddrLen);
#endif //NOLAYERS

	if (!getpeername(m_SocketData.hSocket, lpSockAddr, lpSockAddrLen))
	{
		return TRUE;
	}
	else
	{
		return FALSE;
	}
}

#ifdef _AFX
BOOL CAsyncSocketEx::GetSockName(CString& rSocketAddress, UINT& rSocketPort)
{
	SOCKADDR* sockAddr;
	int nSockAddrLen;

	if (m_SocketData.nFamily == AF_INET6)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN6;
		nSockAddrLen = sizeof(SOCKADDR_IN6);
	}
	else if (m_SocketData.nFamily == AF_INET)
	{
		sockAddr = (SOCKADDR*)new SOCKADDR_IN;
		nSockAddrLen = sizeof(SOCKADDR_IN);
	}

	memset(sockAddr, 0, nSockAddrLen);

	BOOL bResult = GetSockName(sockAddr, &nSockAddrLen);

	if (bResult)
	{
		if (m_SocketData.nFamily == AF_INET6)
		{
			rSocketPort = ntohs(((SOCKADDR_IN6*)sockAddr)->sin6_port);
			LPTSTR buf = Inet6AddrToString(((SOCKADDR_IN6*)sockAddr)->sin6_addr);
			rSocketAddress = buf;
			delete [] buf;
		}
		else if (m_SocketData.nFamily == AF_INET)
		{
			rSocketPort = ntohs(((SOCKADDR_IN*)sockAddr)->sin_port);
			rSocketAddress = inet_ntoa(((SOCKADDR_IN*)sockAddr)->sin_addr);
		}
		else
		{
			delete sockAddr;
			return FALSE;
		}
	}
	delete sockAddr;

	return bResult;
}
#endif //_AFX

BOOL CAsyncSocketEx::GetSockName( SOCKADDR* lpSockAddr, int* lpSockAddrLen )
{
	if ( !getsockname(m_SocketData.hSocket, lpSockAddr, lpSockAddrLen) )
		return TRUE;
	else
		return FALSE;
}

BOOL CAsyncSocketEx::ShutDown(int nHow /*=sends*/)
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		return m_pFirstLayer->ShutDown();
	}
	else
#endif //NOLAYERS
	{
		if (!shutdown(m_SocketData.hSocket, nHow))
			return TRUE;
		else
			return FALSE;
	}
}

SOCKET CAsyncSocketEx::Detach()
{
	SOCKET socket = m_SocketData.hSocket;
	DetachHandle(socket);
	m_SocketData.hSocket = INVALID_SOCKET;
	m_SocketData.nFamily = AF_UNSPEC;
	return socket;
}

BOOL CAsyncSocketEx::Attach(SOCKET hSocket, long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/)
{
	if (hSocket==INVALID_SOCKET || !hSocket)
		return FALSE;

	VERIFY(InitAsyncSocketExInstance());
	m_SocketData.hSocket=hSocket;
	AttachHandle(hSocket);

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		m_lEvent = lEvent;
		return !WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE);
	}
	else
#endif //NOLAYERS
	{
		return AsyncSelect(lEvent);
	}		
}

BOOL CAsyncSocketEx::AsyncSelect( long lEvent /*= FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE*/ )
{
	ASSERT(m_pLocalAsyncSocketExThreadData);
	m_lEvent = lEvent;
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return TRUE;
	else
#endif //NOLAYERS
	{
		if (m_SocketData.hSocket == INVALID_SOCKET && m_SocketData.nFamily == AF_UNSPEC)
			return true;

		if ( !WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, lEvent) )
			return TRUE;
		else
			return FALSE;
	}
	return TRUE;
}

BOOL CAsyncSocketEx::Listen( int nConnectionBacklog /*=5*/ )
{
#ifndef NOLAYERS
	if (m_pFirstLayer)
		return m_pFirstLayer->Listen(nConnectionBacklog);
#endif //NOLAYERS

	if (!listen(m_SocketData.hSocket, nConnectionBacklog))
	{
#ifndef NOSOCKETSTATES
		SetState(listening);
#endif //NOSOCKETSTATES
		return TRUE;
	}
	else
		return FALSE;
}

BOOL CAsyncSocketEx::Accept( CAsyncSocketEx& rConnectedSocket, SOCKADDR* lpSockAddr /*=NULL*/, int* lpSockAddrLen /*=NULL*/ )
{
	ASSERT(rConnectedSocket.m_SocketData.hSocket == INVALID_SOCKET);
#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		return m_pFirstLayer->Accept(rConnectedSocket, lpSockAddr, lpSockAddrLen);
	}
	else
#endif //NOLAYERS
	{
		SOCKET hTemp = accept(m_SocketData.hSocket, lpSockAddr, lpSockAddrLen);

		if (hTemp == INVALID_SOCKET)
			return FALSE;
		VERIFY(rConnectedSocket.InitAsyncSocketExInstance());
		rConnectedSocket.m_SocketData.hSocket=hTemp;
		rConnectedSocket.AttachHandle(hTemp);
		rConnectedSocket.SetFamily(GetFamily());
#ifndef NOSOCKETSTATES
		rConnectedSocket.SetState(connected);
#endif //NOSOCKETSTATES
	}
	return TRUE;
}

BOOL CAsyncSocketEx::IOCtl( long lCommand, DWORD* lpArgument )
{
	return ioctlsocket(m_SocketData.hSocket, lCommand, lpArgument) != SOCKET_ERROR;
}

int CAsyncSocketEx::GetLastError()
{
	return WSAGetLastError();
}

BOOL CAsyncSocketEx::TriggerEvent(long lEvent)
{
	if (m_SocketData.hSocket==INVALID_SOCKET)
		return FALSE;

	ASSERT(m_pLocalAsyncSocketExThreadData);
	ASSERT(m_pLocalAsyncSocketExThreadData->m_pHelperWindow);
	ASSERT(m_SocketData.nSocketIndex!=-1);

#ifndef NOLAYERS
	if (m_pFirstLayer)
	{
		CAsyncSocketExLayer::t_LayerNotifyMsg *pMsg = new CAsyncSocketExLayer::t_LayerNotifyMsg;
		pMsg->hSocket = m_SocketData.hSocket;
		pMsg->lEvent=lEvent%0xFFFF;
		pMsg->pLayer=0;
		BOOL res=PostMessage(GetHelperWindowHandle(), WM_USER, (WPARAM)m_SocketData.nSocketIndex, (LPARAM)pMsg);
		if (!res)
			delete pMsg;
		return res;
	}
	else
#endif //NOLAYERS
		return PostMessage(GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, m_SocketData.hSocket, lEvent%0xFFFF);

}

SOCKET CAsyncSocketEx::GetSocketHandle()
{
	return m_SocketData.hSocket;
}

HWND CAsyncSocketEx::GetHelperWindowHandle()
{
	if (!m_pLocalAsyncSocketExThreadData)
		return 0;
	if (!m_pLocalAsyncSocketExThreadData->m_pHelperWindow)
		return 0;
	return m_pLocalAsyncSocketExThreadData->m_pHelperWindow->GetHwnd();
}

#ifndef NOLAYERS
BOOL CAsyncSocketEx::AddLayer(CAsyncSocketExLayer *pLayer)
{
	ASSERT(pLayer);

	if (m_pFirstLayer)
	{
		ASSERT(m_pLastLayer);
		m_pLastLayer=m_pLastLayer->AddLayer(pLayer, this);
		return m_pLastLayer?TRUE:FALSE;
	}
	else
	{
		ASSERT(!m_pLastLayer);
		pLayer->Init(0, this);
		m_pFirstLayer=pLayer;
		m_pLastLayer=m_pFirstLayer;
		if (m_SocketData.hSocket != INVALID_SOCKET)
			if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE))
				return FALSE;
	}

	return TRUE;
}

void CAsyncSocketEx::RemoveAllLayers()
{
	for (std::list<t_callbackMsg>::iterator iter = m_pendingCallbacks.begin(); iter != m_pendingCallbacks.end(); iter++)
		delete [] iter->str;
	m_pendingCallbacks.clear();

	m_pFirstLayer = 0;
	m_pLastLayer = 0;

	if (!m_pLocalAsyncSocketExThreadData)
		return;
	if (!m_pLocalAsyncSocketExThreadData->m_pHelperWindow)
		return;
	m_pLocalAsyncSocketExThreadData->m_pHelperWindow->RemoveLayers(this);
}

int CAsyncSocketEx::OnLayerCallback(std::list<t_callbackMsg>& callbacks)
{
	for (std::list<t_callbackMsg>::iterator iter = callbacks.begin(); iter != callbacks.end(); iter++)
	{
		delete [] iter->str;
	}
	return 0;
}

BOOL CAsyncSocketEx::IsLayerAttached() const
{
	return m_pFirstLayer ? TRUE : FALSE;
}
#endif //NOLAYERS

BOOL CAsyncSocketEx::GetSockOpt(int nOptionName, void* lpOptionValue, int* lpOptionLen, int nLevel /*=SOL_SOCKET*/)
{
	return (SOCKET_ERROR != getsockopt(m_SocketData.hSocket, nLevel, nOptionName, (LPSTR)lpOptionValue, lpOptionLen));
}

BOOL CAsyncSocketEx::SetSockOpt(int nOptionName, const void* lpOptionValue, int nOptionLen, int nLevel /*=SOL_SOCKET*/)
{
	return (SOCKET_ERROR != setsockopt(m_SocketData.hSocket, nLevel, nOptionName, (LPSTR)lpOptionValue, nOptionLen));
}

#ifndef NOSOCKETSTATES

int CAsyncSocketEx::GetState() const
{
	return m_nState;
}

void CAsyncSocketEx::SetState(int nState)
{
	m_nState = nState;
}

const TCHAR * CAsyncSocketEx::GetStateDesc(int nState)
{
  switch (nState)
  {
    case notsock:
      return _T("none");
    case unconnected:
      return _T("unconnected");
    case connecting:
      return _T("connecting");
    case listening:
      return _T("listening");
    case connected:
      return _T("connected");
    case closed:
      return _T("closed");
    case aborted:
      return _T("aborted");
    case attached:
      return _T("attached");
    default:
      return _T("unknown");
  }
}

bool CAsyncSocketEx::LogStateChange(int nState1, int nState2)
{
    return (nState2 != notsock) || (nState1 != unconnected);
}

#endif //NOSOCKETSTATES

int CAsyncSocketEx::GetFamily() const
{
	return m_SocketData.nFamily;
}

bool CAsyncSocketEx::SetFamily(int nFamily)
{
	if (m_SocketData.nFamily != AF_UNSPEC)
		return false;

	m_SocketData.nFamily = nFamily;
	return true;	
}

bool CAsyncSocketEx::TryNextProtocol()
{
	DetachHandle(m_SocketData.hSocket);
	closesocket(m_SocketData.hSocket);
	m_SocketData.hSocket = INVALID_SOCKET;

	BOOL ret = FALSE;
	for (; m_SocketData.nextAddr; m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next)
	{
		m_SocketData.hSocket = socket(m_SocketData.nextAddr->ai_family, m_SocketData.nextAddr->ai_socktype, m_SocketData.nextAddr->ai_protocol);

		if (m_SocketData.hSocket == INVALID_SOCKET)
			continue;

		AttachHandle(m_SocketData.hSocket);
		m_SocketData.nFamily = m_SocketData.nextAddr->ai_family;
		if (!AsyncSelect(m_lEvent))
		{
			DetachHandle(m_SocketData.hSocket);
			closesocket(m_SocketData.hSocket);
			m_SocketData.hSocket = INVALID_SOCKET;
			continue;
		}

#ifndef NOLAYERS
		if (m_pFirstLayer)
		{
			if (WSAAsyncSelect(m_SocketData.hSocket, GetHelperWindowHandle(), m_SocketData.nSocketIndex+WM_SOCKETEX_NOTIFY, FD_READ | FD_WRITE | FD_OOB | FD_ACCEPT | FD_CONNECT | FD_CLOSE))
			{
				DetachHandle(m_SocketData.hSocket);
				closesocket(m_SocketData.hSocket);
				m_SocketData.hSocket = INVALID_SOCKET;
				continue;
			}
		}
#endif //NOLAYERS

		if (!Bind(m_nSocketPort, m_lpszSocketAddress))
		{ 
			DetachHandle(m_SocketData.hSocket);
			closesocket(m_SocketData.hSocket);
			m_SocketData.hSocket = INVALID_SOCKET;
			continue; 
		}

		ret = CAsyncSocketEx::Connect(m_SocketData.nextAddr->ai_addr, m_SocketData.nextAddr->ai_addrlen);
		if (!ret && GetLastError() != WSAEWOULDBLOCK)
		{
			DetachHandle(m_SocketData.hSocket);
			closesocket(m_SocketData.hSocket);
			m_SocketData.hSocket = INVALID_SOCKET;
			continue;
		}

		ret = true;
		break;
	}

	if (m_SocketData.nextAddr)
		m_SocketData.nextAddr = m_SocketData.nextAddr->ai_next;

	if (!m_SocketData.nextAddr)
	{
		freeaddrinfo(m_SocketData.addrInfo);
		m_SocketData.nextAddr = 0;
		m_SocketData.addrInfo = 0;
	}

	if (m_SocketData.hSocket == INVALID_SOCKET || !ret)
		return FALSE;
	else
		return TRUE;
}

#ifndef NOLAYERS

void CAsyncSocketEx::AddCallbackNotification(const t_callbackMsg& msg)
{
	m_pendingCallbacks.push_back(msg);

	if(m_pendingCallbacks.size() == 1 && m_SocketData.nSocketIndex != -1)
		PostMessage(GetHelperWindowHandle(), WM_USER + 2, (WPARAM)m_SocketData.nSocketIndex, 0);
}

#endif //NOLAYERS

void CAsyncSocketEx::ResendCloseNotify()
{
	for (std::list<CAsyncSocketEx*>::iterator iter = m_pLocalAsyncSocketExThreadData->layerCloseNotify.begin(); iter != m_pLocalAsyncSocketExThreadData->layerCloseNotify.end(); iter++)
	{
		if (*iter == this)
			return;
	}
	m_pLocalAsyncSocketExThreadData->layerCloseNotify.push_back(this);
	if (m_pLocalAsyncSocketExThreadData->layerCloseNotify.size() == 1)
	{
		SetTimer(m_pLocalAsyncSocketExThreadData->m_pHelperWindow->GetHwnd(), 1, 10, 0);
	}
}
