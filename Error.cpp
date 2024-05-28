#include "pch.h"
#include "CompileDlg.h"

#include "afxdialogex.h"

void CCompileDlg::Error(int n) {
    const TCHAR* errorMessages[] = {
        _T("Use = instead of :="),
        _T("= must be followed by a number"),
        _T("Identifier must be followed by ="),
        _T("const, var, procedure must be followed by an identifier"),
        _T("semicolon or comma missing"),
        _T("Incorrect Symbol after procedure declaration"),
        _T("Statement expected"),
        _T("Incorrect symbol after statement part in block"),
        _T("period expected"),
        _T("Semicolon between statements is missing"),
        _T("Undeclared identifier"),
        _T("Assignment to constant or procedure is not allowed"),
        _T("Assignment operator := expected"),
        _T("call must be followed by an identifier"),
        _T("Call of a constant or variable is meaningless"),
        _T("then expected"),
        _T("Semicolon or end expected"),
        _T("do expected"),
        _T("Incorrect symbol following statement"),
        _T("Relational operator expected"),
        _T("Expression must not contain a procedure identifier"),
        _T("Right parenthesis missing"),
        _T("The preceding factor cannot be followed by this symbol"),
        _T("An expression cannot begin with this symbol"),
        _T("This number is too large")
    };

    CString lineStr(LINE);
    CString errorMessage;
    errorMessage.Format(
        _T("****** compile error (%d) ******\n")
        _T("Find error in line: %s\n")
        _T("On symbol: %d\n")
        _T("Error message: %s\n")
        _T("*******************************"),
        n, lineStr, SYM, (n >= 1 && n <= 25) ? errorMessages[n - 1] : _T("Unknown error")
    );

    logger(errorMessage, _T("error"));

    ERR++;
}

