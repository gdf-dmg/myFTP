
// ClientDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "Client.h"
#include "ClientDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CClientDlg 对话框



CClientDlg::CClientDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CLIENT_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT1, remoteIP);
	DDX_Control(pDX, IDC_EDIT2, remotePort);
	DDX_Control(pDX, IDC_EDIT3, msg);
	DDX_Control(pDX, IDC_EDIT4, download_msg);
	DDX_Control(pDX, IDC_EDIT5, FileMsg);
}

BEGIN_MESSAGE_MAP(CClientDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CClientDlg::OnClickedConnect)
	ON_BN_CLICKED(IDC_BUTTON2, &CClientDlg::OnClickedSend)
	ON_EN_CHANGE(IDC_EDIT2, &CClientDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON3, &CClientDlg::OnClickedDisconnect)
//	ON_BN_CLICKED(IDC_BUTTON4, &CClientDlg::OnBnClickedButton4)
ON_BN_CLICKED(IDC_BUTTON4, &CClientDlg::OnClickedDownload)
ON_BN_CLICKED(IDC_BUTTON5, &CClientDlg::getFileList)
END_MESSAGE_MAP()


// CClientDlg 消息处理程序

BOOL CClientDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	SetWindowText("未连接");
	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO: 在此添加额外的初始化代码

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClientDlg::OnPaint()
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
HCURSOR CClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CClientDlg::OnClickedConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	CString ip;
	remoteIP.GetWindowText(ip);		//得到第一个文本框中IP地址
	CString port;
	remotePort.GetWindowText(port);		//得到第二个文本框中端口值
	
	s = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN addrServer;
	addrServer.sin_addr.S_un.S_addr = ::inet_addr(ip);
	addrServer.sin_family = AF_INET;
	addrServer.sin_port = ::htons(atoi(port));		//atoi()将字符串转化为整型
	if(connect(s, (SOCKADDR *)&addrServer, sizeof(addrServer)) == -1)		//if(连接失败)，则报错
	{
		::closesocket(s);
		AfxMessageBox("连接失败");
		return;
	}
	SetWindowText("已连接");
	//连接成功则创建线程接收数据
	//AfxBeginThread(SocketThread, (LPVOID)s);	//创建线程

}


void CClientDlg::OnClickedSend()
{
	// TODO: 在此添加控件通知处理程序代码
	AfxBeginThread(SocketThread, (LPVOID)s);	//创建线程
}

UINT CClientDlg::SocketThread(LPVOID lParam)	//数据发送线程函数
{
	//AfxMessageBox("faq");
	SOCKET s = (SOCKET)lParam;

	char upload = 'u';
	::send(s, (char*)&upload, 1, 0);

	CClientDlg* dlg = (CClientDlg*)AfxGetApp()->GetMainWnd();
	CString filesendname;
	dlg->msg.GetWindowText(filesendname);
	HANDLE filehandle = (HANDLE)::CreateFile(filesendname, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
	if (filehandle == INVALID_HANDLE_VALUE)
	{
		//::CloseHandle(filehandle);
		return 0;
	}
	WORD filenamelen = filesendname.GetLength() + 1;
	if (::send(s, (char*)&filenamelen, 2, 0) <= 0)		//发送文件名长度
	{
		::CloseHandle(filehandle);
		return 0;
	}
	if (::send(s, filesendname, filenamelen, 0) <= 0)		//发送文件名
	{
		::CloseHandle(filehandle);
		return 0;
	}
	//发送文件数据
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
	return 0;
}


void CClientDlg::OnEnChangeEdit2()
{
	// TODO:  如果该控件是 RICHEDIT 控件，它将不
	// 发送此通知，除非重写 CDialogEx::OnInitDialog()
	// 函数并调用 CRichEditCtrl().SetEventMask()，
	// 同时将 ENM_CHANGE 标志“或”运算到掩码中。

	// TODO:  在此添加控件通知处理程序代码
}


void CClientDlg::OnClickedDisconnect()
{
	// TODO: 在此添加控件通知处理程序代码
	char close = 'c';
	::send(s, (char*)&close, 1, 0);
	SetWindowText("未连接");
	::closesocket(s);
}


//void CClientDlg::OnBnClickedButton4()
//{
//	// TODO: 在此添加控件通知处理程序代码
//}


void CClientDlg::OnClickedDownload()
{
	// TODO: 在此添加控件通知处理程序代码
	AfxBeginThread(DownloadThread, (LPVOID)s);		//开一个下载线程	
}


UINT CClientDlg::DownloadThread(LPVOID lParam)
{
	SOCKET s = (SOCKET)lParam;
	//先发送下载指令
	char download = 'd';
	::send(s, (char*)&download, 1, 0);
	//首先获取文本框内的文件名和文件名长度
	CClientDlg* dlg = (CClientDlg*)AfxGetApp()->GetMainWnd();
	CString filename;
	dlg->download_msg.GetWindowText(filename);
	WORD filenamelen = filename.GetLength() + 1;
	//发送文件名长度和文件名
	if (::send(s, (char*)&filenamelen, 2, 0) <= 0)		//发送文件名长度
	{
		return 0;
	}
	if (::send(s, filename, filenamelen, 0) <= 0)		//发送文件名
	{
		return 0;
	}
	//接收文件数据
	//创建文件
	HANDLE h = (HANDLE)::CreateFile(filename, GENERIC_WRITE, FILE_SHARE_WRITE, NULL, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, NULL);
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
	int size = 0;
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
		::DeleteFile(filename);
		::AfxMessageBox("文件接收失败");
	}
	return 0;
}


void CClientDlg::getFileList()		//获取服务端文件列表
{
	// TODO: 在此添加控件通知处理程序代码
	AfxBeginThread(ShowFileListThread, (LPVOID)s);
}


UINT CClientDlg::ShowFileListThread(LPVOID lParam)
{
	SOCKET s = (SOCKET)lParam;
	//发送获取文件列表指令
	char showlist = 'l';
	::send(s, (char*)&showlist, 1, 0);
	//接收数据
	DWORD length1 = 0;
	if (::recv(s, (char*)&length1, 4, 0) <= 0)
	{
		AfxMessageBox("recv error!");
		return 0;
	}

	char* lpdata1 = (char*)::HeapAlloc(::GetProcessHeap(), HEAP_ZERO_MEMORY, length1);
	if (lpdata1 == NULL)
	{
		AfxMessageBox("NULL");
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
		::HeapFree(::GetProcessHeap(), HEAP_NO_SERIALIZE, lpdata1);
		return 0;
	}

	lpdata2 = lpdata1;
	CString content;
	CClientDlg* p = (CClientDlg*)AfxGetApp()->GetMainWnd();
	p->FileMsg.GetWindowText(content);
	content += "服务端文件列表\r\n";
	p->FileMsg.SetWindowText(content);
	while ((lpdata2 - lpdata1) != length1)
	{
		CString NewFile = lpdata2;
		p->FileMsg.GetWindowText(content);
		content += "文件名：" + NewFile + "\r\n";
		p->FileMsg.SetWindowText(content);
		lpdata2 = lpdata2 + NewFile.GetLength() + 1;
	}
	::HeapFree(::GetProcessHeap(), HEAP_NO_SERIALIZE, lpdata1);

}
