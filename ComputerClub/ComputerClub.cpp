// ComputerClub.cpp: определяет точку входа для приложения.

#include "ComputerClub.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <iomanip>
#include <stdexcept>

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

//функция йпреобразования минут во время HH:MM
static std::string formatTime(int t) {
	int h = t / 60;
	int m = t % 60;
	std::ostringstream oss;
	oss << std::setw(2) << std::setfill('0') << h
		<< ':'
		<< std::setw(2) << std::setfill('0') << m;
	return oss.str();
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
					out_events.emplace_back(new ErrorEvent(timeM, "BadFormat"));
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
	cout << openTime << "\n";
	for (auto& ev : out_events) {
		cout << ev->toString() << "\n";
	}
	cout << closeTime << "\n";

	for (int i = 1; i <= numTables; ++i) {
		int rev = state.getTableRevenue(i);
		int mins = state.getTableUsage(i);
		int h = mins / 60, m = mins % 60;

		cout << i << " "
			<< rev << " "
			<< setw(2) << setfill('0') << h << ":"
			<< setw(2) << setfill('0') << m
			<< "\n";
	}
	return 0;
}

// ---------------------- ArriveEvent ----------------------
ArriveEvent::ArriveEvent(int t, const std::string& clientName)
	: Event(t), clientName(clientName) {}

void ArriveEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.arrive(clientName, time, out_events);
}

string ArriveEvent::toString() const {
	return formatTime(time) + " 1 " + clientName;
}

// ---------------------- SitEvent ----------------------
SitEvent::SitEvent(int t, const std::string& clientName, int tableNumber)
	: Event(t), clientName(clientName), table(tableNumber) {}

void SitEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.sit(clientName, table, time, out_events);
}

string SitEvent::toString() const {
	return formatTime(time) + " 2 " + clientName + " " + to_string(table);
}

// ---------------------- WaitEvent ----------------------
WaitEvent::WaitEvent(int t, const std::string& clientName)
	: Event(t), clientName(clientName) {}

void WaitEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.wait(clientName, time, out_events);
}

string WaitEvent::toString() const {
	return formatTime(time) + " 3 " + clientName;
}

// ---------------------- LeaveEvent ----------------------
LeaveEvent::LeaveEvent(int t, const std::string& clientName)
	: Event(t), client(clientName) {}

void LeaveEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.leave(client, time, out_events);
}

string LeaveEvent::toString() const {
	return formatTime(time) + " 4 " + client;
}

// ---------------------- ClientLeftEvent ----------------------
ClientLeftEvent::ClientLeftEvent(int t, const std::string& clientName)
	: Event(t), clientName(clientName) {}

void ClientLeftEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.leave(clientName, time, out_events);
}

string ClientLeftEvent::toString() const {
	return formatTime(time) + " 11 " + clientName;
}

// ---------------------- AutoSeatEvent ----------------------
AutoSeatEvent::AutoSeatEvent(int t, const std::string& clientName, int tableNumber)
	: Event(t), clientName(clientName), table(tableNumber) {}

void AutoSeatEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	state.sit(clientName, table, time, out_events);
}

string AutoSeatEvent::toString() const {
	return formatTime(time) + " 12 " + clientName + " " + to_string(table);
}

// ---------------------- ErrorEvent ----------------------
ErrorEvent::ErrorEvent(int t, const std::string& errorMsg)
	: Event(t), message(errorMsg) {}

void ErrorEvent::apply(ClubState& state, vector<unique_ptr<Event>>& out_events) {
	// Ничего не делает
}

string ErrorEvent::toString() const {
	return formatTime(time) + " 13 " + message;
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
	for (const auto& table : tables) {
		if (!table.occupied) {
			out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "ICanWaitNoLonger!"));
			return;
			
		}
	}

	// Добавление в очередь
	waiting.push(clientName);
	    // Если после вставания очередь стала длиннее, чем столов — клиент уходит
		if ((int)waiting.size() > numTables) {
		        // удаляем его из очереди и генерируем уход (ID 11)
			waiting.pop();
		out_events.emplace_back(make_unique<ClientLeftEvent>(timeMinutes, clientName));
		
	}
}


void ClubState::leave(const string& clientName, int timeMinutes, vector<unique_ptr<Event>>& out_events) {
	if (!inClub.count(clientName)) {
		out_events.emplace_back(make_unique<ErrorEvent>(timeMinutes, "ClientUnknown"));
		return;
	}

	// Поиск и освобождение стола, если клиент сидел
	for (int i = 0; i < numTables; ++i) {
		if (tables[i].occupied && tables[i].clientName == clientName) {
			int sessionMinutes = timeMinutes - tables[i].startMinutes;
			tables[i].totalMinutes += sessionMinutes;

			// Округление вверх по часам
			int hours = (sessionMinutes + 59) / 60;
			int earned = hours * price;
			tables[i].totalRevenue += earned;
			totalRevenue += earned;

			tables[i].occupied = false;
			tables[i].clientName.clear();
			break;
		}
	}

	inClub.erase(clientName);

	// Если кто-то ждет — посадить автоматически
	if (!waiting.empty()) {
		string nextClient = waiting.front();
		waiting.pop();

		// Найти первый свободный стол
		for (int i = 0; i < numTables; ++i) {
			if (!tables[i].occupied) {
				tables[i].occupied = true;
				tables[i].clientName = nextClient;
				tables[i].startMinutes = timeMinutes;
				out_events.emplace_back(make_unique<AutoSeatEvent>(timeMinutes, nextClient, i + 1));
				break;
			}
		}
	}
}


void ClubState::closeDay(vector<unique_ptr<Event>>& out_events) {
	// Закрытие всех занятых столов
	for (int i = 0; i < numTables; ++i) {
		if (tables[i].occupied) {
			int sessionMinutes = closeMinutes - tables[i].startMinutes;
			tables[i].totalMinutes += sessionMinutes;

			int hours = (sessionMinutes + 59) / 60;
			int earned = hours * price;
			tables[i].totalRevenue += earned;
			totalRevenue += earned;

			// Событие ухода клиента
			out_events.emplace_back(make_unique<ClientLeftEvent>(closeMinutes, tables[i].clientName));

			tables[i].occupied = false;
			tables[i].clientName.clear();
		}
	}

	// Удаление клиентов из очереди
	while (!waiting.empty()) {
		string client = waiting.front();
		waiting.pop();
		out_events.emplace_back(make_unique<ClientLeftEvent>(closeMinutes, client));
	}

	inClub.clear();
}


int ClubState::getTableRevenue(int table) const {
	if (table < 1 || table > numTables) return 0;
	return tables[table - 1].totalRevenue;
}

int ClubState::getTableUsage(int table) const {
	if (table < 1 || table > numTables) return 0;
	return tables[table - 1].totalMinutes;
}

int ClubState::getTotalRevenue() const {
	return totalRevenue;
}


