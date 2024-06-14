#include "pch.h"
#include "CompileDlg.h"
#include "Compile.h"
#include "afxdialogex.h"
#include <stdio.h>
#include "CInputDialog.h"

int CCompileDlg::BASE(int L, int B, Data S[]) {
	int B1 = B; /*FIND BASE L LEVELS DOWN*/
	while (L > 0) {
		B1 = S[B1].i;
		L = L - 1;
	}
	return B1;
}

void CCompileDlg::Interpret() {
	const int STACKSIZE = 500;
	int P, B, T;        // 程序计数器，基址寄存器，栈顶寄存器
	INSTRUCTION I;      // 当前指令
	Data S[STACKSIZE];   // 数据存储

	memset(S,0, sizeof(S));

	logger(_T("~~~ RUN PL0 ~~~"), _T("info"));
	fprintf(FOUT, "~~~ RUN PL0 ~~~\n");

	// 初始化寄存器
	T = 0; 
	B = 1; 
	P = 0;

	do {
		I = CODE[P]; 
		P = P + 1;
		switch (I.F) {
			case LIT: 
				T++; 
				if (I.D != 0.0) { // 浮点数处理
					if (S[T].i != 0) {
						S[T].i = 0;
					}
					S[T].d = I.D;
				}
				else { // 整数处理
					S[T].i = I.A;
				}
				break;
			case OPR:
				// 运算符处理
				switch (I.A) {
					case 0: /*RETURN*/ 
						T = B - 1; 
						P = S[T + 3].i; 
						B = S[T + 2].i;
						break;
					case 1: /*NEG*/
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].d = -S[T].d;
						}
						else { // 整数处理
							S[T].i = -S[T].i;
						}
						break;
					case 2: /*ADD*/
						T--; 
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].d = S[T].d + S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i + S[T + 1].i;
						}
						break;
					case 3: /*SUB*/
						T--; 
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].d = S[T].d - S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i - S[T + 1].i;
						}
						break;
					case 4: /*MUL*/
						T--; 
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].d = S[T].d * S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i * S[T + 1].i;
						}
						break;
					case 5: /*DIV*/
						T--; 
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].d = S[T].d / S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i / S[T + 1].i;
						}
						break;
					case 6: // MOD
						if (S[T].d != 0.0) { // 浮点数处理
							Error(33);
						}
						else { // 整数处理
							S[T].i = S[T].i % S[T + 1].i;
						}
						break;
					case 8: // EQL
						T--;
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].i = fabs(S[T].d - S[T + 1].d) < 1e-9;
						}
						else { // 整数处理
							S[T].i = S[T].i == S[T + 1].i;
						}
						break;
					case 9: // NEQ
						T--;
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].i = fabs(S[T].d - S[T + 1].d) >= 1e-9;
						}
						else { // 整数处理
							S[T].i = S[T].i != S[T + 1].i;
						}
						break;
					case 10: // LSS
						T--;
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].i = S[T].d < S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i < S[T + 1].i;
						}
						break;
					case 11: // GEQ
						T--;
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].i = S[T].d >= S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i >= S[T + 1].i;
						}
						break;
					case 12: // GTR
						T--;
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].i = S[T].d > S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i > S[T + 1].i;
						}
						break;
					case 13: // LEQ
						T--;
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].i = S[T].d <= S[T + 1].d;
						}
						else { // 整数处理
							S[T].i = S[T].i <= S[T + 1].i;
						}
						break;
					case 14: // WRITE
						{
							CString str;
							if (S[T].d != 0.0) { // 浮点数处理
								str.Format(_T("%f"), S[T].d);
							}
							else { // 整数处理
								str.Format(_T("%d"), S[T].i);
							}
							logger(str, _T("info"));
							T--;
							break;
						}
					case 15: 
						{
							//logger(_T(""), _T("info"));
						}
						break;
					case 16: // WRITEC
						{
							CString str;
							if (S[T].d != 0.0) { // 浮点数处理
								str.Format(_T("%c"), (int)S[T].d);
							}
							else { // 整数处理
								str.Format(_T("%c"), S[T].i);
							}
							logger(str, _T("info"));
							T--;
							break;
						}
					case 17: /*NOT*/
						if (S[T].d != 0.0) { // 浮点数处理
							S[T].d = !S[T].d;
						}
						else { // 整数处理
							S[T].i = !S[T].i;
						}
						break;
					case 18: // READ
						T++;
						CInputDialog inputDlg;
						if (inputDlg.DoModal() == IDOK)
						{
							S[T].i = _ttoi(inputDlg.m_input);
							CString message;
							message.Format(_T("? %d"), S[T].i);
							logger(message, _T("info"));
							fprintf(FOUT, "? %d\n", S[T].i);
						}
						break;
				}
				break;
			case LOD:
				T++; 
				S[T] = S[BASE(I.L, B, S) + I.A]; 
				break;
			case STO:
				S[BASE(I.L, B, S) + I.A] = S[T]; 
				T--; 
				break;
			case CAL: /*GENERAT NEW Block MARK*/
				S[T + 1].i = BASE(I.L, B, S); 
				S[T + 2].i = B; 
				S[T + 3].i = P;
				B = T + 1; 
				P = I.A; 
				break;
			case INI: 
				T = T + I.A;  
				break;
			case JMP: 
				P = I.A; 
				break;
			case JPC: 
				if (S[T].i == 0) 
					P = I.A;  
				T--;  
				break;
		} /*switch*/
	} while (P != 0);
	memset(CODE, 0, sizeof(CODE));
	logger(_T("~~~ END PL0 ~~~"), _T("info"));
	fprintf(FOUT, "~~~ END PL0 ~~~\n");
}