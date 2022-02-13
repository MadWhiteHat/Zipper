#include "include\zipper.h"

//void Zipper::Print() {
//	for (auto& dir : _files) {
//		std::tcout << TEXT("Dir: ") << dir.first << std::endl;
//		for (auto& file : dir.second) {
//			std::tcout << file << std::endl;
//		}
//	}
//}

Zipper::Zipper(std::vector<std::tstring> dirPaths, std::vector<std::tstring> filePaths, std::tstring outPath) {
	std::error_code ec;
	for (auto& dir : dirPaths) {
		if (fs::exists(dir, ec)) {
			for (auto& file : fs::recursive_directory_iterator(dir)) {
				if (!fs::is_directory(file)) {
					std::tstring path;
					try {
						path = fs::absolute(file, ec).tstring();
					}
					catch (...) {}
					path = path.substr(0, path.find(fs::path(path).filename()));
					_files[path].push_back(fs::path(file).filename().tstring());
				}
			}	
		} else {
			std::tcout << TEXT("Directory: ") << dir.data() << TEXT(" doesn't exists. Excluding from archieve") << std::endl;
		}
	}
	for (auto& file : filePaths) {
		
		if (fs::exists(file, ec)) {
			std::tstring path;
			try {
				path = fs::absolute(file, ec).tstring();
			}
			catch (...) {}
			path = path.substr(0, path.find(fs::path(path).filename()));
			_files[path].push_back(fs::path(file).filename().tstring());
		} else {
			std::tcout << TEXT("File: ") << file.data() << TEXT(" doesn't exists. Excluding from archieve") << std::endl;
		}
	}
	std::tstring path;
	try {
		path = fs::absolute(outPath).tstring();
		path = path.substr(0, path.find(fs::path(path).filename()));
		fs::create_directories(path);
	}
	catch (...) {}
	std::fstream fd;
	fd.open(fs::path(outPath).tstring(), std::ios::out);
	if (!fd.is_open()) {
		_outPath = (fs::current_path() += TEXT("\\zipped.myzip")).tstring();
		std::tcout << TEXT("Cannot resolve output filename. Default filename set:") << std::endl << _outPath << std::endl;
		try {
			fs::remove_all(path, ec);
		} catch(...) {}
	}
	else {
		_outPath = outPath;
		fd.close();
		fs::remove(outPath,ec);
	}
}
	

// interface
// --dirs folder relative/absolute
// --files file relative/absolute
// -u unpack
// -p pack
// --out out file name

std::tstring find(const std::tstring& str, std::tstring what, std::tstring delim) {
	if (str.find(what) != -1) {
		size_t sPos = str.find(what) + what.length();
		size_t ePos = str.find(delim, sPos);
		return str.substr(sPos, ePos - sPos);
	}
	return std::tstring(TEXT(""));
}

std::vector<std::tstring> vectorize(const std::tstring& str, std::tstring what, std::tstring delim) {
	std::tstring toVect = find(str, what, delim);
	std::vector<std::tstring> vect;

	while (toVect.length()) {
		std::tstring buf = find(toVect, std::tstring(toVect.begin(), toVect.begin()), std::tstring(TEXT(" "), 1));
		if (buf.length() > 0) {
			vect.push_back(buf);
		}
		toVect.erase(0, buf.length() + 1);
	}
	return vect;
}

int tmain(int argc, const tchar* argv[]) {
	
	
	std::tstring args;
	for (auto i = 1; i < argc; ++i) {
		args += argv[i];
		args += TEXT(" ");
	}
	//std::tcout << TEXT("Dirs:") << std::endl;
	auto dirs = vectorize(args, TEXT("--dirs "), TEXT(" --"));
	//for (auto& el : dirs) {
		//std::tcout << el << " " << el.length() << std::endl;
	//}
	//std::tcout << TEXT("Files:") << std::endl;
	auto files = vectorize(args, TEXT("--files "), TEXT(" --"));
	//for (auto& el : files) {
		//std::tcout << el << " " << el.length() << std::endl;
	//}
	//std::tcout << TEXT("Output: ") << std::endl;
	auto out = find(args, TEXT("--out "), TEXT(" --"));
	//std::tcout << out << std::endl;
	Zipper zip(dirs, files, out);
}