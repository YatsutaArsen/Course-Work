#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <algorithm>
#include <ctime>
using namespace std;

const int ROWS = 7;
const int COLS = 6;

struct Domino { int a, b; };

struct GameData {
    bool* active;
    int* field;
    int* placement;
    Domino* dominoes;
    int dominoCount;
    bool* used;
    vector<int>* rowHints;
    vector<int>* colHints;
    int* dominoIndexMap;
};

void initData(GameData* g);
void cleanupData(GameData* g);
int utf8len(const string& s);
void printLine(int width);
void printMenuItem(string text, int width);
void printMenu();
int safeReadInt(int minVal, int maxVal);
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

int main() {
    srand(time(0));
    GameData g;
    initData(&g);

    int choice = 0;
    do {
        printMenu();
        choice = safeReadInt(0, 4);
        switch (choice) {
            case 1: autoSolve(&g); break;
            case 2: userSolve(&g); break;
            case 3: printField(&g, false); break;
            case 4: runDeveloperTests(&g); break;
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

void initData(GameData* g) {
    g->active = new bool[ROWS * COLS];
    g->field = new int[ROWS * COLS];
    g->placement = new int[ROWS * COLS];
    g->dominoes = new Domino[28];
    g->dominoCount = 0;
    g->used = new bool[28];
    g->rowHints = new vector<int>[ROWS];
    g->colHints = new vector<int>[COLS];
    g->dominoIndexMap = new int[7 * 7];

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
        *(g->active + i) = initActive[i / COLS][i % COLS];
        *(g->field + i) = -1;
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

int utf8len(const string& s) {
    int len = 0;
    for (int i = 0; i < (int)s.length(); ) {
        unsigned char c = s[i];
        if (c < 0x80) i += 1;
        else if (c < 0xE0) i += 2;
        else i += 3;
        len++;
    }
    return len;
}

void printLine(int width) {
    cout << "+";
    for (int i = 0; i < width; i++) cout << "-";
    cout << "+\n";
}

void printMenuItem(string text, int width) {
    cout << "| " << text;
    for (int i = utf8len(text); i < width - 1; i++) cout << " ";
    cout << "|\n";
}

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
    printMenuItem("  [0] Вийти", w);
    printLine(w);
    cout << "  Ваш вибір: ";
}

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

bool checkHints(GameData* g) {
    for (int i = 0; i < ROWS; i++) if (!canSatisfyRow(g, i)) return false;
    for (int j = 0; j < COLS; j++) if (!canSatisfyCol(g, j)) return false;
    return true;
}

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
                if (opts == 0) return false;
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
                        *(g->used + idx) = true;
                        *(g->placement + r * COLS + c) = idx;
                        *(g->placement + nr * COLS + nc) = idx;
                        *(g->field + r * COLS + c) = v1;
                        *(g->field + nr * COLS + nc) = v2;

                        if (canSatisfyRow(g, r) && canSatisfyCol(g, c) &&
                            canSatisfyRow(g, nr) && canSatisfyCol(g, nc)) {
                            if (solve(g)) return true;
                        }

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

void resetField(GameData* g) {
    for (int i = 0; i < ROWS * COLS; i++) {
        *(g->placement + i) = -1;
        *(g->field + i) = -1;
    }
    for (int i = 0; i < 28; i++) *(g->used + i) = false;
}

bool runTests(GameData* g) {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("  перевірка розв'язку", w);
    printLine(w);

    bool fullCoverage = true;
    for (int i = 0; i < ROWS * COLS; i++) {
        if (*(g->active + i) && *(g->placement + i) == -1) fullCoverage = false;
    }
    cout << "  заповненість поля.................. "
         << (fullCoverage ? "ok" : "не пройдено") << "\n";
    if (!fullCoverage) {
        printLine(w);
        printMenuItem("  поле не заповнене повністю", w);
        printLine(w);
        return false;
    }

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
                    else { r2 = pos / COLS; c2 = pos % COLS; }
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
    cout << "  унікальність кісточок.............. "
         << ((uniqueValid && valuesValid) ? "ok" : "не пройдено") << "\n";
    cout << "  сусідство клітинок................. "
         << (adjValid ? "ok" : "не пройдено") << "\n";

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
    cout << "  підказки рядків.................... "
         << (rowsOk ? "ok" : "не пройдено") << "\n";

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
    cout << "  підказки колонок................... "
         << (colsOk ? "ok" : "не пройдено") << "\n";

    printLine(w);
    if (fullCoverage && uniqueValid && adjValid && valuesValid && rowsOk && colsOk) {
        printMenuItem("  розв'язок правильний", w);
        printLine(w);
        return true;
    } else {
        printMenuItem("  у розв'язку є помилки", w);
        printLine(w);
        return false;
    }
}

void runDeveloperTests(GameData* g) {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("  інженерне тестування", w);
    printLine(w);

    generateDominoes(g, 6);

    cout << "\n  тест 1: порожнє поле\n";
    resetField(g);
    printField(g, true);
    bool t1 = runTests(g);
    if (!t1) cout << "  система правильно відхилила порожнє поле\n";
    else cout << "  помилка: система прийняла порожнє поле\n";

    cout << "\n  тест 2: правильний розв'язок\n";
    resetField(g);
    solve(g);
    printField(g, true);
    bool t2 = runTests(g);
    if (t2) cout << "  система правильно прийняла коректний розв'язок\n";
    else cout << "  помилка: система відхилила правильний розв'язок\n";

    cout << "\n  тест 3: одне число замінено на 9\n";
    int originalVal = *(g->field + 0);
    *(g->field + 0) = 9;
    printField(g, true);
    bool t3 = runTests(g);
    if (!t3) cout << "  система побачила невірне число\n";
    else cout << "  помилка: система пропустила невірне число\n";
    *(g->field + 0) = originalVal;

    cout << "\n  тест 4: одна кісточка розірвана на два кути поля\n";
    int orig1 = *(g->placement + 0);
    int orig2 = *(g->placement + (ROWS * COLS - 1));
    *(g->placement + 0) = 5;
    *(g->placement + (ROWS * COLS - 1)) = 5;
    printField(g, true);
    bool t4 = runTests(g);
    if (!t4) cout << "  система побачила розірване доміно\n";
    else cout << "  помилка: система не помітила розірване доміно\n";
    *(g->placement + 0) = orig1;
    *(g->placement + (ROWS * COLS - 1)) = orig2;

    cout << "\n";
    printLine(w);
    printMenuItem("  усі модулі захисту працюють коректно", w);
    printLine(w);
}

void autoSolve(GameData* g) {
    generateDominoes(g, 6);
    resetField(g);

    cout << "\n  шукаю розв'язок...\n";

    auto start = chrono::high_resolution_clock::now();
    bool success = solve(g);
    auto end = chrono::high_resolution_clock::now();
    chrono::duration<double> elapsed = end - start;

    if (success) {
        cout << "  розв'язок знайдено\n";
        int w = 55;
        printLine(w);
        printMenuItem("  розв'язок", w);
        printLine(w);
        printField(g, true);
        cout << "\n  використані кісточки:\n";
        printLine(w);
        for (int i = 0; i < g->dominoCount; i++) {
            if (*(g->used + i)) {
                string line = "  " + to_string((g->dominoes + i)->a) +
                              "-" + to_string((g->dominoes + i)->b) + "  [+]";
                printMenuItem(line, w);
            }
        }
        printLine(w);
        cout << "  час пошуку: " << elapsed.count() << " с\n";
        runTests(g);
    } else {
        cout << "\n  розв'язку не знайдено\n";
        cout << "  час пошуку: " << elapsed.count() << " с\n";
    }
}

void userSolve(GameData* g) {
    cout << "\n";
    int w = 55;
    printLine(w);
    printMenuItem("  режим гравця", w);
    printLine(w);
    cout << "\n  вводьте координати двох клітинок та значення\n";
    cout << "  введіть 0 щоб завершити\n\n";

    generateDominoes(g, 6);
    resetField(g);
    bool userUsed[28] = {};

    while (true) {
        printField(g, true);

        cout << "\n  рядок першої клітинки (0 = вихід): ";
        int r1 = safeReadInt(0, ROWS);
        if (r1 == 0) break;

        cout << "  колонка першої клітинки: ";
        int c1 = safeReadInt(1, COLS);

        cout << "  рядок другої клітинки: ";
        int r2 = safeReadInt(1, ROWS);

        cout << "  колонка другої клітинки: ";
        int c2 = safeReadInt(1, COLS);
        r1--; c1--; r2--; c2--;

        if (!*(g->active + r1 * COLS + c1) || !*(g->active + r2 * COLS + c2)) {
            cout << "  одна з клітинок неактивна\n";
            continue;
        }
        bool adjacent = (r1 == r2 && abs(c1 - c2) == 1) ||
                        (c1 == c2 && abs(r1 - r2) == 1);
        if (!adjacent) {
            cout << "  клітинки мають бути сусідніми\n";
            continue;
        }
        if (*(g->placement + r1 * COLS + c1) != -1 ||
            *(g->placement + r2 * COLS + c2) != -1) {
            cout << "  одна з клітинок вже зайнята\n";
            continue;
        }

        cout << "  значення першої половинки (0-6): ";
        int val1 = safeReadInt(0, 6);
        cout << "  значення другої половинки (0-6): ";
        int val2 = safeReadInt(0, 6);

        int idx = *(g->dominoIndexMap + val1 * 7 + val2);
        if (*(userUsed + idx)) {
            cout << "  кісточка " << val1 << "-" << val2 << " вже використана\n";
            continue;
        }

        *(g->placement + r1 * COLS + c1) = idx;
        *(g->placement + r2 * COLS + c2) = idx;
        *(g->field + r1 * COLS + c1) = val1;
        *(g->field + r2 * COLS + c2) = val2;
        *(userUsed + idx) = true;
        cout << "  кісточку " << val1 << "-" << val2 << " розміщено\n";

        bool allPlaced = true;
        for (int i = 0; i < ROWS * COLS; i++) {
            if (*(g->active + i) && *(g->placement + i) == -1) {
                allPlaced = false; break;
            }
        }

        if (allPlaced) {
            if (runTests(g))
                cout << "\n  правильно, головоломку розв'язано!\n";
            else
                cout << "\n  поле заповнене, але є помилки у розміщенні\n";
            break;
        }
    }
}