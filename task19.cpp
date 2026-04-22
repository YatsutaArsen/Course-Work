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

struct Domino { int a, b; };
Domino* dominoes;
int* dominoCount;
int* dominoIndexMap;
bool* used;

void initData();
void cleanupData();
int utf8len(const string* s);
void printLine(int* width);
void printMenuItem(string* text, int* width);
void printMenu();
int safeReadInt(int* minVal, int* maxVal);
void printField();
void generateDominoes(int* maxVal);
void printFieldWithPlacement();
bool isHintAllowed(int* r, int* c, int* val);
bool canSatisfyRow(int* r);
bool canSatisfyCol(int* c);
bool canSatisfyHints(int* r, int* c);
bool checkHints();
bool solve();
void printSolution();
void autoSolve();
void userSolve();

int main() {
    initData();
    int choice = 0;
    int* ptrChoice = &choice;
    do {
        printMenu();
        int minV = 0, maxV = 3;
        *ptrChoice = safeReadInt(&minV, &maxV);
        switch (*ptrChoice) {
            case 1: autoSolve();  break;
            case 2: userSolve();  break;
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
    dominoes = new Domino[28];
    dominoCount = new int(0);
    dominoIndexMap = new int[7 * 7];
    used = new bool[28];

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
    delete[] dominoIndexMap;
    delete[] used;
    delete[] rowHints;
    delete[] colHints;
}

int utf8len(const string* s) {
    int len = 0;
    for (int i = 0; i < (int)s->length(); ) {
        unsigned char c = (*s)[i];
        if (c < 0x80) i += 1;
        else if (c < 0xE0) i += 2;
        else i += 3;
        len++;
    }
    return len;
}

void printLine(int* width) {
    cout << "+";
    for (int i = 0; i < *width; i++) cout << "-";
    cout << "+\n";
}

void printMenuItem(string* text, int* width) {
    cout << "| " << *text;
    for (int i = utf8len(text); i < *width - 1; i++) cout << " ";
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
                    cout << "  Число має бути від " << *minVal
                         << " до " << *maxVal << ". Спробуйте ще: ";
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

void printFieldWithPlacement() {
    cout << "\n  +";
    for (int j = 0; j < COLS; j++) cout << "----+";
    cout << "\n";

    for (int i = 0; i < ROWS; i++) {
        cout << "  |";
        for (int j = 0; j < COLS; j++) {
            if (!*(active + i * COLS + j)) { cout << "    |"; continue; }

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
            if (i + 1 < ROWS &&
                *(active + i * COLS + j) &&
                *(active + (i + 1) * COLS + j) &&
                *(placement + i * COLS + j) != -1 &&
                *(placement + i * COLS + j) == *(placement + (i + 1) * COLS + j)) {
                cout << "    +";
            } else {
                cout << "----+";
            }
        }
        cout << "\n";
    }

    cout << "    ";
    for (int j = 0; j < COLS; j++) cout << "  ^  ";
    cout << "\n";
    for (int line = 0; line < 4; line++) {
        cout << "    ";
        for (int j = 0; j < COLS; j++) {
            if (line < (int)(*(colHints + j)).size())
                cout << "  " << (*(colHints + j))[line] << "  ";
            else
                cout << "     ";
        }
        cout << "\n";
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
    int unfulfilled = 0;
    for (int h : *(rowHints + *r)) {
        bool found = false;
        for (int j = 0; j < COLS; j++) {
            if (*(active + *r * COLS + j) && *(field + *r * COLS + j) == h) {
                found = true; break;
            }
        }
        if (!found) unfulfilled++;
    }
    int emptyCells = 0;
    for (int j = 0; j < COLS; j++) {
        if (*(active + *r * COLS + j) && *(placement + *r * COLS + j) == -1)
            emptyCells++;
    }
    return emptyCells >= unfulfilled;
}

bool canSatisfyCol(int* c) {
    if ((*(colHints + *c)).empty()) return true;
    int unfulfilled = 0;
    for (int h : *(colHints + *c)) {
        bool found = false;
        for (int i = 0; i < ROWS; i++) {
            if (*(active + i * COLS + *c) && *(field + i * COLS + *c) == h) {
                found = true; break;
            }
        }
        if (!found) unfulfilled++;
    }
    int emptyCells = 0;
    for (int i = 0; i < ROWS; i++) {
        if (*(active + i * COLS + *c) && *(placement + i * COLS + *c) == -1)
            emptyCells++;
    }
    return emptyCells >= unfulfilled;
}

bool canSatisfyHints(int* r, int* c) {
    return canSatisfyRow(r) && canSatisfyCol(c);
}

// День 13: під час тестування помітили що checkHints викликав
// canSatisfyHints з двома індексами замість окремих перевірок
// через це колонки перевірялись некоректно — виправлено на
// canSatisfyRow і canSatisfyCol окремо
bool checkHints() {
    for (int i = 0; i < ROWS; i++) if (!canSatisfyRow(&i)) return false;
    for (int j = 0; j < COLS; j++) if (!canSatisfyCol(&j)) return false;
    return true;
}

bool solve() {
    int bestR = -1, bestC = -1, minOpts = 999999;
    int emptyCount = 0;

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!*(active + r * COLS + c)) continue;
            if (*(placement + r * COLS + c) != -1) continue;

            emptyCount++;
            int opts = 0;
            int dr[] = {0, 1, 0, -1};
            int dc[] = {1, 0, -1, 0};

            for (int i = 0; i < 4; i++) {
                int nr = r + *(dr + i), nc = c + *(dc + i);
                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (!*(active + nr * COLS + nc)) continue;
                if (*(placement + nr * COLS + nc) != -1) continue;

                for (int v1 = 0; v1 <= 6; v1++) {
                    if (!isHintAllowed(&r, &c, &v1)) continue;
                    for (int v2 = 0; v2 <= 6; v2++) {
                        if (!isHintAllowed(&nr, &nc, &v2)) continue;
                        int idx = *(dominoIndexMap + v1 * 7 + v2);
                        if (!*(used + idx)) opts++;
                    }
                }
            }

            if (opts == 0) return false;

            if (opts < minOpts) {
                minOpts = opts;
                bestR = r; bestC = c;
            }
        }
    }

    if (emptyCount == 0) return checkHints();

    int r = bestR, c = bestC;
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};

    for (int i = 0; i < 4; i++) {
        int nr = r + *(dr + i), nc = c + *(dc + i);
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
        if (!*(active + nr * COLS + nc)) continue;
        if (*(placement + nr * COLS + nc) != -1) continue;

        for (int v1 = 0; v1 <= 6; v1++) {
            if (!isHintAllowed(&r, &c, &v1)) continue;
            for (int v2 = 0; v2 <= 6; v2++) {
                if (!isHintAllowed(&nr, &nc, &v2)) continue;
                int idx = *(dominoIndexMap + v1 * 7 + v2);
                if (*(used + idx)) continue;

                *(used + idx) = true;
                *(placement + r * COLS + c) = idx;
                *(placement + nr * COLS + nc) = idx;
                *(field + r * COLS + c) = v1;
                *(field + nr * COLS + nc) = v2;

                if (canSatisfyHints(&r, &c) && canSatisfyHints(&nr, &nc)) {
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
    return false;
}

void printSolution() {
    int w = 40;
    printLine(&w);
    string t = "  РОЗВ'ЯЗОК";
    printMenuItem(&t, &w);
    printLine(&w);
    printFieldWithPlacement();
    cout << "\n  Використані кісточки:\n";
    printLine(&w);
    for (int i = 0; i < *dominoCount; i++) {
        if (!*(used + i)) continue;
        string line = "  " + to_string((dominoes + i)->a) +
                      "-" + to_string((dominoes + i)->b) + "  [+]";
        printMenuItem(&line, &w);
    }
    printLine(&w);
}

void autoSolve() {
    int maxV = 6;
    generateDominoes(&maxV);
    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            *(placement + i * COLS + j) = -1;
            *(field + i * COLS + j) = -1;
        }
    }
    for (int i = 0; i < *dominoCount; i++) *(used + i) = false;
    cout << "\n  Шукаю розв'язок (MRV + Pruning)...\n";
    if (solve()) {
        cout << "  Розв'язок знайдено!\n";
        printSolution();
    } else {
        cout << "\n  Розв'язку не знайдено.\n";
    }
}

void userSolve() {
    int w = 40;
    cout << "\n";
    printLine(&w);
    string t = "  РЕЖИМ ГРАВЦЯ";
    printMenuItem(&t, &w);
    printLine(&w);
    cout << "\n  Вводьте координати двох клітинок та значення.\n";
    cout << "  Введіть 0 щоб завершити.\n\n";

    int maxV = 6;
    generateDominoes(&maxV);

    for (int i = 0; i < ROWS; i++) {
        for (int j = 0; j < COLS; j++) {
            *(placement + i * COLS + j) = -1;
            *(field + i * COLS + j) = -1;
        }
    }

    bool userUsed[28] = {};

    while (true) {
        printFieldWithPlacement();

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
        for (int i = 0; i < ROWS; i++)
            for (int j = 0; j < COLS; j++)
                if (*(active + i * COLS + j) && *(placement + i * COLS + j) == -1)
                    allPlaced = false;

        if (allPlaced) {
            if (checkHints())
                cout << "\n  Вітаємо! Ви правильно розв'язали головоломку!\n";
            else
                cout << "\n  Всі клітинки заповнені, але підказки не збігаються.\n";
            break;
        }
    }
}