#ifndef REPOSITORY_H
#define REPOSITORY_H

#define _CRT_SECURE_NO_WARNINGS

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include <ctime>
#include <fstream>
#include <sstream>

using namespace std;

enum class LockerStatus { Free, Occupied };
enum class EquipmentStatus { Available, CheckedOut};
enum class HallStatus { Free, Booked};
enum class BookingStatus { Confirmed, Cancelled, Completed };
enum class UserType { Client, Administrator, Trainer };

// Вспомогательные функции
inline string lockerStatusToString(LockerStatus status) {
    return (status == LockerStatus::Occupied) ? "Occupied" : "Free";
}

inline LockerStatus stringToLockerStatus(const string& str) {
    if (str == "Occupied") return LockerStatus::Occupied;
    return LockerStatus::Free;
}

inline string equipmentStatusToString(EquipmentStatus status) {
    switch (status) {
    case EquipmentStatus::CheckedOut: return "CheckedOut";
    default: return "Available";
    }
}

inline EquipmentStatus stringToEquipmentStatus(const string& str) {
    if (str == "CheckedOut") return EquipmentStatus::CheckedOut;
    return EquipmentStatus::Available;
}

inline string hallStatusToString(HallStatus status) {
    switch (status) {
    case HallStatus::Booked: return "Booked";
    default: return "Free";
    }
}

inline HallStatus stringToHallStatus(const string& str) {
    if (str == "Booked") return HallStatus::Booked;
    return HallStatus::Free;
}

class User {
public:
    int Id;
    string Name;
    virtual UserType GetType() const = 0;
    virtual string GetRoleName() const = 0;
    virtual ~User() {}
};

class Client : public User {
public:
    string MembershipType = "Стандартный";
    string NfcTag;
    vector<int> BookingIds;
    bool isEntered = false;
    UserType GetType() const override { return UserType::Client; }
    string GetRoleName() const override { return "клиент"; }
};

class Administrator : public User {
public:
    UserType GetType() const override { return UserType::Administrator; }
    string GetRoleName() const override { return "администратор"; }
};

class Trainer : public User {
public:
    string Specialization;
    UserType GetType() const override { return UserType::Trainer; }
    string GetRoleName() const override { return "тренер"; }
};

struct Locker {
    int Id;
    LockerStatus Status = LockerStatus::Free;
    int AssignedToClientId = -1;
    void Occupy(int cid) { Status = LockerStatus::Occupied; AssignedToClientId = cid; }
    void Release() { Status = LockerStatus::Free; AssignedToClientId = -1; }
};

struct Equipment {
    int Id;
    string Name;
    EquipmentStatus Status = EquipmentStatus::Available;
    int CheckedOutToClientId = -1;
    void CheckOut(int cid) { Status = EquipmentStatus::CheckedOut; CheckedOutToClientId = cid; }
    void CheckIn() { Status = EquipmentStatus::Available; CheckedOutToClientId = -1; }
};

struct Hall {
    int Id;
    string Name;
    int Capacity;
    HallStatus Status = HallStatus::Free;
    void Book() { Status = HallStatus::Booked; }
    void Release() { Status = HallStatus::Free; }
};

struct ScheduleEntry {
    int Id;
    string Activity;
    int TrainerId = -1, HallId = -1;
    time_t StartTime = 0, EndTime = 0;
    int MaxClients = 0;
    vector<int> ClientIds;
    int FreeSlots() const { return MaxClients - (int)ClientIds.size(); }
    bool IsCompleted() const { return time(nullptr) > EndTime; }
};

struct Booking {
    int Id = 0, ClientId = -1, TrainerId = -1, ScheduleId = -1;
    BookingStatus Status = BookingStatus::Confirmed;
    void Cancel() { Status = BookingStatus::Cancelled; }
    void Complete() { Status = BookingStatus::Completed; }
};

struct EventLog {
    int Id;
    string Timestamp;
    string Actor;
    string ActorName;
    string Action;
    string Details;
    int RelatedId;
};

class Repository {
public:
    vector<Client> Clients;
    vector<Administrator> Admins;
    vector<Trainer> Trainers;
    vector<Locker> Lockers;
    vector<Equipment> EquipmentList;
    vector<Hall> Halls;
    vector<ScheduleEntry> Schedule;
    vector<Booking> Bookings;
    vector<EventLog> EventLogs;
    int nextLogId = 1000;
    int nextBookingId = 1000;
    int nextScheduleId = 500;

    Repository() {
        cout << "=== Loading data from database.txt ===" << endl;
        LoadAllFromFile();
        cout << "=== Load complete ===" << endl;
    }

    string Trim(const string& str) {
        size_t first = str.find_first_not_of(" \t\r\n");
        size_t last = str.find_last_not_of(" \t\r\n");
        return (first == string::npos || last == string::npos) ? "" : str.substr(first, last - first + 1);
    }

    void LoadAllFromFile() {
        // Пробуем открыть файл
        ifstream file("database.txt");
        if (!file.is_open()) {
            cout << "ERROR: database.txt not found!" << endl;
            return;
        }

        // Очищаем существующие данные
        Clients.clear();
        Admins.clear();
        Trainers.clear();
        Lockers.clear();
        EquipmentList.clear();
        Halls.clear();

        string line;
        string currentSection = "";
        int lineNum = 0;

        // Вспомогательная лямбда-функция для удаления пробелов и символов возврата каретки
        auto clean = [](string& s) {
            // Удаляем \r
            if (!s.empty() && s.back() == '\r') {
                s.pop_back();
            }
            // Удаляем пробелы в начале и конце
            size_t start = s.find_first_not_of(" \t");
            size_t end = s.find_last_not_of(" \t");
            if (start == string::npos) {
                s = "";
            }
            else {
                s = s.substr(start, end - start + 1);
            }
            };

        // Вспомогательная лямбда-функция для split по разделителю
        auto split = [](const string& str, char delimiter) -> vector<string> {
            vector<string> result;
            stringstream ss(str);
            string item;
            while (getline(ss, item, delimiter)) {
                // Очищаем каждый элемент от пробелов
                size_t start = item.find_first_not_of(" \t");
                size_t end = item.find_last_not_of(" \t");
                if (start != string::npos) {
                    item = item.substr(start, end - start + 1);
                }
                result.push_back(item);
            }
            return result;
            };

        while (getline(file, line)) {
            lineNum++;
            clean(line);

            // Пропускаем пустые строки и комментарии
            if (line.empty()) continue;
            if (line[0] == '#') continue;
            if (line[0] == '/' && line[1] == '/') continue;

            // Проверка на секцию
            if (line[0] == '[' && line.back() == ']') {
                currentSection = line;
                cout << "Reading section: " << currentSection << " (line " << lineNum << ")" << endl;
                continue;
            }

            // Если нет активной секции, пропускаем
            if (currentSection.empty()) continue;

            // Разбиваем строку на части
            vector<string> parts = split(line, '|');

            // Обработка секций
            if (currentSection == "[CLIENTS]") {
                if (parts.size() >= 4) {
                    Client c;
                    c.Id = stoi(parts[0]);
                    c.Name = parts[1];
                    c.MembershipType = parts[2];
                    c.NfcTag = parts[3];
                    Clients.push_back(c);
                    cout << "  Loaded client: " << c.Id << "|" << c.Name << endl;
                }
                else {
                    cout << "  WARNING: Invalid client line: " << line << endl;
                }
            }
            else if (currentSection == "[ADMINS]") {
                if (parts.size() >= 2) {
                    Administrator a;
                    a.Id = stoi(parts[0]);
                    a.Name = parts[1];
                    Admins.push_back(a);
                    cout << "  Loaded admin: " << a.Id << "|" << a.Name << endl;
                }
                else {
                    cout << "  WARNING: Invalid admin line: " << line << endl;
                }
            }
            else if (currentSection == "[TRAINERS]") {
                if (parts.size() >= 3) {
                    Trainer t;
                    t.Id = stoi(parts[0]);
                    t.Name = parts[1];
                    t.Specialization = parts[2];
                    Trainers.push_back(t);
                    cout << "  Loaded trainer: " << t.Id << "|" << t.Name << endl;
                }
                else {
                    cout << "  WARNING: Invalid trainer line: " << line << endl;
                }
            }
            else if (currentSection == "[LOCKERS]") {
                if (parts.size() >= 3) {
                    Locker l;
                    l.Id = stoi(parts[0]);
                    l.Status = (parts[1] == "Occupied" || parts[1] == "occupied") ? LockerStatus::Occupied : LockerStatus::Free;
                    l.AssignedToClientId = stoi(parts[2]);
                    Lockers.push_back(l);
                    cout << "  Loaded locker: " << l.Id << "|" << parts[1] << endl;
                }
                else {
                    cout << "  WARNING: Invalid locker line: " << line << endl;
                }
            }
            else if (currentSection == "[EQUIPMENT]") {
                if (parts.size() >= 4) {
                    Equipment e;
                    e.Id = stoi(parts[0]);
                    e.Name = parts[1];

                    string status = parts[2];
                    if (status == "CheckedOut" || status == "checkedout") {
                        e.Status = EquipmentStatus::CheckedOut;
                    }
                    else {
                        e.Status = EquipmentStatus::Available;
                    }

                    e.CheckedOutToClientId = stoi(parts[3]);
                    EquipmentList.push_back(e);
                    cout << "  Loaded equipment: " << e.Id << "|" << e.Name << "|" << parts[2] << endl;
                }
                else {
                    cout << "  WARNING: Invalid equipment line: " << line << endl;
                }
            }
            else if (currentSection == "[HALLS]") {
                if (parts.size() >= 4) {
                    Hall h;
                    h.Id = stoi(parts[0]);
                    h.Name = parts[1];
                    h.Capacity = stoi(parts[2]);

                    string status = parts[3];
                    if (status == "Booked" || status == "booked") {
                        h.Status = HallStatus::Booked;
                    }
                    else {
                        h.Status = HallStatus::Free;
                    }

                    Halls.push_back(h);
                    cout << "  Loaded hall: " << h.Id << "|" << h.Name << "|" << parts[3] << endl;
                }
                else {
                    cout << "  WARNING: Invalid hall line: " << line << endl;
                }
            }
        }

        file.close();


        // Вывод итогов загрузки
        cout << "\n========================================" << endl;
        cout << "FINAL LOADED DATA:" << endl;
        cout << "  Clients: " << Clients.size() << endl;
        cout << "  Admins: " << Admins.size() << endl;
        cout << "  Trainers: " << Trainers.size() << endl;
        cout << "  Lockers: " << Lockers.size() << endl;
        cout << "  Equipment: " << EquipmentList.size() << endl;
        cout << "  Halls: " << Halls.size() << endl;
        cout << "========================================\n" << endl;
    }

    void SaveAllToFile() {
        ofstream file("database.txt");
        if (!file.is_open()) return;

        if (!Clients.empty()) {
            file << "[CLIENTS]\n";
            for (auto& c : Clients) {
                file << c.Id << "|" << c.Name << "|" << c.MembershipType << "|" << c.NfcTag << "\n";
            }
        }

        if (!Admins.empty()) {
            file << "\n[ADMINS]\n";
            for (auto& a : Admins) {
                file << a.Id << "|" << a.Name << "\n";
            }
        }

        if (!Trainers.empty()) {
            file << "\n[TRAINERS]\n";
            for (auto& t : Trainers) {
                file << t.Id << "|" << t.Name << "|" << t.Specialization << "\n";
            }
        }

        if (!Lockers.empty()) {
            file << "\n[LOCKERS]\n";
            for (auto& l : Lockers) {
                file << l.Id << "|" << lockerStatusToString(l.Status) << "|" << l.AssignedToClientId << "\n";
            }
        }

        if (!EquipmentList.empty()) {
            file << "\n[EQUIPMENT]\n";
            for (auto& e : EquipmentList) {
                file << e.Id << "|" << e.Name << "|" << equipmentStatusToString(e.Status) << "|" << e.CheckedOutToClientId << "\n";
            }
        }

        if (!Halls.empty()) {
            file << "\n[HALLS]\n";
            for (auto& h : Halls) {
                file << h.Id << "|" << h.Name << "|" << h.Capacity << "|" << hallStatusToString(h.Status) << "\n";
            }
        }

        file.close();
    }

    void SaveChanges() {
        SaveAllToFile();
    }

    template<typename T>
    T* Find(vector<T>& vec, int id) {
        auto it = find_if(vec.begin(), vec.end(), [id](const T& obj) { return obj.Id == id; });
        return it != vec.end() ? &(*it) : nullptr;
    }

    Client* FindClient(int id) { return Find(Clients, id); }
    Trainer* FindTrainer(int id) { return Find(Trainers, id); }
    Hall* FindHall(int id) { return Find(Halls, id); }
    Locker* FindLocker(int id) { return Find(Lockers, id); }
    Equipment* FindEquipment(int id) { return Find(EquipmentList, id); }
    ScheduleEntry* FindSchedule(int id) { return Find(Schedule, id); }
    Booking* FindBooking(int id) { return Find(Bookings, id); }

    User* FindUser(int id) {
        if (auto* u = FindClient(id)) return u;
        if (auto* u = Find(Trainers, id)) return u;
        if (auto* u = Find(Admins, id)) return u;
        return nullptr;
    }

    Locker* FindLockerByClient(int clientId) {
        auto it = find_if(Lockers.begin(), Lockers.end(),
            [clientId](Locker& l) { return l.AssignedToClientId == clientId; });
        return it != Lockers.end() ? &(*it) : nullptr;
    }

    Equipment* FindEquipmentByClient(int clientId) {
        auto it = find_if(EquipmentList.begin(), EquipmentList.end(),
            [clientId](Equipment& e) { return e.CheckedOutToClientId == clientId; });
        return it != EquipmentList.end() ? &(*it) : nullptr;
    }

    vector<Client*> GetClientsWithUnreturnedItems() {
        vector<Client*> result;
        for (auto& c : Clients) {
            if (!c.isEntered) {
                bool hasLocker = (FindLockerByClient(c.Id) != nullptr);
                bool hasEquipment = (FindEquipmentByClient(c.Id) != nullptr);
                if (hasLocker || hasEquipment) {
                    result.push_back(&c);
                }
            }
        }
        return result;
    }

    void AddLog(const string& actor, const string& actorName,
        const string& action, const string& details, int relatedId = -1) {
        EventLog log;
        log.Id = nextLogId++;
        time_t now = time(nullptr);
        char buf[26];
#ifdef _WIN32
        ctime_s(buf, sizeof(buf), &now);
#else
        ctime_r(&now, buf);
#endif
        log.Timestamp = string(buf);
        if (!log.Timestamp.empty() && log.Timestamp.back() == '\n') log.Timestamp.pop_back();
        log.Actor = actor;
        log.ActorName = actorName;
        log.Action = action;
        log.Details = details;
        log.RelatedId = relatedId;
        EventLogs.push_back(log);
    }
};

#endif