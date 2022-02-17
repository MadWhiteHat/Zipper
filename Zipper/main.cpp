#include "include/main.h"
#include "include/zipper.h"

std::tstring find(const std::tstring& str, std::tstring what, std::tstring delim) {
	if (str.find(what) != std::tstring::npos) {
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
	bool bUpack = (args.find(TEXT("--unpack")) == std::tstring::npos) ? false : true;
	Zipper zip(out);
	bUpack ? zip.Extract() : zip.Create(dirs, files);
}