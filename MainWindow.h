#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QDialog>
#include <QComboBox>
#include <QVBoxLayout>
#include "FitnessClubSystem.h"

// Класс диалогового окна для статистики
class StatsDialog : public QDialog
{
    Q_OBJECT
public:
    StatsDialog(FitnessClubSystem* system, QWidget* parent = nullptr);

private slots:
    void onStatsTypeChanged(int index);
    void onRefreshClicked();

private:
    FitnessClubSystem* m_system;
    QComboBox* m_statsTypeCombo;
    QTextEdit* m_statsDisplay;
    QPushButton* m_refreshBtn;

    void updateStats();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void onLogin();
    void onLogout();
    void onClientMenu(int choice);
    void onAdminMenu(int choice);
    void onTrainerMenu(int choice);

private:
    QStackedWidget* stackedWidget;
    QWidget* loginWidget;
    QWidget* clientWidget;
    QWidget* adminWidget;
    QWidget* trainerWidget;

    QLineEdit* idInput;
    QTextEdit* displayArea;

    QListWidget* clientMenuList;
    QListWidget* adminMenuList;
    QListWidget* trainerMenuList;

    QTextEdit* clientDisplay;
    QTextEdit* adminDisplay;
    QTextEdit* trainerDisplay;

    FitnessClubSystem* system;
    QString currentUserRole;

    void setupLoginWidget();
    void setupClientWidget();
    void setupAdminWidget();
    void setupTrainerWidget();
    void appendMessage(const QString& msg);
};

#endif