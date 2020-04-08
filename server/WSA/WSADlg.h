
// WSADlg.h: 头文件
//

#pragma once
#define WM_SOCKET (WM_USER+1)	//定义socket消息
#define WM_QU (WM_USER+200)

// CWSADlg 对话框
class CWSADlg : public CDialogEx
{
// 构造
public:
	CWSADlg(CWnd* pParent = nullptr);	// 标准构造函数

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_WSA_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持

public:
	SOCKET s;
	static UINT SocketThread(LPVOID lParam);
	void change(int count);
// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnClickedListen();
	afx_msg LRESULT OnSocketMsg(WPARAM wParam, LPARAM lParam);
	afx_msg LRESULT OnSocketQuit(WPARAM wParam, LPARAM lParam);		//消息响应函数
	char buff[200];
	CEdit mess;
	char filename[20000];
	int count;		//记录当前连接到服务端的客户端的数量
};
