
// MFCDemoDlg.h: 头文件
//

#pragma once

#include <DX11GraphicAPI.h>
#include <DX11GraphicInstance.h>


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

	IDX11GraphicInstance *m_pGraphic = nullptr;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
};
