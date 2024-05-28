
// CompileDlg.h: 头文件
//

#pragma once

#define WM_COMPILE_DONE (WM_USER + 101)

const int AL = 10; /* LENGTH OF IDENTIFIERS */
const int NORW = 18; /* # OF RESERVED WORDS */
const int TXMAX = 100; /* LENGTH OF IDENTIFIER TABLE */
const int NMAX = 14; /* MAX NUMBER OF DEGITS IN NUMBERS */
const int AMAX = 2047; /* MAXIMUM ADDRESS */
const int LEVMAX = 3; /* MAX DEPTH OF BLOCK NESTING */
const int CXMAX = 200; /* SIZE OF CODE ARRAY */

// CCompileDlg 对话框
class CCompileDlg : public CDialogEx
{
// 构造
public:
	CCompileDlg(CWnd* pParent = nullptr);	// 标准构造函数
	virtual ~CCompileDlg();

// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_COMPILE_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton3();
	void CCompileDlg::CompileCode();
	CRichEditCtrl m_RichEdit; // 富文本编辑框控件的成员变量
	CWinThread* m_pCompileThread;
	LRESULT CCompileDlg::OnCompileDone(WPARAM wParam, LPARAM lParam);
	void logger(const CString& strText, const CString& strType);

	FILE* FIN = NULL;
	FILE* FOUT = NULL;
	CString m_strFilePath; // 文件路径

	void CCompileDlg::GetSym();
	void CCompileDlg::GetCh();
	void CCompileDlg::Error(int n);
	void CCompileDlg::ConstDeclaration(int LEV, int& TX, int& DX);
	void CCompileDlg::VarDeclaration(int LEV, int& TX, int& DX);
	void CCompileDlg::ListCode(int CX0);
	void CCompileDlg::Interpret();
	int CCompileDlg::BASE(int L, int B, int S[]);

	int ERR; /*ERROR FLAG*/
	int NUM; /*LAST NUMBER READ*/
	int CC;  /*CHARACTER COUNT*/
	int LL;  /*LINE LENGTH*/
	int CX;  /*CODE ALLOCATION INDEX*/
	char CH;  /*LAST CHAR READ*/
	char LINE[81];

	typedef enum {
		// 特殊符号
		NUL, IDENT, NUMBER,

		// 运算符和分隔符
		PLUS, MINUS, TIMES, SLASH,
		ODDSYM, EQL, NEQ, LSS, LEQ, GTR, GEQ,
		LPAREN, RPAREN, COMMA, SEMICOLON, PERIOD, BECOMES,

		// 关键字
		BEGINSYM, ENDSYM, IFSYM, THENSYM,
		WHILESYM, WRITESYM, READSYM, DOSYM, CALLSYM,
		CONSTSYM, VARSYM, PROCSYM, PROGSYM,
		ELSESYM, FORSYM, STEPSYM, UNTILSYM,

		// 新增运算符
		INCREMENT, DECREMENT,
		PLUSEQUAL, MINUSEQUAL,
		TIMESEQUAL, SLASHEQUAL,

		EOFSYM,
	} SYMBOL;

	char* SYMOUT[EOFSYM] = { 
		"NUL", "IDENT", "NUMBER", 
		
		"PLUS", "MINUS", "TIMES",
		"SLASH", "ODDSYM", "EQL", 
		"NEQ", "LSS", "LEQ", "GTR", "GEQ",
		"LPAREN", "RPAREN", "COMMA", "SEMICOLON", "PERIOD",
		"BECOMES", 
		
		"BEGINSYM", "ENDSYM", "IFSYM", "THENSYM",
		"WHILESYM", "WRITESYM", "READSYM", "DOSYM", "CALLSYM",
		"CONSTSYM", "VARSYM", "PROCSYM", "PROGSYM",
		"ELSESYM", "FORSYM", "STEPSYM", "UNTILSYM",

		"INCREMENT", "DECREMENT",
		"PLUSEQUAL", "MINUSEQUAL",
		"TIMESEQUAL", "SLASHEQUAL",
	};

	typedef enum { LIT, OPR, LOD, STO, CAL, INI, JMP, JPC } FCT;
	typedef struct {
		FCT F;     /*FUNCTION CODE*/
		int L; 	   /*0..LEVMAX  LEVEL*/
		int A;     /*0..AMAX    DISPLACEMENT ADDR*/
	} INSTRUCTION;
	typedef  char ALFA[11];
	typedef  int* SYMSET; // SET OF SYMBOL;
	typedef  enum { CONSTANT, VARIABLE, PROCEDUR } OBJECTS;

	struct {
		ALFA NAME;
		OBJECTS KIND;
		union {
			int VAL;   /*CONSTANT*/
			struct { int LEVEL, ADR, SIZE; } vp;  /*VARIABLE,PROCEDUR:*/
		};
	} TABLE[TXMAX];

	void CCompileDlg::GEN(FCT X, int Y, int Z);
	void CCompileDlg::Block(int LEV, int TX, SYMSET FSYS);
	void CCompileDlg::ENTER(OBJECTS K, int LEV, int& TX, int& DX);
	void CCompileDlg::TEST(SYMSET S1, SYMSET S2, int N);
	int CCompileDlg::SymIn(SYMBOL SYM, SYMSET S1);
	void CCompileDlg::STATEMENT(SYMSET FSYS, int LEV, int& TX);
	void CCompileDlg::EXPRESSION(SYMSET FSYS, int LEV, int& TX);
	void CCompileDlg::CONDITION(SYMSET FSYS, int LEV, int& TX);
	int CCompileDlg::POSITION(ALFA ID, int TX);
	void CCompileDlg::TERM(SYMSET FSYS, int LEV, int& TX);
	void CCompileDlg::FACTOR(SYMSET FSYS, int LEV, int& TX);

	ALFA    KWORD[NORW + 1];
	ALFA    MNEMONIC[9];
	ALFA   ID;
	SYMBOL  WSYM[NORW + 1];
	SYMBOL  SSYM['^' + 1];
	SYMSET  DECLBEGSYS, STATBEGSYS, FACBEGSYS;
	SYMBOL SYM; /*LAST SYMBOL READ*/

	INSTRUCTION  CODE[CXMAX];

	SYMSET SymSetAdd(SYMBOL SY, int* S) {
		SYMSET S1;
		S1 = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (int i = 0; i < EOFSYM; i++) S1[i] = S[i];
		S1[SY] = 1;
		return S1;
	}

	SYMSET SymSetUnion(SYMSET S1, SYMSET S2) {
		SYMSET S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (int i = 0; i < EOFSYM; i++)
			if (S1[i] || S2[i]) S[i] = 1;
			else S[i] = 0;
		return S;
	}

	SYMSET SymSetNULL() {
		SYMSET S; int i, n, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a) {
		SYMSET S; int i, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		S[a] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b) {
		SYMSET S; int i, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		S[a] = 1;  S[b] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c) {
		SYMSET S; int i, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		S[a] = 1;  S[b] = 1; S[c] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d) {
		SYMSET S; int i, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		S[a] = 1;  S[b] = 1; S[c] = 1; S[d] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d, SYMBOL e) {
		SYMSET S; int i, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		S[a] = 1;  S[b] = 1; S[c] = 1; S[d] = 1; S[e] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d, SYMBOL e, SYMBOL f) {
		SYMSET S; int i, k;
		S = (SYMSET)malloc(sizeof(int) * EOFSYM);
		for (i = 0; i < EOFSYM; i++) S[i] = 0;
		S[a] = 1;  S[b] = 1; S[c] = 1; S[d] = 1; S[e] = 1; S[f] = 1;
		return S;
	}
	afx_msg void OnBnClickedButton4();
};
