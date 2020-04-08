
// ClientDlg.h: 头文件
//

#pragma once


// CClientDlg 对话框
class CClientDlg : public CDialogEx
{
// 构造
public:
	CClientDlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	SOCKET s;	//socket句柄
	static UINT SocketThread(LPVOID lParam);	//线程函数
	static UINT DownloadThread(LPVOID lParam);
	static UINT ShowFileListThread(LPVOID lParam);

// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedConnect();
	CEdit remoteIP;
	CEdit remotePort;
	afx_msg void OnClickedSend();
	afx_msg void OnEnChangeEdit2();
	CEdit msg;
	afx_msg void OnClickedDisconnect();
	CEdit download_msg;
//	afx_msg void OnBnClickedButton4();
	afx_msg void OnClickedDownload();
	CEdit FileMsg;
	afx_msg void getFileList();
};
