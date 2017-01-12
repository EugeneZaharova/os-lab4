#include "TextProcessor.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>

using namespace std;


/*
================================
SUBSUDUARY FUNCTIONS
================================
*/

bool IsSeparator(char sym) {
	switch (sym) {
		case '\t':
		case '\n':
		case '\v':
		case '\f':
		case '\r':
		case ' ':
		case '\0':
			return true;
		default:
			return false;
	}
}

size_t ConvertToBytes(size_t num, size_t unit) {
	for (size_t i = 0; i < unit; i++) {
		num *= 1024;
	}
	return num;
}

size_t FileSize(string filename) {
	const char *filename_bas = filename.c_str();
	FILE *file = fopen(filename_bas, "rb");
	if(file == NULL) {
		return 0;
	}
	fseek(file, 0, SEEK_END);
	size_t size = ftell(file);
	fclose(file);
	return size;
}

bool IsFileExists(string filename) {
	const char *filename_bas = filename.c_str();
	FILE *file = fopen(filename_bas, "r");
	if (file != NULL) {
		fclose(file);
		return true;
	}
	return false;
}

string SetSpaces(string str) {
	for (int i = 0; i < str.size(); i++) {
		if (str[i] == '_') {
			str[i] = ' ';
		}
	}
	return str;
}

string GetParameter(string str, size_t num) {
	str = RemoveSpecial(str);
	size_t cnt = 0;
	int i;
	bool special = false;
	for (i = 0; cnt < num && i < str.size(); i++) {
		if (special) {
			special = false;
			continue;
		}
		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') {
			cnt++;
		}
		if (str[i] == '\\') {
		   special = true;
		}
	}

	special = false;
	string res = "";
	for (; cnt == num && i < str.size(); i++) {
		if (special) {
			res += str[i];
			special = false;
			continue;
		}

		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') {
			break;
		}
		if (str[i] == '\\') {
		   special = true;
		   continue;
		}
		res += str[i];
	}
	return res;
}
string RemoveSpecial(string str) {
	string new_str = "";
	bool is_first_space = true;
	for (size_t i = 0; i < str.size(); i++) {
		if (!IsSeparator(str[i])) {
			is_first_space = true;
		}
		bool is_set_space = false;
		if (str[i] == ' ' && is_first_space) {
			is_set_space = true;
			is_first_space = false;
		}

		if (!IsSeparator(str[i]) || is_set_space) {
			new_str += str[i];
		}
	}
	return new_str;
}

string StringReverse(std::string str) {
	string new_str = "";
	for (size_t i = str.size(); i > 0; i--) {
		new_str += str[i - 1];
	}
	return new_str;
}

bool IsCharNum(char sym) {
	return sym >= '0' && sym <= '9';
}
bool IsStringNum(string str) {
	for (size_t i = 0; i < str.size(); i++) {
		if (!IsCharNum(str[i])) {
			return false;
		}
	}
	return true;
}

char UNumToChar(size_t num) {
	return num + '0';
}
string UNumToString(size_t num) {
	string str = "";
	while (num > 0) {
		size_t md = num % 10;
		num /= 10;
		str += UNumToChar(md);
	}
	return StringReverse(str);
}
size_t CharToUNum(char sym) {
	if (IsCharNum(sym)) {
		return sym - '0';
	}
	return 0;
}
size_t StringToUNum(string str) {
	size_t res = 0;
	for (size_t i = 0; i < str.size(); i++) {
		res *= 10;
		res += CharToUNum(str[i]);
	}
	return res;
}

string GetName(string path) {
	size_t last_slash = 0;
	for (size_t i = 0; i < path.size(); i++) {
		if (path[i] == SLASH) {
			last_slash = i;
		}
	}
	if (path[last_slash] != SLASH) {
		return path;
	}
	return path.substr(last_slash + 1);

}
string GetPath(string path) {
	size_t last_slash = 0;
	for (size_t i = 0; i < path.size(); i++) {
		if (path[i] == SLASH) {
			last_slash = i;
		}
	}
	if (path[last_slash] != SLASH) {
		return "";
	}
	path.resize(last_slash + 1);
	return path;
}
bool MappingRefresh(FileMapping *mapping, size_t *pos, size_t *page, Settings *settings) {
	if (*page + *pos >= mapping->FileSize) {
		munmap(mapping->DataPtr, settings->MaxMapping);
		return false;
	}
	if (*pos < settings->MaxMapping) {
		return true;
	}
	munmap(mapping->DataPtr, settings->MaxMapping);
	*pos = 0;
	*page += settings->MaxMapping;
	mapping->DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping->fd, *page);
	if (mapping->DataPtr == MAP_FAILED) {
		cout << MAPPING_FATAL_ERROR_MESSAGE;
		munmap(mapping->DataPtr, mapping->fd);
		return false;
	}
	return true;}

/*
================================
MAIN FUNCTIONS
================================
*/

string GetRange(string filename, Settings *settings) {
	const size_t alphabet_len = 256;
	size_t arr[alphabet_len];
	for (size_t i = 0; i < alphabet_len; i++) {
		arr[i] = 0;
	}
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;
	while (page < mapping.FileSize) {
		mapping.DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		if (mapping.DataPtr == MAP_FAILED) {
			result = MAPPING_FATAL_ERROR_MESSAGE;
			munmap(mapping.DataPtr, mapping.fd);
			close(mapping.fd);
			return result;
		}
		for (size_t i = 0; i < settings->MaxMapping; i++) {
			arr[(size_t) mapping.DataPtr[i]]++;
		}
		munmap(mapping.DataPtr, settings->MaxMapping);
		page += settings->MaxMapping;
	}
	close(mapping.fd);
	for (size_t i = 0; i < alphabet_len; i++) {
		if (IsSeparator(arr[i])) {
			continue;
		}
		if (arr[i] > 0) {
			result += (char) i;
			result += " (";
			result += UNumToString(i);
			result += "): ";
			result += UNumToString(arr[i]);
			result += '\n';
		}
	}
	return result;
}

string SearchInFilePrefix(string filename, string substr, Settings *settings) {
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	bool ignore = false;
	bool found = false;
	size_t pos = 0;

	size_t col = 1;
	size_t row = 1;

	size_t current_col = 0;
	size_t current_row = 0;
	while (page < mapping.FileSize) {
		mapping.DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		if (mapping.DataPtr == MAP_FAILED) {
			result = MAPPING_FATAL_ERROR_MESSAGE;
			munmap(mapping.DataPtr, mapping.fd);
			close(mapping.fd);
			return result;
		}
		for (size_t i = 0; i < settings->MaxMapping; i++) {
			if (ignore) {
				if (mapping.DataPtr[i] == '\n') {
					col = 1;
					row++;
				} else {
					col++;
				}
				if (IsSeparator(mapping.DataPtr[i])) {
					ignore = false;
				}
				continue;
			}

			if (substr[(int) pos] == mapping.DataPtr[i]) {
				if (!pos) {
					current_col = col;
					current_row = row;
				}
				if (pos >= (size_t) substr.size() - 1) {
					found = true;
					break;
				}
				pos++;
			} else {
				pos = 0;
				if (!IsSeparator(mapping.DataPtr[i])) {
					ignore = true;
				}
			}
			if (mapping.DataPtr[i] == '\n') {
				col = 1;
				row++;
			} else {
				col++;
			}
		}
		munmap(mapping.DataPtr, settings->MaxMapping);
		page += settings->MaxMapping;
	}
	close(mapping.fd);
	if (found) {
		result = "Search (prefix): First found position: (";
		result += UNumToString(current_row) + ", " + UNumToString(current_col) + ")\n";
	} else {
		result = "Search (prefix): Not found\n";
	}
	return result;
}

string SearchInFilePostfix(string filename, string substr, Settings *settings) {
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	bool found = false;
	size_t col = 1;
	size_t row = 1;

	size_t current_col = 0;
	size_t current_row = 0;

	vector <char> arr_tmp;
	arr_tmp.resize(substr.size());
	size_t nw_pos = arr_tmp.size() - 1;
	while (page < mapping.FileSize) {
		mapping.DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		if (mapping.DataPtr == MAP_FAILED) {
			result = MAPPING_FATAL_ERROR_MESSAGE;
			munmap(mapping.DataPtr, mapping.fd);
			close(mapping.fd);
			return result;
		}
		for (size_t i = 0; i < settings->MaxMapping; i++) {
			if (IsSeparator(mapping.DataPtr[i])) {
				found = true;
				for (size_t i = 0; i < (size_t) substr.size(); i++) {
					if (arr_tmp[i] != substr[(int) i]) {
						found = false;
						break;
					}
				}
				if (found) {
					current_col = col;
					current_row = row;
					break;
				}
			}
			for (size_t j = nw_pos + 1; j < (size_t) arr_tmp.size(); j++) {
				arr_tmp[j - 1] = arr_tmp[j];
			}
			arr_tmp[arr_tmp.size() - 1] = mapping.DataPtr[i];

			if (mapping.DataPtr[i] == '\n') {
				col = 1;
				row++;
			} else {
				col++;
			}
			if (nw_pos > 0) {
				nw_pos--;
			}
		}
		munmap(mapping.DataPtr, settings->MaxMapping);
		page += settings->MaxMapping;
	}
	close(mapping.fd);
	if (found) {
		result = "Search (postfix): First found position: (";
		result += UNumToString(current_row) + ", " + UNumToString(current_col - substr.size()) + ")\n";
	} else {
		result = "Search (postfix): Not found\n";
	}
	return result;
}

string SearchInFileBasic(string filename, string substr, Settings *settings) {
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	bool found = false;
	size_t pos = 0;

	vector <char> arr_tmp;
	vector <size_t> cols;
	vector <size_t> rows;
	arr_tmp.resize(substr.size());
	cols.resize(substr.size());
	rows.resize(substr.size());
	size_t nw_pos = arr_tmp.size() - 1;
	cols[cols.size() - 1] = 1;
	rows[rows.size() - 1] = 1;
	while (page < mapping.FileSize) {
		mapping.DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		if (mapping.DataPtr == MAP_FAILED) {
			result = MAPPING_FATAL_ERROR_MESSAGE;
			munmap(mapping.DataPtr, mapping.fd);
			close(mapping.fd);
			return result;
		}
		for (size_t i = 0; i < settings->MaxMapping; i++) {
			for (size_t j = nw_pos + 1; j < (size_t) arr_tmp.size(); j++) {
				arr_tmp[j - 1] = arr_tmp[j];
			}
			arr_tmp[arr_tmp.size() - 1] = mapping.DataPtr[i];
			for (size_t j = nw_pos + pos; j < (size_t) arr_tmp.size(); j++) {
				if (substr[(int) pos] == arr_tmp[j]) {
					if (pos >= (size_t) substr.size() - 1) {
						found = true;
						break;
					}
					pos++;
				} else {
					pos = 0;
					break;
				}
			}
			if (found) {
				break;
			}

			if (nw_pos > 0) {
				nw_pos--;
			}
			for (size_t j = nw_pos + 1; j < (size_t) arr_tmp.size(); j++) {
				cols[j - 1] = cols[j];
				rows[j - 1] = rows[j];
			}
			if (mapping.DataPtr[i] == '\n') {
				cols[cols.size() - 1] = 1;
				rows[rows.size() - 1]++;
			} else {
				cols[cols.size() - 1]++;
			}
		}
		munmap(mapping.DataPtr, settings->MaxMapping);
		page += settings->MaxMapping;
	}
	close(mapping.fd);
	if (found) {
		result = "Search (basic): First found position: (";
		result += UNumToString(rows[0]) + ", " + UNumToString(cols[0]) + ")\n";
	} else {
		result = "Search (basic): Not found\n";
	}
	return result;
}

string EditWrite(string filename, size_t row, size_t col, string text, Settings *settings) {
	string result = "";
	FileMapping mapping;
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	string filename_tmp = GetPath(filename) + ".tmp_" + GetName(filename);
	const char *filename_tmp_bas = filename_tmp.c_str();
	FILE *file_tmp = fopen(filename_tmp_bas, "r");
	if (file_tmp != NULL) {
		fclose(file_tmp);
		unlink(filename_tmp_bas);
	}
	file_tmp = fopen(filename_tmp_bas, "w");
	size_t current_row = 1;
	size_t current_col = 1;
	mapping.DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
	if (mapping.DataPtr == MAP_FAILED) {
		result = MAPPING_FATAL_ERROR_MESSAGE;
		munmap(mapping.DataPtr, mapping.fd);
		close(mapping.fd);
		return result;
	}
	size_t pos = 0;
	while (current_row < row || (current_row == row && current_col < col)) {
		if (!MappingRefresh(&mapping, &pos, &page, settings)) {
			break;
		}
		if (mapping.DataPtr[pos] == '\n') {
			current_col = 1;
			current_row++;
		} else {
			current_col++;
		}
		fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		pos++;
	}
	for (size_t i = 0; i < (size_t) text.size(); i++) {
		char tmp = text[(int) i];
		fwrite(&tmp, sizeof(char), 1, file_tmp);
	}
	while (MappingRefresh(&mapping, &pos, &page, settings)) {
		fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		pos++;
	}
	close(mapping.fd);
	fclose(file_tmp);
	unlink(filename_bas);
	rename(filename_tmp_bas, filename_bas);
	
	return "Edit (Write): Success\n";
}

string EditDelete(string filename, size_t row, size_t col, size_t cnt, Settings *settings) {
	string result = "";

	FileMapping mapping;
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	string filename_tmp = GetPath(filename) + ".tmp_" + GetName(filename);
	const char *filename_tmp_bas = filename_tmp.c_str();
	FILE *file_tmp = fopen(filename_tmp_bas, "r");
	if (file_tmp != NULL) {
		fclose(file_tmp);
		unlink(filename_tmp_bas);
	}
	file_tmp = fopen(filename_tmp_bas, "w");
	size_t current_row = 1;
	size_t current_col = 1;
	mapping.DataPtr = (char *) mmap(NULL, settings->MaxMapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
	if (mapping.DataPtr == MAP_FAILED) {
		result = MAPPING_FATAL_ERROR_MESSAGE;
		munmap(mapping.DataPtr, mapping.fd);
		close(mapping.fd);
		return result;
	}
	size_t pos = 0;
	while (current_row < row || (current_row == row && current_col < col)) {
		if (!MappingRefresh(&mapping, &pos, &page, settings)) {
			break;
		}
		if (mapping.DataPtr[pos] == '\n') {
			current_col = 1;
			current_row++;
		} else {
			current_col++;
		}
		fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		pos++;
	}
	size_t pos_tmp = page + pos + cnt;
	while (MappingRefresh(&mapping, &pos, &page, settings)) {
		if (page + pos >= pos_tmp) {
			fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		}
		pos++;
	}

	fclose(file_tmp);
	unlink(filename_bas);
	rename(filename_tmp_bas, filename_bas);
	return "Edit (Delete): Success\n";
}
