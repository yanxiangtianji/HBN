// visualizationDlg.h : header file
//

#pragma once
#include "../bayesian/def.h"
#include "../bayesian/Network.h"
#include "afxwin.h"

// CvisualizationDlg dialog
class CvisualizationDlg : public CDialog
{
// Construction
public:
	CvisualizationDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_VISUALIZATION_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
private:
	Network net;
	vector<size_t> input;
	map<node_name_t,pair<state_t,double> > m_res;
	static string name_given[];
	static string name_query[];
	static CPoint m_pos[6];
	CRect m_pic_rect;
public:
	CString m_state;
	CString m_input;
	CString m_output;
	CStatic m_pic;

	afx_msg void OnBnClickedButtonPredict();
	void draw_state(const vector<state_t>& state);
	afx_msg void OnBnClickedButtonCurrent();
	afx_msg void OnBnClickedButtonF1P();
	afx_msg void OnBnClickedButtonF2P();
	afx_msg void OnBnClickedButtonF1R();
	afx_msg void OnBnClickedButtonF2R();
};
