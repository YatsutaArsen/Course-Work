#include <iostream>
#include <string>
#include <vector>
using namespace std;

const int ROWS = 7;
const int COLS = 6;

bool* active;
int* field;
int* placement;
vector<int>* rowHints;
vector<int>* colHints;

// День 4: структура для зберігання однієї кісточки доміно
struct Domino { int a, b; };

// День 4: масив усіх кісточок (0-0, 0-1, ... 6-6) — всього 28 штук
Domino* dominoes;
int* dominoCount;

// День 4: кеш для миттєвого пошуку індексу кісточки за двома значеннями
// зберігаємо пари (0-6) x (0-6) — тобто 7*7 = 49 комірок
// звертаємось як [a * 7 + b]
int* dominoIndexMap;

void initData();
void cleanupData();
void printLine(int* width);
void printMenuItem(string* text, int* width);
void printMenu();
int safeReadInt(int* minVal, int* maxVal);
void printField();
void generateDominoes(int* maxVal);

int main() {
    initData();
    int choice = 0;
    int* ptrChoice = &choice;
    do {
        printMenu();
        int minV = 0, maxV = 3;
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
    field = new int[ROWS * COLS];
    placement = new int[ROWS * COLS];

    // День 4: виділяємо пам'ять для кісточок і кешу
    dominoes = new Domino[28];
    dominoCount = new int(0);
    dominoIndexMap = new int[7 * 7];

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
            *(field + i * COLS + j) = -1;
            *(placement + i * COLS + j) = -1;
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
    delete[] field;
    delete[] placement;
    // День 4: чистимо нову пам'ять
    delete[] dominoes;
    delete dominoCount;     // не delete[] бо це один int, а не масив
    delete[] dominoIndexMap;
    delete[] rowHints;
    delete[] colHints;
}

void printLine(int* width) {
    cout << "+";
    for (int i = 0; i < *width; i++) cout << "-";
    cout << "+\n";
}

void printMenuItem(string* text, int* width) {
    cout << "| " << *text;
    for (int i = (int)text->length(); i < *width - 1; i++) cout << " ";
    cout << "|\n";
}

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

// День 4: генеруємо всі можливі кісточки від 0-0 до maxVal-maxVal
// і одразу заповнюємо кеш dominoIndexMap для швидкого пошуку
void generateDominoes(int* maxVal) {
    *dominoCount = 0;
    for (int i = 0; i <= *maxVal; i++) {
        for (int j = i; j <= *maxVal; j++) {
            // День 4: зберігаємо кісточку
            (dominoes + *dominoCount)->a = i;
            (dominoes + *dominoCount)->b = j;

            // День 4: заповнюємо кеш в обидва боки
            // 0-1 і 1-0 — це одна й та сама кісточка
            *(dominoIndexMap + i * 7 + j) = *dominoCount;
            *(dominoIndexMap + j * 7 + i) = *dominoCount;

            (*dominoCount)++;
        }
    }
}