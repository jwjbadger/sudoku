// CHANGE THAt
#include <iostream>
#include <algorithm>
#include <iterator>

class Board {
    private:
        int _board[9][9] = {
                {9, 5, 0, 6, 7, 0, 3, 0, 0},
                {1, 0, 6, 3, 0, 0, 0, 9, 0},
                {0, 0, 2, 0, 0, 0, 1, 0, 7},
                {0, 4, 3, 0, 1, 0, 0, 0, 0},
                {2, 0, 8, 0, 9, 0, 4, 0, 6},
                {0, 0, 0, 0, 3, 0, 8, 1, 0},
                {4, 0, 5, 0, 0, 0, 9, 0, 0},
                {0, 6, 0, 0, 0, 9, 2, 0, 5},
                {0, 0, 9, 0, 5, 4, 0, 8, 3}
            };
    
    public:
        Board() {}

        bool validate() {
            for (int i = 0; i < 9; ++i) {
                int row[9]; 
                int col[9];
                int group[9];

                std::copy(std::begin(_board[i]), std::end(_board[i]), std::begin(row));

                for (int j = 0; j < 9; ++j) {
                    col[j] = _board[j][i];
                    group[j] = _board[(i / 3) * 3 + j / 3][(i % 3) * 3 + j % 3];
                }

                std::sort(std::begin(row), std::end(row));
                std::sort(std::begin(col), std::end(col));
                std::sort(std::begin(group), std::end(group));

                for (int j = 1; j < 9; ++j)
                    if ((row[j] != 0 && row[j] == row[j - 1]) ||
                        (col[j] != 0 && col[j] == col[j - 1]) ||
                        (group[j] != 0 && group[j] == group[j - 1]))
                        return false;
            } 

            return true;
        }

        bool canMove(int r, int c, int num) {
            int row[9]; 
            int col[9];
            int group[9];

            std::copy(std::begin(_board[r]), std::end(_board[r]), std::begin(row));

            int g = ((r / 3) * 3 + c / 3);
            for (int j = 0; j < 9; ++j) {
                col[j] = _board[j][c];
                group[j] = _board[(g / 3) * 3 + j / 3][(g % 3) * 3 + j % 3];
            }

            for (int j = 0; j < 9; ++j)
                if ((row[j] == num) ||
                    (col[j] == num) ||
                    (group[j] == num))
                    return false;

            return true;
        }

        friend std::ostream& operator<<(std::ostream& os, const Board& yourfriendlyneighbordhoodspiderman) {
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    os << yourfriendlyneighbordhoodspiderman._board[i][j] << " ";
                }
                os << std::endl;
            }

            return os;
        }
};

int main() {
    Board board;
    std::cout << board << std::endl;
    std::cout << board.validate() << std::endl; // true

    std::cout << board.canMove(0, 2, 2) << std::endl; // false
    std::cout << board.canMove(4, 1, 1) << std::endl; // true
    std::cout << board.canMove(8, 6, 8) << std::endl; // false
}
