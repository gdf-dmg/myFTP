
// WSADlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "WSA.h"
#include "WSADlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CWSADlg 对话框



CWSADlg::CWSADlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_WSA_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CWSADlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, mess);
}

BEGIN_MESSAGE_MAP(CWSADlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CWSADlg::OnClickedListen)
	ON_MESSAGE(WM_SOCKET, OnSocketMsg)	//绑定消息
	ON_MESSAGE(WM_QU, OnSocketQuit)	//绑定消息
END_MESSAGE_MAP()


// CWSADlg 消息处理程序

BOOL CWSADlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText("Server");
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码
	count = 0;
	change(count);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CWSADlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CWSADlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CWSADlg::change(int count)
{
	char buf[100];
	::wsprintf(buf, "当前连接用户数：%d", count);
	CString up_info = buf;
	SetWindowText(up_info);
}



void CWSADlg::OnClickedListen()
{
	// TODO: 在此添加控件通知处理程序代码
	s = ::socket(AF_INET, SOCK_STREAM, 0);
	sockaddr_in sin;
	sin.sin_family = AF_INET;
	sin.sin_port = ::ntohs(9000);
	sin.sin_addr.S_un.S_addr = INADDR_ANY;
	if (::bind(s, (sockaddr*)&sin, sizeof(sin)) == SOCKET_ERROR)
	{
		return;
	}
	::WSAAsyncSelect(s, m_hWnd, WM_SOCKET, FD_ACCEPT);		//这个模型的好处是不用单独开一个线程来监听
	::listen(s, 5);
}


LRESULT CWSADlg::OnSocketMsg(WPARAM wParam, LPARAM lParam)		//(哪一个socket上的事件，哪一种事件)
{
	SOCKET s = wParam;
	switch (WSAGETSELECTEVENT(lParam))
	{
	case FD_ACCEPT://检测到有套接字连上来  
	{
		
		sockaddr_in clientAddr;
		int clientLen = sizeof(clientAddr);
		SOCKET client = ::accept(s, (struct sockaddr*)&clientAddr, &clientLen);//注意：这时这个socket是一个异步的而不是堵塞
		CString ip = ::inet_ntoa(clientAddr.sin_addr);
		unsigned short port = ::ntohs(clientAddr.sin_port);
		char address[100];
		::wsprintf(address, "ip:%s  port:%d 连接到服务器", ip, port);
		CString info = address;
		CString content;
		mess.GetWindowText(content);
		content += info + "\r\n";
		mess.SetWindowText(content);
		count++;
		change(count);

		unsigned long mode = 0;
		::WSAAsyncSelect(client, m_hWnd, WM_SOCKET, 0);		//这一步不可少
		::ioctlsocket(client, FIONBIO, &mode);//设置为阻塞模式
		::AfxBeginThread(SocketThread, (LPVOID)client);	//创建单独线程接收，主线程就不会被卡死
		
		//::WSAAsyncSelect(client, m_hWnd, WM_SOCKET, FD_READ | FD_WRITE | FD_CLOSE);
		//AfxMessageBox("connect");
		//此处有坑，新手常犯
	}
	break;
	//case FD_READ:		不用这种方式，在主线程里不好，而且遇到大数据不能一次接收完
	//{
	//	//接收数据并在文本框中显示
	//	CString content = "";			
	//	mess.GetWindowText(content);
	//	int len = recv(s, buff, 200, 0);
	//	buff[len] = '\0';
	//	content = content + (CString)buff + "\r\n";
	//	mess.SetWindowText(content);
	//}
	//break;
	//case FD_CLOSE:
	//{
	//	AfxMessageBox("disconnected");
	//	::closesocket(s);
	//}
	//break;
	}
	return true;
}


UINT CWSADlg::SocketThread(LPVOID lParam)	//数据接收线程函数
{
	//先接收长度再接收数据
	SOCKET s = (SOCKET)lParam;

	/*sockaddr_in clientAddr;
	int clientLen = sizeof(clientAddr);
	::getpeername(s, (struct sockaddr*)&clientAddr, &clientLen);		//获取客户端ip和端口号
	CString ip = ::inet_ntoa(clientAddr.sin_addr);
	unsigned short port = ::ntohs(clientAddr.sin_port);
	char address[100];
	::wsprintf(address, "ip:%s  port:%d:", ip, port);*/

	//DWORD length1 = 0;		//用来接客户端发来的文件名长度
	while (1)
	{
		char choice;
		::recv(s, (char*)&choice, sizeof(choice), 0);
		if (choice == 'u')
		{
			//AfxMessageBox("收到上传请求");
			//::AfxMessageBox("lalala");
			//SOCKET s = (SOCKET)lParam;
			WORD length1 = 0;
			if (::recv(s, (char*)&length1, sizeof(length1), 0) != 2)		//断开
			{
				//AfxMessageBox("客户端已断开连接");	//接收失败
				//::PostMessage(((CWSADlg*)AfxGetApp()->GetMainWnd())->m_hWnd, WM_QU, (WPARAM)s, 0);
				::closesocket(s);
				return 0;
			}
			//AfxMessageBox("我到这了");
			char* lpdata1 = (char*)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, length1);		//HeapAlloc是一个Windows API函数，它用来在指定的堆上分配内存，并且分配后的内存不可移动
			if (lpdata1 == NULL)		//if分配空间失败
			{
				//AfxMessageBox("NULL");
				//::PostMessage(((CWSADlg*)AfxGetApp()->GetMainWnd())->m_hWnd, WM_QU, (WPARAM)s, 0);
				::closesocket(s);
				return 0;
			}

			char* lpdata2 = lpdata1;
			DWORD length2 = length1;
			int size = 0;
			while (1)
			{
				size = ::recv(s, (char*)lpdata2, length2, 0);
				if (size <= 0) break;
				lpdata2 = lpdata2 + size;
				length2 = length2 - size;
				if (length2 == 0) break;
			}
			if (length2 != 0)
			{
				//::HeapFree(::GetProcessHeap(), HEAP_NO_SERIALIZE, lpdata1);		//释放缓冲区
				::closesocket(s);
				return 0;
			}

			//接收数据完毕，开始处理业务逻辑
			/*CString message = (CString)lpdata1;		//用message接收收到的数据
			CString content;
			CWSADlg* p = (CWSADlg*)AfxGetApp()->GetMainWnd();	//获取主对话框指针，因为在线程中不能直接操作做主线程中的控件，要用这种方式
			p->mess.GetWindowText(content);		//先获取文本框本来的内容
			content += address + message + "\r\n";
			p->mess.SetWindowText(content);
			::HeapFree(::GetProcessHeap(), HEAP_NO_SERIALIZE, lpdata1);*/
			//创建文件
			CString File = lpdata1;
			::HeapFree(::GetProcessHeap(), HEAP_NO_SERIALIZE, lpdata1);
			HANDLE h = (HANDLE)::CreateFile(File, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
			if (h == INVALID_HANDLE_VALUE)
			{
				//::CloseHandle(filehandle);
				::closesocket(s);
				return 0;
			}
			//接收文件数据
			DWORD filelength;
			if (::recv(s, (char*)&filelength, 4, 0) != 4)
			{
				::CloseHandle(h);
				::closesocket(s);
				return 0;
			}

			char filedata[1024];
			DWORD sum = 0;	//计数器变量
			while (1)
			{
				size = ::recv(s, (char*)filedata, 1024, 0);
				DWORD fileread;
				if (size <= 0) break;
				else
				{
					WriteFile(h, filedata, (DWORD)size, &fileread, NULL);
					sum += size;
				}
				if (sum == filelength) break;
			}
			//AfxMessageBox("我到这了");
			if (sum == filelength)
			{
				::CloseHandle(h);
				//::closesocket(s);
				::AfxMessageBox("文件接收成功");
				//::ShellExecute(NULL, "open", File, NULL, NULL, SW_SHOWNORMAL);
			}
			else
			{
				::CloseHandle(h);
				::closesocket(s);
				::DeleteFile(File);
				::AfxMessageBox("文件接收失败");
			}
			//return 0;
			//}
		}
		else if (choice == 'd')
		{
			//首先接收文件名长度和文件名
			WORD filenamelen = 0;
			if (::recv(s, (char*)&filenamelen, sizeof(filenamelen), 0) != 2)		//接收文件名长度
			{
				::closesocket(s);
				//::AfxMessageBox("长度!");
				return 0;
			}
			char* lpdata1 = (char*)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, filenamelen);		//HeapAlloc是一个Windows API函数，它用来在指定的堆上分配内存，并且分配后的内存不可移动
			if (lpdata1 == NULL)		//if分配空间失败
			{
				::closesocket(s);
				//::AfxMessageBox("分配!");
				return 0;
			}
			char* lpdata2 = lpdata1;
			WORD length2 = filenamelen;
			int size = 0;
			while (1)
			{
				size = ::recv(s, (char*)lpdata2, length2, 0);
				if (size <= 0) break;
				lpdata2 = lpdata2 + size;
				length2 = length2 - size;
				if (length2 == 0) break;
			}
			if (length2 != 0)
			{
				::closesocket(s);
				//::AfxMessageBox("这里跳出的！");
				return 0;
			}
			CString File = lpdata1;		//File存放接收到的文件名
			//::AfxMessageBox(File);
			//至此收到文件名，经验证，成功收到文件名
			//发送文件数据
			HANDLE filehandle = (HANDLE)::CreateFile(File, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
			if (filehandle == INVALID_HANDLE_VALUE)
			{
				//::CloseHandle(filehandle);
				return 0;
			}
			char data[100];
			DWORD readsize = 0;
			DWORD total = ::GetFileSize(filehandle, 0);
			if (::send(s, (char*)&total, 4, 0) <= 0)		//发送文件长度
			{
				::CloseHandle(filehandle);
				return 0;
			}
			DWORD sum = total;
			DWORD left = total % 100;
			total = total - left;
			int time = total / 100;
			::ReadFile(filehandle, data, left, &readsize, NULL);
			if (::send(s, data, left, 0) <= 0)		//先发小于缓存区部分
			{
				::CloseHandle(filehandle);
				return 0;
			}
			int i;
			for (i = 0; i < time; i++)		//再循环发其余部分
			{
				::ReadFile(filehandle, data, 100, &readsize, NULL);
				if (::send(s, data, 100, 0) <= 0)
				{
					::CloseHandle(filehandle);
					::AfxMessageBox("文件发送失败");
					return 0;
				}
			}
			::AfxMessageBox("文件发送成功");

		}
		else if (choice == 'l')
		{
			//::AfxMessageBox("处理获取文件列表请求");
			CString dirname = "D:\\MFC_workspace\\MyFtp\\server\\WSA\\";
			DWORD length = 0;
			WIN32_FIND_DATA FindFileData;
			HANDLE hlistfile = ::FindFirstFile(dirname + "*", &FindFileData);
			if (hlistfile == INVALID_HANDLE_VALUE)
			{
				return 0;
			}
			CWSADlg* dlg = (CWSADlg*)AfxGetApp()->GetMainWnd();
			char *buff = dlg->filename;
			do
			{
				if (strcmp(FindFileData.cFileName, ".") == 0 || strcmp(FindFileData.cFileName, "..") == 0)	continue;
				length = length + ::strlen(FindFileData.cFileName) + 1;
				::memcpy(buff, FindFileData.cFileName, ::strlen(FindFileData.cFileName) + 1);
				buff = buff + ::strlen(FindFileData.cFileName) + 1;
			} while (FindNextFile(hlistfile, &FindFileData));
			int result = ::send(s, (char*)&length, 4, 0);
			if (result != 4)
			{
				::AfxMessageBox("发送包长度失败");
				return 0;
			}
			result = ::send(s, dlg->filename, length, 0);
			if (result != length)
			{
				::AfxMessageBox("发送包数据失败");
				return 0;
			}
		}
		else if (choice == 'c')
		{
			::PostMessage(((CWSADlg*)AfxGetApp()->GetMainWnd())->m_hWnd, WM_QU, (WPARAM)s, 0);
			//::closesocket(s);
			return 0;
		}
	}

}


LRESULT CWSADlg::OnSocketQuit(WPARAM wParam, LPARAM lParam)		//退出消息函数
{
	SOCKET s = (SOCKET)wParam;
	sockaddr_in clientAddr;
	int clientLen = sizeof(clientAddr);
	::getpeername(s, (struct sockaddr*)&clientAddr, &clientLen);		//获取客户端ip和端口号
	CString ip = ::inet_ntoa(clientAddr.sin_addr);
	unsigned short port = ::ntohs(clientAddr.sin_port);
	char address[100];
	::wsprintf(address, "ip:%s  port:%d 断开连接", ip, port);
	CString info = address;
	CString content;
	mess.GetWindowText(content);
	content += info + "\r\n";
	mess.SetWindowText(content);
	::closesocket(s);
	count--;
	change(count);
	return true;
}
