// CInputDialog.cpp: 实现文件
//

#include "pch.h"
#include "Compile.h"
#include "afxdialogex.h"
#include "CInputDialog.h"


// CInputDialog 对话框

IMPLEMENT_DYNAMIC(CInputDialog, CDialogEx)

CInputDialog::CInputDialog(CWnd* pParent /*=nullptr*/)
	: CDialogEx(IDD_INPUT_DIALOG, pParent)
	, m_input(_T(""))
{
}

CInputDialog::~CInputDialog()
{
}

void CInputDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_INPUT_EDIT, m_input);
}


BEGIN_MESSAGE_MAP(CInputDialog, CDialogEx)
END_MESSAGE_MAP()


// CInputDialog 消息处理程序
