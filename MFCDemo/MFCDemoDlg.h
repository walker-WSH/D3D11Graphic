
// MFCDemoDlg.h: 头文件
//

#pragma once

#include <Windows.h>
#include <process.h>

#include <DX11GraphicAPI.h>

#define WINDOW_WIDTH 1920
#define WINDOW_HEIGHT 1080

// CMFCDemoDlg 对话框
class CMFCDemoDlg : public CDialogEx
{
// 构造
public:
	CMFCDemoDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CMFCDemoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCDEMO_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	bool m_bExit = false;
	HANDLE m_hThread = 0;
	IDX11GraphicInstance *m_pGraphic = nullptr;
	static unsigned __stdcall ThreadFunc(void *pParam);

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
