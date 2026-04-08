#include <iostream>

void printWelcomeMessage(const char* message);

int main () {
    const char* text = "Початок виконання курсової роботи";
    printWelcomeMessage(text);

    return 0;
}

void printWelcomeMessage(const char* message) {
    if (message != nullptr) {
        std::cout << message << std::endl;
    }
}