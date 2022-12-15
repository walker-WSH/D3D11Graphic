
// MFCDemoDlg.cpp: 实现文件
//

#include "pch.h"
#include "framework.h"
#include "MFCDemo.h"
#include "MFCDemoDlg.h"
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx {
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	// 实现
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX) {}

void CAboutDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CMFCDemoDlg 对话框

CMFCDemoDlg::CMFCDemoDlg(CWnd *pParent /*=nullptr*/) : CDialogEx(IDD_MFCDEMO_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_pGraphic = CreateGraphicSession();
}

CMFCDemoDlg::~CMFCDemoDlg()
{
	m_bExit = true;
	WaitForSingleObject(m_hThread, INFINITE);
	CloseHandle(m_hThread);
	DestroyGraphicSession(m_pGraphic);
}

void CMFCDemoDlg::DoDataExchange(CDataExchange *pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CMFCDemoDlg, CDialogEx)
ON_WM_SYSCOMMAND()
ON_WM_PAINT()
ON_WM_QUERYDRAGICON()
ON_WM_LBUTTONDOWN()
ON_WM_NCHITTEST()
ON_WM_NCLBUTTONDOWN()
ON_WM_SETCURSOR()
END_MESSAGE_MAP()

// CMFCDemoDlg 消息处理程序

BOOL CMFCDemoDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu *pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr) {
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty()) {
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);  // 设置大图标
	SetIcon(m_hIcon, FALSE); // 设置小图标

	// TODO: 在此添加额外的初始化代码
	MoveWindow(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
	CenterWindow();
	m_bExit = false;
	m_hThread = (HANDLE)_beginthreadex(0, 0, ThreadFunc, this, 0, 0);

	return TRUE; // 除非将焦点设置到控件，否则返回 TRUE
}

void CMFCDemoDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX) {
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	} else {
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CMFCDemoDlg::OnPaint()
{
	if (IsIconic()) {
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
	} else {
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CMFCDemoDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

LRESULT CMFCDemoDlg::OnNcHitTest(CPoint pt)
{
	RECT rcWindow;
	::GetWindowRect(m_hWnd, &rcWindow);

	// 最好将四个角的判断放在前面
	if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE && pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOPLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE &&
		 pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOPRIGHT;
	else if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE &&
		 pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOMLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE &&
		 pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOMRIGHT;
	else if (pt.x <= rcWindow.left + RESIZE_REGION_SIZE)
		return HTLEFT;
	else if (pt.x >= rcWindow.right - RESIZE_REGION_SIZE)
		return HTRIGHT;
	else if (pt.y <= rcWindow.top + RESIZE_REGION_SIZE)
		return HTTOP;
	else if (pt.y >= rcWindow.bottom - RESIZE_REGION_SIZE)
		return HTBOTTOM;

	if (pt.y <= (rcWindow.top + DRAGE_REGION_SIZE))
		return HTCAPTION;
	else
		return __super::OnNcHitTest(pt);
}

void CMFCDemoDlg::OnNcLButtonDown(UINT nHitTest, CPoint point)
{
	switch (nHitTest) {
	case HTTOP:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOP, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOM:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOM, MAKELPARAM(point.x, point.y));
		break;
	case HTLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_LEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_RIGHT, MAKELPARAM(point.x, point.y));
		break;
	case HTTOPLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPLEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTTOPRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_TOPRIGHT, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOMLEFT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMLEFT, MAKELPARAM(point.x, point.y));
		break;
	case HTBOTTOMRIGHT:
		SendMessage(WM_SYSCOMMAND, SC_SIZE | WMSZ_BOTTOMRIGHT,
			    MAKELPARAM(point.x, point.y));
		break;
	default:
		__super::OnNcLButtonDown(nHitTest, point);
	}
}

BOOL CMFCDemoDlg::OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message)
{
	switch (nHitTest) {
	case HTTOP:
	case HTBOTTOM:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENS)));
		return TRUE;

	case HTLEFT:
	case HTRIGHT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZEWE)));
		return TRUE;

	case HTTOPLEFT:
	case HTBOTTOMRIGHT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENWSE)));
		return TRUE;

	case HTTOPRIGHT:
	case HTBOTTOMLEFT:
		SetCursor(LoadCursor(NULL, MAKEINTRESOURCE(IDC_SIZENESW)));
		return TRUE;

	default:
		return __super::OnSetCursor(pWnd, nHitTest, message);
	}
}
