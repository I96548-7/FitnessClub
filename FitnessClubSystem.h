#ifndef FITNESS_CLUB_SYSTEM_H
#define FITNESS_CLUB_SYSTEM_H

#include <QString>
#include "Repository.h"

class FitnessClubSystem
{
private:
    Repository repo;
    User* currentUser;

public:
    FitnessClubSystem();
    ~FitnessClubSystem();

    QString authorizeUser(int id);
    QString getCurrentUserRole();
    void logout();

    QString executeClientCommand(int command);
    QString executeAdminCommand(int command);
    QString executeTrainerCommand(int command);

    // Публичные методы для прямого вызова из MainWindow
    Client* getClient();

    // Клиентские методы
    QString clientEnterByNfc();
    QString clientTakeLocker();
    QString clientReleaseLocker();      // Клиент освобождает свой шкафчик
    QString clientTakeEquipment();
    QString clientReturnEquipment();    // Клиент возвращает инвентарь
    QString clientBookTrainer();
    QString clientShowMyBookings();
    QString clientCancelBooking();
    QString clientExitClub();

    // Методы отображения
    QString showSchedule();
    QString showAllClients();

    // Административные методы (только выдача)
    // Добавить в класс FitnessClubSystem:
    QString adminReleaseLocker();      // Админ принудительно освобождает шкафчик
    QString adminReturnEquipment();    // Админ принудительно возвращает инвентарь
    QString showClientsWithDebts();    // Показать клиентов с невозвращенными ресурсами
    QString adminBuyMembership();
    QString adminShowStats();
    QString adminReleaseHall();
    QString showLockerStats();      // Статистика загрузки шкафчиков
    QString showEquipmentStats();   // Статистика по инвентарю  
    QString showHallStats();        // Статистика загрузки залов

    // Методы тренера
    QString trainerBookHall();
    QString trainerShowMyTrainings();
    QString trainerShowBookings();

private:
    QString formatTime(time_t t);
    void updateCompletedBookings();

    // Helper methods
    Trainer* getTrainer();
};

#endif