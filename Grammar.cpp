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
        logger(CString("�������󣺱���������"), _T("error"));
        throw std::out_of_range("�������󣺱���������");
        return -1;
    }
    return S1[SYM];
}

void CCompileDlg::GEN(FCT X, int Y, int Z) {
    if (CX > CXMAX) {
        logger(_T("PROGRAM TOO LONG"), _T("error"));
        fprintf(FOUT, "PROGRAM TOO LONG\n");
        fclose(FOUT);
        throw std::out_of_range("PROGRAM TOO LONG");
    }
    CODE[CX].F = X;  // ���ú�������
    CODE[CX].L = Y;  // ���ò㼶
    CODE[CX].A = Z;  // ����ƫ�Ƶ�ַ
    CX++;  // ������������
}

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
            else Error(2);
        }
        else Error(3);
    }
    else Error(4);
}

void CCompileDlg::FACTOR(SYMSET FSYS, int LEV, int& TX) {
    int i;

    // ��鵱ǰ�����Ƿ��ڿ�ʼ���ż�����
    TEST(FACBEGSYS, FSYS, 24);

    // �����ǰ�����ڿ�ʼ���ż����У����д���
    while (SymIn(SYM, FACBEGSYS)) {
        if (SYM == IDENT) {
            i = POSITION(ID, TX);  // ���ұ�ʶ��
            if (i == 0) 
                Error(11);
            else {
                // ���ݱ�ʶ�������ͽ��д���
                switch (TABLE[i].KIND) {
                case CONSTANT:  // ����
                    GEN(LIT, 0, TABLE[i].VAL);
                    break;
                case VARIABLE:  // ����
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    // �����׺����
                    GetSym();  
                    if (SYM == INCREMENT) {
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // ����װ�ر���ֵ
                        GEN(LIT, 0, 1);
                        GEN(OPR, 0, 2);  // �ӷ�
                        GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // ���±���ֵ
                        GetSym();
                    }
                    else if (SYM == DECREMENT) {
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // ����װ�ر���ֵ
                        GEN(LIT, 0, 1);
                        GEN(OPR, 0, 3);  // ����
                        GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);  // ���±���ֵ
                        GetSym();
                    }
                    break;
                case PROCEDUR:  // ����
                    Error(21);
                    break;
                }
            }
        }
        else if (SYM == NUMBER) {  // ����
            if (NUM > AMAX) {  
                Error(31);
                NUM = 0;
            }
            GEN(LIT, 0, NUM);  // ����
            GetSym();
        }
        else if (SYM == LPAREN) {  // ���ʽ
            GetSym();
            // �������ʽ
            EXPRESSION(SymSetAdd(RPAREN, FSYS), LEV, TX);
            if (SYM == RPAREN)
                GetSym();
            else
                Error(22);
        }

        TEST(FSYS, FACBEGSYS, 23);
    }
}

// ����˷��ͳ�������
void CCompileDlg::TERM(SYMSET FSYS, int LEV, int& TX) {
    SYMBOL MULOP;

    // ��������
    FACTOR(SymSetUnion(FSYS, SymSetNew(TIMES, SLASH)), LEV, TX);

    while (SYM == TIMES || SYM == SLASH) {
        MULOP = SYM;
        GetSym();

        FACTOR(SymSetUnion(FSYS, SymSetNew(TIMES, SLASH)), LEV, TX);

        // �˷�
        if (MULOP == TIMES)
            GEN(OPR, 0, 4);
        // ����
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
        // ����ǰ׺����
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
                // ִ���������Լ����������޸ĺ��ֵ
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

// ��������
void CCompileDlg::VarDeclaration(int LEV, int& TX, int& DX) {
    if (SYM == IDENT) { 
        ENTER(VARIABLE, LEV, TX, DX); 
        GetSym(); 
    }
    else 
        Error(4);
}

// ����
void CCompileDlg::TEST(SYMSET S1, SYMSET S2, int N) {
    if (!SymIn(SYM, S1)) { // ������Ų���S1��
        Error(N);
        while (!SymIn(SYM, SymSetUnion(S1, S2))) { // �����Ų���S1��S2�Ĳ�����ʱ
            GetSym();
        }
    }
}

// �ڱ��в��ұ�ʶ��
int CCompileDlg::POSITION(ALFA ID, int TX) {
    int i = TX;
    strcpy_s(TABLE[0].NAME, sizeof(TABLE[0].NAME), ID);
    while (strcmp(TABLE[i].NAME, ID) != 0) 
        i--;
    return i;
}

// �����������ʽ
void CCompileDlg::CONDITION(SYMSET FSYS, int LEV, int& TX) {
    SYMBOL RELOP;
    if (SYM == ODDSYM) {
        GetSym();
        EXPRESSION(FSYS, LEV, TX);
        GEN(OPR, 0, 6);
    }
    else {
        // �����ϵ�����
        EXPRESSION(SymSetUnion(SymSetNew(EQL, NEQ, LSS, LEQ, GTR, GEQ), FSYS), LEV, TX);
        if (!SymIn(SYM, SymSetNew(EQL, NEQ, LSS, LEQ, GTR, GEQ))) Error(20);
        else {
            RELOP = SYM; GetSym(); EXPRESSION(FSYS, LEV, TX);
            switch (RELOP) {
            case EQL: GEN(OPR, 0, 8);  break;  // ����
            case NEQ: GEN(OPR, 0, 9);  break;  // ������
            case LSS: GEN(OPR, 0, 10); break;  // С��
            case GEQ: GEN(OPR, 0, 11); break;  // ���ڵ���
            case GTR: GEN(OPR, 0, 12); break;  // ����
            case LEQ: GEN(OPR, 0, 13); break;  // С�ڵ���
            }
        }
    }
}

void CCompileDlg::STATEMENT(SYMSET FSYS, int LEV, int& TX) {   /*STATEMENT*/
    int i, CX1, CX2;
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
                        GEN(OPR, 0, 2);  // �ӷ�
                        GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    }
                    else if (op == DECREMENT) {
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                        GEN(LIT, 0, 1);
                        GEN(OPR, 0, 3);  // ����
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
            // ��׺����
            if (SYM == INCREMENT || SYM == DECREMENT) {
                op = SYM;
                GetSym();
                if (op == INCREMENT) {
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LIT, 0, 1);
                    GEN(OPR, 0, 2);  // �ӷ�
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
                else if (op == DECREMENT) {
                    GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                    GEN(LIT, 0, 1);
                    GEN(OPR, 0, 3);  // ����
                    GEN(STO, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                }
            }
            else if (SYM == BECOMES || SYM == PLUSEQUAL || SYM == MINUSEQUAL || SYM == TIMESEQUAL || SYM == SLASHEQUAL) {
                op = SYM;
                GetSym();
                EXPRESSION(FSYS, LEV, TX);
                if (i != 0) {
                    if (op != BECOMES) {
                        // �ȼ��ر����ľ�ֵ
                        GEN(LOD, LEV - TABLE[i].vp.LEVEL, TABLE[i].vp.ADR);
                        // ִ����Ӧ������
                        switch (op) {
                            case PLUSEQUAL: GEN(OPR, 0, 2); break;
                            case MINUSEQUAL: GEN(OPR, 0, 3); break;
                            case TIMESEQUAL: GEN(OPR, 0, 4); break;
                            case SLASHEQUAL: GEN(OPR, 0, 5); break;
                        }
                    }
                    // �洢���
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
                Error(16);  // ��then�� expected

            CX1 = CX;
            GEN(JPC, 0, 0);
            STATEMENT(FSYS, LEV, TX);

            GetSym();  // ���Զ�ȡ��һ������

            if (SYM == ELSESYM) {
                // ����ELSE����
                CODE[CX1].A = CX + 1;  // ����JPC����ת��ַ��ELSE���ֵ���һλ��
                CX2 = CX;  // ��¼������ELSE�����ֵ���ת��ַ
                GEN(JMP, 0, 0);  // ������������ת����THEN��������

                GetSym();
                STATEMENT(FSYS, LEV, TX);
                CODE[CX2].A = CX;  // ����JMP����ת��ַ
            }
            else {
                Retreat();
                CODE[CX1].A = CX;
            }
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

    for (int i = CX0; i < CX; i++) {
        CString s;
        s.Format(_T("%3d"), i);
        CString mnemonic(MNEMONIC[CODE[i].F]);
        CString line;
        line.Format(_T("%s %s %4d %4d"), s, mnemonic.GetString(), CODE[i].L, CODE[i].A);
        logger(line, _T("info"));

        fprintf(FOUT, "%3d%5s%4d%4d\n", i, MNEMONIC[CODE[i].F], CODE[i].L, CODE[i].A);
    }
}


void CCompileDlg::Block(int LEV, int TX, SYMSET FSYS) {
    int DX = 3;    /*DATA ALLOCATION INDEX*/
    int TX0 = TX;  /*INITIAL TABLE INDEX*/
    int CX0 = CX;  /*INITIAL CODE INDEX*/
    TABLE[TX].vp.ADR = CX; GEN(JMP, 0, 0);
    if (LEV > LEVMAX) Error(32);
    do {
        if (SYM == CONSTSYM) {
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
        if (SYM == VARSYM) {
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
        while (SYM == PROCSYM) {
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
            Block(LEV + 1, TX, SymSetAdd(SEMICOLON, FSYS));
            if (SYM == SEMICOLON) {
                GetSym();
                TEST(SymSetUnion(SymSetNew(IDENT, PROCSYM), STATBEGSYS), FSYS, 6);
            }
            else Error(5);
        }
        TEST(SymSetAdd(IDENT, STATBEGSYS), DECLBEGSYS, 7);
    } while (SymIn(SYM, DECLBEGSYS));
    CODE[TABLE[TX0].vp.ADR].A = CX;
    TABLE[TX0].vp.ADR = CX;   /*START ADDR OF CODE*/
    TABLE[TX0].vp.SIZE = DX;  /*SIZE OF DATA SEGMENT*/
    GEN(INI, 0, DX);
    STATEMENT(SymSetUnion(SymSetNew(SEMICOLON, ENDSYM), FSYS), LEV, TX);
    GEN(OPR, 0, 0);  /*RETURN*/
    TEST(FSYS, SymSetNULL(), 8);
    ListCode(CX0);
}