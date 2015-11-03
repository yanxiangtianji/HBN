// visualizationDlg.cpp : implementation file
//

#include "stdafx.h"
#include "visualization.h"
#include "visualizationDlg.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CvisualizationDlg dialog




CvisualizationDlg::CvisualizationDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CvisualizationDlg::IDD, pParent)
	, m_state(_T(""))
	, m_input(_T(""))
	, m_output(_T(""))
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CvisualizationDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_STATIC_STATE, m_state);
	DDX_Text(pDX, IDC_EDIT_IN, m_input);
	DDX_Text(pDX, IDC_EDIT_OUT, m_output);
	DDX_Control(pDX, IDC_PIC, m_pic);
}

BEGIN_MESSAGE_MAP(CvisualizationDlg, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON_PREDICT, &CvisualizationDlg::OnBnClickedButtonPredict)
	ON_BN_CLICKED(IDC_BUTTON_CURRENT, &CvisualizationDlg::OnBnClickedButtonCurrent)
	ON_BN_CLICKED(IDC_BUTTON_F1_P, &CvisualizationDlg::OnBnClickedButtonF1P)
	ON_BN_CLICKED(IDC_BUTTON_F2_P, &CvisualizationDlg::OnBnClickedButtonF2P)
	ON_BN_CLICKED(IDC_BUTTON_F1_R, &CvisualizationDlg::OnBnClickedButtonF1R)
	ON_BN_CLICKED(IDC_BUTTON_F2_R, &CvisualizationDlg::OnBnClickedButtonF2R)
END_MESSAGE_MAP()


// CvisualizationDlg message handlers

BOOL CvisualizationDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	if(!net.initial(string("../node_traffic.txt"),string("../structure_traffic.txt")))
		m_state="State: Error!";
	else
		m_state="State: Model loaded.";
	UpdateData(false);
	m_pic.GetClientRect(&m_pic_rect);

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CvisualizationDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
	//load_pic();
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CvisualizationDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

string CvisualizationDlg::name_given[13]={"time_bin","week_day","temperature","precipitation","visibility","snow","wind","current_0","current_1","current_2","current_3","current_4","current_5"};
string CvisualizationDlg::name_query[12]={"future_1_0","future_1_1","future_1_2","future_1_3","future_1_4","future_1_5","future_2_0","future_2_1","future_2_2","future_2_3","future_2_4","future_2_5"};


void CvisualizationDlg::OnBnClickedButtonPredict()
{
	using namespace std;
	// TODO: 在此添加控件通知处理程序代码
//inputing:
	UpdateData();
	string str(m_input.GetBuffer());
	m_input.ReleaseBuffer();
	if(str.empty())
		return;
	input.clear();
	size_t plast=0,p=str.find(',');
	while(p!=string::npos){
		input.push_back(stoi(str.substr(plast,p-plast)));
		plast=p+1;
		p=str.find(',',plast);
	}
	input.push_back(stoi(str.substr(plast)));
	if(input.size()!=25){
		m_state="Worng input! 25 part needed.";
		UpdateData(false);
		return;
	}
	
//predicting:
	m_state="Predicting...";
	UpdateData(false);
	map<Network::node_name_t,Network::state_t> given;
	vector<Network::node_name_t> query;
	for(size_t i=0;i<13;i++){
		given[name_given[i]]=input[i];
	}
	for(size_t i=0;i<12;i++){
		query.push_back(name_query[i]);
	}
//	map<Network::node_name_t,pair<Network::state_t,double> > res;//member
	m_res.clear();
	m_res=net.predict_with_poss(given,query);

//outputing:
	m_state="Done.";
	m_output.Format("(%d,%lf)",m_res[name_query[0]].first,m_res[name_query[0]].second);
	for(size_t i=1;i<12;i++){
		CString t;
		t.Format(",(%d,%lf)",m_res[name_query[i]].first,m_res[name_query[i]].second);
		if(i==5){
			t.Append("\r\n");
		}
		m_output.Append(t);
	}
	OnPaint();
	UpdateData(false);
	return;
}

CPoint CvisualizationDlg::m_pos[6]={CPoint(168,37),CPoint(131,74),CPoint(157,162),
	CPoint(207,164),CPoint(114,230),CPoint(136,284)};

void CvisualizationDlg::draw_state(const vector<Network::state_t>& state)
{
	if(state.size()!=6){
		m_state="Error inner state parameter!";
		UpdateData(false);
		return;
	}
	CClientDC dc(&m_pic);
	for(size_t i=0;i<state.size();i++){
		int x=m_pos[i].x;
		int y=m_pos[i].y;
		if(state[i]==0){
			dc.SetTextColor(RGB(0,255,0));
		}else if(state[i]==1){
			dc.SetTextColor(RGB(255,0,0));
		}
		CString temp;
		temp.Format("%d",i);
		dc.TextOutA(x,y,temp);
	}
}

void CvisualizationDlg::OnBnClickedButtonCurrent()
{
	// TODO: 在此添加控件通知处理程序代码
	if(input.size()!=25){
		m_state="Input data not ready!";
		UpdateData(false);
		return;
	}
	vector<Network::state_t> state;
	state.insert(state.end(),input.begin()+7,input.begin()+13);
	draw_state(state);
}

void CvisualizationDlg::OnBnClickedButtonF1R()
{
	// TODO: 在此添加控件通知处理程序代码
	if(input.size()!=25){
		m_state="Input data not ready!";
		UpdateData(false);
		return;
	}
	vector<Network::state_t> state;
	state.insert(state.end(),input.begin()+13,input.begin()+19);
	draw_state(state);
}

void CvisualizationDlg::OnBnClickedButtonF2R()
{
	// TODO: 在此添加控件通知处理程序代码
	if(input.size()!=25){
		m_state="Input data not ready!";
		UpdateData(false);
		return;
	}
	vector<Network::state_t> state;
	state.insert(state.end(),input.begin()+19,input.end());
	draw_state(state);
}

void CvisualizationDlg::OnBnClickedButtonF1P()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_state!="Done."){
		m_state="Prediction have not been performed!";
		UpdateData(false);
		return;
	}
	vector<Network::state_t> state;
	for(size_t i=0;i<6;i++){
		state.push_back(m_res[name_query[i]].first);
	}
	draw_state(state);
}

void CvisualizationDlg::OnBnClickedButtonF2P()
{
	// TODO: 在此添加控件通知处理程序代码
	if(m_state!="Done."){
		m_state="Prediction have not been performed!";
		UpdateData(false);
		return;
	}
	vector<Network::state_t> state;
	for(size_t i=6;i<12;i++){
		state.push_back(m_res[name_query[i]].first);
	}
	draw_state(state);
}
