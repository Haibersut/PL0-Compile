#include "pch.h"
#include "CompileDlg.h"
#include "Compile.h"

#include <stdexcept>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

int CCompileDlg::SymIn(SYMBOL SYM, SYMSET S1) {
    if (SYM < 0 || SYM >= EOFSYM) {
        logger(CString("致命错误：编译程序错误"), _T("error"));
        throw std::out_of_range("致命错误：编译程序错误");
        return -1;
    }
    return S1[SYM];
}

void CCompileDlg::GEN(FCT X, int Y, double Z) {
    if (CX > CXMAX) {
        logger(_T("PROGRAM TOO LONG"), _T("error"));
        fprintf(FOUT, "PROGRAM TOO LONG\n");
        fclose(FOUT);
        throw std::out_of_range("PROGRAM TOO LONG");
    }
    CODE[CX].F = X;  // 设置函数代码
    CODE[CX].L = Y;  // 设置层级
    CODE[CX].A = Z;  // 设置偏移地址
    CX++;  // 代码索引增加
}

// 将对象（常量、变量或过程）插入到符号表中
void CCompileDlg::ENTER(OBJECTS K, int LEV, int& TX, int& DX) { /*ENTER OBJECT INTO TABLE*/
    TX++;
    strcpy_s(TABLE[TX].NAME, sizeof(TABLE[TX].NAME), ID);
    TABLE[TX].KIND = K;
    switch (K) {
        case CONSTANT:
            if (NUM > AMAX) { 
                Error(31); 
                NUM = 0; 
            }
            TABLE[TX].VAL = NUM;
            break;
        case CHAR:
            TABLE[TX].CVAL = CHVAR;
            break;
        case REAL:
            TABLE[TX].RVAL = REALNUM;
            break;
        case VARIABLE:
            TABLE[TX].vp.LEVEL = LEV; 
            TABLE[TX].vp.ADR = DX; 
            DX++;
            break;
        case PROCEDUR:
            TABLE[TX].vp.LEVEL = LEV;
            break;
    }
}

// 常量声明函数
void CCompileDlg::ConstDeclaration(int LEV, int& TX, int& DX) {
    if (SYM == IDENT) {
        GetSym();
        if (SYM == EQL || SYM == BECOMES) {
            if (SYM == BECOMES) 
                Error(1);
            GetSym();
            if (SYM == NUMBER) { 
                ENTER(CONSTANT, LEV, TX, DX); 
                GetSym(); 
            }
            else if (SYM == CHARSYM) {
                ENTER(CHAR, LEV, TX, DX);
                GetSym();
            }
            else if (SYM == REALSYM) {
                ENTER(REAL, LEV, TX, DX);
                GetSym();
            }
            else Error(2);
        }
        else Error(3);
    }
    else Error(4);
}

void CCompileDlg::FACTOR(SYMSET FSYS, int LEV, int& TX) {
    int i;

    // 检查当前符号是否在开始符号集合中
    TEST(FACBEGSYS, FSYS, 24);

    // 如果当前符号在开始符号集合中，进行处理
    while (SymIn(SYM, FACBEGSYS)) {
        if (SYM == IDENT) {
            i = POSITION(ID, TX);  // 查找标识符
            if (i == 0) 
                Error(11);
            else {
                // 根据标识符的类型进行处理
                switch (TABLE[i].KIND) {
                    case CONSTANT:  // 常量
                        GEN(LIT, 0, TABLE[i].VAL);
                        break;
                    case CHAR:  // 字符常量
                        GEN(LIT, 0, TABLE[i].CVAL);
                        break;
                    case REAL:  // 实数常量
                        GEN(LIT, 0, TABLE[i].RVAL);
                        break;
                    case VARIABLE:  // 变量
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                        // 处理后缀操作
                        GetSym();  
                        if (SYM == INCREMENT) {
                            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 重新装载变量值
                            GEN(LIT, 0, 1);
                            GEN(OPR, 0, 2);  // 加法
                            GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 更新变量值
                            GetSym();
                        }
                        else if (SYM == DECREMENT) {
                            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 重新装载变量值
                            GEN(LIT, 0, 1);
                            GEN(OPR, 0, 3);  // 减法
                            GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 更新变量值
                            GetSym();
                        }
                        break;
                    case PROCEDUR:  // 过程
                        Error(21);
                        break;
                }
            }
        }
        else if (SYM == NUMBER) {  // 数字
            if (NUM > AMAX) {  
                Error(31);
                NUM = 0;
            }
            GEN(LIT, 0, NUM);  // 常量
            GetSym();
        }
        else if (SYM == CHARSYM) {  // 字符常量
            GEN(LIT, 0, CHVAR);  
            GetSym();
        }
        else if (SYM == REALSYM) {  // 实数常量
            GEN(LIT, 0, REALNUM);  
            GetSym();
        }
        else if (SYM == LPAREN) {  // 表达式
            GetSym();
            // 解析表达式
            EXPRESSION(SymSetAdd(RPAREN, FSYS), LEV, TX);
            if (SYM == RPAREN)
                GetSym();
            else
                Error(22);
        }

        TEST(FSYS, FACBEGSYS, 23);
    }
}

// 处理乘法和除法操作
void CCompileDlg::TERM(SYMSET FSYS, int LEV, int& TX) {
    SYMBOL MULOP;

    // 处理因子
    FACTOR(SymSetUnion(FSYS, SymSetNew(TIMES, SLASH, CHARSYM, REALSYM)), LEV, TX);

    while (SYM == TIMES || SYM == SLASH) {
        MULOP = SYM;
        GetSym();

        FACTOR(SymSetUnion(FSYS, SymSetNew(TIMES, SLASH, CHARSYM, REALSYM)), LEV, TX);

        // 乘法
        if (MULOP == TIMES)
            GEN(OPR, 0, 4);
        // 除法
        else
            GEN(OPR, 0, 5);
    }
}

void CCompileDlg::EXPRESSION(SYMSET FSYS, int LEV, int& TX) {
    SYMBOL ADDOP;
    int i;

    if (SYM == PLUS || SYM == MINUS) {
        ADDOP = SYM; 
        GetSym();
        TERM(SymSetUnion(FSYS, SymSetNew(PLUS, MINUS)), LEV, TX);
        if (ADDOP == MINUS)
            GEN(OPR, 0, 1); // NEG
    }
    else if (SYM == INCREMENT || SYM == DECREMENT) {
        // 处理前缀操作
        ADDOP = SYM;
        GetSym();
        if (SYM == IDENT) {
            i = POSITION(ID, TX);
            if (i == 0) {
                Error(11);
            }
            else if (TABLE[i].KIND != VARIABLE) {
                Error(12);
                i = 0;
            }
            else {
                // 执行自增或自减，并载入修改后的值
                if (ADDOP == INCREMENT) {
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LIT, 0, 1);
                    GEN(OPR, 0, 2);
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
                else if (ADDOP == DECREMENT) {
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LIT, 0, 1);
                    GEN(OPR, 0, 3);
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
            }
            GetSym();
        }
        else {
            Error(10);
        }
    }
    else {
        TERM(SymSetUnion(FSYS, SymSetNew(PLUS, MINUS)), LEV, TX);
    }
       
    while (SYM == PLUS || SYM == MINUS) {
        ADDOP = SYM; 
        GetSym();
        TERM(SymSetUnion(FSYS, SymSetNew(PLUS, MINUS)), LEV, TX);
        if (ADDOP == PLUS)
            GEN(OPR, 0, 2);
        else
            GEN(OPR, 0, 3);
    }
}

// 变量声明
void CCompileDlg::VarDeclaration(int LEV, int& TX, int& DX) {
    if (SYM == IDENT) { 
        ENTER(VARIABLE, LEV, TX, DX); 
        GetSym(); 
    }
    else if (SYM == CHARSYM) {
        ENTER(CHAR, LEV, TX, DX);
        GetSym();
    }
    else if (SYM == REALSYM) {
        ENTER(REAL, LEV, TX, DX);
        GetSym();
    }
    else 
        Error(4);
}

// 测试
void CCompileDlg::TEST(SYMSET S1, SYMSET S2, int N) {
    if (!SymIn(SYM, S1)) { // 如果符号不在S1中
        Error(N);
        while (!SymIn(SYM, SymSetUnion(S1, S2))) { // 当符号不在S1和S2的并集中时
            GetSym();
        }
    }
}

// 在表中查找标识符
int CCompileDlg::POSITION(ALFA ID, int TX) {
    int i = TX;
    strcpy_s(TABLE[0].NAME, sizeof(TABLE[0].NAME), ID);
    while (strcmp(TABLE[i].NAME, ID) != 0) 
        i--;
    return i;
}

// 处理条件表达式
void CCompileDlg::CONDITION(SYMSET FSYS, int LEV, int& TX) {
    SYMBOL RELOP;
    if (SYM == ODDSYM) {
        GetSym();
        EXPRESSION(FSYS, LEV, TX);
        GEN(OPR, 0, 6);
    }
    else {
        // 处理关系运算符
        EXPRESSION(SymSetUnion(SymSetNew(EQL, NEQ, LSS, LEQ, GTR, GEQ), FSYS), LEV, TX);
        if (!SymIn(SYM, SymSetNew(EQL, NEQ, LSS, LEQ, GTR, GEQ))) Error(20);
        else {
            RELOP = SYM; GetSym(); EXPRESSION(FSYS, LEV, TX);
            switch (RELOP) {
            case EQL: GEN(OPR, 0, 8);  break;  // 等于
            case NEQ: GEN(OPR, 0, 9);  break;  // 不等于
            case LSS: GEN(OPR, 0, 10); break;  // 小于
            case GEQ: GEN(OPR, 0, 11); break;  // 大于等于
            case GTR: GEN(OPR, 0, 12); break;  // 大于
            case LEQ: GEN(OPR, 0, 13); break;  // 小于等于
            }
        }
    }
}

void CCompileDlg::STATEMENT(SYMSET FSYS, int LEV, int& TX) {   /*STATEMENT*/
    int i, CX1, CX2, CX3, CX4;
    SYMBOL op;

    switch (SYM) {
        case INCREMENT:
        case DECREMENT:
            op = SYM;
            GetSym();
            if (SYM == IDENT) {
                i = POSITION(ID, TX);
                if (i == 0)
                    Error(11);
                else if (TABLE[i].KIND != VARIABLE) {
                    Error(12); 
                    i = 0;
                }
                else {
                    if (op == INCREMENT) {
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                        GEN(LIT, 0, 1);
                        GEN(OPR, 0, 2);  // 加法
                        GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    }
                    else if (op == DECREMENT) {
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                        GEN(LIT, 0, 1);
                        GEN(OPR, 0, 3);  // 减法
                        GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    }
                }
                GetSym();
            }
            break;
        case IDENT:
            i = POSITION(ID, TX);
            if (i == 0)
                Error(11);
            else if (TABLE[i].KIND != VARIABLE) {
                Error(12); 
                i = 0;
            }
            GetSym();
            // 后缀操作
            if (SYM == INCREMENT || SYM == DECREMENT) {
                op = SYM;
                GetSym();
                if (op == INCREMENT) {
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LIT, 0, 1);
                    GEN(OPR, 0, 2);  // 加法
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
                else if (op == DECREMENT) {
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LIT, 0, 1);
                    GEN(OPR, 0, 3);  // 减法
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
            }
            else if (SYM == BECOMES || SYM == PLUSEQUAL || SYM == MINUSEQUAL || SYM == TIMESEQUAL || SYM == SLASHEQUAL) {
                op = SYM;
                GetSym();
                EXPRESSION(FSYS, LEV, TX);
                if (i != 0) {
                    if (op != BECOMES) {
                        // 先加载变量的旧值
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                        // 执行相应的运算
                        switch (op) {
                            case PLUSEQUAL: GEN(OPR, 0, 2); break;
                            case MINUSEQUAL: GEN(OPR, 0, 3); break;
                            case TIMESEQUAL: GEN(OPR, 0, 4); break;
                            case SLASHEQUAL: GEN(OPR, 0, 5); break;
                        }
                    }
                    // 存储结果
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
            }
            else {
                Error(13);
            }
            break;
        case READSYM:
            GetSym();
            if (SYM != LPAREN) Error(34);
            else
                do {
                    GetSym();
                    if (SYM == IDENT) 
                        i = POSITION(ID, TX);
                    else 
                        i = 0;
                    if (i == 0) 
                        Error(35);
                    else {
                        GEN(OPR, 0, 16);
                        GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    }
                    GetSym();
                } while (SYM == COMMA);
                if (SYM != RPAREN) {
                    Error(33);
                    while (!SymIn(SYM, FSYS)) GetSym();
                }
                else GetSym();
                break; /* READSYM */
        case WRITESYM:
            GetSym();
            if (SYM == LPAREN) {
                do {
                    GetSym();
                    EXPRESSION(SymSetUnion(SymSetNew(RPAREN, COMMA), FSYS), LEV, TX);
                    GEN(OPR, 0, 14);
                } while (SYM == COMMA);
                if (SYM != RPAREN) 
                    Error(33);
                else 
                    GetSym();
            }
            GEN(OPR, 0, 15);
            break; /*WRITESYM*/
        case CALLSYM:
            GetSym();
            if (SYM != IDENT) 
                Error(14);
            else {
                i = POSITION(ID, TX);
                if (i == 0) 
                    Error(11);
                else
                    if (TABLE[i].KIND == PROCEDUR)
                        GEN(CAL, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    else 
                        Error(15);
                GetSym();
            }
            break;
        case IFSYM:
            GetSym();
            CONDITION(SymSetUnion(SymSetNew(THENSYM, DOSYM), FSYS), LEV, TX);
            if (SYM == THENSYM)
                GetSym();
            else
                Error(16);  // “then” expected

            CX1 = CX;
            GEN(JPC, 0, 0);
            STATEMENT(FSYS, LEV, TX);

            GetSym();  // 尝试读取下一个符号

            if (SYM == ELSESYM) {
                // 处理ELSE部分
                CODE[CX1].A = CX + 1;  // 更新JPC的跳转地址到ELSE部分的下一位置
                CX2 = CX;  // 记录跳过“ELSE”部分的跳转地址
                GEN(JMP, 0, 0);  // 生成无条件跳转，从THEN部分跳出

                GetSym();
                STATEMENT(FSYS, LEV, TX);
                CODE[CX2].A = CX;  // 更新JMP的跳转地址
            }
            else {
                Retreat();
                CODE[CX1].A = CX;
                STATEMENT(FSYS, LEV, TX);
            }
            break;
        case FORSYM:
            GetSym();
            if (SYM != IDENT)
                Error(4);  // 应该是变量名
            i = POSITION(ID, TX);
            if (i == 0)
                Error(11);  // 未声明的标识符
            if (TABLE[i].KIND != VARIABLE)
                Error(12);  // 不是变量
            GetSym();
            if (SYM != BECOMES)
                Error(13);  // 缺少赋值符号
            GetSym();
            EXPRESSION(SymSetUnion(FSYS, SymSetNew(STEPSYM, UNTILSYM, DOSYM)), LEV, TX);
            GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 初始值赋给变量

            CX1 = CX;  // 记录循环开始位置
            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 加载变量

            if (SYM != STEPSYM)
                Error(24);  // 缺少STEP
            GetSym();
            EXPRESSION(SymSetUnion(FSYS, SymSetNew(UNTILSYM, DOSYM)), LEV, TX);
            GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR + 1);  // 保存步长

            if (SYM != UNTILSYM)
                Error(25);  // 缺少UNTIL
            GetSym();
            EXPRESSION(SymSetUnion(FSYS, SymSetNew(DOSYM)), LEV, TX);
            GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR + 2);  // 保存终止条件

            CX2 = CX;  // 准备跳转的地方
            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 加载变量
            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR + 2);  // 加载终止条件
            GEN(OPR, 0, 13);  // OPR 0,13 <= 比较变量和终止条件
            CX3 = CX;
            GEN(JPC, 0, 0);  // 处理终止条件

            if (SYM != DOSYM)
                Error(26);  // 缺少DO
            GetSym();
            STATEMENT(FSYS, LEV, TX);

            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 加载变量
            GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR + 1);  // 加载步长
            GEN(OPR, 0, 2);  // OPR 0,2 + 变量 + 步长
            GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // 保存变量

            GEN(JMP, 0, CX1);  // 跳回循环开始
            CODE[CX3].A = CX;  // 回填JPC的跳转地址

            break;
        case BEGINSYM:
            GetSym();
            STATEMENT(SymSetUnion(SymSetNew(SEMICOLON, ENDSYM), FSYS), LEV, TX);
            while (SymIn(SYM, SymSetAdd(SEMICOLON, STATBEGSYS))) {
                if (SYM == SEMICOLON) 
                    GetSym();
                else 
                    Error(10);
                STATEMENT(SymSetUnion(SymSetNew(SEMICOLON, ENDSYM), FSYS), LEV, TX);
            }
            if (SYM == ENDSYM) 
                GetSym();
            else
                Error(5);
            break;
        case WHILESYM:
            CX1 = CX; 
            GetSym(); 
            CONDITION(SymSetAdd(DOSYM, FSYS), LEV, TX);
            CX2 = CX; 
            GEN(JPC, 0, 0);
            if (SYM == DOSYM) 
                GetSym();
            else 
                Error(18);
            STATEMENT(FSYS, LEV, TX);
            GEN(JMP, 0, CX1);
            CODE[CX2].A = CX;
            break;
        case EOFSYM:
            return;
    }
    TEST(FSYS, SymSetNULL(), 19);
}

void CCompileDlg::ListCode(int CX0) {

    CButton* pCheck = (CButton*)GetDlgItem(IDC_CHECK1);
    int nCheck = pCheck->GetCheck();

    if (nCheck == 0) {
        return;
    }

    logger(_T("--- START OF BLOCK ---"), _T("debug"));

    for (int i = CX0; i < CX; i++) {
        CString s;
        s.Format(_T("%3d"), i);
        CString mnemonic(MNEMONIC[CODE[i].F]);
        CString line;
        line.Format(_T("%s %s %4d %4d"), s.GetString(), mnemonic.GetString(), CODE[i].L, CODE[i].A);
        logger(line, _T("info"));

        fprintf(FOUT, "%3d%5s%4d%4d\n", i, MNEMONIC[CODE[i].F], CODE[i].L, CODE[i].A);
    }

    logger(_T("--- END OF BLOCK ---"), _T("debug"));
}


void CCompileDlg::Block(int LEV, int TX, SYMSET FSYS) {
    int DX = 3;    // 数据分配索引
    int TX0 = TX;  // 初始表索引
    int CX0 = CX;  // 初始代码索引
    TABLE[TX].vp.ADR = CX; GEN(JMP, 0, 0);
    if (LEV > LEVMAX) 
        Error(32);
    do {
        if (SYM == CONSTSYM) { // 处理常量声明
            GetSym();
            do {
                ConstDeclaration(LEV, TX, DX);
                while (SYM == COMMA) {
                    GetSym();  
                    ConstDeclaration(LEV, TX, DX);
                }
                if (SYM == SEMICOLON) 
                    GetSym();
                else 
                    Error(5);
            } while (SYM == IDENT);
        }
        if (SYM == VARSYM) { // 处理变量声明
            GetSym();
            do {
                VarDeclaration(LEV, TX, DX);
                while (SYM == COMMA) { 
                    GetSym(); 
                    VarDeclaration(LEV, TX, DX); 
                }
                if (SYM == SEMICOLON) 
                    GetSym();
                else 
                    Error(5);
            } while (SYM == IDENT);
        }
        while (SYM == PROCSYM) { // 处理过程声明
            GetSym();
            if (SYM == IDENT) { 
                ENTER(PROCEDUR, LEV, TX, DX); 
                GetSym(); 
            }
            else 
                Error(4);
            if (SYM == SEMICOLON) 
                GetSym();
            else 
                Error(5);
            Block(LEV + 1, TX, SymSetAdd(SEMICOLON, FSYS)); // 处理过程体
            if (SYM == SEMICOLON) {
                GetSym();
                TEST(SymSetUnion(SymSetNew(IDENT, PROCSYM), STATBEGSYS), FSYS, 6);
            }
            else Error(5);
        }
        TEST(SymSetAdd(IDENT, STATBEGSYS), DECLBEGSYS, 7); // 检查声明部分是否结束
    } while (SymIn(SYM, DECLBEGSYS));
    CODE[TABLE[TX0].vp.ADR].A = CX; // 回填跳转地址
    TABLE[TX0].vp.ADR = CX;   // 记录代码开始地址
    TABLE[TX0].vp.SIZE = DX;  // 记录数据段大小
    GEN(INI, 0, DX); // 生成初始化指令
    STATEMENT(SymSetUnion(SymSetNew(SEMICOLON, ENDSYM), FSYS), LEV, TX);
    GEN(OPR, 0, 0);  // 生成返回指令
    TEST(FSYS, SymSetNULL(), 8); // 检查程序块结束
    ListCode(CX0);
}