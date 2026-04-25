#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
using namespace std;

const int ROWS = 7;
const int COLS = 6;

struct Domino { int a, b; };

bool* active;
int* field;
int* placement;
Domino* dominoes;
int* dominoCount;
bool* used;
vector<int>* rowHints;
vector<int>* colHints;
int* dominoIndexMap;

void initData();
void cleanupData();
void printLine(int* width);
void printMenuItem(string* text, int* width);
void printMenu();
int safeReadInt(int* minVal, int* maxVal);
void printField(bool* showPlacement); // День 16: об'єднана функція
void generateDominoes(int* maxVal);
bool isHintAllowed(int* r, int* c, int* val);
bool canSatisfyRow(int* r);
bool canSatisfyCol(int* c);
bool checkHints();
bool solve();
void resetField(); // День 16: новий прототип
void autoSolve();
void userSolve();

int main() {
    initData();
    int choice = 0;
    int* ptrChoice = &choice;
    do {
        printMenu();
        int minV = 0, maxV = 4; // День 16: розширили до 4 пунктів
        *ptrChoice = safeReadInt(&minV, &maxV);
        switch (*ptrChoice) {
            case 1: autoSolve(); break;
            case 2: userSolve(); break;
            case 3: {
                bool f = false;
                printField(&f); // День 16: передаємо false — показати поле без кісточок
                break;
            }
            case 4: {
                // TODO: день 17 — тут буде runDeveloperTests()
                cout << "\n  [4] Ще в розробці...\n";
                break;
            }
            case 0: {
                cout << "\n";
                int w = 55;
                string m = "  До побачення!";
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
    dominoes = new Domino[28];
    dominoCount = new int(0);
    used = new bool[28];
    rowHints = new vector<int>[ROWS];
    colHints = new vector<int>[COLS];
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

    // День 16: спрощений цикл ініціалізації через одновимірний індекс
    for (int i = 0; i < ROWS * COLS; i++) {
        *(active + i) = initActive[i / COLS][i % COLS];
        *(field + i) = -1;
        *(placement + i) = -1;
    }

    for (int i = 0; i < 28; i++) *(used + i) = false;

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
    delete[] dominoes;
    delete dominoCount;
    delete[] used;
    delete[] rowHints;
    delete[] colHints;
    delete[] dominoIndexMap;
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

// День 16: розширили меню до w=55, додали пункт [4]
void printMenu() {
    int w = 55;
    cout << "\n";
    printLine(&w);
    string t1 = "             ГОЛОВОЛОМКА ДОМІНО";
    printMenuItem(&t1, &w);
    printLine(&w);
    string t2 = "  [1] Розв'язати автоматично (Турбо)";
    string t3 = "  [2] Спробувати самому";
    string t4 = "  [3] Показати поле";
    string t5 = "  [4] Інженерне тестування (в розробці)";
    string t6 = "  [0] Вийти";
    printMenuItem(&t2, &w);
    printMenuItem(&t3, &w);
    printMenuItem(&t4, &w);
    printMenuItem(&t5, &w);
    printMenuItem(&t6, &w);
    printLine(&w);
    cout << "  Ваш вибір: ";
}

int safeReadInt(int* minVal, int* maxVal) {
    string line;
    while (getline(cin, line)) {
        bool valid = !line.empty();
        for (int i = 0; i < (int)line.length(); i++) {
            if (i == 0 && line[i] == '-') continue;
            if (line[i] < '0' || line[i] > '9') { valid = false; break; }
        }
        if (valid) {
            try {
                int val = stoi(line);
                if (val >= *minVal && val <= *maxVal) return val;
                cout << "  Число має бути від " << *minVal << " до "
                     << *maxVal << ". Спробуйте ще: ";
                continue;
            } catch (...) {}
        }
        cout << "  Помилка! Введіть ціле число: ";
    }
    return *minVal;
}

// День 16: об'єднали printField і printFieldWithPlacement в одну функцію
// sp = false — показує поле з # (режим перегляду)
// sp = true  — показує поле з цифрами і кісточками (режим розв'язку)
void printField(bool* sp) {
    cout << "\n  +";
    for (int j = 0; j < COLS; j++) cout << (*sp ? "----+" : "---+");
    cout << "\n";

    for (int i = 0; i < ROWS; i++) {
        cout << "  |";
        for (int j = 0; j < COLS; j++) {
            if (!*(active + i * COLS + j)) {
                cout << (*sp ? "    |" : "   |");
                continue;
            }
            if (!*sp) {
                // режим перегляду — просто #
                cout << " # |";
                continue;
            }
            // режим розв'язку — визначаємо правий кордон
            string rightBorder = "|";
            if (j + 1 < COLS &&
                *(active + i * COLS + (j + 1)) &&
                *(placement + i * COLS + j) != -1 &&
                *(placement + i * COLS + j) == *(placement + i * COLS + (j + 1))) {
                rightBorder = " ";
            }
            if (*(field + i * COLS + j) >= 0)
                cout << " " << *(field + i * COLS + j) << " " << rightBorder;
            else
                cout << " . " << rightBorder;
        }

        if (!(*(rowHints + i)).empty()) {
            cout << "  <";
            for (int h : *(rowHints + i)) cout << " " << h;
        }

        cout << "\n  +";
        for (int j = 0; j < COLS; j++) {
            if (*sp && i + 1 < ROWS &&
                *(active + i * COLS + j) &&
                *(active + (i + 1) * COLS + j) &&
                *(placement + i * COLS + j) != -1 &&
                *(placement + i * COLS + j) == *(placement + (i + 1) * COLS + j)) {
                cout << "    +";
            } else {
                cout << (*sp ? "----+" : "---+");
            }
        }
        cout << "\n";
    }

    cout << "    ";
    for (int j = 0; j < COLS; j++) cout << (*sp ? "  ^  " : "  ^ ");
    cout << "\n";
    for (int line = 0; line < 4; line++) {
        cout << "    ";
        for (int j = 0; j < COLS; j++) {
            if (line < (int)(*(colHints + j)).size())
                cout << "  " << (*(colHints + j))[line] << (*sp ? "  " : " ");
            else
                cout << (*sp ? "     " : "    ");
        }
        cout << "\n";
    }
}

void generateDominoes(int* maxVal) {
    *dominoCount = 0;
    for (int i = 0; i <= *maxVal; i++) {
        for (int j = i; j <= *maxVal; j++) {
            (dominoes + *dominoCount)->a = i;
            (dominoes + *dominoCount)->b = j;
            *(dominoIndexMap + i * 7 + j) = *dominoCount;
            *(dominoIndexMap + j * 7 + i) = *dominoCount;
            (*dominoCount)++;
        }
    }
}

bool isHintAllowed(int* r, int* c, int* val) {
    if (!(*(rowHints + *r)).empty()) {
        bool found = false;
        for (int h : *(rowHints + *r)) {
            if (h == *val) { found = true; break; }
        }
        if (!found) return false;
    }
    if (!(*(colHints + *c)).empty()) {
        bool found = false;
        for (int h : *(colHints + *c)) {
            if (h == *val) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

bool canSatisfyRow(int* r) {
    if ((*(rowHints + *r)).empty()) return true;
    int unfulfilled = 0, emptyCells = 0;
    for (int j = 0; j < COLS; j++) {
        if (*(active + *r * COLS + j) && *(placement + *r * COLS + j) == -1)
            emptyCells++;
    }
    for (int h : *(rowHints + *r)) {
        bool found = false;
        for (int j = 0; j < COLS; j++) {
            if (*(active + *r * COLS + j) && *(field + *r * COLS + j) == h) {
                found = true; break;
            }
        }
        if (!found) unfulfilled++;
    }
    return emptyCells >= unfulfilled;
}

bool canSatisfyCol(int* c) {
    if ((*(colHints + *c)).empty()) return true;
    int unfulfilled = 0, emptyCells = 0;
    for (int i = 0; i < ROWS; i++) {
        if (*(active + i * COLS + *c) && *(placement + i * COLS + *c) == -1)
            emptyCells++;
    }
    for (int h : *(colHints + *c)) {
        bool found = false;
        for (int i = 0; i < ROWS; i++) {
            if (*(active + i * COLS + *c) && *(field + i * COLS + *c) == h) {
                found = true; break;
            }
        }
        if (!found) unfulfilled++;
    }
    return emptyCells >= unfulfilled;
}

bool checkHints() {
    for (int i = 0; i < ROWS; i++) if (!canSatisfyRow(&i)) return false;
    for (int j = 0; j < COLS; j++) if (!canSatisfyCol(&j)) return false;
    return true;
}

bool solve() {
    int bestR = -1, bestC = -1, minOpts = 999999, emptyCount = 0;
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (*(active + r * COLS + c) && *(placement + r * COLS + c) == -1) {
                emptyCount++;
                int opts = 0;
                for (int i = 0; i < 4; i++) {
                    int nr = r + *(dr + i), nc = c + *(dc + i);
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
                        *(active + nr * COLS + nc) &&
                        *(placement + nr * COLS + nc) == -1) {
                        for (int v1 = 0; v1 <= 6; v1++) {
                            if (!isHintAllowed(&r, &c, &v1)) continue;
                            for (int v2 = 0; v2 <= 6; v2++) {
                                if (!isHintAllowed(&nr, &nc, &v2)) continue;
                                if (!*(used + *(dominoIndexMap + v1 * 7 + v2))) opts++;
                            }
                        }
                    }
                }
                if (opts == 0) return false;
                if (opts < minOpts) {
                    minOpts = opts; bestR = r; bestC = c;
                }
            }
        }
    }

    if (emptyCount == 0) return checkHints();

    int r = bestR, c = bestC;
    for (int i = 0; i < 4; i++) {
        int nr = r + *(dr + i), nc = c + *(dc + i);
        if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
            *(active + nr * COLS + nc) &&
            *(placement + nr * COLS + nc) == -1) {
            for (int v1 = 0; v1 <= 6; v1++) {
                if (!isHintAllowed(&r, &c, &v1)) continue;
                for (int v2 = 0; v2 <= 6; v2++) {
                    if (!isHintAllowed(&nr, &nc, &v2)) continue;
                    int idx = *(dominoIndexMap + v1 * 7 + v2);
                    if (!*(used + idx)) {
                        *(used + idx) = true;
                        *(placement + r * COLS + c) = idx;
                        *(placement + nr * COLS + nc) = idx;
                        *(field + r * COLS + c) = v1;
                        *(field + nr * COLS + nc) = v2;

                        if (canSatisfyRow(&r) && canSatisfyCol(&c) &&
                            canSatisfyRow(&nr) && canSatisfyCol(&nc)) {
                            if (solve()) return true;
                        }

                        *(used + idx) = false;
                        *(placement + r * COLS + c) = -1;
                        *(placement + nr * COLS + nc) = -1;
                        *(field + r * COLS + c) = -1;
                        *(field + nr * COLS + nc) = -1;
                    }
                }
            }
        }
    }
    return false;
}

// День 16: окрема функція скидання поля
// раніше цей код дублювався в autoSolve і userSolve
void resetField() {
    for (int i = 0; i < ROWS * COLS; i++) {
        *(placement + i) = -1;
        *(field + i) = -1;
    }
    for (int i = 0; i < 28; i++) *(used + i) = false;
}

void autoSolve() {
    int maxV = 6;
    generateDominoes(&maxV);
    resetField(); // День 16: використовуємо resetField замість дублювання коду

    cout << "\n  Шукаю розв'язок (MRV + Pruning)...\n";
    if (solve()) {
        cout << "  Розв'язок знайдено!\n";
        bool t = true;
        int w = 55;
        printLine(&w);
        string msg = "  РОЗВ'ЯЗОК";
        printMenuItem(&msg, &w);
        printLine(&w);
        printField(&t);
        cout << "\n  Використані кісточки:\n";
        printLine(&w);
        for (int i = 0; i < *dominoCount; i++) {
            if (*(used + i)) {
                string line = "  " + to_string((dominoes + i)->a) +
                              "-" + to_string((dominoes + i)->b) + "  [+]";
                printMenuItem(&line, &w);
            }
        }
        printLine(&w);
    } else {
        cout << "\n  Розв'язку не знайдено.\n";
    }
}

void userSolve() {
    cout << "\n";
    int w = 55;
    printLine(&w);
    string t = "  РЕЖИМ ГРАВЦЯ";
    printMenuItem(&t, &w);
    printLine(&w);
    cout << "\n  Вводьте координати двох клітинок та значення.\n";
    cout << "  Введіть 0 щоб завершити.\n\n";

    int maxV = 6;
    generateDominoes(&maxV);
    resetField(); // День 16: використовуємо resetField
    bool userUsed[28] = {};

    while (true) {
        bool t_val = true;
        printField(&t_val);

        cout << "\n  Рядок першої клітинки (0 = вихід): ";
        int minR0 = 0, maxR = ROWS;
        int r1 = safeReadInt(&minR0, &maxR);
        if (r1 == 0) break;

        cout << "  Колонка першої клітинки: ";
        int minC1 = 1, maxC = COLS;
        int c1 = safeReadInt(&minC1, &maxC);

        cout << "  Рядок другої клітинки: ";
        int minR1 = 1;
        int r2 = safeReadInt(&minR1, &maxR);

        cout << "  Колонка другої клітинки: ";
        int c2 = safeReadInt(&minC1, &maxC);
        r1--; c1--; r2--; c2--;

        if (!*(active + r1 * COLS + c1) || !*(active + r2 * COLS + c2)) {
            cout << "  Одна з клітинок неактивна!\n";
            continue;
        }
        bool adjacent = (r1 == r2 && abs(c1 - c2) == 1) ||
                        (c1 == c2 && abs(r1 - r2) == 1);
        if (!adjacent) {
            cout << "  Клітинки мають бути сусідніми!\n";
            continue;
        }
        if (*(placement + r1 * COLS + c1) != -1 ||
            *(placement + r2 * COLS + c2) != -1) {
            cout << "  Одна з клітинок вже зайнята!\n";
            continue;
        }

        cout << "  Значення першої половинки (0-6): ";
        int minV = 0, maxVIn = 6;
        int val1 = safeReadInt(&minV, &maxVIn);
        cout << "  Значення другої половинки (0-6): ";
        int val2 = safeReadInt(&minV, &maxVIn);

        int idx = *(dominoIndexMap + val1 * 7 + val2);
        if (*(userUsed + idx)) {
            cout << "  Кісточка " << val1 << "-" << val2 << " вже використана!\n";
            continue;
        }

        *(placement + r1 * COLS + c1) = idx;
        *(placement + r2 * COLS + c2) = idx;
        *(field + r1 * COLS + c1) = val1;
        *(field + r2 * COLS + c2) = val2;
        *(userUsed + idx) = true;
        cout << "  Кісточку " << val1 << "-" << val2 << " розміщено!\n";

        bool allPlaced = true;
        for (int i = 0; i < ROWS * COLS; i++) {
            if (*(active + i) && *(placement + i) == -1) {
                allPlaced = false; break;
            }
        }

        if (allPlaced) {
            if (checkHints())
                cout << "\n  Вітаємо! Ви правильно розв'язали головоломку!\n";
            else
                cout << "\n  Всі клітинки заповнені, але підказки не збігаються.\n";
            break;
        }
    }
}