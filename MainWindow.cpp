#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollArea>

// ======================== СТИЛИ ========================

// Общий стиль для всего приложения
static const QString MAIN_STYLE =
"QMainWindow { background-color: #E8F5E9; }"
"QWidget { background-color: #E8F5E9; font-family: 'Segoe UI', 'Arial'; font-size: 16px; }"
"QGroupBox { font-weight: bold; font-size: 18px; border: 2px solid #A5D6A7; border-radius: 8px; margin-top: 15px; padding-top: 12px; background-color: #F1F8E9; }"
"QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 5px 0 5px; color: #2E7D32; font-size: 17px; }"
"QLabel { color: #1B5E20; font-size: 16px; }"
"QLineEdit { border: 2px solid #A5D6A7; border-radius: 6px; padding: 10px; background-color: white; color: #1B5E20; selection-background-color: #66BB6A; font-size: 16px; }"
"QLineEdit:focus { border-color: #4CAF50; }"
"QTextEdit { border: 2px solid #A5D6A7; border-radius: 8px; background-color: #FFFFFF; color: #2E3B2E; font-family: 'Courier New', monospace; font-size: 15px; }"
"QListWidget { border: 2px solid #A5D6A7; border-radius: 8px; background-color: white; color: #1B5E20; outline: none; font-size: 16px; }"
"QListWidget::item { padding: 12px; border-bottom: 1px solid #E8F5E9; }"
"QListWidget::item:selected { background-color: #C8E6C9; color: #1B5E20; }"
"QListWidget::item:hover { background-color: #E8F5E9; }"
"QPushButton { background-color: #4CAF50; color: white; border: none; border-radius: 8px; padding: 12px 20px; font-weight: bold; font-size: 16px; }"
"QPushButton:hover { background-color: #388E3C; }"
"QPushButton:pressed { background-color: #2E7D32; }"
"QPushButton#logoutBtn { background-color: #E53935; font-size: 16px; }"
"QPushButton#logoutBtn:hover { background-color: #C62828; }"
"QPushButton#loginBtn { background-color: #43A047; font-size: 18px; padding: 14px 32px; }"
"QComboBox { border: 2px solid #A5D6A7; border-radius: 6px; padding: 10px; background-color: white; color: #1B5E20; font-size: 16px; }"
"QComboBox:drop-down { border: none; }"
"QComboBox::down-arrow { image: none; border-left: 6px solid transparent; border-right: 6px solid transparent; border-top: 6px solid #4CAF50; margin-right: 8px; }"
"QComboBox QAbstractItemView { border: 2px solid #A5D6A7; background-color: white; selection-background-color: #C8E6C9; font-size: 16px; }"
"QScrollBar:vertical { border: none; background: #E8F5E9; width: 14px; margin: 0; }"
"QScrollBar::handle:vertical { background: #A5D6A7; border-radius: 7px; min-height: 30px; }"
"QScrollBar::handle:vertical:hover { background: #81C784; }"
"QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical { border: none; background: none; }";

// Стиль для заголовка
static const QString TITLE_STYLE =
"font-size: 38px; font-weight: bold; color: #2E7D32; padding: 25px;"
"background-color: #C8E6C9; border-radius: 15px; margin: 15px;";
StatsDialog::StatsDialog(FitnessClubSystem* system, QWidget* parent)
    : QDialog(parent)
    , m_system(system)
{
    setWindowTitle("Статистика использования ресурсов");
    setMinimumSize(800, 700);
    setStyleSheet("QDialog { background-color: #E8F5E9; }"
        "QLabel { color: #2E7D32; font-weight: bold; font-size: 18px; }"
        "QPushButton { background-color: #4CAF50; color: white; border-radius: 8px; padding: 10px 20px; font-size: 16px; }"
        "QPushButton:hover { background-color: #388E3C; }"
        "QComboBox { border: 2px solid #A5D6A7; border-radius: 6px; padding: 10px; background-color: white; font-size: 16px; }"
        "QTextEdit { border: 2px solid #A5D6A7; border-radius: 8px; background-color: white; font-family: 'Courier New'; font-size: 16px; font-weight: bold; }");

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    mainLayout->setSpacing(20);

    // Выпадающий список для выбора типа статистики
    QHBoxLayout* selectorLayout = new QHBoxLayout();
    selectorLayout->setSpacing(15);

    QLabel* label = new QLabel("Тип статистики:");
    label->setStyleSheet("font-size: 18px; font-weight: bold;");

    m_statsTypeCombo = new QComboBox();
    m_statsTypeCombo->addItem("📊 Загрузка шкафчиков");
    m_statsTypeCombo->addItem("📦 Статистика по инвентарю");
    m_statsTypeCombo->addItem("🏢 Загрузка залов");
    m_statsTypeCombo->setStyleSheet("font-size: 16px; padding: 8px;");

    m_refreshBtn = new QPushButton(" ");
    m_refreshBtn->setStyleSheet("QPushButton { background-color: #E8F5E9; font-size: 16px; padding: 10px 20px; } QPushButton:hover { background-color: #E8F5E9; }");

    selectorLayout->addWidget(label);
    selectorLayout->addWidget(m_statsTypeCombo);
    selectorLayout->addWidget(m_refreshBtn);
    selectorLayout->addStretch();

    // Область для вывода статистики - ЖИРНЫЙ ШРИФТ
    m_statsDisplay = new QTextEdit();
    m_statsDisplay->setReadOnly(true);
    m_statsDisplay->setFontFamily("Courier New");
    m_statsDisplay->setStyleSheet("QTextEdit { font-size: 16px; font-weight: bold; background-color: #FAFFFA; padding: 15px; }");

    mainLayout->addLayout(selectorLayout);
    mainLayout->addWidget(m_statsDisplay);

    // Кнопка закрытия
    QPushButton* closeBtn = new QPushButton("✖ Закрыть");
    closeBtn->setStyleSheet("QPushButton { background-color: #E53935; font-size: 16px; padding: 10px 20px; } QPushButton:hover { background-color: #C62828; }");
    connect(closeBtn, &QPushButton::clicked, this, &QDialog::accept);
    mainLayout->addWidget(closeBtn);

    // Подключение сигналов
    connect(m_statsTypeCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
        this, &StatsDialog::onStatsTypeChanged);
    connect(m_refreshBtn, &QPushButton::clicked, this, &StatsDialog::onRefreshClicked);

    // Загружаем статистику по умолчанию
    updateStats();
}
void StatsDialog::onStatsTypeChanged(int index)
{
    updateStats();
}

void StatsDialog::onRefreshClicked()
{
    updateStats();
}

void StatsDialog::updateStats()
{
    QString stats;
    int index = m_statsTypeCombo->currentIndex();

    switch (index) {
    case 0: // Загрузка шкафчиков
        stats = m_system->showLockerStats();
        break;
    case 1: // Статистика по инвентарю
        stats = m_system->showEquipmentStats();
        break;
    case 2: // Загрузка залов
        stats = m_system->showHallStats();
        break;
    default:
        stats = "Неизвестный тип статистики";
    }

    m_statsDisplay->clear();
    m_statsDisplay->append(stats);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , system(new FitnessClubSystem())
{
    setWindowTitle("🌿 Фитнес Клуб Управление");
    setMinimumSize(1000, 700);
    setStyleSheet(MAIN_STYLE);

    stackedWidget = new QStackedWidget(this);
    setCentralWidget(stackedWidget);

    setupLoginWidget();
    setupClientWidget();
    setupAdminWidget();
    setupTrainerWidget();

    stackedWidget->setCurrentWidget(loginWidget);
}

MainWindow::~MainWindow()
{
    delete system;
}

void MainWindow::setupLoginWidget()
{
    loginWidget = new QWidget();
    QVBoxLayout* layout = new QVBoxLayout(loginWidget);
    layout->setSpacing(25);
    layout->setContentsMargins(50, 50, 50, 50);

    QLabel* titleLabel = new QLabel("🏋️‍♂️ ФИТНЕС КЛУБ 🧘‍♀️");
    titleLabel->setStyleSheet(TITLE_STYLE);
    titleLabel->setAlignment(Qt::AlignCenter);

    QGroupBox* loginBox = new QGroupBox("🔐 Авторизация");
    QVBoxLayout* loginLayout = new QVBoxLayout();
    loginLayout->setSpacing(20);

    QHBoxLayout* inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(15);

    QLabel* idLabel = new QLabel("ID пользователя:");
    idLabel->setStyleSheet("font-size: 16px; font-weight: bold;");
    idInput = new QLineEdit();
    idInput->setPlaceholderText("Введите ваш ID");
    idInput->setFixedWidth(250);
    idInput->setStyleSheet("QLineEdit { font-size: 15px; }");

    QPushButton* loginBtn = new QPushButton("🚪 Войти");
    loginBtn->setObjectName("loginBtn");
    loginBtn->setStyleSheet("QPushButton { background-color: #43A047; font-size: 16px; padding: 10px 24px; }");
    connect(loginBtn, &QPushButton::clicked, this, &MainWindow::onLogin);

    inputLayout->addWidget(idLabel);
    inputLayout->addWidget(idInput);
    inputLayout->addWidget(loginBtn);
    inputLayout->setAlignment(Qt::AlignCenter);

    loginLayout->addLayout(inputLayout);
    loginBox->setLayout(loginLayout);

    displayArea = new QTextEdit();
    displayArea->setReadOnly(true);
    displayArea->setMaximumHeight(350);
    displayArea->setPlaceholderText("Здесь будет отображаться информация о входе...");

    layout->addWidget(titleLabel);
    layout->addStretch();
    layout->addWidget(loginBox);
    layout->addWidget(displayArea);
    layout->addStretch();

    stackedWidget->addWidget(loginWidget);
}

void MainWindow::setupClientWidget()
{
    clientWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(clientWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    QGroupBox* menuBox = new QGroupBox("📋 Меню клиента");
    menuBox->setStyleSheet("QGroupBox { font-size: 16px; }");
    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->setSpacing(12);

    clientMenuList = new QListWidget();
    QStringList clientMenu;
    clientMenu << "📱 Вход по NFC"
        << "🔑 Занять шкафчик"
        << "🔓 Освободить шкафчик"
        << "🏋️ Взять инвентарь"
        << "📦 Вернуть инвентарь"
        << "📅 Расписание"
        << "👨‍🏫 Записаться к тренеру"
        << "📝 Мои записи"
        << "❌ Отменить запись"
        << "💳 Мой абонемент"
        << "🚪 Выход по NFC";
    clientMenuList->addItems(clientMenu);

    connect(clientMenuList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        int choice = clientMenuList->row(item) + 1;
        onClientMenu(choice);
        });

    QPushButton* logoutBtn = new QPushButton("🚪 Выйти из системы");
    logoutBtn->setObjectName("logoutBtn");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);

    menuLayout->addWidget(clientMenuList);
    menuLayout->addWidget(logoutBtn);
    menuBox->setLayout(menuLayout);

    QGroupBox* displayBox = new QGroupBox("📄 Информация");
    displayBox->setStyleSheet("QGroupBox { font-size: 16px; }");
    QVBoxLayout* displayLayout = new QVBoxLayout();

    clientDisplay = new QTextEdit();
    clientDisplay->setReadOnly(true);
    clientDisplay->setPlaceholderText("Здесь будет отображаться результат ваших действий...");
    displayLayout->addWidget(clientDisplay);
    displayBox->setLayout(displayLayout);

    layout->addWidget(menuBox, 1);
    layout->addWidget(displayBox, 2);

    stackedWidget->addWidget(clientWidget);
}

void MainWindow::setupAdminWidget()
{
    adminWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(adminWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    QGroupBox* menuBox = new QGroupBox("🛠️ Меню администратора");
    menuBox->setStyleSheet("QGroupBox { font-size: 16px; }");
    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->setSpacing(12);

    adminMenuList = new QListWidget();
    QStringList adminMenu;
    adminMenu << "🔓 Освободить шкафчик"
        << "📦 Вернуть инвентарь"
        << "💳 Оформить абонемент"
        << "📊 Статистика"
        << "👥 Все клиенты"
        << "⚠️ Клиенты с долгами"
        << "🏢 Освободить зал";
    adminMenuList->addItems(adminMenu);

    connect(adminMenuList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        int choice = adminMenuList->row(item) + 1;
        onAdminMenu(choice);
        });

    QPushButton* logoutBtn = new QPushButton("🚪 Выйти из системы");
    logoutBtn->setObjectName("logoutBtn");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);

    menuLayout->addWidget(adminMenuList);
    menuLayout->addWidget(logoutBtn);
    menuBox->setLayout(menuLayout);

    QGroupBox* displayBox = new QGroupBox("📄 Информация");
    displayBox->setStyleSheet("QGroupBox { font-size: 16px; }");
    QVBoxLayout* displayLayout = new QVBoxLayout();

    adminDisplay = new QTextEdit();
    adminDisplay->setReadOnly(true);
    adminDisplay->setPlaceholderText("Здесь будет отображаться результат ваших действий...");
    displayLayout->addWidget(adminDisplay);
    displayBox->setLayout(displayLayout);

    layout->addWidget(menuBox, 1);
    layout->addWidget(displayBox, 2);

    stackedWidget->addWidget(adminWidget);
}

void MainWindow::setupTrainerWidget()
{
    trainerWidget = new QWidget();
    QHBoxLayout* layout = new QHBoxLayout(trainerWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(20, 20, 20, 20);

    QGroupBox* menuBox = new QGroupBox("🏋️ Меню тренера");
    menuBox->setStyleSheet("QGroupBox { font-size: 16px; }");
    QVBoxLayout* menuLayout = new QVBoxLayout();
    menuLayout->setSpacing(12);

    trainerMenuList = new QListWidget();
    QStringList trainerMenu;
    trainerMenu << "🏢 Забронировать зал"
        << "📅 Расписание"
        << "📋 Мои тренировки"
        << "👥 Записанные клиенты";
    trainerMenuList->addItems(trainerMenu);

    connect(trainerMenuList, &QListWidget::itemClicked, [this](QListWidgetItem* item) {
        int choice = trainerMenuList->row(item) + 1;
        onTrainerMenu(choice);
        });

    QPushButton* logoutBtn = new QPushButton("🚪 Выйти из системы");
    logoutBtn->setObjectName("logoutBtn");
    connect(logoutBtn, &QPushButton::clicked, this, &MainWindow::onLogout);

    menuLayout->addWidget(trainerMenuList);
    menuLayout->addWidget(logoutBtn);
    menuBox->setLayout(menuLayout);

    QGroupBox* displayBox = new QGroupBox("📄 Информация");
    displayBox->setStyleSheet("QGroupBox { font-size: 16px; }");
    QVBoxLayout* displayLayout = new QVBoxLayout();

    trainerDisplay = new QTextEdit();
    trainerDisplay->setReadOnly(true);
    trainerDisplay->setPlaceholderText("Здесь будет отображаться результат ваших действий...");
    displayLayout->addWidget(trainerDisplay);
    displayBox->setLayout(displayLayout);

    layout->addWidget(menuBox, 1);
    layout->addWidget(displayBox, 2);

    stackedWidget->addWidget(trainerWidget);
}

void MainWindow::onLogin()
{
    int id = idInput->text().toInt();
    QString result = system->authorizeUser(id);

    if (result.contains("авторизовался")) {
        currentUserRole = system->getCurrentUserRole();
        appendMessage(result);

        if (currentUserRole == "клиент") {
            stackedWidget->setCurrentWidget(clientWidget);
            clientDisplay->clear();
            clientDisplay->append("🌿 Добро пожаловать в клиентское меню!");
        }
        else if (currentUserRole == "администратор") {
            stackedWidget->setCurrentWidget(adminWidget);
            adminDisplay->clear();
            adminDisplay->append("🌿 Добро пожаловать в меню администратора!");
        }
        else if (currentUserRole == "тренер") {
            stackedWidget->setCurrentWidget(trainerWidget);
            trainerDisplay->clear();
            trainerDisplay->append("🌿 Добро пожаловать в меню тренера!");
        }
    }
    else {
        QMessageBox::warning(this, "Ошибка", result);
    }
}

void MainWindow::onLogout()
{
    system->logout();
    currentUserRole = "";
    stackedWidget->setCurrentWidget(loginWidget);
    idInput->clear();
    appendMessage("🚪 Вы вышли из системы");
}

void MainWindow::onClientMenu(int choice)
{
    QString result;
    switch (choice) {
    case 1: result = system->clientEnterByNfc(); break;
    case 2: result = system->clientTakeLocker(); break;
    case 3: result = system->clientReleaseLocker(); break;
    case 4: result = system->clientTakeEquipment(); break;
    case 5: result = system->clientReturnEquipment(); break;
    case 6: result = system->showSchedule(); break;
    case 7: result = system->clientBookTrainer(); break;
    case 8: result = system->clientShowMyBookings(); break;
    case 9: result = system->clientCancelBooking(); break;
    case 10: {
        Client* c = system->getClient();
        result = "💳 Абонемент: " + QString::fromStdString(c->MembershipType) + " (активен)";
        break;
    }
    case 11: result = system->clientExitClub(); break;
    default: result = "❌ Неверный выбор";
    }
    appendMessage(result);
    clientDisplay->clear();
    clientDisplay->append(result);
}

void MainWindow::onAdminMenu(int choice)
{
    QString result;
    switch (choice) {
    case 1: result = system->adminReleaseLocker(); break;
    case 2: result = system->adminReturnEquipment(); break;
    case 3: result = system->adminBuyMembership(); break;
    case 4: {
        StatsDialog* statsDialog = new StatsDialog(system, this);
        statsDialog->setAttribute(Qt::WA_DeleteOnClose);
        statsDialog->show();
        return;
    }
    case 5: result = system->showAllClients(); break;
    case 6: result = system->showClientsWithDebts(); break;
    case 7: result = system->adminReleaseHall(); break;
    default: result = "❌ Неверный выбор";
    }
    appendMessage(result);
    adminDisplay->clear();
    adminDisplay->append(result);
}

void MainWindow::onTrainerMenu(int choice)
{
    QString result;
    switch (choice) {
    case 1: result = system->trainerBookHall(); break;
    case 2: result = system->showSchedule(); break;
    case 3: result = system->trainerShowMyTrainings(); break;
    case 4: result = system->trainerShowBookings(); break;
    default: result = "❌ Неверный выбор";
    }
    appendMessage(result);
    trainerDisplay->clear();
    trainerDisplay->append(result);
}

void MainWindow::appendMessage(const QString& msg)
{
    QTextEdit* currentDisplay = nullptr;

    if (stackedWidget->currentWidget() == loginWidget) {
        currentDisplay = displayArea;
    }
    else if (stackedWidget->currentWidget() == clientWidget) {
        currentDisplay = clientDisplay;
    }
    else if (stackedWidget->currentWidget() == adminWidget) {
        currentDisplay = adminDisplay;
    }
    else if (stackedWidget->currentWidget() == trainerWidget) {
        currentDisplay = trainerDisplay;
    }

    if (currentDisplay) {
        currentDisplay->append(msg);
    }
}