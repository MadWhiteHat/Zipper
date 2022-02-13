#ifndef _ZIPPER_H
#define _ZIPPER_H

#include <iostream>
#include <string>
#include <vector>
#include <clocale>
#include <utility>
#include <fstream>
#include <filesystem>
#include <map>
#include <initializer_list>

namespace fs = std::filesystem;

#ifdef _UNICODE
#define tstring wstring
#define tchar wchar_t
#define tcout wcout
#define TEXT(str) L ## str
#define tmain wmain
#else 
#define tstring string
#define tchar char
#define tcout cout
#define TEXT(str) str
#define tmain main
#endif // _UNICODE

#ifdef WIN32
#endif // WIN32


// Add syslog status

class Zipper {
	typedef std::map<std::tstring, std::vector<std::tstring>> file;
	private:
		file _files;
		std::tstring _outPath;
	public:

		Zipper(std::vector<std::tstring>, std::vector<std::tstring>, std::tstring);
		
		void GetInfo();
		void Compress();
		void Decompress();
		//void Print();

	private:
		int Digits(unsigned long long int);
};

#endif // !_ZIPPER_H
