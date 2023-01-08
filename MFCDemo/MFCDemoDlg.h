
// MFCDemoDlg.h: 头文件
//

#pragma once

#include <Windows.h>
#include <process.h>
#include "IDX11GraphicEngine.h"

#define WINDOW_WIDTH 1280
#define WINDOW_HEIGHT 720

#define DRAGE_REGION_SIZE 40
#define RESIZE_REGION_SIZE 10

// CMFCDemoDlg 对话框
class CMFCDemoDlg : public CDialogEx {
	// 构造
public:
	CMFCDemoDlg(CWnd *pParent = nullptr); // 标准构造函数
	virtual ~CMFCDemoDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_MFCDEMO_DIALOG };
#endif

protected:
	virtual void DoDataExchange(CDataExchange *pDX); // DDX/DDV 支持

	// 实现
protected:
	HICON m_hIcon;

	bool m_bExit = false;
	HANDLE m_hThread = 0;
	IDX11GraphicSession *m_pGraphic = nullptr;
	static unsigned __stdcall ThreadFunc(void *pParam);

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg LRESULT OnNcHitTest(CPoint point);
	afx_msg void OnNcLButtonDown(UINT nHitTest, CPoint point);
	afx_msg BOOL OnSetCursor(CWnd *pWnd, UINT nHitTest, UINT message);
	afx_msg void OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void OnMButtonDown(UINT nFlags, CPoint point);
	afx_msg void OnRButtonDown(UINT nFlags, CPoint point);
	DECLARE_MESSAGE_MAP()
};
