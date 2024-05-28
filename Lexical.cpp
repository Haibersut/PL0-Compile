#include "pch.h"
#include "CompileDlg.h"

#include <stdexcept>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include "afxdialogex.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

void CCompileDlg::GetCh() {
	if (CC == LL) {
		if (feof(FIN)) {
			logger(_T("PROGRAM INCOMPLETE"), _T("error"));
			fprintf(FOUT, "PROGRAM INCOMPLETE\n");
            fclose(FIN);
			fclose(FOUT);
            CH = EOF;
            SYM = EOFSYM;
            return;
		}
		LL = 0; 
        CC = 0;
		CH = ' ';
		while (!feof(FIN) && CH != 10) {
			CH = fgetc(FIN);
            LINE[LL++] = CH;
			if (CH == EOF) {
				break;
			}
		}
        if (LL > 0) 
            LINE[LL - 1] = ' ';
        LINE[LL] = 0;
		CString s;
		s.Format(_T("%d"), CX);
		while (s.GetLength() < 3) s = _T(" ") + s;
		s = s + _T(" ") + CString(LINE);
		logger(s, _T("info"));
		fprintf(FOUT, "%s\n", CStringA(s).GetString());
	}
	if (CH == EOF) {
		SYM = EOFSYM;
		return;
	}
	CH = LINE[CC++];
}

void CCompileDlg::GetSym() {
    int i, J, K;    // ����ѭ������������
    ALFA A;         // �����ʶ������

    while (CH!=EOF && CH <= ' ') {
        GetCh(); // �����հ��ַ�
    }

    if (CH >= 'A' && CH <= 'Z') {
        K = 0;
        do {
            if (K < AL) 
                A[K++] = CH;
            GetCh();
        } while ((CH >= 'A' && CH <= 'Z') || (CH >= '0' && CH <= '9'));
        A[K] = '\0';
        strcpy_s(ID, sizeof(ID), A);
        i = 1; 
        J = NORW;
        do {
            K = (i + J) / 2;
            if (strcmp(ID, KWORD[K]) <= 0) 
                J = K - 1;
            if (strcmp(ID, KWORD[K]) >= 0) 
                i = K + 1;
        } while (i <= J);
        if (i - 1 > J) {
            SYM = WSYM[K];
            char temp[50];
            sprintf_s(temp, sizeof(temp), "FOUND KEYWORD: %s", ID);
            logger(CString(temp), _T("success"));
        }
        else {// ����SYMΪ��ʶ��
            SYM = IDENT;
        }
    }
    else if (CH >= '0' && CH <= '9') { // ��������֣�������ֵ
        K = 0; 
        NUM = 0; 
        SYM = NUMBER;
        do {
            NUM = 10 * NUM + (CH - '0');
            K++; 
            GetCh();
        } while (CH >= '0' && CH <= '9'); 
        if (K > NMAX) 
            Error(30);
    }
    else if (CH == ':') { // ��鸳ֵ����
        GetCh();
        if (CH == '=') {
            SYM = BECOMES; 
            GetCh(); 
        }
        else 
            SYM = NUL;
    }
    else if (CH == '+') { // ���ӺŻ����������
        GetCh();
        if (CH == '+') {
            SYM = INCREMENT;
            logger(_T("FOUND INCREMENT OPERATOR"), _T("success"));
            GetCh();
        }
        else if (CH == '=') {
            SYM = PLUSEQUAL;
            logger(_T("FOUND PLUSEQUAL OPERATOR"), _T("success"));
            GetCh();
        }
        else {
            SYM = PLUS;
        }
    }
    else if (CH == '-') { // �����Ż��Լ������
        GetCh();
        if (CH == '-') {
            SYM = DECREMENT;
            logger(_T("FOUND DECREMENT OPERATOR"), _T("success"));
            GetCh();
        }
        else if (CH == '=') {
            SYM = MINUSEQUAL;
            logger(_T("FOUND MINUSEQUAL OPERATOR"), _T("success"));
            GetCh();
        }
        else {
            SYM = MINUS;
        }
    }
    else if (CH == '*') { // ���˺Ż�˵������
        GetCh();
        if (CH == '=') {
            SYM = TIMESEQUAL;
            logger(_T("FOUND TIMESEQUAL OPERATOR"), _T("success"));
            GetCh();
        }
        else {
            SYM = TIMES;
        }
    }
    else if (CH == '/') { // �����Ż���������
        GetCh();
        if (CH == '=') {
            SYM = SLASHEQUAL;
            logger(_T("FOUND SLASHEQUAL OPERATOR"), _T("success"));
            GetCh();
        }
        else {
            SYM = SLASH;
        }
    }
    else if (CH == '<') { // ���С��/С�ڵ���/�����ڷ���
        GetCh();
        if (CH == '=') { 
            SYM = LEQ; 
            GetCh(); 
        }
        else if (CH == '>') {
            SYM = NEQ;
            logger(_T("FOUND NEQ OPERATOR"), _T("success"));
            GetCh();
        }
        else {
            SYM = LSS;
        }
    }
    else if (CH == '>') { // �����ڻ���ڵ��ڷ���
        GetCh();
        if (CH == '=') { 
            SYM = GEQ; 
            GetCh(); 
        } 
        else SYM = GTR;
    }
    else { // �����ַ�
        if (CH > '^') {
            logger(CString("����������Ч���ַ�"), _T("error"));
            throw std::runtime_error("����������Ч���ַ�");
            return;
        }
        SYM = SSYM[CH]; // �����ַ�����SYM
        GetCh();
    }
}