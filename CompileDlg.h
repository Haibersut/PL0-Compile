
// CompileDlg.h: 头文件
//

#pragma once

#include <vector>
#include <cstdlib>
#include <fstream>
#include <string>

#define WM_COMPILE_DONE (WM_USER + 101)

const int AL = 10; /* LENGTH OF IDENTIFIERS */
const int NORW = 21; /* # OF RESERVED WORDS */
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
	afx_msg void OnBnClickedButton4();
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

	typedef struct {
		int i;
		double d;
	} Data;

	int CCompileDlg::BASE(int L, int B, Data S[]);

	std::vector<bool> s_CODE;
	bool startSIGN = false;

	int ERR = 0; /*ERROR FLAG*/
	int NUM = 0; /*LAST NUMBER READ*/
	char CHVAR = 0;
	double REALNUM = 0.0;
	int CC = 0;  // 字符计数
	int LL = 0;  // 行长度
	int CX = 0;  // 代码分配索引
	char CH = 0;  /*LAST CHAR READ*/
	char LINE[81];

	int prevCC;  // 上一个字符的位置记录
	bool needRetreat = false;  // 是否需要回退的标志

	void CCompileDlg::Retreat() {
		// 设置回退标志
		needRetreat = true;
	}

	typedef enum {
		// 特殊符号
		NUL, IDENT, NUMBER,

		// 运算符和分隔符
		PLUS, MINUS, TIMES, SLASH,
		ODDSYM, EQL, NEQ, LSS, LEQ, GTR, GEQ,
		LPAREN, RPAREN, COMMA, SEMICOLON, PERIOD, BECOMES,
		SQUOTES,

		// 关键字
		BEGINSYM, ENDSYM, IFSYM, THENSYM,
		WHILESYM, WRITESYM, READSYM, DOSYM, CALLSYM,
		CONSTSYM, VARSYM, PROCSYM, PROGSYM,
		ELSESYM, FORSYM, STEPSYM, UNTILSYM,
		REALSYM, CHARSYM, WRITECSYM,

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
		"BECOMES", "SQUOTES",
		
		"BEGINSYM", "ENDSYM", "IFSYM", "THENSYM",
		"WHILESYM", "WRITESYM", "READSYM", "DOSYM", "CALLSYM",
		"CONSTSYM", "VARSYM", "PROCSYM", "PROGSYM",
		"ELSESYM", "FORSYM", "STEPSYM", "UNTILSYM",
		"REALSYM", "CHARSYM", "WRITECSYM",

		"INCREMENT", "DECREMENT",
		"PLUSEQUAL", "MINUSEQUAL",
		"TIMESEQUAL", "SLASHEQUAL",
	};

	// 定义指令集的枚举类型
	typedef enum {
		LIT,  // Literal
		OPR,  // Operator
		LOD,  // Load
		STO,  // Store
		CAL,  // Call
		INI,  // Initialize
		JMP,  // Jump
		JPC   // Jump Conditional
	} FCT;

	// 定义指令的结构体
	typedef struct {
		FCT F;     // 函数代码
		int L;     // 0..LEVMAX  层级
		int A;     // 0..AMAX    偏移地址
		double D;  // 用于存储浮点数
	} INSTRUCTION;

	typedef char ALFA[11];
	typedef std::vector<int> SYMSET;  // SET OF SYMBOL;
	typedef enum {
		CONSTANT,  // 常量
		VARIABLE,  // 变量
		PROCEDUR,  // 过程
		CHAR,      // 字符
		REAL       // 实数
	} OBJECTS;     // 对象的种类

	struct {
		ALFA NAME;  // 名称
		OBJECTS KIND;  // 对象种类
		union {
			int VAL;   // 常量的值
			char CVAL; // 字符常量的值
			double RVAL; // 实数常量的值
			struct {
				int LEVEL;  // 变量或过程的层级
				int ADR;    // 变量或过程的地址
				int SIZE;   // 变量或过程的大小
				bool CVAL;  // 是否为字符变量
				bool RVAL;  // 是否为实数变量
			} vp;  // 变量或过程的信息
		};
	} TABLE[TXMAX];

	bool setCVAL = false;
	bool setRVAL = false;

	void CCompileDlg::GEN(FCT X, int Y, int Z);
	void CCompileDlg::GEN(FCT X, int Y, double D);
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
	int CCompileDlg::findIDENT(const ALFA& ID);

	ALFA    KWORD[NORW + 1] = {};
	ALFA    MNEMONIC[9] = {};
	ALFA    ID = {};
	SYMBOL  WSYM[NORW + 1] = {};
	SYMBOL  SSYM[128] = {};
	SYMSET  DECLBEGSYS, STATBEGSYS, FACBEGSYS;
	SYMBOL  SYM = {}; /*LAST SYMBOL READ*/

	INSTRUCTION  CODE[CXMAX] = {};

	SYMSET SymSetAdd(SYMBOL SY, const SYMSET& S) {
		SYMSET S1 = S;
		if (SY >= S1.size()) {
			S1.resize(EOFSYM, 0); // 调整大小并初始化新元素为0
		}
		S1[SY] = 1;
		return S1;
	}

	SYMSET SymSetUnion(const SYMSET& S1, const SYMSET& S2) {
		SYMSET S(EOFSYM, 0);
		for (int i = 0; i < EOFSYM; i++) {
			if (S1[i] || S2[i]) S[i] = 1;
		}
		return S;
	}

	SYMSET SymSetNULL() {
		return SYMSET(EOFSYM, 0);
	}

	SYMSET SymSetNew(SYMBOL a) {
		SYMSET S(EOFSYM, 0);
		S[a] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b) {
		SYMSET S(EOFSYM, 0);
		S[a] = 1;
		S[b] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c) {
		SYMSET S(EOFSYM, 0);
		S[a] = 1;
		S[b] = 1;
		S[c] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d) {
		SYMSET S(EOFSYM, 0);
		S[a] = 1;
		S[b] = 1;
		S[c] = 1;
		S[d] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d, SYMBOL e) {
		SYMSET S(EOFSYM, 0);
		S[a] = 1;
		S[b] = 1;
		S[c] = 1;
		S[d] = 1;
		S[e] = 1;
		return S;
	}

	SYMSET SymSetNew(SYMBOL a, SYMBOL b, SYMBOL c, SYMBOL d, SYMBOL e, SYMBOL f) {
		SYMSET S(EOFSYM, 0);
		S[a] = 1;
		S[b] = 1;
		S[c] = 1;
		S[d] = 1;
		S[e] = 1;
		S[f] = 1;
		return S;
	}
};
