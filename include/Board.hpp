#pragma once

#include <array>
#include <cstddef>
#include <random>
#include <utility>

class Board
{
public:
    static constexpr std::size_t Size = 4;
    using Grid = std::array<std::array<int, Size>, Size>;
    using Position = std::pair<std::size_t, std::size_t>;

    Board();

    void reset();
    void addRandomTile();
    bool moveTile(Position source, Position destination);

    [[nodiscard]] bool hasWon() const;
    [[nodiscard]] bool canMove() const;
    [[nodiscard]] int valueAt(Position position) const;
    [[nodiscard]] int score() const noexcept;
    [[nodiscard]] const Grid& cells() const noexcept;

private:
    Grid cells_{};
    int score_{0};
    std::mt19937 randomGenerator_;
};
