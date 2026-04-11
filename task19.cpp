#include <iostream>
#include <string>
#include <vector>
using namespace std;

const int ROWS = 7;
const int COLS = 6;

bool* active;
int* field;        // День 3: масив значень клітинок (які цифри розміщені)
int* placement;    // День 3: масив індексів кісточок (яка кісточка займає клітинку)
vector<int>* rowHints;
vector<int>* colHints;

void initData();
void cleanupData();
void printLine(int* width);
void printMenuItem(string* text, int* width);
void printMenu();
int safeReadInt(int* minVal, int* maxVal);
void printField();

int main() {
    initData();
    int choice = 0;
    int* ptrChoice = &choice;
    do {
        printMenu();
        int minV = 0, maxV = 3;
        // День 3: розширили меню до 3 пунктів
        // TODO: пункт 1 — автовирішення (день 8+)
        // TODO: пункт 2 — режим гравця (день 12+)
        *ptrChoice = safeReadInt(&minV, &maxV);
        switch (*ptrChoice) {
            case 3: printField(); break;
            case 0: {
                string m = "  До побачення!";
                int w = 40;
                printLine(&w);
                printMenuItem(&m, &w);
                printLine(&w);
                cout << "\n";
                break;
            }
        }
    } while (*ptrChoice != 0);
    cleanupData();
    return 0;
}

void initData() {
    active = new bool[ROWS * COLS];
    // День 3: виділяємо пам'ять для нових масивів
    // BUG: спочатку забули ініціалізувати field і placement — були сміттєві значення
    // FIX: одразу заповнюємо -1 (означає "порожньо")
    field = new int[ROWS * COLS];
    placement = new int[ROWS * COLS];

    bool initActive[ROWS][COLS] = {
        {1,1,1, 1,1,1},
        {1,0,1, 1,0,0},
        {1,0,1, 1,0,0},
        {1,1,1, 1,1,1},
        {1,0,1, 1,0,1},
        {1,0,1, 1,0,1},
        {1,1,1, 1,1,1}
    };

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            *(active + i * COLS + j) = initActive[i][j];
            *(field + i * COLS + j) = -1;       // -1 = клітинка порожня
            *(placement + i * COLS + j) = -1;   // -1 = кісточка не розміщена
        }
    }

    rowHints = new vector<int>[ROWS];
    colHints = new vector<int>[COLS];

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
    delete[] field;      // День 3: чистимо нову пам'ять
    delete[] placement;  // День 3: чистимо нову пам'ять
    delete[] rowHints;
    delete[] colHints;
}

// День 3: функція для друку горизонтальної лінії рамки
// BUG: спочатку width використовували як значення, а не вказівник — не компілювалось
// FIX: передаємо int* width і використовуємо *width
void printLine(int* width) {
    cout << "+";
    for (int i = 0; i < *width; i++) cout << "-";
    cout << "+\n";
}

// День 3: функція для друку одного рядка меню з вирівнюванням
void printMenuItem(string* text, int* width) {
    cout << "| " << *text;
    for (int i = (int)text->length(); i < *width - 1; i++) cout << " ";
    cout << "|\n";
}

// День 3: меню тепер з красивими рамками замість простого тексту
void printMenu() {
    int w = 40;
    cout << "\n";
    printLine(&w);
    string t1 = "        ГОЛОВОЛОМКА ДОМІНО";
    printMenuItem(&t1, &w);
    printLine(&w);
    string t2 = "  [1] Розв'язати автоматично (Турбо)";
    string t3 = "  [2] Спробувати самому";
    string t4 = "  [3] Показати поле";
    string t5 = "  [0] Вийти";
    printMenuItem(&t2, &w);
    printMenuItem(&t3, &w);
    printMenuItem(&t4, &w);
    printMenuItem(&t5, &w);
    printLine(&w);
    cout << "  Ваш вибір: ";
}

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
                    cout << "  Число від " << *minVal << " до " << *maxVal << ": ";
                } catch (...) {
                    cout << "  Помилка! Введіть ціле число: ";
                }
            } else {
                cout << "  Помилка! Введіть ціле число: ";
            }
        }
    }
}

// День 3: printField тепер виводиться всередині красивої рамки меню
void printField() {
    int w = 40;
    cout << "\n";
    printLine(&w);
    string t = "  ПОЛЕ ГОЛОВОЛОМКИ";
    printMenuItem(&t, &w);
    printLine(&w);
    cout << "\n  +";
    for (int j = 0; j < COLS; j++) cout << "---+";
    cout << "\n";

    for (int i = 0; i < ROWS; i++) {
        cout << "  |";
        for (int j = 0; j < COLS; j++) {
            if (*(active + i * COLS + j)) cout << " # |";
            else cout << "   |";
        }
        if (!(*(rowHints + i)).empty()) {
            cout << "  <";
            for (int h : *(rowHints + i)) cout << " " << h;
        }
        cout << "\n  +";
        for (int j = 0; j < COLS; j++) cout << "---+";
        cout << "\n";
    }

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