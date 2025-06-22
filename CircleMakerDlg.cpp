// CircleMakerDlg.cpp: 구현 파일
//

#include "pch.h"
#include "framework.h"
#include "CircleMaker.h"
#include "CircleMakerDlg.h"
#include "afxdialogex.h"
#include <regex>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

constexpr int WIDTH = 960;
constexpr int HEIGHT = 640;
constexpr LPTSTR INITIAL_RADIUS = L"5.0";
constexpr LPTSTR INITIAL_THICKNESS = L"2.0";

// 응용 프로그램 정보에 사용되는 CAboutDlg 대화 상자입니다.

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 대화 상자 데이터입니다.
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 지원입니다.

// 구현입니다.
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()

// CCircleMakerDlg 대화 상자



CCircleMakerDlg::CCircleMakerDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_CIRCLE_MAKER_DIALOG, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CCircleMakerDlg::~CCircleMakerDlg()
{
	if (m_pointView)
	{
		m_pointView->clearAllPoints();
		m_pointView.reset();
	}
	CDialogEx::~CDialogEx();
}

void CCircleMakerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_EDIT_RADIUS, m_editRadius);
	DDX_Control(pDX, IDC_EDIT_THICKNESS, m_editThickness);
	DDX_Control(pDX, IDC_BTN_RANDOMIZE_POINTS, m_btnRandomizePoints);
}

BEGIN_MESSAGE_MAP(CCircleMakerDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BTN_RANDOMIZE_POINTS, &CCircleMakerDlg::OnBnClickedBtnRandomizePoints)
	ON_BN_CLICKED(IDC_BTN_RESET, &CCircleMakerDlg::OnBnClickedBtnReset)
	ON_MESSAGE(WM_APP_POINT_COUNT_CHANGED, &CCircleMakerDlg::OnPointCountChanged)
	ON_BN_CLICKED(IDC_BTN_SET, &CCircleMakerDlg::OnBnClickedBtnSet)
END_MESSAGE_MAP()


// CCircleMakerDlg 메시지 처리기

BOOL CCircleMakerDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 시스템 메뉴에 "정보..." 메뉴 항목을 추가합니다.

	// IDM_ABOUTBOX는 시스템 명령 범위에 있어야 합니다.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != nullptr)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// 이 대화 상자의 아이콘을 설정합니다.  응용 프로그램의 주 창이 대화 상자가 아닐 경우에는
	//  프레임워크가 이 작업을 자동으로 수행합니다.
	SetIcon(m_hIcon, TRUE);			// 큰 아이콘을 설정합니다.
	SetIcon(m_hIcon, FALSE);		// 작은 아이콘을 설정합니다.

	// TODO: 여기에 추가 초기화 작업을 추가합니다.

	CRect rect;
	GetWindowRect(rect);
	MoveWindow(rect.left, rect.top, WIDTH, HEIGHT);
	
	// attach point view
	{
		auto placeholder = GetDlgItem(IDC_VIEW_PLACEHOLDER);
		assert(placeholder);

		placeholder->GetWindowRect(&rect);
		ScreenToClient(&rect);
		// set width
		rect.right = rect.left + WIDTH - 40;

		m_pointView = std::make_shared<PointView>();
		m_pointView->Create(
			NULL,
			NULL,
			WS_CHILD | WS_VISIBLE,
			rect,
			this,
			IDC_POINTVIEW
		);

		placeholder->DestroyWindow();
	}

	// initialize
	{
		m_editRadius.SetWindowTextW(INITIAL_RADIUS);
		m_editThickness.SetWindowTextW(INITIAL_THICKNESS);
		m_btnRandomizePoints.EnableWindow(false);
	}

	return TRUE;  // 포커스를 컨트롤에 설정하지 않으면 TRUE를 반환합니다.
}

void CCircleMakerDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// 대화 상자에 최소화 단추를 추가할 경우 아이콘을 그리려면
//  아래 코드가 필요합니다.  문서/뷰 모델을 사용하는 MFC 애플리케이션의 경우에는
//  프레임워크에서 이 작업을 자동으로 수행합니다.

void CCircleMakerDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 그리기를 위한 디바이스 컨텍스트입니다.
		
		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 클라이언트 사각형에서 아이콘을 가운데에 맞춥니다.
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 아이콘을 그립니다.
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// 사용자가 최소화된 창을 끄는 동안에 커서가 표시되도록 시스템에서
//  이 함수를 호출합니다.
HCURSOR CCircleMakerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCircleMakerDlg::OnBnClickedBtnReset()
{
	assert(m_pointView);
	m_pointView->clearAllPoints();
}

LRESULT CCircleMakerDlg::OnPointCountChanged(WPARAM wParam, LPARAM lParam)
{
	int pointCount = static_cast<int>(wParam);

	// [요구사항] 2. [랜덤 이동] 버튼은 3개의 점이 입력된 상태에서만 동작한다.
	m_btnRandomizePoints.EnableWindow(pointCount == 3);

	return 0;
}

void CCircleMakerDlg::OnBnClickedBtnRandomizePoints()
{
	assert(m_pointView);
	m_pointView->randomizePoints();
}

void CCircleMakerDlg::OnOK()
{
	return;
}

void CCircleMakerDlg::OnCancel()
{
	if (m_pointView)
	{
		m_pointView->clearAllPoints();
		m_pointView.reset();
	}
	CDialogEx::OnCancel();
}

void CCircleMakerDlg::OnBnClickedBtnSet()
{
	assert(m_pointView);

	auto _getValidValue = [](CEdit& edit) -> double
		{
			CString str;
			edit.GetWindowTextW(str);

			if (str.IsEmpty())
			{
				return false;
			}

			LPTSTR endptr;
			double value = _tcstod(str, &endptr);

			if ((*endptr == '\0' && value > 0.) == false) 
			{
				value = _ttof(INITIAL_RADIUS);
				edit.SetWindowTextW(INITIAL_RADIUS);
			}

			return value;
		};

	// [요구사항] 3. 점의 크기는 사용자로부터 입력 받는다.
	m_pointView->setRadius(_getValidValue(m_editRadius));

	// [요구사항] 4. 두께는 사용자로부터 입력 받는다.
	m_pointView->setThickness(_getValidValue(m_editThickness));
}
