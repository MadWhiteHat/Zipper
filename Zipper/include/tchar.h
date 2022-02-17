#ifndef _TCHAR_H
#define _TCHAR_H

#ifdef _UNICODE
#define tstring wstring
#define tchar wchar_t
#define tcout wcout
#define TEXT(str) L ## str
#define to_tstring to_wstring
#define tfstream wfstream
#define tmain wmain
#else 
#define tstring string
#define tchar char
#define tcout cout
#define TEXT(str) str
#define to_tstring to_string
#define tfstream fstream
#define tmain main
#endif // _UNICODE


#endif // !_TCHAR_H
