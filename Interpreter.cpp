#include "pch.h"
#include "CompileDlg.h"
#include "Compile.h"
#include "afxdialogex.h"
#include <stdio.h>

int CCompileDlg::BASE(int L, int B, int S[]) {
	int B1 = B; /*FIND BASE L LEVELS DOWN*/
	while (L > 0) { B1 = S[B1]; L = L - 1; }
	return B1;
}

void CCompileDlg::Interpret() {
	const int STACKSIZE = 500;
	int P, B, T;        // 程序计数器，基址寄存器，栈顶寄存器
	INSTRUCTION I;      // 当前指令
	int S[STACKSIZE];   // 数据存储

	//memset(S,0, sizeof(S));

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
							logger(_T(""), _T("info"));
						}
						break;
					case 16: 
						///* 无对应实现
						//T++;  
						//S[T] = InputBox("Input", "Enter a number:", 0).ToInt();
						//Form1->printls("? ", S[T]); 
						//fprintf(FOUT, "? %d\n", S[T]);
						//*/
						//// 确保富文本编辑框获得焦点，以便用户可以开始输入
						//m_RichEdit.SetFocus();

						//// 创建一个消息循环，等待用户按下回车键
						//MSG msg;
						//while (::GetMessage(&msg, nullptr, 0, 0))
						//{
						//	if (msg.message == WM_KEYDOWN && msg.wParam == VK_RETURN) // 检测回车键
						//	{
						//		// 用户按下了回车键，跳出循环
						//		break;
						//	}
						//	::TranslateMessage(&msg);
						//	::DispatchMessage(&msg);
						//}

						//// 获取用户输入的文本
						//CString strText;
						//m_RichEdit.GetWindowText(strText);

						//// 将CString转换为int（这里假设用户输入的是数字）
						//int nInput = _ttoi(strText);
						//logger(_T("Input: ") + strText, _T("info"));
						//// 接下来，你可以根据需要使用nInput变量
						//// 例如：S[T] = nInput;

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