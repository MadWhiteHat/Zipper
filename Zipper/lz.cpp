#include "include/lz.h"

void fileOut(std::fstream& fd, union outByte* code, int bits, unsigned char ch, size_t& parentIndex) {
	int bitsCount = code->bytes[7];
	size_t num = parentIndex << 8;
	num += ch;
	code->num += num << (32 - bits);
	bits += 8;
	for (int i = 0; i < bits; i++) {
		code->num = code->num << 1;
		bitsCount++;
		if (bitsCount == 8) {
			fd.put(code->bytes[5]);
			code->bytes[5] = 0;
			bitsCount = 0;
		}
	}
	code->bytes[7] = bitsCount;
}

void encode(std::string fileName) {
	std::fstream fdIn, fdOut;
	fdIn.open(fileName, std::ios::binary | std::ios::in);
	fdOut.open(fileName + ".tmp", std::ios::binary | std::ios::out);
	if (!fdIn.is_open() || !fdOut.is_open()) {
		std::cout << "Cannot open files" << std::endl;
		return;
	}
	char ch = 0;
	size_t pos = 0;
	int count = 1;
	char temp = 0;
	size_t parentIndex = 0;
	union outByte code = { .num = 0x00 };
	std::list<char> byteSeq;
	std::list<char> byteSubSeq;
	std::vector<std::list<char>> dict;
	dict.push_back(std::list<char>());
	fdOut.put(ch);
	while (fdIn.get(ch)) {
		byteSeq.push_back(ch);
		for (pos = 0; pos < dict.size(); ++pos) {
			if (std::is_eq(dict[pos] <=> byteSeq)) break;
		}
		if (pos == dict.size()) {
			for (parentIndex = 0; parentIndex < dict.size(); ++parentIndex) {
				if (std::is_eq(dict[parentIndex] <=> byteSubSeq)) break;
			}
			fileOut(fdOut, &code, count, ch, parentIndex);
			if ((dict.size() + 1) >> count) count++;
			if (count < BITS_OVERFLOW) {
				dict.push_back(byteSeq);
				byteSeq.clear();
				byteSubSeq.clear();
			}
			else {
				dict.clear();
				dict.push_back(std::list<char>());
				count = 1;
			}
		}
		else if (!fdIn.eof()) {
			byteSubSeq.push_back(ch);
		}
	}
	if (!byteSeq.empty()) {
		for (parentIndex = 0; parentIndex < dict.size(); ++parentIndex) {
			if (std::is_eq(dict[parentIndex] <=> byteSubSeq)) break;
		}
		fileOut(fdOut, &code, count, byteSeq.back(), parentIndex);
	}
	if (code.bytes[7]) {
		fdOut.put(code.bytes[5] << (8 - code.bytes[7]));
	}

	fdOut.seekp(0, std::ios::beg);
	fdOut.put(code.bytes[7]);
	fdIn.close();
	fdOut.close();
}

void decode(std::string fileName) {
	std::fstream fdIn, fdOut;
	fdIn.open(fileName + ".tmp", std::ios::binary | std::ios::in);
	fdOut.open(fileName, std::ios::binary | std::ios::out);
	if (!fdIn.is_open() || !fdOut.is_open()) {
		std::cout << "Cannot open files" << std::endl;
		return;
	}
	int count = 1;
	char ch;
	int shift = 25;
	union outByte code = { .num = 0x00 };
	char bytes = 0;
	unsigned int parentIndex;
	fdIn.get(bytes);
	std::vector<std::list<char>> dict;
	dict.push_back(std::list<char>());
	while (fdIn.get(ch)) {
		code.num |= (ch & 0xff);
		if (shift < 8) {
			code.num = code.num << shift;
			parentIndex = code.index[1];
			if (!dict[parentIndex].empty()) {
				for (auto& el : dict[parentIndex]) {
					fdOut.put(el);
				}
			}
			fdOut.put(code.bytes[3]);
			if ((dict.size() + 1) >> count) ++count;
			if (count < BITS_OVERFLOW) {
				auto tmp = dict[parentIndex];
				tmp.push_back(code.bytes[3]);
				dict.push_back(tmp);
			}
			else {
				dict.clear();
				dict.push_back(std::list<char>());
				count = 1;
			}
			code.num &= 0x0000000000FFFFFF;
			code.num = code.num << (8 - shift);
			shift += count;
		}
		else {
			code.num = code.num << 8;
			shift -= 8;
		}
	}
	code.num = code.num << shift;
	parentIndex = code.index[1];
	if (!dict[parentIndex].empty()) {
		for (auto& el : dict[parentIndex]) {
			fdOut.put(el);
		}
	}
	fdOut.put(code.bytes[3]);
	if ((dict.size() + 1) >> count) count++;
	if (count < BITS_OVERFLOW) {
		auto tmp = dict[parentIndex];
		tmp.push_back(code.bytes[3]);
		dict.push_back(tmp);
	}
	else {
		dict.clear();
		dict.push_back(std::list<char>());
		count = 1;
	}
	code.num &= 0x0000000000FFFFFF;
	code.num = code.num << (8 + count);
	if (shift + count == bytes) {
		parentIndex = code.index[1];
		for (auto& el : dict[parentIndex]) {
			fdOut.put(el);
		}
		fdOut.put(code.bytes[3]);
	}
}