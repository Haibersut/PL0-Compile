#include "pch.h"
#include "CompileDlg.h"
#include "Compile.h"
#include "afxdialogex.h"
#include <stdio.h>
#include "CInputDialog.h"

int CCompileDlg::BASE(int L, int B, int S[]) {
	int B1 = B; /*FIND BASE L LEVELS DOWN*/
	while (L > 0) { 
		B1 = S[B1]; 
		L = L - 1; 
	}
	return B1;
}

void CCompileDlg::Interpret() {
	const int STACKSIZE = 500;
	int P, B, T;        // 程序计数器，基址寄存器，栈顶寄存器
	INSTRUCTION I;      // 当前指令
	int S[STACKSIZE];   // 数据存储

	memset(S,0, sizeof(S));

	logger(_T("~~~ RUN PL0 ~~~"), _T("info"));
	fprintf(FOUT, "~~~ RUN PL0 ~~~\n");

	// 初始化寄存器
	T = 0; 
	B = 1; 
	P = 0;
	S[1] = 0; 
	S[2] = 0; 
	S[3] = 0;

	do {
		I = CODE[P]; 
		P = P + 1;
		switch (I.F) {
			case LIT: 
				T++; 
				S[T] = I.A; 
				break;
			case OPR:
				// 运算符处理
				switch (I.A) {
					case 0: /*RETURN*/ 
						T = B - 1; 
						P = S[T + 3]; 
						B = S[T + 2]; 
						break;
					case 1: /*NEG*/
						S[T] = -S[T];  
						break;
					case 2: /*ADD*/
						T--; 
						S[T] = S[T] + S[T + 1];   
						break;
					case 3: /*SUB*/
						T--; 
						S[T] = S[T] - S[T + 1];   
						break;
					case 4: /*MUL*/
						T--; 
						S[T] = S[T] * S[T + 1];   
						break;
					case 5: /*DIV*/
						T--; 
						S[T] = S[T] / S[T + 1]; 
						break;
					case 6: 
						S[T] = (S[T] % 2 != 0);        
						break;
					case 8: 
						T--; 
						S[T] = S[T] == S[T + 1];  
						break;
					case 9: 
						T--; 
						S[T] = S[T] != S[T + 1];  
						break;
					case 10: 
						T--; 
						S[T] = S[T] < S[T + 1];   
						break;
					case 11: 
						T--; 
						S[T] = S[T] >= S[T + 1];  
						break;
					case 12: 
						T--; 
						S[T] = S[T] > S[T + 1];   
						break;
					case 13: 
						T--; 
						S[T] = S[T] <= S[T + 1];  
						break;
					case 14: 
						{
							CString str;
							str.Format(_T("%d"), S[T]);
							logger(str, _T("info"));
							T--;
							break;
						}
					case 15: 
						{
							//logger(_T(""), _T("info"));
						}
						break;
					case 16: 
						T++;
						CInputDialog inputDlg;
						if (inputDlg.DoModal() == IDOK)
						{
							S[T] = _ttoi(inputDlg.m_input);
							CString message;
							message.Format(_T("? %d"), S[T]);
							logger(message, _T("info"));
							fprintf(FOUT, "? %d\n", S[T]);
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
				S[T + 1] = BASE(I.L, B, S); 
				S[T + 2] = B; 
				S[T + 3] = P;
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
				if (S[T] == 0) 
					P = I.A;  
				T--;  
				break;
		} /*switch*/
	} while (P != 0);
	logger(_T("~~~ END PL0 ~~~"), _T("info"));
	fprintf(FOUT, "~~~ END PL0 ~~~\n");
}