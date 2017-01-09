#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "TextProcessor.h"

using namespace std;

class Settings {
public:
	string Filename;
	bool IsFileLenLimit;
	size_t MinLength;
	size_t MaxLength;
	short int SearchMode;
	size_t MaxMapping;

	Settings() {
		Filename = "";
		IsFileLenLimit = false;
		MinLength = 0;
		MaxLength = 0;
		SearchMode = 0;
		MaxMapping = 256;
	}
};

//Action console
string ConsolePerformer(string command, Settings *settings) {
	size_t max_mapping = settings->MaxMapping;
	//max_mapping = ConvertToBytes(max_mapping, ui->SettingsMemoryLimitType->currentIndex());
	//string command = ui->ConsoleIn->text();
	//command = command.simplified();
	command = RemoveSpecial(command);
	//string action = command.section(' ', 0, 0);
	string action = GetParameter(command, 0);
	//cout << "Dbg: " << action << endl;
	if (action == "help") {
		string res = "";
		res += "Список команд:\n";
		res += "======================================\n";
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

	} else if (action == "range") {
		//string filename = command.section(' ', 1, 1);
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			//ui->ConsoleOut->setText("Cannot open file\n");
			//this->setCursor(Qt::ArrowCursor);
			return "Cannot open file\n";
		}
		//size_t min = command.section(' ', 2, 2).toULongLong();
		//size_t max = command.section(' ', 3, 3).toULongLong();
		size_t min = StringToUNum(GetParameter(command, 2));
		size_t max = StringToUNum(GetParameter(command, 3));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			//ui->ConsoleOut->setText("File size is out of range\n");
			//this->setCursor(Qt::ArrowCursor);
			return "File size is out of range\n";
		}
		return GetRange(filename, max_mapping);
	} else if (action == "search") {
		//string filename = command.section(' ', 1, 1);
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			//ui->ConsoleOut->setText("Cannot open file\n");
			//this->setCursor(Qt::ArrowCursor);
			return "Cannot open file\n";
		}
		//size_t min = command.section(' ', 4, 4).toULongLong();
		//size_t max = command.section(' ', 5, 5).toULongLong();
		size_t min = StringToUNum(GetParameter(command, 4));
		size_t max = StringToUNum(GetParameter(command, 5));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			//ui->ConsoleOut->setText("File size is out of range\n");
			//this->setCursor(Qt::ArrowCursor);
			return "File size is out of range\n";
		}

		//string type = command.section(' ', 2, 2);
		string type = GetParameter(command, 2);
		/*string substr = command.section(' ', 3, 3);
		substr = SetSpaces(substr);*/
		string substr = GetParameter(command, 3);

		//string res = "";
		if (type == "basic") {
			return SearchInFileBasic(filename, substr, max_mapping);
		} else if (type == "prefix") {
			return SearchInFilePrefix(filename, substr, max_mapping);
		} else if (type == "postfix") {
			return SearchInFilePostfix(filename, substr, max_mapping);
		} else {
			return "Parameter is incorrect\n";
		}
		//return res;

	} else if (action == "edit") {
		//string filename = command.section(' ', 1, 1);
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			//ui->ConsoleOut->setText("Cannot open file\n");
			//this->setCursor(Qt::ArrowCursor);
			return "Cannot open file\n";
		}
		//size_t min = command.section(' ', 5, 5).toULongLong();
		//size_t max = command.section(' ', 6, 6).toULongLong();
		size_t min = StringToUNum(GetParameter(command, 5));
		size_t max = StringToUNum(GetParameter(command, 6));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			//ui->ConsoleOut->setText("File size is out of range\n");
			//this->setCursor(Qt::ArrowCursor);
			return "File size is out of range\n";
		}

		//size_t row = command.section(' ', 2, 2).toULongLong();
		//size_t col = command.section(' ', 3, 3).toULongLong();

		size_t row = StringToUNum(GetParameter(command, 2));
		size_t col = StringToUNum(GetParameter(command, 3));
		//string text = command.section(' ', 4, 4);
		string text = GetParameter(command, 4);
		return EditWrite(filename, row, col, text, max_mapping);

	} else if (action == "delete") {
		//string filename = command.section(' ', 1, 1);
		string filename = GetParameter(command, 1);
		if (!IsFileExists(filename)) {
			//ui->ConsoleOut->setText("Cannot open file\n");
			//this->setCursor(Qt::ArrowCursor);
			return "Cannot open file\n";
		}
		//size_t min = command.section(' ', 5, 5).toULongLong();
		//size_t max = command.section(' ', 6, 6).toULongLong();

		size_t min = StringToUNum(GetParameter(command, 5));
		size_t max = StringToUNum(GetParameter(command, 6));
		bool is_limit = true;
		if (min == 0 && max == 0) {
			is_limit = false;
		}
		size_t size = FileSize(filename);
		if ((size > max || size < min) && is_limit) {
			//ui->ConsoleOut->setText("File size is out of range\n");
			//this->setCursor(Qt::ArrowCursor);
			return "File size is out of range\n";
		}

		//size_t row = command.section(' ', 2, 2).toULongLong();
		//size_t col = command.section(' ', 3, 3).toULongLong();
		size_t row = StringToUNum(GetParameter(command, 2));
		size_t col = StringToUNum(GetParameter(command, 3));

		//size_t length = command.section(' ', 4, 4).toULongLong();
		size_t length = StringToUNum(GetParameter(command, 4));
		return EditDelete(filename, row, col, length, max_mapping);

	} else {
		//ui->ConsoleOut->setText("Unknown command\n");
		return "Unknown command: `" + command + "`\n";
	}
	//this->setCursor(Qt::ArrowCursor);

	/*
	this->setCursor(Qt::WaitCursor);
	string filename = ui->InteractiveFileName->text();
	if (!IsCurrentFileInRange(filename, ui)) {
		ui->InteractiveResult->setText("File size is out of range\n");
		return;
	}
	ui->InteractiveResult->setText(GetRange(filename));
	this->setCursor(Qt::ArrowCursor); */
}


int main(void) {
	setlocale(LC_ALL, "russian");
	/*cout << "-------------------------------------------\n";
	cout << "Commands:\n";
	cout << "s <value> - push Square to stack\n";
	cout << "t <value> - push Trapeze to stack\n";
	cout << "r <value> - push Rectangle to stack\n";
	cout << "d - pop from stack and print popped item\n";
	cout << "p - print stack\n";
	cout << "q - exit\n";
	cout << "-------------------------------------------\n";
	//int cnt = 0;*/
	Settings *settings = new Settings();
	cout << "`help` - список команд" << endl << endl;
	while (true) {
		string cmd = "";
		cout << ">>> ";
		//cin >> cmd;
		getline(cin, cmd);
		if (cmd == "exit" || cmd.size() == 0) {
			if (cmd.size() == 0) {
				cout << "exit" << endl;
			}
			cout << "Goodbye!" << endl;
			break;
		}
		//string result = GetParameter(cmd, 0)
		string result = ConsolePerformer(cmd, settings);
		cout << result << endl;
	}
	delete settings;

	return 0;
}
