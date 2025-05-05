#pragma once

#include <vector>
#include <string>
#include <memory>
#include <unordered_set>
#include <queue>

using namespace std;

class ClubState;

//родительский класс
class Event {
protected:
	int time;
public:
	explicit Event(int t) : time(t) {};
	virtual ~Event() = default;

	int getTime() const { return time; } //возвращает время события
	virtual int getId() const = 0; //id события
	virtual void apply(ClubState& state, vector<unique_ptr<Event>>& out_events) = 0; //применить событие к состоянию клуба
	virtual std::string toString() const = 0; //строка для вывода
};


//ID 1. Клиент пришел
class ArriveEvent : public Event {
	std::string clientName;
public:
	ArriveEvent(int t, const std::string& clientName);
	int getId() const override { return 1; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};


//ID 2. Клиент сел за стол 
class SitEvent : public Event {
	std::string clientName;
	int table;
public:
	SitEvent(int t, const std::string& clientName, int tableNumber);
	int getId() const override { return 2; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};

//ID 3. Клиент ожидает
class WaitEvent : public Event {
	std::string clientName;
public:
	WaitEvent(int t, const std::string& clientName);
	int getId() const override { return 3; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};

//ID 4. Клиент ушел
class LeaveEvent : public Event {
	std::string client;
public:
	LeaveEvent(int t, const std::string& clientName);
	int getId() const override { return 4; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};

//ID 11. Клиент ушел
class ClientLeftEvent : public Event {
	std::string clientName;
public:
	ClientLeftEvent(int t, const std::string& clientName);
	int getId() const override { return 11; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};

//ID 12. Клиент сел за стол
class AutoSeatEvent : public Event{
	std::string clientName;
	int table;
public:
	AutoSeatEvent(int t, const std::string& clientName, int tableNumber);
	int getId() const override { return 12; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};

//ID 13. Ошибка
class ErrorEvent : public Event {
	std::string message;
public:
	ErrorEvent(int t, const std::string& errorMsg);
	int getId() const override { return 13; }
	void apply(ClubState& state, std::vector<std::unique_ptr<Event>>& out_events) override;
	std::string toString() const override;
};

//класс хранения данных клуба
class ClubState {
private:
	int numTables;
	int openMinutes;
	int closeMinutes;
	int price;
	unordered_set<string> inClub;
	queue<string> waiting;

	struct TableInfo {
		bool occupied = false; //занят или нет
		string clientName;
		int startMinutes = 0; //когда сел
		int totalMinutes = 0; //общее количество минут за столом
		int totalRevenue = 0; //общая выручка
	};

	vector<TableInfo> tables;
	int totalRevenue = 0;

public:
	ClubState(int numTables, int openMinutes, int closeMinutes, int price); //конструктор

	void arrive(const string& clientName, int timeMinutes, vector <unique_ptr<Event>>& out_events);
	void sit(const string& clientName, int table, int timeMinutes, vector <unique_ptr<Event>>& out_events);
	void wait(const string& clientName, int timeMinutes, vector <unique_ptr<Event>>& out_events);
	void leave(const string& clientName, int timeMinutes, vector <unique_ptr<Event>>& out_events);

	void closeDay(vector <unique_ptr<Event>>& out_events);

	int getTableRevenue(int table) const;
	int getTableUsage(int table) const;
	int getTotalRevenue() const;
};