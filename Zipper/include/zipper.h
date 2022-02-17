#ifndef _ZIPPER_H
#define _ZIPPER_H


#include <filesystem>
#include <map>

#include "tchar.h"
#include "main.h"

namespace fs = std::filesystem;

#define TMP_EXT TEXT(".tmp")

class Zipper {
	typedef std::pair<size_t, size_t> fileSizeInfo;
	typedef std::pair<std::tstring, fileSizeInfo> fileInfo;
	typedef std::map<std::tstring, std::vector<fileInfo>> file;
	typedef std::vector<std::vector<char>> myList;

	private:
		file _files;
		std::tstring _outPath;
	public:

		Zipper(std::tstring);
		
		void Create(std::vector<std::tstring>, std::vector<std::tstring>);
		void Extract();

	private:
		void EncodeAll();
		void DecodeAll();
		void GetSize();
		void VerifyOutput();
		void ParseInput(std::vector<std::tstring>&, std::vector<std::tstring>&);
		void MakeOne();
		bool ParseConfig();
		bool MakeHierarchy();
		void AddFile(fs::path, std::error_code&);
};

#endif // !_ZIPPER_H
