#ifndef TEXTPROCESSOR_H
#define TEXTPROCESSOR_H

#include <string>
#include <cstdio>
#include <cstdlib>
//#include <QFileInfo>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>

class FileMapping {
public:
	int fd;
	size_t FileSize;
	char *DataPtr;
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
std::string UNumToString(size_t num);
size_t StringToUNum(std::string str);

std::string GetRange(std::string filename, size_t max_mapping);
std::string SearchInFilePrefix(std::string filename, std::string substr, size_t max_mapping);
std::string SearchInFilePostfix(std::string filename, std::string substr, size_t max_mapping);
std::string SearchInFileBasic(std::string filename, std::string substr, size_t max_mapping);
std::string EditWrite(std::string filename, size_t row, size_t col,
				  std::string text, size_t max_mapping);
std::string EditDelete(std::string filename, size_t row, size_t col,
				   size_t cnt, size_t max_mapping);

#endif // TEXTPROCESSOR_H
