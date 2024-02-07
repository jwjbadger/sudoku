// CHANGE THAt
#include <iostream>
#include <algorithm>
#include <iterator>
#include <ncurses.h>

class Board {
    private:
        // int _board[9][9] = {
        //         {9, 5, 0, 6, 7, 0, 3, 0, 0},
        //         {1, 0, 6, 3, 0, 0, 0, 9, 0},
        //         {0, 0, 2, 0, 0, 0, 1, 0, 7},
        //         {0, 4, 3, 0, 1, 0, 0, 0, 0},
        //         {2, 0, 8, 0, 9, 0, 4, 0, 6},
        //         {0, 0, 0, 0, 3, 0, 8, 1, 0},
        //         {4, 0, 5, 0, 0, 0, 9, 0, 0},
        //         {0, 6, 0, 0, 0, 9, 2, 0, 5},
        //         {0, 0, 9, 0, 5, 4, 0, 8, 3}
        //     };
        // int _board[9][9] = {
        //         {8, 0, 0, 0, 0, 0, 0, 0, 0},
        //         {0, 0, 3, 6, 0, 0, 0, 0, 0},
        //         {0, 7, 0, 0, 9, 0, 2, 0, 0},
        //         {0, 5, 0, 0, 0, 7, 0, 0, 0},
        //         {0, 0, 0, 0, 4, 5, 7, 0, 0},
        //         {0, 0, 0, 1, 0, 0, 0, 3, 0},
        //         {0, 0, 1, 0, 0, 0, 0, 6, 8},
        //         {0, 0, 8, 5, 0, 0, 0, 1, 0},
        //         {0, 9, 0, 0, 0, 0, 4, 0, 0}
        //     };
        int _board[9][9] = {
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0},
            {0, 0, 0, 0, 0, 0, 0, 0, 0}
        };
    
    public:
        Board() {}

        bool play(int x, int y, int num) {
            bool valid = canMove(y, x, num);
            
            _board[y][x] = num;

            return valid;
        }

        bool full() {
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    if (_board[i][j] < 1 || _board[i][j] > 9)
                        return false;

            return true;
        }

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

        bool solve(int row, int col) {            
            if (col > 8) {
                if (row == 8)
                    return true;

                ++row;
                col = 0;
            }

            if (_board[row][col] > 0)
                return solve(row, col + 1);

            for (int i = 1; i < 10; ++i) {
                if (canMove(row, col, i)) {
                    _board[row][col] = i;

                    if (solve(row, col + 1))
                        return true;

                    _board[row][col] = 0;
                }   
            }

            return false;
        }

        bool solve() {
            return solve(0, 0);
        }

        friend std::ostream& operator<<(std::ostream& os, const Board& board) {
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    os << board._board[i][j] << " ";
                }
                os << std::endl;
            }

            return os;
        }

        operator const char*() const {
            static char ch[163] = "";

            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    ch[i * 18 + j * 2] = '0' + _board[i][j];
                    ch[(i * 18) + (j * 2) + 1] = ' ';
                }

                if (i < 8)
                    ch[i * 18 + 17] = '\n';
            }
            
            ch[161] = '\0';

            return ch;
        }
};

int main() {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();

    Board board;
    printw("%s", static_cast<const char*>(board));
    move(0, 0);

    start_color();
    init_pair(1, COLOR_WHITE, COLOR_RED);
    init_pair(2, COLOR_WHITE, COLOR_GREEN);

    char mode = 'c';

    for (int ch = getch(); ch != 'q'; ch = getch()) {
        switch (ch) {
            case 259:
                if (getcury(stdscr) > 0)
                    move(getcury(stdscr) - 1, getcurx(stdscr));
                else
                    move(8, getcurx(stdscr));
                break;
            case 261:
                if (getcurx(stdscr) < 16)
                    move(getcury(stdscr), getcurx(stdscr) + 2);
                else
                    move(getcury(stdscr), 0);
                break;
            case 258:
                if (getcury(stdscr) < 8)
                    move(getcury(stdscr) + 1, getcurx(stdscr));
                else
                    move(0, getcurx(stdscr));
                break;
            case 260:
                if (getcurx(stdscr) > 0)
                    move(getcury(stdscr), getcurx(stdscr) - 2);
                else
                    move(getcury(stdscr), 16);
                break;
            case 10:
                if (mode == 'c') {
                    curs_set(2);
                    mode = 'e';
                } else if (mode == 'e') {
                    curs_set(1);
                    mode = 'c';
                }
                refresh();
                break;
            case 115:
                if (mode == 'c') {
                    board.solve();

                    move(0, 0);

                    attron(COLOR_PAIR(2));
                    printw("%s", static_cast<const char *>(board));
                    attroff(COLOR_PAIR(2));

                    move(0, 0);
                }
                break;
            default:
                if ('0' <= ch && '9' >= ch && getcurx(stdscr) % 2 == 0 && mode == 'e' && getcurx(stdscr) <= 16 && getcury(stdscr) <= 8) {
                    if (!board.play(getcurx(stdscr) / 2, getcury(stdscr), ch - '0'))
                        attron(COLOR_PAIR(1));

                    printw("%c", ch);

                    if (getcurx(stdscr) < 16)
                        move(getcury(stdscr), getcurx(stdscr) + 1);
                    else if (getcury(stdscr) < 8)
                        move(getcury(stdscr) + 1, 0);
                    else
                        move(0, 0);

                    attroff(COLOR_PAIR(1));

                    if (board.full() && board.validate()) {
                        clear();

                        attron(COLOR_PAIR(2));
                        printw("%s", static_cast<const char*>(board));
                        attroff(COLOR_PAIR(2));

                        move(0, 0);
                    }
                }
                break;
        }
    }
}
