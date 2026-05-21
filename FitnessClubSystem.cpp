#include "FitnessClubSystem.h"
#include <sstream>
#include <iomanip>
#include <algorithm>
#include <QInputDialog>
#include <QMessageBox>
#include <QDate>
#include <QTime>
#include <QDateTime>

FitnessClubSystem::FitnessClubSystem()
    : currentUser(nullptr)
{
}

FitnessClubSystem::~FitnessClubSystem()
{
}

// Публичные методы
QString FitnessClubSystem::authorizeUser(int id)
{
    currentUser = repo.FindUser(id);
    if (currentUser) {
        QString role = QString::fromStdString(currentUser->GetRoleName());
        QString name = QString::fromStdString(currentUser->Name);
        return "✓ " + role + " " + name + " авторизовался!";
    }
    return "✗ Пользователь не найден";
}

QString FitnessClubSystem::getCurrentUserRole()
{
    if (currentUser) {
        return QString::fromStdString(currentUser->GetRoleName());
    }
    return "";
}

void FitnessClubSystem::logout()
{
    currentUser = nullptr;
}

Client* FitnessClubSystem::getClient()
{
    return static_cast<Client*>(currentUser);
}

Trainer* FitnessClubSystem::getTrainer()
{
    return static_cast<Trainer*>(currentUser);
}


// Методы отображения
QString FitnessClubSystem::showAllClients()
{
    std::stringstream ss;
    ss << "\n=== ВСЕ КЛИЕНТЫ ===\n";
    if (repo.Clients.empty()) {
        ss << "  Нет клиентов\n";
    }
    else {
        for (auto& c : repo.Clients) {
            ss << "  ID:" << c.Id << " | " << c.Name
                << " | Абонемент: " << c.MembershipType
                << " | NFC: " << c.NfcTag << "\n";
        }
    }
    return QString::fromStdString(ss.str());
}

QString FitnessClubSystem::showSchedule()
{
    updateCompletedBookings();
    std::stringstream ss;
    ss << "\n=== РАСПИСАНИЕ ===\n";
    if (repo.Schedule.empty()) {
        ss << "  Нет занятий\n";
        ss << "  Тренер может создать занятие\n";
        return QString::fromStdString(ss.str());
    }
    for (auto& s : repo.Schedule) {
        Trainer* trainer = repo.FindTrainer(s.TrainerId);
        Hall* hall = repo.FindHall(s.HallId);
        ss << "  " << formatTime(s.StartTime).toStdString() << " - " << formatTime(s.EndTime).toStdString() << "\n";
        ss << "  " << s.Activity << "\n";
        ss << "    Тренер: " << (trainer ? trainer->Name : "Не назначен")
            << " | Зал: " << (hall ? hall->Name : "Не назначен")
            << " | Мест: " << s.FreeSlots() << "/" << s.MaxClients;
        if (s.IsCompleted()) ss << " [ЗАВЕРШЕНО]";
        ss << "\n";
    }
    return QString::fromStdString(ss.str());
}

// CLIENT METHODS
QString FitnessClubSystem::clientEnterByNfc()
{
    Client* c = getClient();
    c->isEntered = true;
    std::stringstream ss;
    ss << "✓ NFC-метка " << c->NfcTag << " считана\n";
    ss << "✓ Абонемент " << c->MembershipType << " действителен\n";
    ss << "✓ Вход выполнен. Добро пожаловать!\n";
    return QString::fromStdString(ss.str());
}

QString FitnessClubSystem::clientTakeLocker()
{
    Client* c = getClient();
    if (!c->isEntered) {
        return "✗ Сначала выполните вход по NFC.";
    }

    for (auto& l : repo.Lockers) {
        if (l.AssignedToClientId == c->Id) {
            return "У вас уже занят шкафчик #" + QString::number(l.Id);
        }
    }

    QStringList items;
    for (auto& l : repo.Lockers) {
        if (l.Status == LockerStatus::Free) {
            items << QString("#%1").arg(l.Id);
        }
    }

    if (items.isEmpty()) {
        return "Нет свободных шкафчиков";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Выберите шкафчик",
        "Свободные шкафчики:", items, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        int id = selected.mid(1).toInt();
        Locker* l = repo.FindLocker(id);
        if (l && l->Status == LockerStatus::Free) {
            l->Occupy(c->Id);
            return "✓ Шкафчик #" + QString::number(id) + " занят. Ключ выдан!";
        }
    }
    return "✗ Шкафчик недоступен!";
}

QString FitnessClubSystem::clientReleaseLocker()
{
    Client* c = getClient();
    if (!c->isEntered) {
        return "✗ Сначала выполните вход по NFC.";
    }
    for (auto& l : repo.Lockers) {
        if (l.AssignedToClientId == c->Id) {
            l.Release();
            return "✓ Шкафчик #" + QString::number(l.Id) + " освобожден";
        }
    }
    return "✗ У вас нет занятых шкафчиков";
}

QString FitnessClubSystem::clientTakeEquipment()
{
    Client* c = getClient();
    if (!c->isEntered) {
        return "✗ Сначала выполните вход по NFC (пункт 1).";
    }
    QStringList items;
    for (auto& e : repo.EquipmentList) {
        if (e.Status == EquipmentStatus::Available) {
            items << QString("%1 - %2").arg(e.Id).arg(QString::fromStdString(e.Name));
        }
    }

    if (items.isEmpty()) {
        return "Нет доступного инвентаря";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Выберите инвентарь",
        "Доступный инвентарь:", items, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        int id = selected.split(" - ")[0].toInt();
        Equipment* e = repo.FindEquipment(id);
        if (e && e->Status == EquipmentStatus::Available) {
            e->CheckOut(getClient()->Id);
            return "✓ " + QString::fromStdString(e->Name) + " выдан";
        }
    }
    return "✗ Инвентарь недоступен";
}

QString FitnessClubSystem::clientReturnEquipment()
{
    Client* c = getClient();
    if (!c->isEntered) {
        return "✗ Сначала выполните вход по NFC.";
    }

    QStringList items;
    for (auto& e : repo.EquipmentList) {
        if (e.CheckedOutToClientId == c->Id) {
            items << QString("%1 - %2").arg(e.Id).arg(QString::fromStdString(e.Name));
        }
    }

    if (items.isEmpty()) {
        return "Вы не брали инвентарь";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Выберите инвентарь для возврата",
        "Ваш инвентарь:", items, 0, false, &ok);

    if (ok && !selected.isEmpty()) {
        int id = selected.split(" - ")[0].toInt();
        Equipment* e = repo.FindEquipment(id);
        if (e && e->CheckedOutToClientId == c->Id) {
            e->CheckIn();
            return "✓ " + QString::fromStdString(e->Name) + " возвращен";
        }
    }
    return "✗ Ошибка возврата";
}

QString FitnessClubSystem::clientBookTrainer()
{
    Client* c = getClient();
    updateCompletedBookings();

    if (repo.Schedule.empty()) {
        return "Нет доступных тренировок. Тренер еще не добавил занятия.";
    }

    QStringList trainers;
    for (auto& t : repo.Trainers) {
        trainers << QString("%1 - %2 (%3)").arg(t.Id).arg(QString::fromStdString(t.Name))
            .arg(QString::fromStdString(t.Specialization));
    }

    if (trainers.isEmpty()) {
        return "Нет тренеров";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Выберите тренера",
        "Тренеры:", trainers, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return "Выбор отменен";
    }

    int tid = selected.split(" - ")[0].toInt();
    Trainer* trainer = repo.FindTrainer(tid);
    if (!trainer) {
        return "✗ Тренер не найден";
    }

    QStringList availableSessions;
    vector<ScheduleEntry*> available;
    for (auto& s : repo.Schedule) {
        if (s.TrainerId == trainer->Id && s.FreeSlots() > 0 && !s.IsCompleted()) {
            available.push_back(&s);
            availableSessions << QString("ID:%1 - %2 (%3) Мест: %4")
                .arg(s.Id).arg(QString::fromStdString(s.Activity))
                .arg(formatTime(s.StartTime)).arg(s.FreeSlots());
        }
    }

    if (available.empty()) {
        return "✗ Нет доступных тренировок у этого тренера";
    }

    selected = QInputDialog::getItem(nullptr, "Выберите тренировку",
        "Доступные тренировки:", availableSessions, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return "Выбор отменен";
    }

    QString idPart = selected.mid(selected.indexOf("ID:") + 3);
    int sid = idPart.split(" -")[0].toInt();

    auto it = find_if(available.begin(), available.end(),
        [sid](ScheduleEntry* s) { return s->Id == sid; });
    if (it == available.end()) {
        return "✗ Тренировка не найдена";
    }

    ScheduleEntry* sched = *it;
    if (find(sched->ClientIds.begin(), sched->ClientIds.end(), c->Id) != sched->ClientIds.end()) {
        return "✗ Вы уже записаны на эту тренировку";
    }

    Booking newBooking;
    newBooking.Id = repo.nextBookingId++;
    newBooking.ClientId = c->Id;
    newBooking.TrainerId = trainer->Id;
    newBooking.ScheduleId = sched->Id;
    repo.Bookings.push_back(newBooking);
    sched->ClientIds.push_back(c->Id);
    c->BookingIds.push_back(newBooking.Id);

    return "✓ Вы записаны на тренировку '" + QString::fromStdString(sched->Activity) + "'!";
}

QString FitnessClubSystem::clientShowMyBookings()
{
    updateCompletedBookings();
    std::stringstream ss;
    bool has = false;

    for (int bid : getClient()->BookingIds) {
        Booking* b = repo.FindBooking(bid);
        if (b && b->Status == BookingStatus::Confirmed) {
            ScheduleEntry* s = repo.FindSchedule(b->ScheduleId);
            Trainer* t = repo.FindTrainer(b->TrainerId);
            if (s && t) {
                ss << "  " << s->Activity << " - " << formatTime(s->StartTime).toStdString()
                    << "  Тренер: " << t->Name << "\n";
                has = true;
            }
        }
    }
    if (!has) ss << "  Нет активных записей\n";

    return QString::fromStdString(ss.str());
}

QString FitnessClubSystem::clientCancelBooking()
{
    Client* c = getClient();
    updateCompletedBookings();

    QStringList active;
    vector<Booking*> activeBookings;

    for (int bid : c->BookingIds) {
        Booking* b = repo.FindBooking(bid);
        if (b && b->Status == BookingStatus::Confirmed) {
            ScheduleEntry* s = repo.FindSchedule(b->ScheduleId);
            if (s) {
                active << QString::fromStdString(s->Activity + " - " + formatTime(s->StartTime).toStdString());
                activeBookings.push_back(b);
            }
        }
    }

    if (active.isEmpty()) {
        return "Нет записей для отмены";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Отмена записи",
        "Ваши записи:", active, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return "Выбор отменен";
    }

    int idx = active.indexOf(selected);
    if (idx < 0 || idx >= (int)activeBookings.size()) {
        return "✗ Ошибка выбора";
    }

    Booking* b = activeBookings[idx];
    b->Cancel();
    ScheduleEntry* s = repo.FindSchedule(b->ScheduleId);
    if (s) {
        auto& clients = s->ClientIds;
        auto it = find(clients.begin(), clients.end(), b->ClientId);
        if (it != clients.end()) {
            clients.erase(it);
        }
    }
    return "✓ Запись отменена";
}

QString FitnessClubSystem::clientExitClub()
{
    Client* c = getClient();
    if (!c->isEntered) {
        return "Вы не входили в клуб.";
    }
    c->isEntered = false;

    // ПРОВЕРЯЕМ, остались ли у клиента ресурсы
    std::stringstream ss;
    ss << "✓ Вы вышли из клуба.\n";

    bool hasLocker = false;
    for (auto& l : repo.Lockers) {
        if (l.AssignedToClientId == c->Id) {
            hasLocker = true;
            break;
        }
    }

    bool hasEquipment = false;
    for (auto& e : repo.EquipmentList) {
        if (e.CheckedOutToClientId == c->Id) {
            hasEquipment = true;
            break;
        }
    }

    if (hasLocker) {
        ss << "⚠ ВНИМАНИЕ! У вас остался занятый шкафчик!\n";
        ss << "  Обратитесь к администратору для освобождения.\n";
        repo.AddLog("Client", c->Name, "exit_with_locker",
            "Клиент вышел, не освободив шкафчик #" +
            to_string(repo.FindLockerByClient(c->Id)->Id), c->Id);
    }

    if (hasEquipment) {
        ss << "⚠ ВНИМАНИЕ! У вас остался выданный инвентарь!\n";
        ss << "  Обратитесь к администратору для возврата.\n";
        repo.AddLog("Client", c->Name, "exit_with_equipment",
            "Клиент вышел, не вернув инвентарь", c->Id);
    }

    if (!hasLocker && !hasEquipment) {
        ss << "  Все ресурсы освобождены. Спасибо за посещение!\n";
    }

    return QString::fromStdString(ss.str());
}

// ADMIN METHODS

QString FitnessClubSystem::adminBuyMembership()
{
    bool ok;
    QString name = QInputDialog::getText(nullptr, "Новый клиент", "Имя клиента:", QLineEdit::Normal, "", &ok);
    if (!ok || name.isEmpty()) return "Операция отменена";

    QString type = QInputDialog::getText(nullptr, "Новый клиент", "Тип абонемента:", QLineEdit::Normal, "Стандартный", &ok);
    if (!ok || type.isEmpty()) return "Операция отменена";

    QString nfc = QInputDialog::getText(nullptr, "Новый клиент", "NFC-метка:", QLineEdit::Normal, "", &ok);
    if (!ok || nfc.isEmpty()) return "Операция отменена";

    int newId = 100;
    while (repo.FindUser(newId)) newId++;

    Client c;
    c.Id = newId;
    c.Name = name.toStdString();
    c.MembershipType = type.toStdString();
    c.NfcTag = nfc.toStdString();
    repo.Clients.push_back(c);

    return "✓ Абонемент оформлен! ID: " + QString::number(newId) + ", NFC: " + nfc;
}

QString FitnessClubSystem::adminReleaseHall()
{
    // Собираем список занятых залов
    QStringList bookedHalls;
    vector<Hall*> bookedHallsList;

    for (auto& h : repo.Halls) {
        if (h.Status == HallStatus::Booked) {
            bookedHalls << QString("%1 - %2 (вместимость: %3 чел.)")
                .arg(h.Id)
                .arg(QString::fromStdString(h.Name))
                .arg(h.Capacity);
            bookedHallsList.push_back(&h);
        }
    }

    if (bookedHalls.isEmpty()) {
        return "Нет занятых залов для освобождения";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Освобождение зала",
        "Выберите зал для освобождения:", bookedHalls, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return "Операция отменена";
    }

    // Находим выбранный зал
    int hallId = selected.split(" - ")[0].toInt();
    Hall* selectedHall = nullptr;

    for (auto* h : bookedHallsList) {
        if (h->Id == hallId) {
            selectedHall = h;
            break;
        }
    }

    if (!selectedHall) {
        return "✗ Зал не найден";
    }

    // Находим и удаляем все тренировки в этом зале
    int removedCount = 0;
    auto it = repo.Schedule.begin();
    while (it != repo.Schedule.end()) {
        if (it->HallId == selectedHall->Id) {
            // Удаляем записи клиентов из этой тренировки
            for (int clientId : it->ClientIds) {
                Client* c = repo.FindClient(clientId);
                if (c) {
                    // Удаляем ID бронирования из списка клиента
                    auto bookingIt = find(c->BookingIds.begin(), c->BookingIds.end(), it->Id);
                    if (bookingIt != c->BookingIds.end()) {
                        c->BookingIds.erase(bookingIt);
                    }
                }
            }

            // Удаляем бронирования из репозитория
            auto bookingIt = repo.Bookings.begin();
            while (bookingIt != repo.Bookings.end()) {
                if (bookingIt->ScheduleId == it->Id) {
                    bookingIt = repo.Bookings.erase(bookingIt);
                }
                else {
                    ++bookingIt;
                }
            }

            it = repo.Schedule.erase(it);
            removedCount++;
        }
        else {
            ++it;
        }
    }

    // Освобождаем зал
    selectedHall->Release();
    repo.SaveChanges();

    QString result = "✓ Зал \"" + QString::fromStdString(selectedHall->Name) + "\" освобожден!\n";
    if (removedCount > 0) {
        result += "  Удалено тренировок в этом зале: " + QString::number(removedCount);
    }

    return result;
}
// НОВЫЙ МЕТОД - показать клиентов с невозвращенными ресурсами
QString FitnessClubSystem::showClientsWithDebts()
{
    vector<Client*> debtors = repo.GetClientsWithUnreturnedItems();

    std::stringstream ss;
    ss << "\n=== КЛИЕНТЫ С НЕВОЗВРАЩЕННЫМИ РЕСУРСАМИ ===\n";

    if (debtors.empty()) {
        ss << "  Нет клиентов с задолженностями\n";
    }
    else {
        for (Client* c : debtors) {
            ss << "  ID:" << c->Id << " | " << c->Name << "\n";

            Locker* l = repo.FindLockerByClient(c->Id);
            if (l) {
                ss << "    - Шкафчик #" << l->Id << " (не освобожден)\n";
            }

            Equipment* e = repo.FindEquipmentByClient(c->Id);
            if (e) {
                ss << "    - Инвентарь: " << e->Name << " #" << e->Id << " (не возвращен)\n";
            }
        }
    }
    ss << "===========================================\n";

    return QString::fromStdString(ss.str());
}

// ИСПРАВЛЕННЫЙ метод освобождения шкафчика админом
QString FitnessClubSystem::adminReleaseLocker()
{
    vector<Client*> debtors = repo.GetClientsWithUnreturnedItems();

    if (debtors.empty()) {
        return "Нет клиентов с невозвращенными шкафчиками";
    }

    QStringList clientItems;
    for (Client* c : debtors) {
        Locker* l = repo.FindLockerByClient(c->Id);
        if (l) {
            clientItems << QString("%1 - %2 (шкафчик #%3)")
                .arg(c->Id).arg(QString::fromStdString(c->Name)).arg(l->Id);
        }
    }

    if (clientItems.isEmpty()) {
        return "Нет занятых шкафчиков у вышедших клиентов";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Освобождение шкафчика",
        "Выберите клиента, который не освободил шкафчик:", clientItems, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return "Операция отменена";
    }

    int cid = selected.split(" - ")[0].toInt();
    Client* c = repo.FindClient(cid);
    if (!c) {
        return "✗ Клиент не найден";
    }

    Locker* l = repo.FindLockerByClient(cid);
    if (!l) {
        return "✗ У клиента нет занятого шкафчика";
    }

    int lockerId = l->Id;
    l->Release();
    repo.AddLog("Admin", "Admin", "force_release_locker",
        "Принудительно освобожден шкафчик #" + to_string(lockerId) +
        " у клиента " + c->Name, c->Id);

    return "✓ Шкафчик #" + QString::number(lockerId) + " принудительно освобожден!\n" +
        "  Клиент " + QString::fromStdString(c->Name) + " получит штраф.";
}

// ИСПРАВЛЕННЫЙ метод возврата инвентаря админом
QString FitnessClubSystem::adminReturnEquipment()
{
    vector<Client*> debtors = repo.GetClientsWithUnreturnedItems();

    if (debtors.empty()) {
        return "Нет клиентов с невозвращенным инвентарем";
    }

    QStringList clientItems;
    for (Client* c : debtors) {
        Equipment* e = repo.FindEquipmentByClient(c->Id);
        if (e) {
            clientItems << QString("%1 - %2 (инвентарь: %3 #%4)")
                .arg(c->Id).arg(QString::fromStdString(c->Name))
                .arg(QString::fromStdString(e->Name)).arg(e->Id);
        }
    }

    if (clientItems.isEmpty()) {
        return "Нет выданного инвентаря у вышедших клиентов";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Возврат инвентаря",
        "Выберите клиента, который не вернул инвентарь:", clientItems, 0, false, &ok);

    if (!ok || selected.isEmpty()) {
        return "Операция отменена";
    }

    int cid = selected.split(" - ")[0].toInt();
    Client* c = repo.FindClient(cid);
    if (!c) {
        return "✗ Клиент не найден";
    }

    Equipment* e = repo.FindEquipmentByClient(cid);
    if (!e) {
        return "✗ У клиента нет выданного инвентаря";
    }

    string eqName = e->Name;
    int eqId = e->Id;
    e->CheckIn();
    repo.AddLog("Admin", "Admin", "force_return_equipment",
        "Принудительно возвращен инвентарь " + eqName + " #" + to_string(eqId) +
        " от клиента " + c->Name, c->Id);

    return "✓ " + QString::fromStdString(eqName) + " принудительно возвращен!\n" +
        "  Клиент " + QString::fromStdString(c->Name) + " получит штраф.";
}
// TRAINER METHODS
QString FitnessClubSystem::trainerBookHall()
{
    Trainer* trainer = getTrainer();

    QStringList freeHalls;
    for (auto& h : repo.Halls) {
        if (h.Status == HallStatus::Free) {
            freeHalls << QString("%1 - %2").arg(h.Id).arg(QString::fromStdString(h.Name));
        }
    }

    if (freeHalls.isEmpty()) {
        return "Нет свободных залов";
    }

    bool ok;
    QString selected = QInputDialog::getItem(nullptr, "Бронирование зала",
        "Выберите свободный зал:", freeHalls, 0, false, &ok);

    if (!ok || selected.isEmpty()) return "Операция отменена";

    int hid = selected.split(" - ")[0].toInt();
    Hall* hall = repo.FindHall(hid);
    if (!hall || hall->Status != HallStatus::Free) {
        return "✗ Зал недоступен";
    }

    QString activity = QInputDialog::getText(nullptr, "Бронирование зала",
        "Название тренировки:", QLineEdit::Normal, "", &ok);
    if (!ok || activity.isEmpty()) return "Операция отменена";

    QDate date = QDate::currentDate();
    QTime time = QTime::currentTime().addSecs(3600);
    QString dateTimeStr = QInputDialog::getText(nullptr, "Бронирование зала",
        "Дата и время (ГГГГ-ММ-ДД ЧЧ:ММ):", QLineEdit::Normal,
        date.toString("yyyy-MM-dd") + " " + time.toString("hh:mm"), &ok);
    if (!ok || dateTimeStr.isEmpty()) return "Операция отменена";

    // ПРОВЕРКА: нельзя бронировать на прошедшую дату/время
    QDateTime selectedDateTime = QDateTime::fromString(dateTimeStr, "yyyy-MM-dd hh:mm");
    if (!selectedDateTime.isValid()) {
        return "✗ Неверный формат даты и времени! Используйте ГГГГ-ММ-ДД ЧЧ:ММ";
    }

    QDateTime currentDateTime = QDateTime::currentDateTime();
    if (selectedDateTime <= currentDateTime) {
        return "✗ Нельзя забронировать зал на прошедшее или текущее время!\n"
            "   Пожалуйста, выберите будущую дату и время.";
    }

    int dur = QInputDialog::getInt(nullptr, "Бронирование зала",
        "Длительность (часы):", 1, 1, 8, 1, &ok);
    if (!ok) return "Операция отменена";

    int maxc = QInputDialog::getInt(nullptr, "Бронирование зала",
        "Макс. участников:", 10, 1, hall->Capacity, 1, &ok);
    if (!ok) return "Операция отменена";

    // ПРОВЕРКА: нельзя бронировать зал, если уже есть занятие в это время
    time_t startTime = selectedDateTime.toSecsSinceEpoch();
    time_t endTime = startTime + dur * 3600;

    for (auto& existing : repo.Schedule) {
        if (existing.HallId == hall->Id) {
            // Проверяем пересечение интервалов
            if (!(endTime <= existing.StartTime || startTime >= existing.EndTime)) {
                return "✗ Зал уже забронирован на это время!\n"
                    "   Выберите другое время или дату.";
            }
        }
    }

    struct tm tm = { 0 };
    sscanf(dateTimeStr.toStdString().c_str(), "%d-%d-%d %d:%d",
        &tm.tm_year, &tm.tm_mon, &tm.tm_mday, &tm.tm_hour, &tm.tm_min);
    tm.tm_year -= 1900;
    tm.tm_mon -= 1;
    time_t start = mktime(&tm);

    hall->Book();
    ScheduleEntry s;
    s.Id = repo.nextScheduleId++;
    s.Activity = activity.toStdString();
    s.TrainerId = trainer->Id;
    s.HallId = hall->Id;
    s.StartTime = start;
    s.EndTime = start + dur * 3600;
    s.MaxClients = maxc;
    repo.Schedule.push_back(s);

    repo.SaveChanges();

    return "✓ Зал '" + QString::fromStdString(hall->Name) + "' забронирован!\n"
        "✓ Тренировка '" + activity + "' добавлена в расписание!\n"
        "  Дата и время: " + dateTimeStr + "\n"
        "  Длительность: " + QString::number(dur) + " час(ов)";
}

QString FitnessClubSystem::trainerShowMyTrainings()
{
    updateCompletedBookings();
    std::stringstream ss;
    bool has = false;

    for (auto& s : repo.Schedule) {
        if (s.TrainerId == getTrainer()->Id) {
            Hall* hall = repo.FindHall(s.HallId);
            ss << "  " << s.Activity << " - " << formatTime(s.StartTime).toStdString()
                << "  Зал: " << (hall ? hall->Name : "Не назначен")
                << " | Запись: " << s.ClientIds.size() << "/" << s.MaxClients;
            if (s.IsCompleted()) ss << " [ЗАВЕРШЕНО]";
            ss << "\n";
            has = true;
        }
    }
    if (!has) {
        ss << "  Нет тренировок\n";
        ss << "  Чтобы создать тренировку, выберите пункт 1 'Забронировать зал'\n";
    }

    return QString::fromStdString(ss.str());
}

QString FitnessClubSystem::trainerShowBookings()
{
    updateCompletedBookings();
    std::stringstream ss;
    bool has = false;

    for (auto& s : repo.Schedule) {
        if (s.TrainerId == getTrainer()->Id && !s.ClientIds.empty() && !s.IsCompleted()) {
            ss << "\n" << s.Activity << " (" << formatTime(s.StartTime).toStdString() << "):\n";
            for (int cid : s.ClientIds) {
                Client* c = repo.FindClient(cid);
                if (c) ss << "    - " << c->Name << " (ID:" << c->Id << ")\n";
            }
            has = true;
        }
    }
    if (!has) ss << "  Нет записанных клиентов\n";

    return QString::fromStdString(ss.str());
}

// Приватные вспомогательные методы
QString FitnessClubSystem::formatTime(time_t t)
{
    char buf[26];
#ifdef _WIN32
    ctime_s(buf, sizeof(buf), &t);
#else
    ctime_r(&t, buf);
#endif
    std::string result(buf);
    if (result.length() > 0 && result.back() == '\n') {
        result.pop_back();
    }
    return QString::fromStdString(result);
}

void FitnessClubSystem::updateCompletedBookings()
{
    time_t now = time(nullptr);
    for (auto& s : repo.Schedule) {
        if (now > s.EndTime) {
            for (auto& b : repo.Bookings) {
                if (b.ScheduleId == s.Id && b.Status == BookingStatus::Confirmed) {
                    b.Complete();
                }
            }
        }
    }
}
// Добавьте в конец файла FitnessClubSystem.cpp:

QString FitnessClubSystem::showLockerStats()
{
    std::stringstream ss;
    ss << "\n========== СТАТИСТИКА ЗАГРУЗКИ ШКАФЧИКОВ ==========\n\n";

    if (repo.Lockers.empty()) {
        ss << "  Нет данных о шкафчиках\n";
        return QString::fromStdString(ss.str());
    }

    int occupied = 0;
    int free = 0;

    for (auto& l : repo.Lockers) {
        if (l.Status == LockerStatus::Occupied) {
            occupied++;
        }
        else {
            free++;
        }
    }

    double occupancyRate = (double)occupied / repo.Lockers.size() * 100;

    ss << "  Общее количество шкафчиков: " << repo.Lockers.size() << "\n";
    ss << "  Занято: " << occupied << "\n";
    ss << "  Свободно: " << free << "\n";
    ss << "  Загрузка: " << std::fixed << std::setprecision(1) << occupancyRate << "%\n\n";

    // Детальный список шкафчиков
    ss << "  ДЕТАЛЬНЫЙ СПИСОК:\n";
    for (auto& l : repo.Lockers) {
        ss << "    Шкафчик #" << l.Id << ": ";
        if (l.Status == LockerStatus::Occupied) {
            Client* c = repo.FindClient(l.AssignedToClientId);
            if (c) {
                ss << "ЗАНЯТ (Клиент: " << c->Name << ", ID:" << c->Id << ")";
            }
            else {
                ss << "ЗАНЯТ (Клиент не найден)";
            }
        }
        else {
            ss << "СВОБОДЕН";
        }
        ss << "\n";
    }

    ss << "\n=====================================================\n";
    return QString::fromStdString(ss.str());
}

QString FitnessClubSystem::showEquipmentStats()
{
    std::stringstream ss;
    ss << "\n========== СТАТИСТИКА ПО ИНВЕНТАРЮ ==========\n\n";

    if (repo.EquipmentList.empty()) {
        ss << "  Нет данных об инвентаре\n";
        return QString::fromStdString(ss.str());
    }

    int available = 0;
    int checkedOut = 0;

    for (auto& e : repo.EquipmentList) {
        switch (e.Status) {
        case EquipmentStatus::Available: available++; break;
        case EquipmentStatus::CheckedOut: checkedOut++; break;
        }
    }

    ss << "  Статистика по состоянию инвентаря:\n";
    ss << "    Доступно: " << available << "\n";
    ss << "    Выдано: " << checkedOut << "\n";
    ss << "    Всего единиц: " << repo.EquipmentList.size() << "\n\n";

    double availabilityRate = (double)available / repo.EquipmentList.size() * 100;
    ss << "  Доступность инвентаря: " << std::fixed << std::setprecision(1) << availabilityRate << "%\n\n";

    // Детальный список инвентаря
    ss << "  ДЕТАЛЬНЫЙ СПИСОК ИНВЕНТАРЯ:\n";
    std::string statusStr[] = { "Доступен", "Выдан"};
    for (auto& e : repo.EquipmentList) {
        ss << "    #" << e.Id << " - " << e.Name << ": " << statusStr[(int)e.Status];
        if (e.Status == EquipmentStatus::CheckedOut && e.CheckedOutToClientId != -1) {
            Client* c = repo.FindClient(e.CheckedOutToClientId);
            if (c) {
                ss << " (Выдан: " << c->Name << ", ID:" << c->Id << ")";
            }
        }
        ss << "\n";
    }

    ss << "\n=================================================\n";
    return QString::fromStdString(ss.str());
}

QString FitnessClubSystem::showHallStats()
{
    std::stringstream ss;
    ss << "\n========== СТАТИСТИКА ЗАГРУЗКИ ЗАЛОВ ==========\n\n";

    if (repo.Halls.empty()) {
        ss << "  Нет данных о залах\n";
        return QString::fromStdString(ss.str());
    }

    int booked = 0;
    int free = 0;

    for (auto& h : repo.Halls) {
        switch (h.Status) {
        case HallStatus::Booked: booked++; break;
        case HallStatus::Free: free++; break;
        }
    }

    double occupancyRate = (double)booked / repo.Halls.size() * 100;

    ss << "  Общее количество залов: " << repo.Halls.size() << "\n";
    ss << "  Занято (забронировано): " << booked << "\n";
    ss << "  Свободно: " << free << "\n";
    ss << "  Загрузка: " << std::fixed << std::setprecision(1) << occupancyRate << "%\n\n";

    // Детальный список залов
    ss << "  ДЕТАЛЬНЫЙ СПИСОК ЗАЛОВ:\n";
    std::string hallStatusStr[] = { "СВОБОДЕН", "ЗАБРОНИРОВАН", "НА РЕМОНТЕ" };
    for (auto& h : repo.Halls) {
        ss << "    Зал \"" << h.Name << "\" (ID:" << h.Id << "): " << hallStatusStr[(int)h.Status];
        ss << " | Вместимость: " << h.Capacity << " чел.\n";

        // Показать тренировки в этом зале
        bool hasTraining = false;
        for (auto& s : repo.Schedule) {
            if (s.HallId == h.Id && !s.IsCompleted()) {
                if (!hasTraining) {
                    ss << "      Текущие тренировки:\n";
                    hasTraining = true;
                }
                Trainer* t = repo.FindTrainer(s.TrainerId);
                ss << "        - " << s.Activity << " (Тренер: " << (t ? t->Name : "Не назначен");
                ss << ", Запись: " << s.ClientIds.size() << "/" << s.MaxClients << ")\n";
            }
        }
        if (!hasTraining && h.Status == HallStatus::Booked) {
            ss << "      (Нет активных тренировок в расписании)\n";
        }
        ss << "\n";
    }

    ss << "================================================\n";
    return QString::fromStdString(ss.str());
}