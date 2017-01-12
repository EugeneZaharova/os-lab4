#ifndef TEXTPROCESSOR_H
#define TEXTPROCESSOR_H

#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>

const char SLASH = '/';
const std::string MAPPING_FATAL_ERROR_MESSAGE = "Reading file: FATAL ERROR\n";
const std::string PARAMETER_INCORRECT_MESSAGE = "ERROR: Parameter(s) is incorrect\n";

class FileMapping {
public:
	int fd;
	size_t FileSize;
	char *DataPtr;
};

class Settings {
public:
	bool IsFileLenLimit;
	size_t MinLength;
	size_t MaxLength;
	size_t MaxMapping;

	Settings() {
		IsFileLenLimit = false;
		MinLength = 0;
		MaxLength = 0;
		MaxMapping = 256;
	}
};

bool IsSeparator(char sym);
size_t ConvertToBytes(size_t num, size_t unit);
size_t FileSize(std::string filename);
bool IsFileExists(std::string filename);
std::string SetSpaces(std::string str);
std::string GetParameter(std::string str, size_t num);
std::string RemoveSpecial(std::string str);
std::string StringReverse(std::string str);
bool IsCharNum(char sym);
bool IsStringNum(std::string str);
std::string UNumToString(size_t num);
size_t StringToUNum(std::string str);
bool MappingRefresh(FileMapping *mapping, size_t *pos, size_t *page, Settings *settings);

std::string GetRange(std::string filename, Settings *settings);
std::string SearchInFilePrefix(std::string filename, std::string substr, Settings *settings);
std::string SearchInFilePostfix(std::string filename, std::string substr, Settings *settings);
std::string SearchInFileBasic(std::string filename, std::string substr, Settings *settings);
std::string EditWrite(std::string filename, size_t row, size_t col,
				  std::string text, Settings *settings);
std::string EditDelete(std::string filename, size_t row, size_t col,
				   size_t cnt, Settings *settings);

#endif // TEXTPROCESSOR_H
