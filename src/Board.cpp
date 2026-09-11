#include "Board.hpp"

#include <algorithm>
#include <random>
#include <utility>
#include <vector>

Board::Board()
    : randomGenerator_(std::random_device{}())
{
    reset();
}

void Board::reset()
{
    for (auto& row : cells_)
    {
        row.fill(0);
    }

    score_ = 0;
    //cells_[0][0] = 2048;
    addRandomTile();
    addRandomTile();
}

void Board::addRandomTile()
{
    std::vector<Position> emptyCells;

    for (std::size_t row = 0; row < Size; ++row)
    {
        for (std::size_t column = 0; column < Size; ++column)
        {
            if (cells_[row][column] == 0)
            {
                emptyCells.emplace_back(row, column);
            }
        }
    }

    if (emptyCells.empty())
    {
        return;
    }

    std::uniform_int_distribution<std::size_t> cellDistribution(
        0,
        emptyCells.size() - 1
    );

    // 90% вероятность появления 2,
    // 10% вероятность появления 4
    std::bernoulli_distribution valueDistribution(0.9);

    const Position position =
        emptyCells[cellDistribution(randomGenerator_)];

    const auto [row, column] = position;

    cells_[row][column] =
        valueDistribution(randomGenerator_) ? 2 : 4;
}

bool Board::moveTile(Position source, Position destination)
{
    const auto [sourceRow, sourceColumn] = source;
    const auto [destinationRow, destinationColumn] = destination;

    // Проверяем границы игрового поля
    if (sourceRow >= Size ||
        sourceColumn >= Size ||
        destinationRow >= Size ||
        destinationColumn >= Size)
    {
        return false;
    }

    // Вычисляем расстояние между клетками
    const std::size_t rowDistance =
        sourceRow > destinationRow
        ? sourceRow - destinationRow
        : destinationRow - sourceRow;

    const std::size_t columnDistance =
        sourceColumn > destinationColumn
        ? sourceColumn - destinationColumn
        : destinationColumn - sourceColumn;

    // Разрешено перемещение только на одну соседнюю клетку.
    // Это также запрещает движение по диагонали.
    if (rowDistance + columnDistance != 1)
    {
        return false;
    }

    int& sourceValue =
        cells_[sourceRow][sourceColumn];

    int& destinationValue =
        cells_[destinationRow][destinationColumn];

    // Исходная клетка пустая
    if (sourceValue == 0)
    {
        return false;
    }

    // Если впереди находится другая цифра,
    // перемещение запрещено
    if (destinationValue != 0 &&
        destinationValue != sourceValue)
    {
        return false;
    }

    // Если числа одинаковые — объединяем
    if (destinationValue == sourceValue)
    {
        destinationValue *= 2;
        score_ += destinationValue;
    }
    else
    {
        // Иначе перемещаем плитку в пустую клетку
        destinationValue = sourceValue;
    }

    // Освобождаем предыдущую клетку
    sourceValue = 0;

    return true;
}

bool Board::hasWon() const
{
    for (const auto& row : cells_)
    {
        const bool winningTileExists = std::any_of(
            row.begin(),
            row.end(),
            [](int value)
            {
                return value >= 2048;
            }
        );

        if (winningTileExists)
        {
            return true;
        }
    }

    return false;
}

bool Board::canMove() const
{
    for (std::size_t row = 0; row < Size; ++row)
    {
        for (std::size_t column = 0; column < Size; ++column)
        {
            const int currentValue =
                cells_[row][column];

            // Если есть пустая клетка,
            // перемещение ещё возможно
            if (currentValue == 0)
            {
                return true;
            }

            // Проверяем соседнюю клетку справа
            if (column + 1 < Size &&
                cells_[row][column + 1] == currentValue)
            {
                return true;
            }

            // Проверяем соседнюю клетку снизу
            if (row + 1 < Size &&
                cells_[row + 1][column] == currentValue)
            {
                return true;
            }
        }
    }

    return false;
}

int Board::valueAt(Position position) const
{
    const auto [row, column] = position;

    if (row >= Size || column >= Size)
    {
        return 0;
    }

    return cells_[row][column];
}

int Board::score() const noexcept
{
    return score_;
}

const Board::Grid& Board::cells() const noexcept
{
    return cells_;
}
