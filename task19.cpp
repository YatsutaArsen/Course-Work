#include <iostream>
#include <string>
using namespace std;

//День 1: визначились з розмірами поля
// Поле 7 рядків х 6 колонок - під літери B та G
const int ROWS = 7;
const int COLS = 6;

// День 1: вирішили зберігати поле в динамічній пам'яті (згідно з вимогою курсової роботи)
bool* active;

// День1: прототип, будемо одразу дотримуватись такої структури коду
void initData();
void cleanupData();
void printField();

int main() {
    initData();
    printField();
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
        *(active + i * COLS + j) =
initActive[i][j];
}

void cleanupData() {
    delete[] active;
}

// День 1: найпростіше відображення: #, де активна клітинка
// TODO: потім додати підказки рядків і колонок
// TODO: потім додати відображення значень кісточок

void printField() {
    cout << "\n";

    for (int i = 0; i < ROWS; i ++) {
        cout << " |";
        for (int j = 0; j < COLS; j++) {
            // День 1: поки просто #, числа з'являться пізніше
            if (*(active + i * COLS +j)) cout << "#|";
            else cout << " |";
        }
        cout << "\n +";
        for (int j = 0; j < COLS; j++) cout << "---+";
        cout << "\n";
    }
}