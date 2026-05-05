/* ----------------------------------------------------------------<Header>-
Name:        task19.cpp
Title:       Domino Puzzle Solver
Group:       TV-52
Student:     Yatsuta Arsen
Written:     05.05.2026
Revised:     05.05.2026
Description: Console program for solving a domino puzzle. The program
             stores the playing field, generates a complete set of
             dominoes from 0-0 to 6-6, checks row and column hints and finds
             a solution using recursive backtracking with pruning.
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
const int ROWS = 11;
const int COLS = 11;
/* ----------------------------------------------------------------------[<]-
Struct: Domino
Synopsis: stores two values of one domino tile.
---------------------------------------------------------------------[>]-*/
struct Domino { int a, b; };
/* ----------------------------------------------------------------------[<]-
Struct: GameData
Synopsis: stores all puzzle data: active cells, current values,
          tile placement indexes, hints and generated domino set.
---------------------------------------------------------------------[>]-*/
struct GameData {
    bool*            active;
    int*             field;
    int*             placement;
    Domino*          dominoes;
    int              dominoCount;
    bool*            used;
    vector<int>*     rowHints;
    vector<int>*     colHints;
    int*             dominoIndexMap;
};

void initData(GameData* g);
void cleanupData(GameData* g);
int  utf8len(const string& s);
void printLine(int width);
void printMenuItem(string text, int width);
void printMenu();
int  safeReadInt(int minVal, int maxVal);
void printField(GameData* g, bool showPlacement);
int  countActiveCells(GameData* g);
int  countUsedDominoes(GameData* g);
void printDominoPositions(GameData* g);
void generateDominoes(GameData* g, int maxVal);
bool isHintAllowed(GameData* g, int r, int c, int val);
bool valueInHints(vector<int>* hints, int val);
bool checkLineHintsStrict(GameData* g, int index, bool row);
bool isTouchAllowed(GameData* g, int r, int c, int val, int dominoIdx);
bool checkAllTouches(GameData* g);
bool canSatisfyLine(GameData* g, int index, bool row);
bool checkHints(GameData* g);
bool canPlaceDomino(GameData* g, int r1, int c1, int r2, int c2, int v1, int v2);
void placeDomino(GameData* g, int r1, int c1, int r2, int c2, int v1, int v2, int idx);
void removeDomino(GameData* g, int r1, int c1, int r2, int c2, int idx);
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
Synopsis: initializes puzzle data and runs the main console menu.
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
            default: break;
        }
    } while (choice != 0);
    cleanupData(&g);
    return 0;
}

/* ----------------------------------------------------------------------[<]-
Function: initData
Synopsis: allocates arrays and fills the initial field shape, hints
          and service structures used by the solver.
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
    bool initActive[ROWS][COLS] = {
        {1,1,1,0,0, 0, 0,1,1,1,0},
        {1,0,1,1,0, 0, 1,1,0,1,1},
        {1,0,0,1,0, 0, 1,0,0,0,1},
        {1,0,0,1,0, 0, 1,0,0,0,0},
        {1,1,1,1,0, 0, 1,0,0,0,0},
        {1,0,0,1,1, 0, 1,0,0,0,0},
        {1,0,0,0,1, 0, 1,0,0,0,0},
        {1,0,0,0,1, 0, 1,0,1,1,1},
        {1,0,0,0,1, 0, 1,0,0,0,1},
        {1,0,0,0,1, 0, 1,1,0,0,1},
        {1,1,1,1,1, 0, 0,1,1,1,1}
    };

    for (int i = 0; i < ROWS * COLS; i++) {
        *(g->active + i)    = initActive[i / COLS][i % COLS];
        *(g->field + i)     = -1;
        *(g->placement + i) = -1;
    }

    for (int i = 0; i < 28; i++) *(g->used + i) = false;
    *(g->rowHints + 0) = {2, 0, 1, 5};
    *(g->rowHints + 6) = {1, 3, 4};
    *(g->rowHints + 9) = {0, 2, 3};
    *(g->colHints + 0)  = {2, 4, 6};
    *(g->colHints + 2)  = {0, 1, 5};
    *(g->colHints + 4)  = {0, 3, 6};
    *(g->colHints + 6)  = {1, 3, 6};
    *(g->colHints + 8)  = {0, 1, 5};
    *(g->colHints + 10) = {1, 2, 3, 4};
}

/* ----------------------------------------------------------------------[<]-
Function: cleanupData
Synopsis: releases all dynamically allocated memory from GameData.
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
Synopsis: returns the visual length of a UTF-8 string for correct
          alignment of Ukrainian menu text.
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
Synopsis: prints a horizontal menu border with a selected width.
---------------------------------------------------------------------[>]-*/
void printLine(int width) {
    cout << "+";
    for (int i = 0; i < width; i++) cout << "-";
    cout << "+\n";
}

/* ----------------------------------------------------------------------[<]-
Function: printMenuItem
Synopsis: prints one formatted menu line inside a text frame.
---------------------------------------------------------------------[>]-*/
void printMenuItem(string text, int width) {
    cout << "| " << text;
    for (int i = utf8len(text); i < width - 1; i++) cout << " ";
    cout << "|\n";
}

/* ----------------------------------------------------------------------[<]-
Function: printMenu
Synopsis: displays the main menu of the console application.
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
Synopsis: reads an integer value and repeats input until it belongs
          to the required range.
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
Synopsis: prints the puzzle field. Empty active cells are shown as #,
          solved cells as numbers, and borders visualize dominoes.
---------------------------------------------------------------------[>]-*/
void printField(GameData* g, bool sp) {
    cout << "\n";
    cout << "  Пояснення: # = активна порожня клітинка, . = неактивне місце, цифра = половинка доміно\n";
    cout << "             між половинками одного доміно межа не друкується.\n\n";
    cout << "       ";
    for (int c = 0; c < COLS; c++) {
        if (c + 1 < 10) cout << "  " << c + 1 << " ";
        else            cout << " " << c + 1 << " ";
    }
    cout << "\n";
    for (int r = 0; r < ROWS; r++) {
        cout << "     ";
        for (int c = 0; c < COLS; c++) {
            bool activeCell = *(g->active + r * COLS + c);
            if (!activeCell) {
                cout << "    ";
                continue;
            }

            bool sameAsTop = false;
            if (sp && r > 0 && *(g->active + (r - 1) * COLS + c) &&
                *(g->placement + r * COLS + c) != -1 &&
                *(g->placement + r * COLS + c) == *(g->placement + (r - 1) * COLS + c)) {
                sameAsTop = true;
            }

            if (sameAsTop) cout << "    ";
            else           cout << "+---";
        }
        cout << "+\n";
        if (r + 1 < 10) cout << "   " << r + 1 << " ";
        else            cout << "  " << r + 1 << " ";
        for (int c = 0; c < COLS; c++) {
            bool activeCell = *(g->active + r * COLS + c);
            if (!activeCell) {
                cout << "    ";
                continue;
            }

            bool sameAsLeft = false;
            if (sp && c > 0 && *(g->active + r * COLS + (c - 1)) &&
                *(g->placement + r * COLS + c) != -1 &&
                *(g->placement + r * COLS + c) == *(g->placement + r * COLS + (c - 1))) {
                sameAsLeft = true;
            }

            if (sameAsLeft) cout << " ";
            else            cout << "|";
            char symbol = '#';
            if (sp) {
                int val = *(g->field + r * COLS + c);
                symbol = (val >= 0 && val <= 9) ? char('0' + val) : '#';
            }
            cout << " " << symbol << " ";
        }
        cout << "|";
        if (!(*(g->rowHints + r)).empty()) {
            cout << "   <";
            for (int h : *(g->rowHints + r)) cout << " " << h;
        }
        cout << "\n";
    }

    cout << "     ";
    for (int c = 0; c < COLS; c++) {
        if (*(g->active + (ROWS - 1) * COLS + c)) cout << "+---";
        else                                      cout << "    ";
    }
    cout << "+\n";
    cout << "       ";
    for (int c = 0; c < COLS; c++) {
        if (!(*(g->colHints + c)).empty()) cout << "  ^ ";
        else                               cout << "    ";
    }
    cout << "\n";
    int maxHintHeight = 0;
    for (int c = 0; c < COLS; c++) {
        if ((int)(*(g->colHints + c)).size() > maxHintHeight)
            maxHintHeight = (*(g->colHints + c)).size();
    }

    for (int line = 0; line < maxHintHeight; line++) {
        cout << "       ";
        for (int c = 0; c < COLS; c++) {
            if (line < (int)(*(g->colHints + c)).size())
                cout << "  " << (*(g->colHints + c))[line] << " ";
            else
                cout << "    ";
        }
        cout << "\n";
    }

    cout << "\n  Активних клітинок: " << countActiveCells(g)
         << " / потрібно 56 для 28 доміно\n";
    if (sp) cout << "  Використано доміно: " << countUsedDominoes(g) << " / 28\n";
}

/* ----------------------------------------------------------------------[<]-
Function: countActiveCells
Synopsis: counts all playable cells of the current field shape.
---------------------------------------------------------------------[>]-*/
int countActiveCells(GameData* g) {
    int cnt = 0;
    for (int i = 0; i < ROWS * COLS; i++) {
        if (*(g->active + i)) cnt++;
    }
    return cnt;
}

/* ----------------------------------------------------------------------[<]-
Function: countUsedDominoes
Synopsis: counts how many domino tiles are currently marked as used.
---------------------------------------------------------------------[>]-*/
int countUsedDominoes(GameData* g) {
    int cnt = 0;
    for (int i = 0; i < 28; i++) {
        if (*(g->used + i)) cnt++;
    }
    return cnt;
}

/* ----------------------------------------------------------------------[<]-
Function: printDominoPositions
Synopsis: prints coordinates of every placed domino tile in the field.
---------------------------------------------------------------------[>]-*/
void printDominoPositions(GameData* g) {
    cout << "\n  Розташування кісточок за координатами:\n";
    printLine(55);
    for (int idx = 0; idx < g->dominoCount; idx++) {
        if (!*(g->used + idx)) continue;
        int r1 = -1, c1 = -1, r2 = -1, c2 = -1;
        for (int pos = 0; pos < ROWS * COLS; pos++) {
            if (*(g->active + pos) && *(g->placement + pos) == idx) {
                if (r1 == -1) { r1 = pos / COLS; c1 = pos % COLS; }
                else          { r2 = pos / COLS; c2 = pos % COLS; }
            }
        }

        string line = "  " + to_string((g->dominoes + idx)->a) + "-" +
                      to_string((g->dominoes + idx)->b) + ": (" +
                      to_string(r1 + 1) + "," + to_string(c1 + 1) + ") і (" +
                      to_string(r2 + 1) + "," + to_string(c2 + 1) + ")";
        printMenuItem(line, 55);
    }
    printLine(55);
}

/* ----------------------------------------------------------------------[<]-
Function: generateDominoes
Synopsis: creates all domino tiles from 0-0 to maxVal-maxVal and fills
          the fast index map for both value orders.
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
Function: valueInHints
Synopsis: checks whether a value is present in the specified hint list.
---------------------------------------------------------------------[>]-*/
bool valueInHints(vector<int>* hints, int val) {
    for (int h : *hints) {
        if (h == val) return true;
    }
    return false;
}

/* ----------------------------------------------------------------------[<]-
Function: isHintAllowed
Synopsis: checks whether a value can be placed in the selected cell
          according to row and column hint restrictions.
---------------------------------------------------------------------[>]-*/
bool isHintAllowed(GameData* g, int r, int c, int val) {
    if (val < 0 || val > 6) return false;
    if (!(*(g->rowHints + r)).empty() &&
        !valueInHints(g->rowHints + r, val)) {
        return false;
    }

    if (!(*(g->colHints + c)).empty() &&
        !valueInHints(g->colHints + c, val)) {
        return false;
    }

    return true;
}

/* ----------------------------------------------------------------------[<]-
Function: isTouchAllowed
Synopsis: verifies that adjacent halves of different dominoes contain
          equal values, as required by the puzzle rules.
---------------------------------------------------------------------[>]-*/
bool isTouchAllowed(GameData* g, int r, int c, int val, int dominoIdx) {
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};
    for (int i = 0; i < 4; i++) {
        int nr = r + dr[i];
        int nc = c + dc[i];
        if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
        if (!*(g->active + nr * COLS + nc)) continue;
        if (*(g->field + nr * COLS + nc) == -1) continue;
        int neighborDomino = *(g->placement + nr * COLS + nc);
        if (neighborDomino == dominoIdx) continue;
        if (*(g->field + nr * COLS + nc) != val) return false;
    }
    return true;
}

/* ----------------------------------------------------------------------[<]-
Function: checkAllTouches
Synopsis: performs final validation of all contacts between neighboring
          cells that belong to different dominoes.
---------------------------------------------------------------------[>]-*/
bool checkAllTouches(GameData* g) {
    int dr[] = {0, 1};
    int dc[] = {1, 0};
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!*(g->active + r * COLS + c)) continue;
            if (*(g->field + r * COLS + c) == -1) continue;
            for (int i = 0; i < 2; i++) {
                int nr = r + dr[i];
                int nc = c + dc[i];
                if (nr < 0 || nr >= ROWS || nc < 0 || nc >= COLS) continue;
                if (!*(g->active + nr * COLS + nc)) continue;
                if (*(g->field + nr * COLS + nc) == -1) continue;
                int curDomino = *(g->placement + r * COLS + c);
                int nextDomino = *(g->placement + nr * COLS + nc);
                if (curDomino != nextDomino &&
                    *(g->field + r * COLS + c) != *(g->field + nr * COLS + nc)) {
                    return false;
                }
            }
        }
    }
    return true;
}

/* ----------------------------------------------------------------------[<]-
Function: checkLineHintsStrict
Synopsis: strictly compares the set of values in a row or column with
          the corresponding hint list, without allowing extra values.
---------------------------------------------------------------------[>]-*/
bool checkLineHintsStrict(GameData* g, int index, bool row) {
    vector<int>* hints = row ? g->rowHints + index : g->colHints + index;
    if (hints->empty()) return true;
    bool present[7] = {false};
    int limit = row ? COLS : ROWS;
    for (int i = 0; i < limit; i++) {
        int r = row ? index : i;
        int c = row ? i : index;
        if (!*(g->active + r * COLS + c)) continue;
        int val = *(g->field + r * COLS + c);
        if (val < 0 || val > 6) return false;
        present[val] = true;
    }

    for (int v = 0; v <= 6; v++) {
        if (present[v] != valueInHints(hints, v)) return false;
    }
    return true;
}

/* ----------------------------------------------------------------------[<]-
Function: canSatisfyLine
Synopsis: checks whether a partially filled row or column can still
          satisfy its hints during recursive search.
---------------------------------------------------------------------[>]-*/
bool canSatisfyLine(GameData* g, int index, bool row) {
    vector<int>* hints = row ? g->rowHints + index : g->colHints + index;
    if (hints->empty()) return true;
    bool present[7] = {false};
    int emptyCells = 0;
    int limit = row ? COLS : ROWS;
    for (int i = 0; i < limit; i++) {
        int r = row ? index : i;
        int c = row ? i : index;
        if (!*(g->active + r * COLS + c)) continue;
        if (*(g->placement + r * COLS + c) == -1) {
            emptyCells++;
            continue;
        }

        int val = *(g->field + r * COLS + c);
        if (val < 0 || val > 6 || !valueInHints(hints, val)) return false;
        present[val] = true;
    }

    int unfulfilled = 0;
    for (int h : *hints) if (!present[h]) unfulfilled++;
    return emptyCells >= unfulfilled;
}

/* ----------------------------------------------------------------------[<]-
Function: checkHints
Synopsis: validates all row and column hints in the final board state.
---------------------------------------------------------------------[>]-*/
bool checkHints(GameData* g) {
    for (int r = 0; r < ROWS; r++) if (!checkLineHintsStrict(g, r, true)) return false;
    for (int c = 0; c < COLS; c++) if (!checkLineHintsStrict(g, c, false)) return false;
    return true;
}

/* ----------------------------------------------------------------------[<]-
Function: canPlaceDomino
Synopsis: checks all placement constraints for one domino: activity,
          adjacency, occupation, uniqueness, hints and touching rules.
---------------------------------------------------------------------[>]-*/
bool canPlaceDomino(GameData* g, int r1, int c1, int r2, int c2, int v1, int v2) {
    if (r1 < 0 || r1 >= ROWS || c1 < 0 || c1 >= COLS) return false;
    if (r2 < 0 || r2 >= ROWS || c2 < 0 || c2 >= COLS) return false;
    if (!*(g->active + r1 * COLS + c1) || !*(g->active + r2 * COLS + c2)) return false;
    if (*(g->placement + r1 * COLS + c1) != -1) return false;
    if (*(g->placement + r2 * COLS + c2) != -1) return false;
    bool adjacent = (r1 == r2 && abs(c1 - c2) == 1) ||
                    (c1 == c2 && abs(r1 - r2) == 1);
    if (!adjacent) return false;
    if (!isHintAllowed(g, r1, c1, v1)) return false;
    if (!isHintAllowed(g, r2, c2, v2)) return false;
    int idx = *(g->dominoIndexMap + v1 * 7 + v2);
    if (*(g->used + idx)) return false;
    if (!isTouchAllowed(g, r1, c1, v1, idx)) return false;
    if (!isTouchAllowed(g, r2, c2, v2, idx)) return false;
    placeDomino(g, r1, c1, r2, c2, v1, v2, idx);
    bool ok = canSatisfyLine(g, r1, true) && canSatisfyLine(g, c1, false) &&
              canSatisfyLine(g, r2, true) && canSatisfyLine(g, c2, false);
    removeDomino(g, r1, c1, r2, c2, idx);
    return ok;
}

/* ----------------------------------------------------------------------[<]-
Function: placeDomino
Synopsis: puts a domino on the field and marks its tile index as used.
---------------------------------------------------------------------[>]-*/
void placeDomino(GameData* g, int r1, int c1, int r2, int c2, int v1, int v2, int idx) {
    *(g->used + idx) = true;
    *(g->placement + r1 * COLS + c1) = idx;
    *(g->placement + r2 * COLS + c2) = idx;
    *(g->field + r1 * COLS + c1) = v1;
    *(g->field + r2 * COLS + c2) = v2;
}

/* ----------------------------------------------------------------------[<]-
Function: removeDomino
Synopsis: removes a previously placed domino and restores search state.
---------------------------------------------------------------------[>]-*/
void removeDomino(GameData* g, int r1, int c1, int r2, int c2, int idx) {
    *(g->used + idx) = false;
    *(g->placement + r1 * COLS + c1) = -1;
    *(g->placement + r2 * COLS + c2) = -1;
    *(g->field + r1 * COLS + c1) = -1;
    *(g->field + r2 * COLS + c2) = -1;
}

/* ----------------------------------------------------------------------[<]-
Function: solve
Synopsis: solves the puzzle using recursive backtracking with MRV-style
          selection of the cell with the smallest number of options.
---------------------------------------------------------------------[>]-*/
bool solve(GameData* g) {
    int bestR = -1, bestC = -1;
    int minOpts = 999999;
    int emptyCount = 0;
    int dr[] = {0, 1, 0, -1};
    int dc[] = {1, 0, -1, 0};
    for (int r = 0; r < ROWS; r++) {
        for (int c = 0; c < COLS; c++) {
            if (!*(g->active + r * COLS + c)) continue;
            if (*(g->placement + r * COLS + c) != -1) continue;
            emptyCount++;
            int opts = 0;
            for (int d = 0; d < 4; d++) {
                int nr = r + dr[d];
                int nc = c + dc[d];
                for (int v1 = 0; v1 <= 6; v1++) {
                    for (int v2 = 0; v2 <= 6; v2++) {
                        if (canPlaceDomino(g, r, c, nr, nc, v1, v2)) opts++;
                    }
                }
            }

            if (opts == 0) return false;
            if (opts < minOpts) {
                minOpts = opts;
                bestR = r;
                bestC = c;
            }
        }
    }

    if (emptyCount == 0) {
        return checkHints(g) && checkAllTouches(g) && countUsedDominoes(g) == 28;
    }

    for (int d = 0; d < 4; d++) {
        int nr = bestR + dr[d];
        int nc = bestC + dc[d];
        for (int v1 = 0; v1 <= 6; v1++) {
            for (int v2 = 0; v2 <= 6; v2++) {
                if (!canPlaceDomino(g, bestR, bestC, nr, nc, v1, v2)) continue;
                int idx = *(g->dominoIndexMap + v1 * 7 + v2);
                placeDomino(g, bestR, bestC, nr, nc, v1, v2, idx);
                if (solve(g)) return true;
                removeDomino(g, bestR, bestC, nr, nc, idx);
            }
        }
    }

    return false;
}

/* ----------------------------------------------------------------------[<]-
Function: resetField
Synopsis: clears all placed values and marks every domino as unused.
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
Synopsis: checks the current solution for field size, coverage, domino
          uniqueness, adjacency, touching rule and exact hints.
---------------------------------------------------------------------[>]-*/
bool runTests(GameData* g) {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("  Перевірка розв'язку", w);
    printLine(w);
    bool shapeValid = countActiveCells(g) == 56;
    cout << "  Кількість активних клітинок........ "
         << (shapeValid ? "ok" : "не пройдено") << "\n";
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
    bool allUsed = countUsedDominoes(g) == 28;
    cout << "  Унікальність кісточок.............. "
         << ((uniqueValid && valuesValid) ? "ok" : "не пройдено") << "\n";
    cout << "  Використано всі 28 кісточок........ "
         << (allUsed ? "ok" : "не пройдено") << "\n";
    cout << "  Сусідство клітинок................. "
         << (adjValid ? "ok" : "не пройдено") << "\n";
    bool touchValid = checkAllTouches(g);
    cout << "  Дотичні половинки різних доміно.... "
         << (touchValid ? "ok" : "не пройдено") << "\n";
    bool rowsOk = true;
    for (int r = 0; r < ROWS; r++) {
        if (!checkLineHintsStrict(g, r, true)) rowsOk = false;
    }
    cout << "  Підказки рядків точно.............. "
         << (rowsOk ? "ok" : "не пройдено") << "\n";
    bool colsOk = true;
    for (int c = 0; c < COLS; c++) {
        if (!checkLineHintsStrict(g, c, false)) colsOk = false;
    }
    cout << "  Підказки колонок точно............. "
         << (colsOk ? "ok" : "не пройдено") << "\n";
    printLine(w);
    if (shapeValid && fullCoverage && uniqueValid && valuesValid && allUsed && adjValid && touchValid && rowsOk && colsOk) {
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
Synopsis: runs diagnostic scenarios that verify acceptance of a correct
          solution and rejection of invalid board states.
---------------------------------------------------------------------[>]-*/
void runDeveloperTests(GameData* g) {
    int w = 55;
    cout << "\n";
    printLine(w);
    printMenuItem("  Інженерне тестування", w);
    printLine(w);
    generateDominoes(g, 6);
    bool allDeveloperTestsPassed = true;
    cout << "\n  Тест 1: правильність форми поля\n";
    bool t0 = countActiveCells(g) == 56;
    if (t0) cout << "  Поле має 56 активних клітинок\n";
    else {
        cout << "  Помилка: кількість активних клітинок не дорівнює 56\n";
        allDeveloperTestsPassed = false;
    }

    cout << "\n  Тест 2: порожнє поле\n";
    resetField(g);
    printField(g, true);
    bool t1 = runTests(g);
    if (!t1) cout << "  Система правильно відхилила порожнє поле\n";
    else {
        cout << "  Помилка: система прийняла порожнє поле\n";
        allDeveloperTestsPassed = false;
    }

    cout << "\n  Тест 3: правильний розв'язок\n";
    resetField(g);
    bool solved = solve(g);
    printField(g, true);
    bool t2 = solved && runTests(g);
    if (t2) cout << "  Система правильно прийняла коректний розв'язок\n";
    else {
        cout << "  Помилка: система не змогла підтвердити правильний розв'язок\n";
        allDeveloperTestsPassed = false;
    }

    cout << "\n  Тест 4: одне число замінено на 9\n";
    int originalVal = *(g->field + 0);
    *(g->field + 0) = 9;
    printField(g, true);
    bool t3 = runTests(g);
    if (!t3) cout << "  Система побачила невірне число\n";
    else {
        cout << "  Помилка: система пропустила невірне число\n";
        allDeveloperTestsPassed = false;
    }
    *(g->field + 0) = originalVal;
    cout << "\n  Тест 5: одна кісточка розірвана на два кути поля\n";
    int orig1 = *(g->placement + 0);
    int orig2 = *(g->placement + (ROWS * COLS - 1));
    *(g->placement + 0) = 5;
    *(g->placement + (ROWS * COLS - 1)) = 5;
    printField(g, true);
    bool t4 = runTests(g);
    if (!t4) cout << "  Система побачила розірване доміно\n";
    else {
        cout << "  Помилка: система не помітила розірване доміно\n";
        allDeveloperTestsPassed = false;
    }
    *(g->placement + 0) = orig1;
    *(g->placement + (ROWS * COLS - 1)) = orig2;
    cout << "\n";
    printLine(w);
    if (allDeveloperTestsPassed)
        printMenuItem("  Усі інженерні тести пройдено", w);
    else
        printMenuItem("  Частина інженерних тестів не пройдена", w);
    printLine(w);
}

/* ----------------------------------------------------------------------[<]-
Function: fillBoardRandomly
Synopsis: recursively fills the field with random valid dominoes for
          generating new playable tasks.
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
                    if (!isTouchAllowed(g, r, c, v1, idx) ||
                        !isTouchAllowed(g, nr, nc, v2, idx)) continue;
                    *(g->used + idx) = true;
                    *(g->placement + r * COLS + c) = idx;
                    *(g->placement + nr * COLS + nc) = idx;
                    *(g->field + r * COLS + c) = v1;
                    *(g->field + nr * COLS + nc) = v2;
                    if (fillBoardRandomly(g)) return true;
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
Synopsis: creates a new puzzle by generating a valid completed board,
          deriving hints from it and clearing the visible solution.
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
Synopsis: resets the board, starts automatic solving, prints the result,
          execution time and final validation report.
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
        printDominoPositions(g);
        cout << "  Час пошуку: " << elapsed.count() << " с\n";
        cout << "  Процесор: Intel(R) Core(TM) i5-9300HF CPU @ 2.40GHz\n";
        cout << "  Пам'ять: 8 ГБ\n";
        runTests(g);
    } else {
        cout << "\n  Розв'язку не знайдено\n";
        cout << "  Час пошуку: " << elapsed.count() << " с\n";
        cout << "  Процесор: Intel(R) Core(TM) i5-9300HF CPU @ 2.40GHz\n";
        cout << "  Пам'ять: 8 ГБ\n";
    }
}

/* ----------------------------------------------------------------------[<]-
Function: userSolve
Synopsis: lets the user place dominoes manually with the same validation
          rules that are used by the automatic solver.
---------------------------------------------------------------------[>]-*/
void userSolve(GameData* g) {
    cout << "\n";
    int w = 55;
    printLine(w);
    printMenuItem("  Режим гравця", w);
    printLine(w);
    cout << "\n  Вводьте координати двох клітинок та значення половинок доміно.\n";
    cout << "  Рядки і колонки вводяться за номерами, які показані над полем.\n";
    cout << "  Введіть 0 у полі рядка першої клітинки, щоб завершити режим гравця.\n\n";
    generateDominoes(g, 6);
    resetField(g);
    while (true) {
        printField(g, true);
        cout << "\n  Рядок першої клітинки (0 = вихід): ";
        int r1 = safeReadInt(0, ROWS);
        if (r1 == 0) {
            cout << "\n  Режим гравця завершено. Поточна перевірка поля:\n";
            runTests(g);
            break;
        }

        cout << "  Колонка першої клітинки: ";
        int c1 = safeReadInt(1, COLS);
        cout << "  Рядок другої клітинки: ";
        int r2 = safeReadInt(1, ROWS);
        cout << "  Колонка другої клітинки: ";
        int c2 = safeReadInt(1, COLS);
        r1--; c1--; r2--; c2--;
        if (!*(g->active + r1 * COLS + c1) || !*(g->active + r2 * COLS + c2)) {
            cout << "  Одна з клітинок неактивна. Обирайте тільки клітинки, які належать фігурі поля.\n";
            continue;
        }

        bool adjacent = (r1 == r2 && abs(c1 - c2) == 1) ||
                        (c1 == c2 && abs(r1 - r2) == 1);
        if (!adjacent) {
            cout << "  Клітинки мають бути сусідніми по горизонталі або вертикалі.\n";
            continue;
        }

        if (*(g->placement + r1 * COLS + c1) != -1 ||
            *(g->placement + r2 * COLS + c2) != -1) {
            cout << "  Одна з клітинок вже зайнята іншою кісточкою.\n";
            continue;
        }

        cout << "  Значення першої половинки (0-6): ";
        int val1 = safeReadInt(0, 6);
        cout << "  Значення другої половинки (0-6): ";
        int val2 = safeReadInt(0, 6);
        int idx = *(g->dominoIndexMap + val1 * 7 + val2);
        if (*(g->used + idx)) {
            cout << "  Кісточка " << val1 << "-" << val2 << " вже використана.\n";
            continue;
        }

        if (!canPlaceDomino(g, r1, c1, r2, c2, val1, val2)) {
            cout << "  Таку кісточку не можна поставити в ці клітинки.\n";
            cout << "  Можливі причини: порушено підказки, правило дотику або проміжну перевірку рядка/колонки.\n";
            continue;
        }

        placeDomino(g, r1, c1, r2, c2, val1, val2, idx);
        cout << "  Кісточку " << val1 << "-" << val2 << " розміщено.\n";
        bool allPlaced = true;
        for (int i = 0; i < ROWS * COLS; i++) {
            if (*(g->active + i) && *(g->placement + i) == -1) {
                allPlaced = false;
                break;
            }
        }

        if (allPlaced) {
            if (runTests(g))
                cout << "\n  Правильно, головоломку розв'язано!\n";
            else
                cout << "\n  Поле заповнене, але є помилки у розміщенні.\n";
            break;
        }
    }
}