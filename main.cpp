#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "TextProcessor.h"

using namespace std;

string GetManual(bool is_interactive, Settings *settings) {
	string res = "";
	res += "Список команд:\n";
	res += "======================================\n";
	if (is_interactive) {
		res += "mem <size> - задать максимальный размер используемой оперативной памяти (байт)\n";
		res += "(текущее значение: " + UNumToString(settings->MaxMapping) + " байт)\n";
	}
	res += "range <file> [<min> <max>] - диапазон\n";
	res += "search <file> prefix <substr> [<min> <max>] - поиск префикса\n";
	res += "search <file> postfix <substr> [<min> <max>] - поиск суффикса\n";
	res += "search <file > basic <substr> [<min> <max>] - обычный поиск\n";
	res += "edit <file> <row> <col> <text> [<min> <max>] - вставка в заданную позицию\n";
	res += "delete <file> <row> <col> <length> [<min> <max>]- удаление из заданной позиции\n";
	res += "exit (или `Ctrl+D`)- выход\n\n";
	res += "Перед пробелом используйте знак '\\'\n";
	res += "--------------------------------------\n";
	
	return res;
}

//Action console
string ConsolePerformer(string command, Settings *settings) {
	command = RemoveSpecial(command);
	string action = GetParameter(command, 0);
	if (action == "help") {
		return GetManual(true, settings);

	} else if (action == "range") {
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			return "Cannot open file\n";
		}
		size_t min = StringToUNum(GetParameter(command, 2));
		size_t max = StringToUNum(GetParameter(command, 3));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			return "File size is out of range\n";
		}
		return GetRange(filename, settings);
	} else if (action == "search") {
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			return "Cannot open file\n";
		}
		size_t min = StringToUNum(GetParameter(command, 4));
		size_t max = StringToUNum(GetParameter(command, 5));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			return "File size is out of range\n";
		}

		string type = GetParameter(command, 2);
		string substr = GetParameter(command, 3);

		if (type == "basic") {
			return SearchInFileBasic(filename, substr, settings);
		} else if (type == "prefix") {
			return SearchInFilePrefix(filename, substr, settings);
		} else if (type == "postfix") {
			return SearchInFilePostfix(filename, substr, settings);
		} else {
			return PARAMETER_INCORRECT_MESSAGE;
		}

	} else if (action == "edit") {
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			return "Cannot open file\n";
		}
		size_t min = StringToUNum(GetParameter(command, 5));
		size_t max = StringToUNum(GetParameter(command, 6));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			return "File size is out of range\n";
		}

		size_t row = StringToUNum(GetParameter(command, 2));
		size_t col = StringToUNum(GetParameter(command, 3));
		string text = GetParameter(command, 4);
		return EditWrite(filename, row, col, text, settings);

	} else if (action == "delete") {
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			return "Cannot open file\n";
		}

		size_t min = StringToUNum(GetParameter(command, 5));
		size_t max = StringToUNum(GetParameter(command, 6));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			return "File size is out of range\n";
		}

		size_t row = StringToUNum(GetParameter(command, 2));
		size_t col = StringToUNum(GetParameter(command, 3));

		size_t length = StringToUNum(GetParameter(command, 4));
		return EditDelete(filename, row, col, length, settings);

	} else {
		return "Unknown command: `" + command + "`\n";
	}
}


int main(int argc, char* argv[]) {
	setlocale(LC_ALL, "russian");
	if (argc < 2) {
		cout << "ERROR: Parameters expected" << endl;
		return 0;
	}
	string argv_1(argv[1]);
	if (argv_1 == "-i" || argv_1 == "--interactive") {
		Settings settings;
		cout << "`help` - список команд" << endl << endl;
		while (true) {
			string cmd = "";
			cout << ">>> ";
			getline(cin, cmd);
			if (cmd == "exit" || cmd.size() == 0) {
				if (cmd.size() == 0) {
					cout << "exit" << endl;
				}
				cout << "Goodbye!" << endl;
				break;
			} else if (GetParameter(cmd, 0) == "mem") {
				string num_param = GetParameter(cmd, 1);
				if (IsStringNum(num_param) && num_param != "") {
					size_t tmp_val = StringToUNum(num_param);
					if (tmp_val == 0) {
						cout << "ERROR: Parameter can not be `0`" << endl << endl;
						continue;
					}
					settings.MaxMapping = tmp_val;
					cout << "Maximal using memory change: Success" << endl;
					cout << "New maximal using memory value: `" << settings.MaxMapping << "`" << endl << endl;
				} else {
					cout << PARAMETER_INCORRECT_MESSAGE << endl;
				}
				continue;
			}
			string result = ConsolePerformer(cmd, &settings);
			cout << result << endl;
		}
	} else if (argv_1 == "-h" || argv_1 == "--help" || argv_1 == "help") {
		Settings settings;
		cout << GetManual(false, &settings) << endl;
	} else {
		Settings settings;
		if (argc < 3) {
			cout << PARAMETER_INCORRECT_MESSAGE << endl;
			return 0;
		}
		size_t first_param = 1;
		if (argv_1 == "-m") {
			string max_mapping_str(argv[2]);
			if (!IsStringNum(max_mapping_str)) {
				cout << "ERROR: Parameter `-m` is not `unsigned long long`" << endl;
				return 0;
			}
			settings.MaxMapping = StringToUNum(max_mapping_str);
			first_param = 3;
		}
		string cmd = "";
		for (size_t i = first_param; i < argc; i++) {
			string str_tmp(argv[i]);
			if (i != first_param) {
				cmd += " ";
			}
			cmd += str_tmp;
		}
		string result = ConsolePerformer(cmd, &settings);
		cout << result << endl;
	}

	return 0;
}
