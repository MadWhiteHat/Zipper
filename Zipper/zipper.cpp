#include "include\zipper.h"
#include "include\lz.h"

// interface
// --dirs folder relative/absolute
// --files file relative/absolute
// --unpack
// --out out file name
// default pack


Zipper::Zipper(std::tstring outPath) : _outPath(outPath) {}

void Zipper::Create(std::vector<std::tstring> dirPaths, std::vector<std::tstring> filePaths) {
	ParseInput(dirPaths, filePaths);
	VerifyOutput();
	EncodeAll();
	GetSize();
	MakeOne();

}

bool Zipper::ParseConfig() {
	std::tfstream fd;
	std::tstring info;

	_files.clear();	
	fd.open(_outPath, std::ios::binary | std::ios::in);
	if (!fd.is_open()) {
		std::tcout << TEXT("Cannot open archieve file.") << std::endl;
		return true;
	}
	
	tchar buf[3] = { 0 };
	fd.read(buf, 2);
	info = buf;

	if (info.find(TEXT("||")) != 0) {
		
		std::tcout << TEXT("Invalid archieve file.") << std::endl;
		return true;
	}

	fd.seekg(0);
	info.clear();
	std::getline(fd, info);
	fd.close();
	if (info.find(0x0d) != std::tstring::npos) { info.erase(info.find(0x0d)); }
	{
		size_t filesCnt = 0;

		for (auto it = info.begin(); it < info.end(); ++it) {
			if (*it == '|' && *(it + 1) == '|') { filesCnt++; }
		}
		if (filesCnt % 4) {
			std::tcout << TEXT("Invalid archieve file.") << std::endl;
			return true;
		}
	}


	{

		std::tstring tmpFile, tmpPath;
		size_t fileStart = 0;
		size_t fileSize = 0;
		size_t pos = 0;
		while (info.find(TEXT("||")) != std::tstring::npos) {
			info.erase(0, pos + 2);
			pos = info.find(TEXT("||"));
			fileStart = std::stoll(info.substr(0, pos));
			info.erase(0, pos + 2);
			pos = info.find(TEXT("||"));
			fileSize = std::stoll(info.substr(0, pos));
			info.erase(0, pos + 2);
			pos = info.find(TEXT("||"));
			tmpPath = info.substr(0, pos);
			info.erase(0, pos + 2);
			pos = info.find(TEXT("||"));
			tmpFile = info.substr(0, pos);
			_files[tmpPath].push_back(fileInfo(tmpFile, fileSizeInfo(fileStart,fileSize)));
		}
	}
	return false;
}


void Zipper::Extract() {
	
	if (ParseConfig()) {
		std::tcout << TEXT("ParseConfig fail") << std::endl;
		return;
	}
	if (MakeHierarchy()) {
		std::tcout << TEXT("MakeHierarchy fail") << std::endl;
		return;
	}
	DecodeAll();

	return;
}

void Zipper::GetSize() {
	uintmax_t fileSize = 0;
	std::error_code ec;
	for (auto& dir : _files) {
		for (auto& file : dir.second) {
			file.second.first = fileSize;
			file.second.second = fs::file_size(dir.first + file.first + TMP_EXT, ec);
			fileSize += file.second.second;
		}
	}
	return;
}

void Zipper::VerifyOutput() {
	std::error_code ec;
	std::tstring path;
	try {
		path = fs::absolute(_outPath).tstring();
		path = path.substr(0, path.find(fs::path(path).filename()));
		fs::create_directories(path);
	}
	catch (...) {}
	std::tfstream fd;
	fd.open(fs::path(_outPath).tstring(), std::ios::out);
	if (!fd.is_open()) {
		_outPath = (fs::current_path() += TEXT("\\zipped.myzip")).tstring();
		std::tcout << TEXT("Cannot resolve output filename. Default filename set:") << std::endl << _outPath << std::endl;
		try {
			fs::remove_all(path, ec);
		}
		catch (...) {}
	}
	else {
		fd.close();
		fs::remove(_outPath, ec);
	}
}


void Zipper::ParseInput(std::vector<std::tstring>& dirPaths, std::vector<std::tstring>& filePaths) {

	std::error_code ec;
	for (auto& dir : dirPaths) {
		if (fs::exists(dir, ec)) {
			for (auto& file : fs::recursive_directory_iterator(dir)) {
				if (!fs::is_directory(file)) {
					AddFile(file.path(), ec);
				}
			}
		}
		else {
			std::tcout << TEXT("Directory: ") << dir.data() << TEXT(" doesn't exists. Excluding from archieve") << std::endl;
		}
	}
	for (auto& file : filePaths) {

		if (fs::exists(file, ec)) {
			AddFile(file, ec);
		}
		else {
			std::tcout << TEXT("File: ") << file.data() << TEXT(" doesn't exists. Excluding from archieve") << std::endl;
		}
	}

}


void Zipper::MakeOne() {

	std::tfstream fd;
	std::error_code ec;

	fd.open(_outPath, std::ios::out);
	if (!fd.is_open()) {
		std::tcout << TEXT("Cannot open output file.") << std::endl;
		return;
	}
	for (auto& dir : _files) {
		for (auto& file : dir.second) {
			fd <<  TEXT("||");
			fd <<  std::to_tstring(file.second.first);
			fd <<  TEXT("||");
			fd <<  std::to_tstring(file.second.second);
			fd <<  TEXT("||");
			{
				std::tstring relative = fs::relative(dir.first, ec).tstring();
				if (!relative.empty()) { relative += '/'; }
				std::replace(relative.begin(), relative.end(), '\\', '/');
				std::tstring pattern(TEXT("../"));
				auto pos = relative.find(pattern);
				while (pos != std::tstring::npos) {
					relative.erase(pos, pattern.length());
					pos = relative.find(pattern);
				}
				if (relative.find(TEXT("./")) == std::tstring::npos) { relative = TEXT("./") + relative; }
				fd << relative;
			}
			fd <<  TEXT("||");
			fd << fs::path(file.first).tstring();
		}
	}
	fd << std::endl;
	fd.close();

	char byte;
	std::fstream fdIn, fdOut;
	fdOut.open(_outPath, std::ios::binary | std::ios::app);
	if (!fdOut.is_open()) {
		std::tcout << TEXT("Cannot open output file.") << std::endl;
		return;
	}

	for (auto& path : _files) {
		for (auto& file : path.second) {
			fdIn.open(path.first + file.first + TMP_EXT, std::ios::binary | std::ios::in);
			if (!fdIn.is_open()) {
				std::tcout << TEXT("Cannot open input file.") << std::endl;
				fdOut.close();
				return;
			}
			while (fdIn.get(byte)) {
				fdOut << byte;
			}
			fdIn.close();
			fs::remove(path.first + file.first + TMP_EXT, ec);
		}
	}
	fdOut.close();
}

bool Zipper::MakeHierarchy() {
	std::fstream fdIn, fdOut;
	std::error_code ec;
	for (auto& dir : _files) {
		fs::create_directories(dir.first, ec);
		if (ec.value() != 0) { 
			std::tcout << TEXT("Cannot create directory.") << std::endl;
			return true; 
		}
	}

	fdIn.open(_outPath, std::ios::binary | std::ios::in);
	if (!fdIn.is_open()) {
		std::tcout << TEXT("Cannot open input file.") << std::endl;
		return true;
	}

	size_t confLen = 0;
	{
		std::string dummy;
		std::getline(fdIn, dummy);
		confLen = fdIn.tellg();
	}

	char byte;
	for (auto& dir : _files) {
		for (auto& file : dir.second) {
			fdOut.open(dir.first + file.first + TMP_EXT, std::ios::binary | std::ios::out);
			if (!fdOut.is_open()) {
				std::tcout << TEXT("Cannot open output file.") << std::endl;
				return true;
			}
			size_t end = confLen + file.second.first + file.second.second;
			for (size_t i = confLen + file.second.first; i < end && fdIn.get(byte); ++i) {
				fdOut << byte;
			}
			fdOut.close();
		}
	}

	fdIn.close();
	return false;
}

void Zipper::AddFile(fs::path file, std::error_code& ec) {
	std::tstring path;
	try {
		path = fs::absolute(file, ec).tstring();
	}
	catch (...) {}
	path = path.substr(0, path.find(fs::path(path).filename()));
	_files[path].push_back(fileInfo(fs::path(file).filename().tstring(), fileSizeInfo(0, fs::file_size(file, ec))));
}

void Zipper::EncodeAll() {
	for (auto& dir : _files) {
		for (auto& file : dir.second) {
			std::tcout << TEXT("Encrypting ") << dir.first + file.first << std::endl;
			encode(fs::path(dir.first + file.first).string());
		}
	}
}

void Zipper::DecodeAll() {
	for (auto& dir : _files) {
		for (auto& file : dir.second) {
			std::tcout << TEXT("Decrypting ") << dir.first + file.first << std::endl;
			decode(fs::path(dir.first + file.first).string());
			fs::remove(dir.first + file.first + TMP_EXT);
		}
	}
}