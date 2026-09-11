#include "Game.hpp"

#include <fmt/color.h>
#include <fmt/core.h>

#include <cctype>
#include <cstdio>

#ifdef _WIN32
#include <conio.h>
#include <windows.h>
#else
#include <termios.h>
#include <unistd.h>
#endif

void Game::run()
{
    prepareConsole();

    while (true)
    {
        display();

        if (board_.hasWon())
        {
            fmt::print(
                "\nПоздравляем! Вы получили плитку 2048!\n"
            );
            break;
        }

        if (!board_.canMove())
        {
            fmt::print(
                "\nИгра окончена. Возможных ходов больше нет.\n"
            );
            break;
        }

        const Command command = readCommand();

        if (command == Command::Quit)
        {
            break;
        }

        handleCommand(command);
    }
}

void Game::display() const
{
    clearConsole();

    fmt::print("\nИгра 2048\n");
    fmt::print("Счёт: {}\n\n", board_.score());

    fmt::print(
        "W/A/S/D или стрелки — передвинуть курсор или захваченную плитку\n"
    );
    fmt::print(
        "ENTER — захватить или отпустить плитку\n"
    );
    fmt::print(
        "ESC/C — отменить выбор, Q — выйти\n"
    );
    fmt::print(
        "КРАСНЫЙ — курсор или захваченная плитка\n\n"
    );

    fmt::print("Статус: {}\n\n", statusMessage_);

    constexpr const char* Border =
        "+------+------+------+------+\n";

    fmt::print("{}", Border);

    for (std::size_t row = 0; row < Board::Size; ++row)
    {
        for (std::size_t column = 0;
             column < Board::Size;
             ++column)
        {
            const Board::Position position{
                row,
                column
            };

            const int value =
                board_.valueAt(position);

            const bool isCursor =
                position == cursor_;

            fmt::print("|");

            if (isCursor)
            {
                const auto style =
                    fmt::bg(fmt::color::red) |
                    fmt::fg(fmt::color::white) |
                    fmt::emphasis::bold;

                if (value == 0)
                {
                    fmt::print(style, "{:^6}", ".");
                }
                else
                {
                    // Выбранная цифра отображается
                    // непосредственно в красной клетке
                    fmt::print(style, "{:^6}", value);
                }
            }
            else if (value == 0)
            {
                fmt::print("{:^6}", ".");
            }
            else
            {
                fmt::print("{:^6}", value);
            }
        }

        fmt::print("|\n{}", Border);
    }

    std::fflush(stdout);
}

Game::Command Game::readCommand() const
{
#ifdef _WIN32

    while (true)
    {
        int key = _getch();

        // Стрелки Windows
        if (key == 0 || key == 224)
        {
            key = _getch();

            switch (key)
            {
            case 72:
                return Command::Up;

            case 80:
                return Command::Down;

            case 75:
                return Command::Left;

            case 77:
                return Command::Right;

            default:
                continue;
            }
        }

        switch (std::tolower(
            static_cast<unsigned char>(key)))
        {
        case 'w':
            return Command::Up;

        case 's':
            return Command::Down;

        case 'a':
            return Command::Left;

        case 'd':
            return Command::Right;

        case 'e':
        case 13:
            return Command::Select;

        case 'c':
        case 27:
            return Command::Cancel;

        case 'q':
            return Command::Quit;

        default:
            break;
        }
    }

#else

    termios originalSettings{};

    if (tcgetattr(STDIN_FILENO, &originalSettings) != 0)
    {
        return Command::Quit;
    }

    termios rawSettings = originalSettings;

    rawSettings.c_lflag &=
        static_cast<tcflag_t>(~(ICANON | ECHO));

    // read() ожидает символ не более 0,1 секунды
    rawSettings.c_cc[VMIN] = 0;
    rawSettings.c_cc[VTIME] = 1;

    if (tcsetattr(
        STDIN_FILENO,
        TCSANOW,
        &rawSettings) != 0)
    {
        return Command::Quit;
    }

    const auto finishCommand =
        [&originalSettings](Command command)
    {
        tcsetattr(
            STDIN_FILENO,
            TCSANOW,
            &originalSettings
        );

        return command;
    };

    while (true)
    {
        unsigned char key = 0;

        const ssize_t result =
            read(STDIN_FILENO, &key, 1);

        if (result < 0)
        {
            return finishCommand(Command::Quit);
        }

        if (result == 0)
        {
            continue;
        }

        // Стрелки Linux/WSL передаются как:
        // ESC [ A, ESC [ B, ESC [ C, ESC [ D
        if (key == 27)
        {
            unsigned char secondKey = 0;
            unsigned char thirdKey = 0;

            const ssize_t secondResult =
                read(STDIN_FILENO, &secondKey, 1);

            // Одиночное нажатие Escape
            if (secondResult <= 0)
            {
                return finishCommand(Command::Cancel);
            }

            if (secondKey != '[')
            {
                return finishCommand(Command::Cancel);
            }

            const ssize_t thirdResult =
                read(STDIN_FILENO, &thirdKey, 1);

            if (thirdResult <= 0)
            {
                return finishCommand(Command::Cancel);
            }

            switch (thirdKey)
            {
            case 'A':
                return finishCommand(Command::Up);

            case 'B':
                return finishCommand(Command::Down);

            case 'C':
                return finishCommand(Command::Right);

            case 'D':
                return finishCommand(Command::Left);

            default:
                continue;
            }
        }

        switch (std::tolower(key))
        {
        case 'w':
            return finishCommand(Command::Up);

        case 's':
            return finishCommand(Command::Down);

        case 'a':
            return finishCommand(Command::Left);

        case 'd':
            return finishCommand(Command::Right);

        case 'e':
        case '\r':
        case '\n':
            return finishCommand(Command::Select);

        case 'c':
            return finishCommand(Command::Cancel);

        case 'q':
            return finishCommand(Command::Quit);

        default:
            break;
        }
    }

#endif
}

void Game::handleCommand(Command command)
{
    switch (command)
    {
    case Command::Up:
    case Command::Down:
    case Command::Left:
    case Command::Right:
        moveCursor(command);
        break;

    case Command::Select:
        selectOrPlaceTile();
        break;

    case Command::Cancel:
    {
        const bool wasMoved =
            selectedTile_.has_value() &&
            tileMovedDuringSelection_;

        if (wasMoved)
        {
            board_.addRandomTile();
        }

        selectedTile_.reset();
        tileMovedDuringSelection_ = false;

        statusMessage_ = wasMoved
            ? "Перенос закончен. Добавлена новая плитка."
            : "Выбор плитки отменён.";

        break;
    }

    case Command::Quit:
        break;
    }
}

void Game::moveCursor(Command command)
{
    auto [row, column] = cursor_;

    switch (command)
    {
    case Command::Up:
        if (row > 0)
        {
            --row;
        }
        break;

    case Command::Down:
        if (row + 1 < Board::Size)
        {
            ++row;
        }
        break;

    case Command::Left:
        if (column > 0)
        {
            --column;
        }
        break;

    case Command::Right:
        if (column + 1 < Board::Size)
        {
            ++column;
        }
        break;

    default:
        return;
    }

    const Board::Position newPosition{
        row,
        column
    };

    if (newPosition == cursor_)
    {
        statusMessage_ =
            "Движение невозможно: край игрового поля.";
        return;
    }

    // Плитка ещё не захвачена:
    // свободно перемещаем красный курсор
    if (!selectedTile_.has_value())
    {
        cursor_ = newPosition;

        statusMessage_ =
            "Выберите плитку и нажмите Enter.";

        return;
    }

    // Плитка захвачена:
    // теперь стрелки перемещают саму цифру
    const int sourceValue =
        board_.valueAt(*selectedTile_);

    const int destinationValue =
        board_.valueAt(newPosition);

    const bool isMerge =
        sourceValue != 0 &&
        sourceValue == destinationValue;

    // moveTile разрешает движение только
    // на одну соседнюю клетку
    if (!board_.moveTile(
        *selectedTile_,
        newPosition))
    {
        // Красный курсор остаётся на выбранной цифре
        statusMessage_ =
            "Проход закрыт: впереди находится другая плитка.";

        return;
    }

    // Плитка и красный курсор переходят вместе
    cursor_ = newPosition;
    tileMovedDuringSelection_ = true;

    if (isMerge)
    {
        selectedTile_.reset();
        tileMovedDuringSelection_ = false;

        board_.addRandomTile();

        statusMessage_ =
            "Плитки объединены. Добавлена новая плитка.";

        return;
    }

    // Сохраняем новое положение захваченной плитки
    selectedTile_ = newPosition;

    statusMessage_ =
        "Плитка передвинута на одну клетку. "
        "Продолжайте движение или нажмите Enter.";
}

void Game::selectOrPlaceTile()
{
    // Захватываем плитку
    if (!selectedTile_.has_value())
    {
        const int value =
            board_.valueAt(cursor_);

        if (value == 0)
        {
            statusMessage_ =
                "Эта клетка пустая. Выберите клетку с числом.";

            return;
        }

        selectedTile_ = cursor_;
        tileMovedDuringSelection_ = false;

        statusMessage_ =
            "Плитка захвачена. Теперь стрелки двигают саму цифру.";

        return;
    }

    // Повторный Enter отпускает плитку
    const bool wasMoved =
        tileMovedDuringSelection_;

    if (wasMoved)
    {
        board_.addRandomTile();
    }

    selectedTile_.reset();
    tileMovedDuringSelection_ = false;

    statusMessage_ = wasMoved
        ? "Плитка установлена. Добавлена новая плитка."
        : "Выбор плитки отменён.";
}

void Game::prepareConsole()
{
#ifdef _WIN32

    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);

    const HANDLE output =
        GetStdHandle(STD_OUTPUT_HANDLE);

    if (output == INVALID_HANDLE_VALUE)
    {
        return;
    }

    DWORD mode = 0;

    if (GetConsoleMode(output, &mode) != 0)
    {
        SetConsoleMode(
            output,
            mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING
        );
    }

#endif
}

void Game::clearConsole()
{
#ifdef _WIN32

    const HANDLE output =
        GetStdHandle(STD_OUTPUT_HANDLE);

    CONSOLE_SCREEN_BUFFER_INFO bufferInfo{};

    if (output == INVALID_HANDLE_VALUE ||
        GetConsoleScreenBufferInfo(
            output,
            &bufferInfo) == 0)
    {
        return;
    }

    const DWORD cellCount =
        static_cast<DWORD>(bufferInfo.dwSize.X) *
        static_cast<DWORD>(bufferInfo.dwSize.Y);

    const COORD homePosition{0, 0};

    DWORD written = 0;

    FillConsoleOutputCharacterA(
        output,
        ' ',
        cellCount,
        homePosition,
        &written
    );

    FillConsoleOutputAttribute(
        output,
        bufferInfo.wAttributes,
        cellCount,
        homePosition,
        &written
    );

    SetConsoleCursorPosition(
        output,
        homePosition
    );

#else

    fmt::print("\x1B[2J\x1B[3J\x1B[H");
    std::fflush(stdout);

#endif
}
