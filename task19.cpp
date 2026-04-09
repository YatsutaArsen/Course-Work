#include <iostream>
#include <string>
#include <vector>
using namespace std;

//День 1: визначились з розмірами поля
// Поле 7 рядків х 6 колонок - під літери B та G
const int ROWS = 7;
const int COLS = 6;

// День 1: вирішили зберігати поле в динамічній пам'яті (згідно з вимогою курсової роботи)
bool* active;

// День 2: додаємо підказки (те, які цифри мають зустрічатись у кожному рядку або колонці)
vector<int>* rowHints;
vector<int>* colHints;

// День 2: прототипи
void initData();
void cleanupData();
void printMenu();
void printField();
int safeReadInt(int* minVal, int* maxVal);

int main() {
    initData();

    int choice = 0;
    int* ptrChoice = &choice;
    do {
        printMenu();
        int minV = 0, maxV = 2;
        // День 2: поки лише 2 пункти меню: показати поле та вийти
        // TODO: потім додати автоматичний розв'язок та режим гравця 
        *ptrChoice = safeReadInt(&minV, &maxV);
        switch (*ptrChoice) {
            case 1: printField(); break;
            case 0: {
                cout << "\n До побачення! \n";
                break;
            }
        }
    } while (*ptrChoice != 0);
    
    cleanupData();
    return 0;
}

void initData() {
    active = new bool[ROWS * COLS];

    // День 1: розклад активних клітинок - літери B та G
    bool initActive[ROWS][COLS] = {
        {1,1,1, 1,1,1,}, // рядок 0
        {1,0,1, 1,0,0}, // рядок 1
        {1,0,1, 1,0,0}, // рядок 2
        {1,1,1, 1,1,1}, // рядок 3
        {1,0,1, 1,0,1}, // рядок 4
        {1,0,1, 1,0,1}, // рядок 5
        {1,1,1, 1,1,1} // рядок 6
    };

    for (int i = 0; i < ROWS; i++)
      for (int j = 0; j < COLS; j++)
        *(active + i * COLS + j) = initActive[i][j];

    // День 2: ініціалізація підказок
    rowHints = new vector<int>[ROWS];
    colHints = new vector<int>[COLS];

    // Заповнюємо лише ті рядки, що мають підказки
    *(rowHints + 0) = {2, 0, 1, 5};
    *(rowHints + 3) = {1, 3, 4};
    *(rowHints + 6) = {0, 2, 3};

    *(colHints + 0) = {2, 4, 6};
    *(colHints + 1) = {0, 1, 5};
    *(colHints + 2) = {0, 3, 6};
    *(colHints + 3) = {1, 3, 6};
    *(colHints + 4) = {0, 1, 5};
    *(colHints + 5) = {1, 2, 3, 4};
}

void cleanupData() {
    delete[] active;
    // День 2: чистимо нову пам'ять
    delete[] rowHints;
    delete[] colHints;
}

// День 2: базове меню без рамок (їх зробимо пізніше)
void printMenu() {
    cout << "\n === ГОЛОВОЛОМКА ДОМІНО ===\n";
    cout << " [1] Показати поле\n";
    cout << " [0] Вийти\n";
    cout << " Ваш вибір\n";
}

// День 2: безпечне читання числа з перевіркою діапазону
int safeReadInt(int* minVal, int* maxVal) {
    int val;
    string line;
    while (true) {
        if (getline(cin, line)) {
            bool valid = !line.empty();
            for (int i = 0; i < (int)line.length(); i++) {
                if (line[i] < '0' || line[i] > '9') { valid = false; break; }
            }
            if (valid) {
                try {
                    val = stoi(line);
                    if (val >= *minVal && val <= *maxVal) return val;
                    cout << "  Число від " << minVal << " до " << *maxVal << ": ";
                } catch (...) {
                    cout << " Помилка! Введіть ціле число: ";
                }
            } else {
                cout << " Помилка! Введіть ціле число: ";
            }
        }
    }
}

// День 2: оновлений printField (додали підказки рядків та колонок)

void printField() {
    cout << "\n  +";
    for (int j = 0; j < COLS; j++) cout << "---+";
    cout <<"\n";

    for (int i = 0; i < ROWS; i ++) {
        cout << " |";
        for (int j = 0; j < COLS; j++) {
            if (*(active + i * COLS +j)) cout << " # |";
            else cout << " |";
        }
        cout << "\n +";
        for (int j = 0; j < COLS; j++) cout << "---+";
        cout << "\n";
        
    // День 2: виводимо підказки рядка праворуч
    if (!(*(rowHints + i)).empty()) {
        cout << " <";
        for (int h : *(rowHints + i)) cout << " " << h;
    }
    cout << "\n +";
    for (int j = 0; j < COLS; j++) cout << "---+";
    cout << "\n";

    // День 2: виводимо підказки колонок внизу
    cout << "    ";
    for (int j = 0; j < COLS; j++) cout << "  ^ ";
    cout << "\n";
    for (int line = 0; line < 4; line++) {
        cout << "    ";
        for (int j = 0; j < COLS; j++) {
            if (line < (int)(*(colHints + j)).size())
            cout << "  " << (*(colHints + j))[line] << " ";
        else 
            cout << "    ";
        }
        cout << "\n";
    }

}