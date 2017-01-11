#include "TextProcessor.h"
#include <iostream>
#include <string>
#include <cstdio>
#include <cstdlib>
//#include <QFileInfo>
#include <unistd.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <vector>
//#include <QDebug>

using namespace std;

const char SLASH = '/';

bool IsSeparator(char sym) {
	//'\t', '\n', '\v', '\f', '\r', and ' '.

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
	/*if (sym == ' ' || sym == '\n' || sym == '\t' || sym == '\0') {
		return true;
	}
	return false;*/
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
	//str.simplified();
	str = RemoveSpecial(str);
	size_t cnt = 0;
	int i;
	bool special = false;
	for (i = 0; cnt < num && i < str.size(); i++) {
		//qDebug() << "Test1";
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
	//size_t start = i;
	special = false;
	string res = "";
	for (; cnt == num && i < str.size(); i++) {
		//qDebug() << "Test2";
		if (special) {
			res += str[i];
			special = false;
			continue;
		}

		if (str[i] == ' ' || str[i] == '\n' || str[i] == '\t') {
			break;
			//cnt++;
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
	//'\t', '\n', '\v', '\f', '\r', and ' '.
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
bool MappingRefresh(FileMapping *mapping, size_t *pos, size_t *page, size_t max_mapping) {
	if (*page + *pos >= mapping->FileSize) {
		munmap(mapping->DataPtr, max_mapping);
		return false;
	}
	if (*pos < max_mapping) {
		return true;
	}
	munmap(mapping->DataPtr, max_mapping);
	//cout << "MappingRefreshLog" << endl;
	//cout << "MaxMapping = " << max_mapping << endl;
	//cout << "PosPrev = " << *pos << endl;
	//cout << "PagePrev = " << *page << endl;
	*pos = 0;
	*page += max_mapping;
	//cout << "PosNew = " << *pos << endl;
	//cout << "PageNew = " << *page << endl;
	//cout << "=======================" << endl;
	//if (*page < mapping->FileSize) {
	mapping->DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping->fd, *page);
	return true;
	//}
	//return false;
}

//================================

string GetRange(string filename, size_t max_mapping) {
	const size_t alphabet_len = 256;
	size_t arr[alphabet_len];
	for (size_t i = 0; i < alphabet_len; i++) {
		arr[i] = 0;
	}
	//FILE *file;
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;
	//qDebug() << mapping.FileSize;
	//qDebug() << filename;
	while (page < mapping.FileSize) {
		//qDebug() << "Cicle1";
		mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		if (mapping.DataPtr == MAP_FAILED) {
			result = "Mapping: fatal error\n";
			munmap(mapping.DataPtr, mapping.fd);
			close(mapping.fd);
			return result;
		}
		for (size_t i = 0; i < max_mapping; i++) {
			//qDebug() << "Cicle2";
			arr[(size_t) mapping.DataPtr[i]]++;
		}
		munmap(mapping.DataPtr, max_mapping);
		page += max_mapping;
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

string SearchInFilePrefix(string filename, string substr, size_t max_mapping) {
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	//char sym;
	bool ignore = false;
	bool found = false;
	size_t pos = 0;

	size_t col = 1;
	size_t row = 1;

	size_t current_col = 0;
	size_t current_row = 0;
	while (page < mapping.FileSize) {
		mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		//while ((sym = fgetc(file)) != EOF) {
		for (size_t i = 0; i < max_mapping; i++) {
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
		munmap(mapping.DataPtr, max_mapping);
		page += max_mapping;
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

string SearchInFilePostfix(string filename, string substr, size_t max_mapping) {
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	//char sym;
	bool found = false;
	//size_t pos = 0;

	size_t col = 1;
	size_t row = 1;

	size_t current_col = 0;
	size_t current_row = 0;

	vector <char> arr_tmp;
	arr_tmp.resize(substr.size());
	//qDebug() << "Size: " << substr << " - " << substr.size();
	size_t nw_pos = arr_tmp.size() - 1;
	while (page < mapping.FileSize) {
		mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		//while ((sym = fgetc(file)) != EOF) {
		for (size_t i = 0; i < max_mapping; i++) {
			if (IsSeparator(mapping.DataPtr[i])) {
				found = true;
				for (size_t i = 0; i < (size_t) substr.size(); i++) {
					//qDebug() << arr_tmp[i] << ":" << substr[(int) i];
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
		munmap(mapping.DataPtr, max_mapping);
		page += max_mapping;
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

string SearchInFileBasic(string filename, string substr, size_t max_mapping) {
	FileMapping mapping;
	string result = "";
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;

	//char sym;
	bool found = false;
	size_t pos = 0;

	//size_t col = 1;
	//size_t row = 1;

	//size_t current_col = 0;
	//size_t current_row = 0;

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
		mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
		//while ((sym = fgetc(file)) != EOF) {
		for (size_t i = 0; i < max_mapping; i++) {
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
		munmap(mapping.DataPtr, max_mapping);
		page += max_mapping;
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

string EditWrite(string filename, size_t row, size_t col, string text, size_t max_mapping) {
	string result = "";
	/*FILE *file;
	const char *filename_bas = filename.c_str();
	if ((file = fopen(filename_bas, "r")) == NULL) {
		result = "Cannot open file\n";
		return result;
	}*/
	FileMapping mapping;
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;
	//mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);

	
	//QFileInfo file_info(filename);
	//string filename_tmp = file_info.absolutePath() + ".tmp_" + file_info.baseName();
	string filename_tmp = GetPath(filename) + ".tmp_" + GetName(filename);
	const char *filename_tmp_bas = filename_tmp.c_str();
	FILE *file_tmp = fopen(filename_tmp_bas, "r");
	if (file_tmp != NULL) {
		fclose(file_tmp);
		unlink(filename_tmp_bas);
	}
	file_tmp = fopen(filename_tmp_bas, "w");
	//char sym;
	size_t current_row = 1;
	size_t current_col = 1;
	mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
	size_t pos = 0;
	while (current_row < row || (current_row == row && current_col < col)) {
		/*if ((sym = fgetc(file)) == EOF) {
			break;
		}*/
		if (!MappingRefresh(&mapping, &pos, &page, max_mapping)) {
			break;
		}
		if (mapping.DataPtr[pos] == '\n') {
			current_col = 1;
			current_row++;
			//pos++;
			//continue;
		} else {
			current_col++;
		}
		//char sym = (char) mapping.DataPtr[pos];
		//cout << "Sym: " << sym << endl;
		fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		pos++;
	}
	for (size_t i = 0; i < (size_t) text.size(); i++) {
		char tmp = text[(int) i];
		fwrite(&tmp, sizeof(char), 1, file_tmp);
	}
	/*if (sym != EOF) {
		while ((sym = fgetc(file)) != EOF) {
			fwrite(&sym, sizeof(char), 1, file_tmp);
		}
	}*/
	while (MappingRefresh(&mapping, &pos, &page, max_mapping)) {
		//char sym = (char) mapping.DataPtr[pos];
		//cout << "Sym: " << sym << endl;
		fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		pos++;
	}
	//fclose(file);
	close(mapping.fd);
	fclose(file_tmp);
	unlink(filename_bas);
	rename(filename_tmp_bas, filename_bas);
	
	return "Edit (Write): Success\n";
}

string EditDelete(string filename, size_t row, size_t col, size_t cnt, size_t max_mapping) {
	string result = "";
	/*FILE *file;
	const char *filename_bas = filename.c_str();
	if ((file = fopen(filename_bas, "r")) == NULL) {
		result = "Cannot open file\n";
		return result;
	}*/
	//QFileInfo file_info(filename);
	//string filename_tmp = file_info.absolutePath() + ".tmp_" + file_info.baseName();

	FileMapping mapping;
	const char *filename_bas = filename.c_str();
	if ((mapping.fd = open(filename_bas, O_RDONLY, 0)) < 0) {
		result = "Cannot open file\n";
		return result;
	}
	mapping.FileSize = FileSize(filename);
	size_t page = 0;
	//mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);

	string filename_tmp = GetPath(filename) + ".tmp_" + GetName(filename);
	const char *filename_tmp_bas = filename_tmp.c_str();
	FILE *file_tmp = fopen(filename_tmp_bas, "r");
	if (file_tmp != NULL) {
		fclose(file_tmp);
		unlink(filename_tmp_bas);
	}
	file_tmp = fopen(filename_tmp_bas, "w");

	//char sym;
	size_t current_row = 1;
	size_t current_col = 1;
	mapping.DataPtr = (char *) mmap(NULL, max_mapping, PROT_READ, MAP_PRIVATE, mapping.fd, page);
	size_t pos = 0;
	while (current_row < row || (current_row == row && current_col < col)) {
		if (!MappingRefresh(&mapping, &pos, &page, max_mapping)) {
			break;
		}
		if (mapping.DataPtr[pos] == '\n') {
			current_col = 1;
			current_row++;
			//pos++;
			//continue;
		} else {
			current_col++;
		}
		fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		pos++;
	}
	//size_t i = 0;
	//while ((sym = fgetc(file)) != EOF) {
	size_t pos_tmp = page + pos + cnt;
	while (MappingRefresh(&mapping, &pos, &page, max_mapping)) {
		if (page + pos >= pos_tmp) {
			fwrite(&(mapping.DataPtr[pos]), sizeof(char), 1, file_tmp);
		}
		pos++;
	}

	//fclose(file);
	fclose(file_tmp);
	unlink(filename_bas);
	rename(filename_tmp_bas, filename_bas);
	return "Edit (Delete): Success\n";
}
