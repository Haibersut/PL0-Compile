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
    if (needRetreat) {
        CC = prevCC;
        needRetreat = false;
        return;
    }

    prevCC = CC;  // ��¼��ǰ�ַ�λ���Ա���ܵĻ���

    if (CC == LL) {
        if (feof(FIN)) {
            logger(_T("PROGRAM INCOMPLETE"), _T("error"));
            fprintf(FOUT, "PROGRAM INCOMPLETE\n");
            fclose(FIN);
            throw std::runtime_error("PROGRAM INCOMPLETE");
        }
        LL = 0;
        CC = 0;
        CH = ' ';
        while (!feof(FIN) && CH != 10) {
            CH = fgetc(FIN);
            LINE[LL++] = CH;
        }

        LINE[LL - 1] = ' ';
        LINE[LL] = 0;

        CString s;
        s.Format(_T("%d"), CX);
        while (s.GetLength() < 3) s = _T(" ") + s;
        s = s + _T(" ") + CString(LINE);
        logger(s, _T("info"));
        fprintf(FOUT, "%s\n", CStringA(s).GetString());
    }

    CH = LINE[CC++];
}

void CCompileDlg::GetSym() {
    int i, J, K;    // ����ѭ������������
    ALFA A;         // �����ʶ������

    double realPart = 0.0;
    double fractionPart = 0.0;
    double fractionDivisor = 1.0;
    bool isFraction = false;

    while (CH != EOF && CH <= ' ') {
        GetCh(); // �����հ��ַ�
    }

    if (isalpha(CH)) {
        K = 0;
        do {
            if (K < AL) 
                A[K++] = CH;
            GetCh();
        } while (isalpha(CH) || isdigit(CH));
        A[K] = '\0';
        strcpy_s(ID, sizeof(ID), A);
        i = 1; 
        J = NORW;
        do {
            K = (i + J) / 2;
            if (_stricmp(ID, KWORD[K]) <= 0)
                J = K - 1;
            if (_stricmp(ID, KWORD[K]) >= 0)
                i = K + 1;
        } while (i <= J);
        if (i - 1 > J) {
            SYM = WSYM[K];
        }
        else {// ����SYMΪ��ʶ��
            SYM = IDENT;
        }
    }
    else if (isdigit(CH)) { // ��������֣�������ֵ
        K = 0;
        NUM = 0;
        SYM = NUMBER;
        do {
            NUM = 10 * NUM + (CH - '0');
            K++;
            GetCh();
        } while (isdigit(CH));
        if (CH == '.') {
            isFraction = true;
            GetCh();
            while (isdigit(CH)) {
                fractionPart = 10 * fractionPart + (CH - '0');
                fractionDivisor *= 10;
                GetCh();
            }
            REALNUM = NUM + fractionPart / fractionDivisor;
            SYM = REALSYM;
        }
        else {
            REALNUM = NUM;
        }
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
            GetCh();
        }
        else if (CH == '=') {
            SYM = PLUSEQUAL;
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
            GetCh();
        }
        else if (CH == '=') {
            SYM = MINUSEQUAL;
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
            GetCh();
        }
        else if (CH == '/') { // ��鵥��ע��
            while (CC != LL) { // ����ֱ����β�������ַ�
                GetCh();
            }
            GetSym();
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
    else if (CH == '\'') {
        GetCh();
        CHVAR = CH;
        GetCh();
        if (CH == '\'') {
            SYM = CHARSYM;
            GetCh();
        }
        else {
            Error(31); // ȱ�ٽ����ĵ�����
        }
    }
    else { // �����ַ�
        if (CH >= 128) {
            logger(CString("����������Ч���ַ�"), _T("error"));
            throw std::runtime_error("����������Ч���ַ�");
        }
        SYM = SSYM[CH]; // �����ַ�����SYM
        GetCh();
    }
}