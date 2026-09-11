#pragma once

#include "Board.hpp"

#include <optional>
#include <string>

class Game
{
public:
    void run();

private:
    enum class Command
    {
        Up,
        Down,
        Left,
        Right,
        Select,
        Cancel,
        Quit
    };

    Board board_;

    // Положение красного курсора
    Board::Position cursor_{0, 0};

    // Положение захваченной плитки
    std::optional<Board::Position> selectedTile_;

    // Показывает, перемещалась ли захваченная плитка
    bool tileMovedDuringSelection_{false};

    // Сообщение, которое выводится над игровым полем
    std::string statusMessage_{
        "Наведите красный курсор на плитку и нажмите Enter."
    };

    // Отображение игрового поля
    void display() const;

    // Чтение команды с клавиатуры
    [[nodiscard]] Command readCommand() const;

    // Обработка введённой команды
    void handleCommand(Command command);

    // Перемещение курсора или захваченной плитки
    void moveCursor(Command command);

    // Захват или отпускание плитки
    void selectOrPlaceTile();

    // Настройка консоли
    static void prepareConsole();

    // Очистка консоли перед перерисовкой
    static void clearConsole();
};