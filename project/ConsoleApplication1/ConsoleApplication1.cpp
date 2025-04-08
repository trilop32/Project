#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>
#include <sstream> 

using namespace std;

// Структура для хранения данных пользователя
struct User {
    string username;
    string password;
    time_t lastLogin;
    int failedLoginAttempts;
    string passport; // Номер паспорта
    string phoneNumber; // Номер телефона
};

// Функция для генерации случайного номера паспорта (скрыты первые 6 цифры)
string generatePassportNumber() {
    stringstream ss;
    ss << "******"; 
    for (int i = 0; i < 4; ++i) {
        ss << rand() % 10; 
    }
    return ss.str();
}

// Функция для генерации случайного номера телефона (скрыты первые цифры)
string generatePhoneNumber() {
    stringstream ss;
    ss << "+7 (***) ***-**-"; 
    ss << rand() % 10;
    ss << rand() % 10;
    ss << rand() % 10;
    ss << rand() % 10;

    return ss.str();
}

// Функция для регистрации нового пользователя
bool registerUser(const string& username, const string& password) {
    // Проверяем, существует ли пользователь с таким именем
    ifstream userFile("users.txt");
    string line;
    while (getline(userFile, line)) {
        string existingUsername = line.substr(0, line.find(':'));
        if (existingUsername == username) {
            userFile.close();
            cout << "Пользователь с таким именем уже существует." << endl;
            return false;
        }
    }
    userFile.close();

    // Генерируем случайные данные для паспорта и телефона
    string passport = generatePassportNumber();
    string phoneNumber = generatePhoneNumber();

    // Записываем нового пользователя в файл
    ofstream outfile("users.txt", ios::app);
    outfile << username << ":" << password << ":0:0:" << passport << ":" << phoneNumber << endl;
    outfile.close();

    cout << "Регистрация прошла успешно!" << endl;
    return true;
}

// Функция для чтения данных пользователя из файла
bool getUserData(const string& username, User& user) {
    ifstream userFile("users.txt");
    string line;
    while (getline(userFile, line)) {
        string existingUsername = line.substr(0, line.find(':'));
        if (existingUsername == username) {
            // Разбираем строку
            size_t pos1 = line.find(':');
            size_t pos2 = line.find(':', pos1 + 1);
            size_t pos3 = line.find(':', pos2 + 1);
            size_t pos4 = line.find(':', pos3 + 1);
            size_t pos5 = line.find(':', pos4 + 1); 
            size_t pos6 = string::npos; // Если последнего разделителя нет

            string password = line.substr(pos1 + 1, pos2 - pos1 - 1);
            string lastLoginStr = line.substr(pos2 + 1, pos3 - pos2 - 1);
            string failedLoginAttemptsStr = line.substr(pos3 + 1, pos4 - pos3 - 1);
            string passport = line.substr(pos4 + 1, pos5 - pos4 - 1); 
            string phoneNumber = line.substr(pos5 + 1, pos6); 

            user.username = username;
            user.password = password;
            user.lastLogin = stoi(lastLoginStr);
            user.failedLoginAttempts = stoi(failedLoginAttemptsStr);
            user.passport = passport; 
            user.phoneNumber = phoneNumber; 

            userFile.close();
            return true;
        }
    }
    userFile.close();
    return false;
}

// Функция для обновления данных пользователя в файле
void updateUserData(const User& user) {
    ifstream userFile("users.txt");
    ofstream tempFile("temp.txt");
    string line;

    while (getline(userFile, line)) {
        string existingUsername = line.substr(0, line.find(':'));
        if (existingUsername == user.username) {
            // Заменяем строку пользователя
            tempFile << user.username << ":" << user.password << ":" << user.lastLogin << ":"
            << user.failedLoginAttempts << ":" << user.passport << ":" << user.phoneNumber << endl;
        }
        else {
            tempFile << line << endl;
        }
    }

    userFile.close();
    tempFile.close();

    remove("users.txt");
    rename("temp.txt", "users.txt");
}

// Функция для входа пользователя
bool loginUser(const string& username, const string& password, User& user) {
    if (!getUserData(username, user)) {
        cout << "Пользователь не найден." << endl;
        return false;
    }

    if (user.password == password) {
        cout << "Вход выполнен!" << endl;
        user.lastLogin = time(0);
        user.failedLoginAttempts = 0;
        updateUserData(user);

        // Выводим информацию о пользователе (паспорт и телефон)
        cout << "\n--- Информация о пользователе ---" << endl;
        cout << "Номер паспорта: " << user.passport << endl;
        cout << "Номер телефона: " << user.phoneNumber << endl;
        cout << "-----------------------------" << endl;

        // Проверка на необычное время входа (например, ночью)
        time_t now = time(0);
        tm ltm;
        if (localtime_s(&ltm, &now) == 0) {
            int hour = ltm.tm_hour;
            if (hour < 6 || hour > 22) {
                cout << "\nВНИМАНИЕ! Вы вошли в систему в необычное время.\n";
                cout << "Если это не вы, немедленно смените пароль.\n";
            }
        }
        else {
            cerr << "Ошибка при вызове localtime_s" << endl;
        }
        return true;
    }
    else {
        cout << "Неверное имя пользователя или пароль." << endl;
        user.failedLoginAttempts++;
        updateUserData(user);

        if (user.failedLoginAttempts >= 3) {
            cout << "\nВНИМАНИЕ! Обнаружена подозрительная активность!\n";
            cout << "Было зафиксировано несколько неудачных попыток входа в ваш аккаунт.\n";
            cout << "Рекомендуется немедленно сменить пароль.\n";
        }
        return false;
    }
}

// Функция для смены пароля
void changePassword(User& user) {
    string oldPassword, newPassword, confirmPassword;

    cout << "Введите старый пароль: ";
    cin >> oldPassword;

    if (user.password != oldPassword) {
        cout << "Неверный старый пароль." << endl;
        return;
    }

    cout << "Введите новый пароль: ";
    cin >> newPassword;
    cout << "Подтвердите новый пароль: ";
    cin >> confirmPassword;

    if (newPassword != confirmPassword) {
        cout << "Новый пароль и подтверждение не совпадают." << endl;
        return;
    }

    user.password = newPassword;
    updateUserData(user);
    cout << "Пароль успешно изменен." << endl;
}

// Функция для моделирования взлома данных
void simulateDataBreach(User& user) {
    cout << "\n*** СИМУЛЯЦИЯ ВЗЛОМА ДАННЫХ ***\n" << endl;
    cout << "Выберите способ взлома:\n";
    cout << "1. Подбор пароля (Brute-force)\n";
    cout << "2. Социальная инженерия (Phishing)\n";
    cout << "3. Эксплойт уязвимости в программном обеспечении\n";
    cout << "Ваш выбор: ";

    int choice;
    cin >> choice;

    srand(time(0));

    switch (choice) {
    case 1: {
        cout << "\nПопытка подбора пароля...\n";
        if (rand() % 2 == 0) {
            cout << "ВЗЛОМ УДАЛСЯ! Пароль подобран.\n";
            cout << "Злоумышленник получил доступ к аккаунту " << user.username << endl;
            cout << "ВНИМАНИЕ! ВАШ АККАУНТ БЫЛ ВЗЛОМАН. СРОЧНО СМЕНИТЕ ПАРОЛЬ!\n";
            changePassword(user);
        }
        else {
            cout << "ВЗЛОМ НЕ УДАЛСЯ. Слишком много неудачных попыток.\n";
            cout << "СИСТЕМА: Аккаунт временно заблокирован. Пожалуйста, повторите попытку позже.\n";
            user.failedLoginAttempts = 3;
            updateUserData(user);
            cout << "РЕКОМЕНДАЦИИ: Обратитесь к администратору для разблокировки аккаунта.\n";
        }
        break;
    }
    case 2: {
        cout << "\nАтака с использованием социальной инженерии...\n";
        if (rand() % 2 == 0) {
            cout << "ВЗЛОМ УДАЛСЯ! Пользователь предоставил свои учетные данные.\n";
            cout << "Злоумышленник получил доступ к аккаунту " << user.username << endl;
            cout << "ВНИМАНИЕ! ВАШ АККАУНТ БЫЛ ВЗЛОМАН. СРОЧНО СМЕНИТЕ ПАРОЛЬ!\n";
            changePassword(user);
        }
        else {
            cout << "ВЗЛОМ НЕ УДАЛСЯ. Пользователь заподозрил неладное.\n";
            cout << "СИСТЕМА: Пользователь сообщил об инциденте.\n";
        }
        break;
    }
    case 3: {
        cout << "\nЭксплуатация уязвимости...\n";
        if (rand() % 2 == 0) {
            cout << "ВЗЛОМ УДАЛСЯ! Обнаружена и использована уязвимость в программном обеспечении.\n";
            cout << "Злоумышленник получил несанкционированный доступ к данным.\n";
            cout << "ВНИМАНИЕ! ВОЗМОЖНО, ВАШ АККАУНТ БЫЛ СКОМПРОМЕТИРОВАН. РЕКОМЕНДУЕТСЯ СМЕНИТЬ ПАРОЛЬ.\n";
            changePassword(user);
        }
        else {
            cout << "ВЗЛОМ НЕ УДАЛСЯ. Уязвимость была вовремя устранена.\n";
            cout << "СИСТЕМА: Обнаружена попытка эксплуатации уязвимости. Информация передана администратору.\n";
        }
        break;
    }
    default:
        cout << "Неверный выбор.\n";
    }
}

int main() {
    srand(time(0)); // Инициализируем генератор случайных чисел
    setlocale(LC_ALL, "Russian");
    int choice;
    string username, password;
    User currentUser;

    while (true) {
        cout << "\nМеню:\n";
        cout << "1. Вход\n";
        cout << "2. Регистрация\n";
        cout << "3. Симуляция взлома\n";
        cout << "4. Сменить пароль\n";
        cout << "5. Выход\n";
        cout << "Ваш выбор: ";
        cin >> choice;

        switch (choice) {
        case 1: {
            cout << "Имя пользователя: ";
            cin >> username;
            cout << "Пароль: ";
            cin >> password;
            loginUser(username, password, currentUser);
            break;
        }
        case 2: {
            cout << "Имя пользователя: ";
            cin >> username;
            cout << "Пароль: ";
            cin >> password;
            registerUser(username, password);
            break;
        }
        case 3: {
            cout << "Имя пользователя: ";
            cin >> username;
            if (getUserData(username, currentUser)) {
                simulateDataBreach(currentUser);
            }
            else {
                cout << "Пользователь не найден." << endl;
            }
            break;
        }
        case 4: {
            cout << "Имя пользователя: ";
            cin >> username;
            if (getUserData(username, currentUser)) {
                changePassword(currentUser);
            }
            else {
                cout << "Пользователь не найден." << endl;
            }
            break;
        }
        case 5: {
            cout << "Выход из программы.\n";
            return 0;
        }
        default:
            cout << "Неверный выбор.\n";
        }
    }

    return 0;
}
