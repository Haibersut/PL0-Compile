
// CompileDlg.cpp: 实现文件
//
#include "pch.h"
#include "framework.h"
#include "Compile.h"
#include "CompileDlg.h"
#include "afxdialogex.h"

#include <string.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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

// CCompileDlg 对话框

CCompileDlg::CCompileDlg(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_COMPILE_DIALOG, pParent)
{
	m_pCompileThread = nullptr;
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

CCompileDlg::~CCompileDlg()
{
	if (m_pCompileThread != nullptr) {
		WaitForSingleObject(m_pCompileThread->m_hThread, INFINITE);  // 等待线程结束
		m_pCompileThread = nullptr;
	}
}

void CCompileDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_RICHEDIT21, m_RichEdit);
}

BEGIN_MESSAGE_MAP(CCompileDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON1, &CCompileDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CCompileDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON3, &CCompileDlg::OnBnClickedButton3)
	ON_MESSAGE(WM_COMPILE_DONE, OnCompileDone)
	ON_BN_CLICKED(IDC_BUTTON4, &CCompileDlg::OnBnClickedButton4)
END_MESSAGE_MAP()

LRESULT CCompileDlg::OnCompileDone(WPARAM wParam, LPARAM lParam)
{
	m_pCompileThread = nullptr;
	if (FIN != nullptr) {
		fclose(FIN);
	}
	if (FOUT != nullptr) {
		fclose(FOUT);
	}
	return 0;
}

// CCompileDlg 消息处理程序

BOOL CCompileDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	// 将“关于...”菜单项添加到系统菜单中。
	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	// 执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 初始化对话框
	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO1);
	if (pCombo != nullptr)
	{
		pCombo->AddString(_T("10")); // 添加字体大小10
		pCombo->AddString(_T("12")); // 添加字体大小12
		pCombo->AddString(_T("14")); // 添加字体大小14
		pCombo->AddString(_T("16")); // 添加字体大小16
		pCombo->AddString(_T("18")); // 添加字体大小18

		pCombo->SetCurSel(1);
	}

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CCompileDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CCompileDlg::OnPaint()
{
	if (IsIconic())
	{
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
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CCompileDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CCompileDlg::logger(const CString& strText, const CString& strType)
{
	CHARFORMAT cf = { 0 };
	cf.cbSize = sizeof(CHARFORMAT);
	cf.dwMask = CFM_COLOR | CFM_SIZE;

	if (strType == _T("error"))
	{
		cf.crTextColor = RGB(255, 0, 0);
	}
	else if (strType == _T("debug"))
	{
		cf.crTextColor = RGB(0, 0, 255);
	}
	else if (strType == _T("success"))
	{
		cf.crTextColor = RGB(0, 128, 0);
	}
	else
	{
		cf.crTextColor = RGB(0, 0, 0);
	}

	CComboBox* pCombo = (CComboBox*)GetDlgItem(IDC_COMBO1);
	if (pCombo != nullptr)
	{
		int nCurSel = pCombo->GetCurSel();
		if (nCurSel != CB_ERR)
		{
			CString strFontSize;
			pCombo->GetLBText(nCurSel, strFontSize);
			int nFontSize = _ttoi(strFontSize);

			cf.yHeight = nFontSize * 20;
		}
	}

	CRichEditCtrl* pEdit = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT21);

	if (pEdit != nullptr)
	{
		int nLength = pEdit->GetWindowTextLength();
		pEdit->SetSel(nLength, nLength);
		pEdit->SetSelectionCharFormat(cf);
		pEdit->ReplaceSel(strText + _T("\r\n"));
	}
}

void CCompileDlg::OnBnClickedButton1()
{
	CFileDialog dlg(TRUE, _T("pl0"), NULL, OFN_FILEMUSTEXIST | OFN_HIDEREADONLY, _T("PL0 Files (*.pl0)|*.pl0|All Files (*.*)|*.*||"), this);
	if (dlg.DoModal() == IDOK)
	{
		if (FIN != NULL)
		{
			fclose(FIN);
			FIN = NULL;
		}
		if (FOUT != NULL)
		{
			fclose(FOUT);
			FOUT = NULL;
		}
		CString filePath = dlg.GetPathName();
		m_strFilePath = filePath;
		logger(_T("已选择文件: ") + filePath, _T("success"));
	}
}

UINT CompileThreadProc(LPVOID pParam)
{
	CCompileDlg* pDlg = reinterpret_cast<CCompileDlg*>(pParam);
	if (pDlg != nullptr) {
		pDlg->CompileCode();
	}
	pDlg->PostMessage(WM_COMPILE_DONE);
	return 0;
}

void CCompileDlg::CompileCode()
{
	if (m_strFilePath.IsEmpty())
	{
		AfxMessageBox(_T("请选择源码文件"));
		return;
	}

	memset(SSYM, 0, sizeof(SSYM));

	// 初始化关键字表
	strcpy_s(KWORD[1], sizeof(KWORD[1]), "BEGIN");
	strcpy_s(KWORD[2], sizeof(KWORD[2]), "CALL");
	strcpy_s(KWORD[3], sizeof(KWORD[3]), "CHAR");
	strcpy_s(KWORD[4], sizeof(KWORD[4]), "CONST");
	strcpy_s(KWORD[5], sizeof(KWORD[5]), "DO");
	strcpy_s(KWORD[6], sizeof(KWORD[6]), "ELSE");
	strcpy_s(KWORD[7], sizeof(KWORD[7]), "END");
	strcpy_s(KWORD[8], sizeof(KWORD[8]), "FOR");
	strcpy_s(KWORD[9], sizeof(KWORD[9]), "IF");
	strcpy_s(KWORD[10], sizeof(KWORD[11]), "ODD");
	strcpy_s(KWORD[11], sizeof(KWORD[12]), "PROCEDURE");
	strcpy_s(KWORD[12], sizeof(KWORD[13]), "PROGRAM");
	strcpy_s(KWORD[13], sizeof(KWORD[14]), "READ");
	strcpy_s(KWORD[14], sizeof(KWORD[10]), "REAL");
	strcpy_s(KWORD[15], sizeof(KWORD[15]), "STEP");
	strcpy_s(KWORD[16], sizeof(KWORD[16]), "THEN");
	strcpy_s(KWORD[17], sizeof(KWORD[17]), "UNTIL");
	strcpy_s(KWORD[18], sizeof(KWORD[18]), "VAR");
	strcpy_s(KWORD[19], sizeof(KWORD[19]), "WHILE");
	strcpy_s(KWORD[20], sizeof(KWORD[20]), "WRITE");
	strcpy_s(KWORD[21], sizeof(KWORD[21]), "WRITEC");

	// 初始化保留字表
	WSYM[1] = BEGINSYM;
	WSYM[2] = CALLSYM;
	WSYM[3] = CHARSYM;
	WSYM[4] = CONSTSYM;
	WSYM[5] = DOSYM;
	WSYM[6] = ELSESYM;
	WSYM[7] = ENDSYM;
	WSYM[8] = FORSYM;
	WSYM[9] = IFSYM;
	WSYM[10] = ODDSYM;
	WSYM[11] = PROCSYM;
	WSYM[12] = PROGSYM;
	WSYM[13] = READSYM;
	WSYM[14] = REALSYM;
	WSYM[15] = STEPSYM;
	WSYM[16] = THENSYM;
	WSYM[17] = UNTILSYM;
	WSYM[18] = VARSYM;
	WSYM[19] = WHILESYM;
	WSYM[20] = WRITESYM;
	WSYM[21] = WRITECSYM;

	// 初始化符号表
	SSYM['+'] = PLUS;
	SSYM['-'] = MINUS;
	SSYM['*'] = TIMES;
	SSYM['/'] = SLASH;
	SSYM['('] = LPAREN;
	SSYM[')'] = RPAREN;
	SSYM['='] = EQL;
	SSYM[','] = COMMA;
	SSYM['.'] = PERIOD;
	// SSYM['#'] = NEQ;
	SSYM[';'] = SEMICOLON;
	SSYM['\''] = SQUOTES;

	// 初始化助记符表
	strcpy_s(MNEMONIC[LIT], sizeof(MNEMONIC[LIT]), "LIT");
	strcpy_s(MNEMONIC[OPR], sizeof(MNEMONIC[OPR]), "OPR");
	strcpy_s(MNEMONIC[LOD], sizeof(MNEMONIC[LOD]), "LOD");
	strcpy_s(MNEMONIC[STO], sizeof(MNEMONIC[STO]), "STO");
	strcpy_s(MNEMONIC[CAL], sizeof(MNEMONIC[CAL]), "CAL");
	strcpy_s(MNEMONIC[INI], sizeof(MNEMONIC[INI]), "INI");
	strcpy_s(MNEMONIC[JMP], sizeof(MNEMONIC[JMP]), "JMP");
	strcpy_s(MNEMONIC[JPC], sizeof(MNEMONIC[JPC]), "JPC");

	// 初始化符号集
	DECLBEGSYS.resize(EOFSYM, 0);
	STATBEGSYS.resize(EOFSYM, 0);
	FACBEGSYS.resize(EOFSYM, 0);

	// 开始符号集
	DECLBEGSYS[CONSTSYM] = 1;
	DECLBEGSYS[VARSYM] = 1;
	DECLBEGSYS[PROCSYM] = 1;
	DECLBEGSYS[CHARSYM] = 1;
	DECLBEGSYS[REALSYM] = 1;

	// 语句开始符号集
	STATBEGSYS[BEGINSYM] = 1;
	STATBEGSYS[CALLSYM] = 1;
	STATBEGSYS[IFSYM] = 1;
	STATBEGSYS[WHILESYM] = 1;
	STATBEGSYS[WRITESYM] = 1;
	STATBEGSYS[WRITECSYM] = 1;

	// 因子开始符号集
	FACBEGSYS[IDENT] = 1;
	FACBEGSYS[NUMBER] = 1;
	FACBEGSYS[LPAREN] = 1;
	FACBEGSYS[REALSYM] = 1;
	FACBEGSYS[CHARSYM] = 1;

	memset(LINE, 0, sizeof(LINE));

	// 初始化TABLE数组
	for (int i = 0; i < TXMAX; i++) {
		memset(TABLE[i].NAME, 0, sizeof(TABLE[i].NAME));  // 清空名称
		TABLE[i].KIND = OBJECTS::CONSTANT;  // 默认设置为常量类型
		TABLE[i].VAL = 0;  // 默认常量值为0
		TABLE[i].CVAL = '\0';  // 默认字符常量为空字符
		TABLE[i].RVAL = 0.0;  // 默认实数常量为0.0
		TABLE[i].vp.LEVEL = 0;  // 默认层级为0
		TABLE[i].vp.ADR = 0;  // 默认地址为0
		TABLE[i].vp.SIZE = 0;  // 默认大小为0
		TABLE[i].vp.CVAL = false;
		TABLE[i].vp.RVAL = false;
	}

	CString strInputFilePath = m_strFilePath;
	int removeDot = strInputFilePath.ReverseFind('.');
	CString strBaseFilePath = (removeDot != -1) ? strInputFilePath.Left(removeDot) : strInputFilePath;
	CString strOutputFilePath = strBaseFilePath + _T(".COD");

	errno_t err;
	err = _tfopen_s(&FIN, strInputFilePath, _T("r"));
	err = _tfopen_s(&FOUT, strOutputFilePath, _T("w"));

	if (FIN == NULL || FOUT == NULL || err != 0) {
		if (FIN != nullptr && FOUT != nullptr) {
			fclose(FIN);
			fclose(FOUT);
		}
		AfxMessageBox(_T("无法打开文件."));
		return;
	}

	// 编译逻辑
	logger(_T("=== COMPILE PL0 ==="), _T("info"));
	fprintf(FOUT, "=== COMPILE PL0 ===\n");

	ERR = 0;
	CC = 0;
	CX = 0;
	LL = 0;
	CH = ' ';

	try {
		// 词法分析
		GetSym();
		if (SYM != PROGSYM)
			Error(0);
		else {
			GetSym();
			if (SYM != IDENT)
				Error(0);
			else {
				GetSym();
				if (SYM != SEMICOLON)
					Error(5);
				else
					GetSym();
			}
		}
		// 语法分析
		Block(0, 0, SymSetAdd(PERIOD, SymSetUnion(DECLBEGSYS, STATBEGSYS)));
	}
	catch (...) {
		if (FIN != nullptr && FOUT != nullptr) {
			fclose(FIN);
			fclose(FOUT);
		}
		return;
	}

	if (SYM != PERIOD)
		Error(9);
	if (ERR == 0)
		Interpret();
	else {
		logger(_T("ERROR IN PL/0 PROGRAM"), _T("error"));
		fprintf(FOUT, "ERROR IN PL/0 PROGRAM");
	}

	fprintf(FOUT, "\n"); // 在文件末尾添加换行
	fclose(FIN);
	fclose(FOUT);
	return;
}

void CCompileDlg::OnBnClickedButton2()
{
	if (m_pCompileThread == nullptr) {
		m_pCompileThread = AfxBeginThread(CompileThreadProc, this);
	}
	else {
		AfxMessageBox(_T("编译已在进行中"));
	}
}


void CCompileDlg::OnBnClickedButton3()
{
	CRichEditCtrl* pRichEdit = (CRichEditCtrl*)GetDlgItem(IDC_RICHEDIT21);
	if (pRichEdit != nullptr)
	{
		pRichEdit->SetWindowText(_T(""));
	}
}


void CCompileDlg::OnBnClickedButton4()
{
	if (m_pCompileThread != nullptr) {
		// 尝试终止线程
		if (TerminateThread(m_pCompileThread->m_hThread, 0)) {
			AfxMessageBox(_T("线程已被强制终止"));
		}
		else {
			AfxMessageBox(_T("线程终止失败"));
		}
		m_pCompileThread = nullptr;
	}
	else {
		AfxMessageBox(_T("没有正在运行的编译线程"));
	}
}

