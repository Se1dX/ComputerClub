// ComputerClub.cpp: определяет точку входа для приложения.

#include "ComputerClub.h"
#include <iostream>
#include <fstream>
#include <sstream>

using namespace std;

//функция парсинга времени в минуты
int timeToMinutes(const string & time) {
	int hour, minute;
	char sep;
	istringstream s(time);
	s >> hour >> sep >> minute;

	if (!s || sep != ':' || hour < 0 || hour > 23 || minute < 0 || minute > 59) {
		throw runtime_error("Error: Неверный формат времени " + time);
	}
	return hour * 60 + minute;
}


int main()
{
	setlocale(LC_ALL, "Russian");


	//поток чтения файла
	ifstream infile("test_file.txt");
	if (!infile) { cerr << "Error: Ошибка  при открытии файла!\n"; return 1; }

	//чтение первой строки
	string firstLine;
	if (!getline(infile, firstLine)) {
		cerr << "Error: Файл пуст.\n";
		return 1;
	}

	//парсинг из строки в число
	istringstream iss1(firstLine);
	int numTables;
	if (!(iss1 >> numTables) || numTables <= 0) {
		cerr << "Error: Неверный формат количества столов.\n";
		return 1;
	}

	//чтение второй строки
	string secondLine;
	if (!getline(infile, secondLine)) {
		cerr << "Error: Отсутствует вторая строка.\n";
		return 1;
	}

	istringstream iss2(secondLine);
	string openTime, closeTime;
	if (!(iss2 >> openTime >> closeTime)) {
		cerr << "Error: Неверный формат времени открытия и закрытия. Ожидается HH:MM HH:MM.\n";
		return 1;
	}

	//парсинг времени в количество минут
	int openMinutes = timeToMinutes(openTime);
	int closeMinutes = timeToMinutes(closeTime);


	//чтение третьей строки
	string thirdLine;
	if (!getline(infile, thirdLine)) {
		cerr << "Error: Отсутствует третья строка.\n";
		return 1;
	}

	istringstream iss3(thirdLine);
	int price;
	if (!(iss3 >> price) || price <= 0) {
		cerr << "Error: Неверный формат цены за час.\n";
		return 1;
	}

	ClubState state(numTables, openMinutes, closeMinutes, price);

	vector<unique_ptr<Event>> out_events;

	string line;
	while (getline(infile, line)) {
		if (line.empty()) continue; //пропуск пустых строк

		istringstream iss(line);
		string timeStr;
		int id;
		iss >> timeStr >> id;
		
		int timeM = timeToMinutes(timeStr);

		switch (id) {
		    case 1: {
				string clientName;
				iss >> clientName;
				if (!iss) {
					out_events.emplace_back(new ErrorEvent(timeM, "BadFormat"));
					break;
				}
				out_events.emplace_back(new ArriveEvent(timeM, clientName));
				state.arrive(clientName, timeM, out_events);
				break;

		    }
			case 2: {
				string clientName;
				int table;
				iss >> clientName >> table;
				if (!iss) {
					out_events.emplace_back(new SitEvent(timeM, "BadFormat"));
					break;
				}
				out_events.emplace_back(new SitEvent(timeM, clientName, table));
				state.sit(clientName, table, timeM, out_events);
				break;
			}
			case 3: {
				string clientName;
				iss >> clientName;
				if (!iss) {
					out_events.emplace_back(new ErrorEvent(timeM, "BadFormat"));
					break;
				}
				out_events.push_back(make_unique<WaitEvent>(timeM, clientName));
				state.wait(clientName, timeM, out_events);
				break;
			}
			case 4: {
				string clientName;
				iss >> clientName;
				if (!iss) {
					out_events.emplace_back(new ErrorEvent(timeM, "BadFormat"));
					break;
				}
				out_events.emplace_back(new LeaveEvent(timeM, clientName));
				state.leave(clientName, timeM, out_events);

				break;
			}
			default:
				out_events.emplace_back(new ErrorEvent(timeM, "UnknownID"));
				break;
		}
	}

	state.closeDay(out_events);

	return 0;
}

// ---------------------- ArriveEvent ----------------------
ArriveEvent::ArriveEvent(int t, const std::string& clientName)
	: Event(t), clientName(clientName) {}

void ArriveEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.arrive(clientName, time, out_events);
}

string ArriveEvent::toString() const {
	return to_string(time) + " 1 " + clientName;
}

// ---------------------- SitEvent ----------------------
SitEvent::SitEvent(int t, const std::string& clientName, int tableNumber)
	: Event(t), clientName(clientName), table(tableNumber) {}

void SitEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.sit(clientName, table, time, out_events);
}

string SitEvent::toString() const {
	return to_string(time) + " 2 " + clientName + " " + to_string(table);
}

// ---------------------- WaitEvent ----------------------
WaitEvent::WaitEvent(int t, const std::string& clientName)
	: Event(t), clientName(clientName) {}

void WaitEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.wait(clientName, time, out_events);
}

string WaitEvent::toString() const {
	return to_string(time) + " 3 " + clientName;
}

// ---------------------- LeaveEvent ----------------------
LeaveEvent::LeaveEvent(int t, const std::string& clientName)
	: Event(t), client(clientName) {}

void LeaveEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.leave(client, time, out_events);
}

string LeaveEvent::toString() const {
	return to_string(time) + " 4 " + client;
}

// ---------------------- ClientLeftEvent ----------------------
ClientLeftEvent::ClientLeftEvent(int t, const std::string& clientName)
	: Event(t), clientName(clientName) {}

void ClientLeftEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.leave(clientName, time, out_events);
}

string ClientLeftEvent::toString() const {
	return to_string(time) + " 11 " + clientName;
}

// ---------------------- AutoSeatEvent ----------------------
AutoSeatEvent::AutoSeatEvent(int t, const std::string& clientName, int tableNumber)
	: Event(t), clientName(clientName), table(tableNumber) {}

void AutoSeatEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.sit(clientName, table, time, out_events);
}

string AutoSeatEvent::toString() const {
	return to_string(time) + " 12 " + clientName + " " + to_string(table);
}

// ---------------------- ErrorEvent ----------------------
ErrorEvent::ErrorEvent(int t, const std::string& errorMsg)
	: Event(t), message(errorMsg) {}

void ErrorEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	// Ничего не делает
}

string ErrorEvent::toString() const {
	return to_string(time) + " 13 " + message;
}

//МЕТОДЫ CLUBSTATE
ClubState::ClubState(int numTables, int openMinutes, int closeMinutes, int price)
	: numTables(numTables), openMinutes(openMinutes), closeMinutes(closeMinutes), price(price), tables(numTables) {}

void ClubState::arrive(const string& clientName, int timeMinutes, vector<unique_ptr<Event>>& out_events) {
	if (timeMinutes < openMinutes || timeMinutes >= closeMinutes) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "NotOpenYet"));
		return;
	}

	if (inClub.count(clientName)) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "YouShallNotPass"));
		return;
	}

	inClub.insert(clientName);
}


void ClubState::sit(const string& clientName, int table, int timeMinutes, vector<unique_ptr<Event>>& out_events) {
	if (!inClub.count(clientName)) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "ClientUnknown"));
		return;
	}

	// Проверим, не сидит ли он уже
	for (const auto& t : tables) {
		if (t.occupied && t.clientName == clientName) {
			out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "PlaceIsBusy"));
			return;
		}
	}

	// Проверка корректности номера стола
	if (table < 1 || table > numTables) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "InvalidTable"));
		return;
	}

	TableInfo& tableInfo = tables[table - 1];
	if (tableInfo.occupied) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "PlaceIsBusy"));
		return;
	}

	// Садим клиента
	tableInfo.occupied = true;
	tableInfo.clientName = clientName;
	tableInfo.startMinutes = timeMinutes;

	// Удаляем из очереди, если он там есть
	queue<string> tempQueue;
	while (!waiting.empty()) {
		string front = waiting.front();
		waiting.pop();
		if (front != clientName) {
			tempQueue.push(front);
		}
	}
	swap(waiting, tempQueue);
}


void ClubState::wait(const string& clientName, int timeMinutes, vector<unique_ptr<Event>>& out_events) {
	// Проверка, что клиент в клубе
	if (!inClub.count(clientName)) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "ClientUnknown"));
		return;
	}

	// Проверка, не сидит ли клиент уже
	for (const auto& table : tables) {
		if (table.occupied && table.clientName == clientName) {
			out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "ClientAlreadySeated"));
			return;
		}
	}

	// Проверка, что очередь не пуста — один может ждать, больше нельзя
	if (!waiting.empty()) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "QueueIsBusy"));
		return;
	}

	// Добавление в очередь
	waiting.push(clientName);
}