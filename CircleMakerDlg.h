// CircleMakerDlg.h: 헤더 파일
//

#pragma once

#include "PointView.h"

constexpr int IDC_POINTVIEW = 1001;

// CCircleMakerDlg 대화 상자
class CCircleMakerDlg : public CDialogEx
{
// 생성입니다.
public:
	CCircleMakerDlg(CWnd* pParent = nullptr);	// 표준 생성자입니다.
	~CCircleMakerDlg();
	
// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_CircleMaker_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 지원입니다.
	void OnOK() final;
	void OnCancel() final;

// 구현입니다.
private:
	HICON m_hIcon;

	// 생성된 메시지 맵 함수
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg void OnBnClickedBtnRandomizePoints();
	afx_msg void OnBnClickedBtnReset();
	afx_msg void OnBnClickedBtnSet();
	afx_msg LRESULT OnPointCountChanged(WPARAM wParam, LPARAM lParam);
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

private:
	CEdit m_editRadius;
	CEdit m_editThickness;

	std::shared_ptr<PointView> m_pointView;
	CButton m_btnRandomizePoints;
};
