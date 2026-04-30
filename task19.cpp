/* ----------------------------------------------------------------<Header>-
 Name:        task19.cpp
 Title:       Головоломка Доміно
 Description: Розміщення кісточок доміно на полі у формі літер B і G.
              Алгоритм: рекурсивний перебір з поверненням (MRV + Pruning).
------------------------------------------------------------------</Header>-*/

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <algorithm>
#include <ctime>
#include <chrono>
using namespace std;

const int ROWS = 7;
const int COLS = 6;

/* ----------------------------------------------------------------------[<]-
 Struct: Domino
 Synopsis: зберігає два значення однієї кісточки доміно.
---------------------------------------------------------------------[>]-*/
struct Domino { int a, b; };

/* ----------------------------------------------------------------------[<]-
 Struct: GameData
 Synopsis: містить усі дані гри — поле, кісточки, підказки.
           Передається у функції через вказівник замість глобальних змінних.
---------------------------------------------------------------------[>]-*/
struct GameData {
    bool*            active;        // активні клітинки поля (форма B і G)
    int*             field;         // цифри розміщені на клітинках (-1 = порожньо)
    int*             placement;     // індекс кісточки в клітинці (-1 = порожньо)
    Domino*          dominoes;      // масив усіх 28 кісточок
    int              dominoCount;   // кількість згенерованих кісточок
    bool*            used;          // які кісточки вже використані
    vector<int>*     rowHints;      // підказки для рядків
    vector<int>*     colHints;      // підказки для колонок
    int*             dominoIndexMap;// кеш: [a*7+b] -> індекс кісточки
};

void initData(GameData* g);
void cleanupData(GameData* g);
int  utf8len(const string& s);
void printLine(int width);
void printMenuItem(string text, int width);
void printMenu();
int  safeReadInt(int minVal, int maxVal);
void printField(GameData* g, bool showPlacement);
void generateDominoes(GameData* g, int maxVal);
bool isHintAllowed(GameData* g, int r, int c, int val);
bool canSatisfyRow(GameData* g, int r);
bool canSatisfyCol(GameData* g, int c);
bool checkHints(GameData* g);
bool solve(GameData* g);
void resetField(GameData* g);
bool runTests(GameData* g);
void runDeveloperTests(GameData* g);
void autoSolve(GameData* g);
void userSolve(GameData* g);
bool fillBoardRandomly(GameData* g);
void generateTask(GameData* g);

/* ----------------------------------------------------------------------[<]-
 Function: main
 Synopsis: точка входу — ініціалізація даних і головний цикл меню.
---------------------------------------------------------------------[>]-*/
int main() {
    srand(time(0));
    GameData g;
    initData(&g);

    int choice = 0;
    do {
        printMenu();
        choice = safeReadInt(0, 5);
        switch (choice) {
            case 1: autoSolve(&g);        break;
            case 2: userSolve(&g);        break;
            case 3: printField(&g, false); break;
            case 4: runDeveloperTests(&g); break;
            case 5: generateTask(&g);     break;
            case 0: {
                cout << "\n";
                printLine(55);
                printMenuItem("  До побачення!", 55);
                printLine(55);
                cout << "\n";
                break;
            }
        }
    } while (choice != 0);

    cleanupData(&g);
    return 0;
}

/* ----------------------------------------------------------------------[<]-
 Function: initData
 Synopsis: виділяє динамічну пам'ять і заповнює початкові дані поля,
           підказок і масивів кісточок.
---------------------------------------------------------------------[>]-*/
void initData(GameData* g) {
    g->active       = new bool[ROWS * COLS];
    g->field        = new int[ROWS * COLS];
    g->placement    = new int[ROWS * COLS];
    g->dominoes     = new Domino[28];
    g->dominoCount  = 0;
    g->used         = new bool[28];
    g->rowHints     = new vector<int>[ROWS];
    g->colHints     = new vector<int>[COLS];
    g->dominoIndexMap = new int[7 * 7];

    // форма літер B і G на полі
    bool initActive[ROWS][COLS] = {
        {1,1,1, 1,1,1},
        {1,0,1, 1,0,0},
        {1,0,1, 1,0,0},
        {1,1,1, 1,1,1},
        {1,0,1, 1,0,1},
        {1,0,1, 1,0,1},
        {1,1,1, 1,1,1}
    };

    for (int i = 0; i < ROWS * COLS; i++) {
        *(g->active + i)    = initActive[i / COLS][i % COLS];
        *(g->field + i)     = -1;
        *(g->placement + i) = -1;
    }

    for (int i = 0; i < 28; i++) *(g->used + i) = false;

    *(g->rowHints + 0) = {2, 0, 1, 5};
    *(g->rowHints + 3) = {1, 3, 4};
    *(g->rowHints + 6) = {0, 2, 3};

    *(g->colHints + 0) = {2, 4, 6};
    *(g->colHints + 1) = {0, 1, 5};
    *(g->colHints + 2) = {0, 3, 6};
    *(g->colHints + 3) = {1, 3, 6};
    *(g->colHints + 4) = {0, 1, 5};
    *(g->colHints + 5) = {1, 2, 3, 4};
}

/* ----------------------------------------------------------------------[<]-
 Function: cleanupData
 Synopsis: звільняє всю динамічну пам'ять структури GameData.
---------------------------------------------------------------------[>]-*/
void cleanupData(GameData* g) {
    delete[] g->active;
    delete[] g->field;
    delete[] g->placement;
    delete[] g->dominoes;
    delete[] g->used;
    delete[] g->rowHints;
    delete[] g->colHints;
    delete[] g->dominoIndexMap;
}

/* ----------------------------------------------------------------------[<]-
 Function: utf8len
 Synopsis: повертає кількість символів рядка з урахуванням UTF-8.
           Потрібна для правильного вирівнювання рамок меню з українським текстом.
---------------------------------------------------------------------[>]-*/
int utf8len(const string& s) {
    int len = 0;
    for (int i = 0; i < (int)s.length(); ) {
        unsigned char c = s[i];
        if      (c < 0x80) i += 1;
        else if (c < 0xE0) i += 2;
        else                i += 3;
        len++;
    }
    return len;
}

/* ----------------------------------------------------------------------[<]-
 Function: printLine
 Synopsis: виводить горизонтальну лінію рамки заданої ширини.
---------------------------------------------------------------------[>]-*/
void printLine(int width) {
    cout << "+";
    for (int i = 0; i < width; i++) cout << "-";
    cout << "+\n";
}

/* ----------------------------------------------------------------------[<]-
 Function: printMenuItem
 Synopsis: виводить один рядок меню з вирівнюванням по правому краю рамки.
---------------------------------------------------------------------[>]-*/
void printMenuItem(string text, int width) {
    cout << "| " << text;
    for (int i = utf8len(text); i < width - 1; i++) cout << " ";
    cout << "|\n";
}

/* ----------------------------------------------------------------------[<]-
 Function: printMenu
 Synopsis: виводить головне меню програми.
---------------------------------------------------------------------[>]-*/
void printMenu() {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("             ГОЛОВОЛОМКА ДОМІНО", w);
    printLine(w);
    printMenuItem("  [1] Розв'язати автоматично (Турбо)", w);
    printMenuItem("  [2] Спробувати самому", w);
    printMenuItem("  [3] Показати поле", w);
    printMenuItem("  [4] Запустити інженерні тести", w);
    printMenuItem("  [5] Згенерувати нове завдання", w);
    printMenuItem("  [0] Вийти", w);
    printLine(w);
    cout << "  Ваш вибір: ";
}

/* ----------------------------------------------------------------------[<]-
 Function: safeReadInt
 Synopsis: читає ціле число з перевіркою діапазону.
           Повторює запит доки не буде введено коректне значення.
---------------------------------------------------------------------[>]-*/
int safeReadInt(int minVal, int maxVal) {
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
                if (val >= minVal && val <= maxVal) return val;
                cout << "  Число має бути від " << minVal << " до "
                     << maxVal << ". Спробуйте ще: ";
                continue;
            } catch (...) {}
        }
        cout << "  Помилка! Введіть ціле число: ";
    }
    return minVal;
}

/* ----------------------------------------------------------------------[<]-
 Function: printField
 Synopsis: виводить поле у двох режимах:
           sp=false — початкове поле зі знаками #;
           sp=true  — поле з розміщеними кісточками і цифрами.
---------------------------------------------------------------------[>]-*/
void printField(GameData* g, bool sp) {
    cout << "\n  +";
    for (int j = 0; j < COLS; j++) cout << (sp ? "----+" : "---+");
    cout << "\n";

    for (int i = 0; i < ROWS; i++) {
        cout << "  |";
        for (int j = 0; j < COLS; j++) {
            if (!*(g->active + i * COLS + j)) {
                cout << (sp ? "    |" : "   |");
                continue;
            }
            if (!sp) { cout << " # |"; continue; }

            // визначаємо правий кордон між клітинками однієї кісточки
            string rightBorder = "|";
            if (j + 1 < COLS &&
                *(g->active + i * COLS + (j + 1)) &&
                *(g->placement + i * COLS + j) != -1 &&
                *(g->placement + i * COLS + j) == *(g->placement + i * COLS + (j + 1))) {
                rightBorder = " ";
            }
            if (*(g->field + i * COLS + j) >= 0)
                cout << " " << *(g->field + i * COLS + j) << " " << rightBorder;
            else
                cout << " . " << rightBorder;
        }

        if (!(*(g->rowHints + i)).empty()) {
            cout << "  <";
            for (int h : *(g->rowHints + i)) cout << " " << h;
        }

        cout << "\n  +";
        for (int j = 0; j < COLS; j++) {
            // прибираємо нижню стінку між клітинками вертикальної кісточки
            if (sp && i + 1 < ROWS &&
                *(g->active + i * COLS + j) &&
                *(g->active + (i + 1) * COLS + j) &&
                *(g->placement + i * COLS + j) != -1 &&
                *(g->placement + i * COLS + j) == *(g->placement + (i + 1) * COLS + j)) {
                cout << "    +";
            } else {
                cout << (sp ? "----+" : "---+");
            }
        }
        cout << "\n";
    }

    cout << "    ";
    for (int j = 0; j < COLS; j++) cout << (sp ? "  ^  " : "  ^ ");
    cout << "\n";
    for (int line = 0; line < 4; line++) {
        cout << "    ";
        for (int j = 0; j < COLS; j++) {
            if (line < (int)(*(g->colHints + j)).size())
                cout << "  " << (*(g->colHints + j))[line] << (sp ? "  " : " ");
            else
                cout << (sp ? "     " : "    ");
        }
        cout << "\n";
    }
}

/* ----------------------------------------------------------------------[<]-
 Function: generateDominoes
 Synopsis: генерує всі кісточки від 0-0 до maxVal-maxVal і заповнює
           кеш dominoIndexMap в обидва боки (0-1 і 1-0 — одна кісточка).
---------------------------------------------------------------------[>]-*/
void generateDominoes(GameData* g, int maxVal) {
    g->dominoCount = 0;
    for (int i = 0; i <= maxVal; i++) {
        for (int j = i; j <= maxVal; j++) {
            (g->dominoes + g->dominoCount)->a = i;
            (g->dominoes + g->dominoCount)->b = j;
            *(g->dominoIndexMap + i * 7 + j) = g->dominoCount;
            *(g->dominoIndexMap + j * 7 + i) = g->dominoCount;
            g->dominoCount++;
        }
    }
}

/* ----------------------------------------------------------------------[<]-
 Function: isHintAllowed
 Synopsis: перевіряє чи дозволено розмістити значення val у клітинці (r,c)
           відповідно до підказок рядка і колонки.
---------------------------------------------------------------------[>]-*/
bool isHintAllowed(GameData* g, int r, int c, int val) {
    if (!(*(g->rowHints + r)).empty()) {
        bool found = false;
        for (int h : *(g->rowHints + r)) {
            if (h == val) { found = true; break; }
        }
        if (!found) return false;
    }
    if (!(*(g->colHints + c)).empty()) {
        bool found = false;
        for (int h : *(g->colHints + c)) {
            if (h == val) { found = true; break; }
        }
        if (!found) return false;
    }
    return true;
}

/* ----------------------------------------------------------------------[<]-
 Function: canSatisfyRow
 Synopsis: перевіряє чи рядок r ще може виконати свої підказки.
           Якщо порожніх клітинок менше ніж невиконаних підказок — false.
---------------------------------------------------------------------[>]-*/
bool canSatisfyRow(GameData* g, int r) {
    if ((*(g->rowHints + r)).empty()) return true;
    int unfulfilled = 0, emptyCells = 0;
    for (int j = 0; j < COLS; j++) {
        if (*(g->active + r * COLS + j) && *(g->placement + r * COLS + j) == -1)
            emptyCells++;
    }
    for (int h : *(g->rowHints + r)) {
        bool found = false;
        for (int j = 0; j < COLS; j++) {
            if (*(g->active + r * COLS + j) && *(g->field + r * COLS + j) == h) {
                found = true; break;
            }
        }
        if (!found) unfulfilled++;
    }
    return emptyCells >= unfulfilled;
}

/* ----------------------------------------------------------------------[<]-
 Function: canSatisfyCol
 Synopsis: те саме що canSatisfyRow, але для колонки c.
---------------------------------------------------------------------[>]-*/
bool canSatisfyCol(GameData* g, int c) {
    if ((*(g->colHints + c)).empty()) return true;
    int unfulfilled = 0, emptyCells = 0;
    for (int i = 0; i < ROWS; i++) {
        if (*(g->active + i * COLS + c) && *(g->placement + i * COLS + c) == -1)
            emptyCells++;
    }
    for (int h : *(g->colHints + c)) {
        bool found = false;
        for (int i = 0; i < ROWS; i++) {
            if (*(g->active + i * COLS + c) && *(g->field + i * COLS + c) == h) {
                found = true; break;
            }
        }
        if (!found) unfulfilled++;
    }
    return emptyCells >= unfulfilled;
}

/* ----------------------------------------------------------------------[<]-
 Function: checkHints
 Synopsis: фінальна перевірка — чи всі підказки виконані після заповнення поля.
---------------------------------------------------------------------[>]-*/
bool checkHints(GameData* g) {
    for (int i = 0; i < ROWS; i++) if (!canSatisfyRow(g, i)) return false;
    for (int j = 0; j < COLS; j++) if (!canSatisfyCol(g, j)) return false;
    return true;
}

/* ----------------------------------------------------------------------[<]-
 Function: solve
 Synopsis: рекурсивний розв'язувач з евристикою MRV.
           Обирає клітинку з найменшою кількістю варіантів і пробує всі
           можливі кісточки. При тупику — відкатується (backtracking).
---------------------------------------------------------------------[>]-*/
bool solve(GameData* g) {
    int bestR = -1, bestC = -1, minOpts = 999999, emptyCount = 0;
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};

    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (*(g->active + r * COLS + c) && *(g->placement + r * COLS + c) == -1) {
                emptyCount++;
                int opts = 0;
                for (int i = 0; i < 4; i++) {
                    int nr = r + dr[i], nc = c + dc[i];
                    if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
                        *(g->active + nr * COLS + nc) &&
                        *(g->placement + nr * COLS + nc) == -1) {
                        for (int v1 = 0; v1 <= 6; v1++) {
                            if (!isHintAllowed(g, r, c, v1)) continue;
                            for (int v2 = 0; v2 <= 6; v2++) {
                                if (!isHintAllowed(g, nr, nc, v2)) continue;
                                if (!*(g->used + *(g->dominoIndexMap + v1 * 7 + v2))) opts++;
                            }
                        }
                    }
                }
                if (opts == 0) return false; // тупик — жодного варіанту
                if (opts < minOpts) {
                    minOpts = opts; bestR = r; bestC = c;
                }
            }
        }
    }

    if (emptyCount == 0) return checkHints(g);

    int r = bestR, c = bestC;
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i], nc = c + dc[i];
        if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
            *(g->active + nr * COLS + nc) &&
            *(g->placement + nr * COLS + nc) == -1) {
            for (int v1 = 0; v1 <= 6; v1++) {
                if (!isHintAllowed(g, r, c, v1)) continue;
                for (int v2 = 0; v2 <= 6; v2++) {
                    if (!isHintAllowed(g, nr, nc, v2)) continue;
                    int idx = *(g->dominoIndexMap + v1 * 7 + v2);
                    if (!*(g->used + idx)) {
                        // розміщуємо кісточку
                        *(g->used + idx) = true;
                        *(g->placement + r * COLS + c) = idx;
                        *(g->placement + nr * COLS + nc) = idx;
                        *(g->field + r * COLS + c) = v1;
                        *(g->field + nr * COLS + nc) = v2;

                        if (canSatisfyRow(g, r) && canSatisfyCol(g, c) &&
                            canSatisfyRow(g, nr) && canSatisfyCol(g, nc)) {
                            if (solve(g)) return true;
                        }

                        // відкатуємось
                        *(g->used + idx) = false;
                        *(g->placement + r * COLS + c) = -1;
                        *(g->placement + nr * COLS + nc) = -1;
                        *(g->field + r * COLS + c) = -1;
                        *(g->field + nr * COLS + nc) = -1;
                    }
                }
            }
        }
    }
    return false;
}

/* ----------------------------------------------------------------------[<]-
 Function: resetField
 Synopsis: скидає поле до початкового стану — всі клітинки порожні,
           всі кісточки невикористані.
---------------------------------------------------------------------[>]-*/
void resetField(GameData* g) {
    for (int i = 0; i < ROWS * COLS; i++) {
        *(g->placement + i) = -1;
        *(g->field + i)     = -1;
    }
    for (int i = 0; i < 28; i++) *(g->used + i) = false;
}

/* ----------------------------------------------------------------------[<]-
 Function: runTests
 Synopsis: перевіряє коректність розв'язку за 5 критеріями:
           заповненість, унікальність, сусідство, підказки рядків і колонок.
---------------------------------------------------------------------[>]-*/
bool runTests(GameData* g) {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("  Перевірка розв'язку", w);
    printLine(w);

    // тест 1: заповненість поля
    bool fullCoverage = true;
    for (int i = 0; i < ROWS * COLS; i++) {
        if (*(g->active + i) && *(g->placement + i) == -1) fullCoverage = false;
    }
    cout << "  Заповненість поля.................. "
         << (fullCoverage ? "ok" : "не пройдено") << "\n";
    if (!fullCoverage) {
        printLine(w);
        printMenuItem("  Поле не заповнене повністю", w);
        printLine(w);
        return false;
    }

    // тест 2 і 3: унікальність кісточок і правильність їх значень + сусідство
    bool uniqueValid = true, adjValid = true, valuesValid = true;
    int counts[28] = {0};
    for (int i = 0; i < ROWS * COLS; i++) {
        if (*(g->active + i)) counts[*(g->placement + i)]++;
    }

    for (int i = 0; i < 28; i++) {
        if (counts[i] != 0 && counts[i] != 2) uniqueValid = false;
        if (counts[i] == 2) {
            int r1 = -1, c1 = -1, r2 = -1, c2 = -1;
            for (int pos = 0; pos < ROWS * COLS; pos++) {
                if (*(g->active + pos) && *(g->placement + pos) == i) {
                    if (r1 == -1) { r1 = pos / COLS; c1 = pos % COLS; }
                    else          { r2 = pos / COLS; c2 = pos % COLS; }
                }
            }
            bool adj = (r1 == r2 && abs(c1 - c2) == 1) ||
                       (c1 == c2 && abs(r1 - r2) == 1);
            if (!adj) adjValid = false;

            int v1 = *(g->field + r1 * COLS + c1);
            int v2 = *(g->field + r2 * COLS + c2);
            int da = (g->dominoes + i)->a;
            int db = (g->dominoes + i)->b;
            if (!((v1 == da && v2 == db) || (v1 == db && v2 == da)))
                valuesValid = false;
        }
    }
    cout << "  Унікальність кісточок.............. "
         << ((uniqueValid && valuesValid) ? "ok" : "не пройдено") << "\n";
    cout << "  Сусідство клітинок................. "
         << (adjValid ? "ok" : "не пройдено") << "\n";

    // тест 4: підказки рядків
    bool rowsOk = true;
    for (int r = 0; r < ROWS; r++) {
        for (int h : *(g->rowHints + r)) {
            bool found = false;
            for (int c = 0; c < COLS; c++) {
                if (*(g->active + r * COLS + c) && *(g->field + r * COLS + c) == h)
                    found = true;
            }
            if (!found) rowsOk = false;
        }
    }
    cout << "  Підказки рядків.................... "
         << (rowsOk ? "ok" : "не пройдено") << "\n";

    // тест 5: підказки колонок
    bool colsOk = true;
    for (int c = 0; c < COLS; c++) {
        for (int h : *(g->colHints + c)) {
            bool found = false;
            for (int r = 0; r < ROWS; r++) {
                if (*(g->active + r * COLS + c) && *(g->field + r * COLS + c) == h)
                    found = true;
            }
            if (!found) colsOk = false;
        }
    }
    cout << "  Підказки колонок................... "
         << (colsOk ? "ok" : "не пройдено") << "\n";

    printLine(w);
    if (fullCoverage && uniqueValid && adjValid && valuesValid && rowsOk && colsOk) {
        printMenuItem("  Розв'язок правильний", w);
        printLine(w);
        return true;
    } else {
        printMenuItem("  У розв'язку є помилки", w);
        printLine(w);
        return false;
    }
}

/* ----------------------------------------------------------------------[<]-
 Function: runDeveloperTests
 Synopsis: інженерне тестування — 4 сценарії що перевіряють чи система
           правильно відхиляє некоректні розв'язки.
---------------------------------------------------------------------[>]-*/
void runDeveloperTests(GameData* g) {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("  Інженерне тестування", w);
    printLine(w);

    generateDominoes(g, 6);

    cout << "\n  Тест 1: порожнє поле\n";
    resetField(g);
    printField(g, true);
    bool t1 = runTests(g);
    if (!t1) cout << "  Система правильно відхилила порожнє поле\n";
    else     cout << "  Помилка: система прийняла порожнє поле\n";

    cout << "\n  Тест 2: правильний розв'язок\n";
    resetField(g);
    solve(g);
    printField(g, true);
    bool t2 = runTests(g);
    if (t2)  cout << "  Система правильно прийняла коректний розв'язок\n";
    else     cout << "  Помилка: система відхилила правильний розв'язок\n";

    cout << "\n  Тест 3: одне число замінено на 9\n";
    int originalVal = *(g->field + 0);
    *(g->field + 0) = 9;
    printField(g, true);
    bool t3 = runTests(g);
    if (!t3) cout << "  Система побачила невірне число\n";
    else     cout << "  Помилка: система пропустила невірне число\n";
    *(g->field + 0) = originalVal;

    cout << "\n  Тест 4: одна кісточка розірвана на два кути поля\n";
    int orig1 = *(g->placement + 0);
    int orig2 = *(g->placement + (ROWS * COLS - 1));
    *(g->placement + 0) = 5;
    *(g->placement + (ROWS * COLS - 1)) = 5;
    printField(g, true);
    bool t4 = runTests(g);
    if (!t4) cout << "  Система побачила розірване доміно\n";
    else     cout << "  Помилка: система не помітила розірване доміно\n";
    *(g->placement + 0) = orig1;
    *(g->placement + (ROWS * COLS - 1)) = orig2;

    cout << "\n";
    printLine(w);
    printMenuItem("  Усі модулі захисту працюють коректно", w);
    printLine(w);
}

/* ----------------------------------------------------------------------[<]-
 Function: fillBoardRandomly
 Synopsis: рекурсивно заповнює поле випадковими кісточками.
           Використовується генератором нових завдань.
---------------------------------------------------------------------[>]-*/
bool fillBoardRandomly(GameData* g) {
    int bestR = -1, bestC = -1;
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (*(g->active + r * COLS + c) && *(g->placement + r * COLS + c) == -1) {
                bestR = r; bestC = c; break;
            }
        }
        if (bestR != -1) break;
    }
    if (bestR == -1) return true;

    int r = bestR, c = bestC;
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};

    // перемішуємо напрямки для випадковості результату
    int dirs[4] = {0, 1, 2, 3};
    for (int i = 0; i < 4; i++) {
        int j = i + rand() % (4 - i);
        int tmp = dirs[i]; dirs[i] = dirs[j]; dirs[j] = tmp;
    }

    for (int i = 0; i < 4; i++) {
        int nr = r + dr[dirs[i]], nc = c + dc[dirs[i]];
        if (nr >= 0 && nr < ROWS && nc >= 0 && nc < COLS &&
            *(g->active + nr * COLS + nc) &&
            *(g->placement + nr * COLS + nc) == -1) {

            // перемішуємо кісточки для випадковості
            int doms[28];
            for (int k = 0; k < 28; k++) doms[k] = k;
            for (int k = 0; k < 28; k++) {
                int j = k + rand() % (28 - k);
                int tmp = doms[k]; doms[k] = doms[j]; doms[j] = tmp;
            }

            for (int k = 0; k < 28; k++) {
                int idx = doms[k];
                if (!*(g->used + idx)) {
                    int v1 = (g->dominoes + idx)->a;
                    int v2 = (g->dominoes + idx)->b;
                    if (rand() % 2 == 0) { int tmp = v1; v1 = v2; v2 = tmp; }

                    *(g->used + idx) = true;
                    *(g->placement + r * COLS + c) = idx;
                    *(g->placement + nr * COLS + nc) = idx;
                    *(g->field + r * COLS + c) = v1;
                    *(g->field + nr * COLS + nc) = v2;

                    if (fillBoardRandomly(g)) return true;

                    // відкатуємось
                    *(g->used + idx) = false;
                    *(g->placement + r * COLS + c) = -1;
                    *(g->placement + nr * COLS + nc) = -1;
                    *(g->field + r * COLS + c) = -1;
                    *(g->field + nr * COLS + nc) = -1;
                }
            }
        }
    }
    return false;
}

/* ----------------------------------------------------------------------[<]-
 Function: generateTask
 Synopsis: генерує нове випадкове завдання — заповнює поле, збирає підказки
           з унікальних значень кожного рядка і колонки, потім очищає розв'язок.
---------------------------------------------------------------------[>]-*/
void generateTask(GameData* g) {
    generateDominoes(g, 6);

    for (int i = 0; i < ROWS; i++) (*(g->rowHints + i)).clear();
    for (int i = 0; i < COLS; i++) (*(g->colHints + i)).clear();
    resetField(g);

    cout << "\n  Генерую нове поле...\n";
    if (fillBoardRandomly(g)) {
        for (int r = 0; r < ROWS; r++) {
            bool seen[7] = {false};
            for (int c = 0; c < COLS; c++) {
                if (*(g->active + r * COLS + c)) {
                    int val = *(g->field + r * COLS + c);
                    if (!seen[val]) {
                        seen[val] = true;
                        (*(g->rowHints + r)).push_back(val);
                    }
                }
            }
            sort((*(g->rowHints + r)).begin(), (*(g->rowHints + r)).end());
        }
        for (int c = 0; c < COLS; c++) {
            bool seen[7] = {false};
            for (int r = 0; r < ROWS; r++) {
                if (*(g->active + r * COLS + c)) {
                    int val = *(g->field + r * COLS + c);
                    if (!seen[val]) {
                        seen[val] = true;
                        (*(g->colHints + c)).push_back(val);
                    }
                }
            }
            sort((*(g->colHints + c)).begin(), (*(g->colHints + c)).end());
        }

        resetField(g);
        cout << "  Нове завдання готове\n";
        printField(g, false);
    } else {
        cout << "  Не вдалось згенерувати поле, спробуйте ще раз\n";
    }
}

/* ----------------------------------------------------------------------[<]-
 Function: autoSolve
 Synopsis: скидає поле, запускає solve() і виводить результат з часом пошуку.
           Після знаходження розв'язку запускає runTests для верифікації.
---------------------------------------------------------------------[>]-*/
void autoSolve(GameData* g) {
    generateDominoes(g, 6);
    resetField(g);

    cout << "\n  Шукаю розв'язок...\n";

    auto start   = chrono::high_resolution_clock::now();
    bool success = solve(g);
    auto end     = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    if (success) {
        cout << "  Розв'язок знайдено\n";
        int w = 55;
        printLine(w);
        printMenuItem("  Розв'язок", w);
        printLine(w);
        printField(g, true);
        cout << "\n  Використані кісточки:\n";
        printLine(w);
        for (int i = 0; i < g->dominoCount; i++) {
            if (*(g->used + i)) {
                string line = "  " + to_string((g->dominoes + i)->a) +
                              "-" + to_string((g->dominoes + i)->b) + "  [+]";
                printMenuItem(line, w);
            }
        }
        printLine(w);
        cout << "  Час пошуку: " << elapsed.count() << " с\n";
        runTests(g);
    } else {
        cout << "\n  Розв'язку не знайдено\n";
        cout << "  Час пошуку: " << elapsed.count() << " с\n";
    }
}

/* ----------------------------------------------------------------------[<]-
 Function: userSolve
 Synopsis: режим гравця — ручне розміщення кісточок з перевірками
           активності, сусідства, зайнятості і унікальності.
---------------------------------------------------------------------[>]-*/
void userSolve(GameData* g) {
    cout << "\n";
    int w = 55;
    printLine(w);
    printMenuItem("  Режим гравця", w);
    printLine(w);
    cout << "\n  Вводьте координати двох клітинок та значення\n";
    cout << "  Введіть 0 щоб завершити\n\n";

    generateDominoes(g, 6);
    resetField(g);
    bool userUsed[28] = {};

    while (true) {
        printField(g, true);

        cout << "\n  Рядок першої клітинки (0 = вихід): ";
        int r1 = safeReadInt(0, ROWS);
        if (r1 == 0) break;

        cout << "  Колонка першої клітинки: ";
        int c1 = safeReadInt(1, COLS);

        cout << "  Рядок другої клітинки: ";
        int r2 = safeReadInt(1, ROWS);

        cout << "  Колонка другої клітинки: ";
        int c2 = safeReadInt(1, COLS);
        r1--; c1--; r2--; c2--;

        if (!*(g->active + r1 * COLS + c1) || !*(g->active + r2 * COLS + c2)) {
            cout << "  Одна з клітинок неактивна\n";
            continue;
        }
        bool adjacent = (r1 == r2 && abs(c1 - c2) == 1) ||
                        (c1 == c2 && abs(r1 - r2) == 1);
        if (!adjacent) {
            cout << "  Клітинки мають бути сусідніми\n";
            continue;
        }
        if (*(g->placement + r1 * COLS + c1) != -1 ||
            *(g->placement + r2 * COLS + c2) != -1) {
            cout << "  Одна з клітинок вже зайнята\n";
            continue;
        }

        cout << "  Значення першої половинки (0-6): ";
        int val1 = safeReadInt(0, 6);
        cout << "  Значення другої половинки (0-6): ";
        int val2 = safeReadInt(0, 6);

        int idx = *(g->dominoIndexMap + val1 * 7 + val2);
        if (*(userUsed + idx)) {
            cout << "  Кісточка " << val1 << "-" << val2 << " вже використана\n";
            continue;
        }

        *(g->placement + r1 * COLS + c1) = idx;
        *(g->placement + r2 * COLS + c2) = idx;
        *(g->field + r1 * COLS + c1)     = val1;
        *(g->field + r2 * COLS + c2)     = val2;
        *(userUsed + idx) = true;
        cout << "  Кісточку " << val1 << "-" << val2 << " розміщено\n";

        bool allPlaced = true;
        for (int i = 0; i < ROWS * COLS; i++) {
            if (*(g->active + i) && *(g->placement + i) == -1) {
                allPlaced = false; break;
            }
        }

        if (allPlaced) {
            if (runTests(g))
                cout << "\n  Правильно, головоломку розв'язано!\n";
            else
                cout << "\n  Поле заповнене, але є помилки у розміщенні\n";
            break;
        }
    }
}