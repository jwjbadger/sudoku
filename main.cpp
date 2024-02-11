#include <iostream>
#include <algorithm>
#include <iterator>
#include <random>
#include <ncurses.h>

class Board {
    private:
        int _board[9][9] = {};
    
    public:
        Board() {}

        const int* operator[](const unsigned int r) const {
            return _board[r];
        }

        bool play(int x, int y, int num) {
            _board[y][x] = num > 0 && num < 10 ? num : 0;

            return canMove(y, x, num);
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
            if (num == 0)
                return true;

            int row[9]; 
            int col[9];
            int group[9];

            int oldVal = _board[r][c];
            _board[r][c] = 0;

            std::copy(std::begin(_board[r]), std::end(_board[r]), std::begin(row));

            int g = ((r / 3) * 3 + c / 3);
            for (int j = 0; j < 9; ++j) {
                col[j] = _board[j][c];
                group[j] = _board[(g / 3) * 3 + j / 3][(g % 3) * 3 + j % 3];
            }

            for (int j = 0; j < 9; ++j) {
                if ((row[j] == num) ||
                    (col[j] == num) ||
                    (group[j] == num)) {
                    _board[r][c] = oldVal;
                    return false;
                }
            }

            _board[r][c] = oldVal;
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
            if (!validate())
                return false;
            if (full())
                return true;
            
            return solve(0, 0);
        }

        int unique(int row = 0, int col = 0, int num = 0) {
            if (num > 1)
                return num;

            if (col > 8) {
                if (row == 8)
                    return num + 1;

                ++row;
                col = 0;
            }

            if (_board[row][col] > 0)
                return unique(row, col + 1, num);

            for (int i = 1; i < 10; ++i) {
                if (canMove(row, col, i)) {
                    _board[row][col] = i;

                    num = unique(row, col + 1, num);

                    _board[row][col] = 0;

                    if (num > 1)
                        return num;
                }   
            }

            return num;
        }

        void generate() {            
            int *shuffledBoard[81] = {};

            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    shuffledBoard[i * 9 + j] = &_board[i][j]; 
                }
            }

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(std::begin(shuffledBoard), std::end(shuffledBoard), g);

            int options[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            for (int i = 0; i < 81; ++i) {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(std::begin(options), std::end(options), g);

                for (int k = 0; k < 9; ++k) {
                   if (canMove((shuffledBoard[i] - &(_board[0][0])) / 9, (shuffledBoard[i] - &(_board[0][0])) % 9, options[k])) {
                        *(shuffledBoard[i]) = options[k];
                        
                        int numSolutions = unique();
                        
                        if (numSolutions > 1)
                            break;
                        else if (numSolutions == 1)
                            return;
                        else
                            *(shuffledBoard[i]) = 0;
                    }
                }
            }
        }

        operator const char*() const {
            static char ch[253] = "";

            for (int i = 1; i < 12; ++i) {
                for (int j = 1; j < 12; ++j) {
                    if (i != 0 && i % 4 == 0) {
                        if (j != 0 && j % 4 == 0)
                            ch[(i - 1) * 23 + (j - 1) * 2] = '+';
                        else
                            ch[(i - 1) * 23 + (j - 1) * 2] = '-';
                        ch[(i - 1) * 23 + (j - 1) * 2 + 1] = j == 11 ? ' ' : '-';
                    } else {
                        if (j != 0 && j % 4 == 0) {
                            ch[(i - 1) * 23 + (j - 1) * 2] = '|';
                            ch[((i - 1) * 23) + ((j - 1) * 2) + 1] = ' ';
                        } else {
                            ch[(i - 1) * 23 + (j - 1) * 2] = '0' + _board[i - (i / 4) - 1][j - (j / 4) - 1];
                            ch[((i - 1) * 23) + ((j - 1) * 2) + 1] = ' ';
                        }
                    }
                }
                
                if (i < 11)
                    ch[(i - 1) * 23 + 22] = '\n';
            }
            
            ch[252] = '\0';

            return ch;
        }
};

void drawBoard(Board board) {
    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            move(i + (i / 3), j * 2 + (j / 3) * 2);

            if (!board.canMove(i, j, board[i][j])) {
                attron(COLOR_PAIR(1));
                printw("%i", board[i][j]);
                attroff(COLOR_PAIR(1));
            } else {
                printw("%i", board[i][j]);
            }
        }
    }

    move(0, 0);
    refresh();
}

void drawStats(char status, char mode) {
    switch (status) {
        case 'i':
            mvprintw(0, 34, "%s", "User Input        ");
            break;
        case 'u':
            mvprintw(0, 34, "%s", "User Solving      ");
            break;
        case 'g':
            mvprintw(0, 34, "%s", "Generating...     ");
            break;
        case 's':
            mvprintw(0, 34, "%s", "Solving...        ");
            break;
        case 'd':
            attron(COLOR_PAIR(2));
            mvprintw(0, 34, "%s", "DONE              ");
            attroff(COLOR_PAIR(2));
            break;
        case 'f':
            attron(COLOR_PAIR(1));
            mvprintw(0, 34, "%s", "FAILED TO SOLVE   ");
            attroff(COLOR_PAIR(1));
            break;
        default:
            mvprintw(0, 34, "%s", "ERROR             ");
            break;
    }

    switch (mode) {
        case 'c':
            mvprintw(1, 32, "%s", "Command           ");
            break;
        case 'e':
            mvprintw(1, 32, "%s", "Edit              ");
            break;
        default:
            mvprintw(1, 32, "%s", "ERROR             ");
            break;
    }

    move(0, 0);
    refresh();
}

int main() {
    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();

    Board board;

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    char mode = 'c';
    char status = 'i';

    mvprintw(0, 0, "%s", static_cast<const char *>(board));

    attron(COLOR_PAIR(3));
    mvprintw(0, 26, "Status: ");
    mvprintw(1, 26, "Mode: ");
    attroff(COLOR_PAIR(3));

    drawStats(status, mode);

    move(0, 0);

    for (int ch = getch(); ch != 'q'; ch = getch()) {
        switch (ch) {
            case 259: // Arrow Up
                if (getcury(stdscr) > 0) {
                    move(getcury(stdscr) - 1, getcurx(stdscr));
                    if ((getcury(stdscr) + 1) % 4 == 0 && getcury(stdscr) > 0)
                        move(getcury(stdscr) - 1, getcurx(stdscr));
                } else {
                    move(10, getcurx(stdscr));
                }
                break;
            case 261: // Arrow Right
                if (getcurx(stdscr) < 20) {
                    move(getcury(stdscr), getcurx(stdscr) + 2);
                    if ((getcurx(stdscr) + 2) % 8 == 0 && getcurx(stdscr) > 0)
                        move(getcury(stdscr), getcurx(stdscr) + 2);
                } else {
                    move(getcury(stdscr), 0);
                }
                break;
            case 258: // Arrow Down
                if (getcury(stdscr) < 10) {
                    move(getcury(stdscr) + 1, getcurx(stdscr));
                    if ((getcury(stdscr) + 1) % 4 == 0 && getcury(stdscr) > 0)
                        move(getcury(stdscr) + 1, getcurx(stdscr));
                } else {
                    move(0, getcurx(stdscr));
                }
                break;
            case 260: // Arrow Left
                if (getcurx(stdscr) > 0) {
                    move(getcury(stdscr), getcurx(stdscr) - 2);
                    if ((getcurx(stdscr) + 2) % 8 == 0 && getcurx(stdscr) > 0)
                        move(getcury(stdscr), getcurx(stdscr) - 2);
                } else {
                    move(getcury(stdscr), 20);
                }
                break;
            case 10: // Enter
                if (mode == 'c') {
                    curs_set(2);
                    mode = 'e';
                } else if (mode == 'e') {
                    curs_set(1);
                    mode = 'c';
                }

                drawStats(status, mode);
                break;
            case 103: // G
                if (mode == 'c') {
                    status = 'g';
                    drawStats(status, mode);

                    board.generate();

                    if (board.unique() == 1)
                        status = 'u';
                    else
                        status = 'E';

                    drawBoard(board);
                    drawStats(status, mode);

                    move(0, 0);
                }

                break;
            case 115: // S
                if (mode == 'c') {
                    status = 's';
                    drawStats(status, mode);

                    board.solve();

                    if (board.full() && board.validate())
                        status = 'd';
                    else
                        status = 'f';
                    drawStats(status, mode);
                    drawBoard(board);
                }
                break;
            default: // Handle numbers
                if ('0' <= ch && '9' >= ch && getcurx(stdscr) % 2 == 0 && mode == 'e' && getcurx(stdscr) <= 20 && getcury(stdscr) <= 10) {
                    board.play((getcurx(stdscr) / 2) - ((getcurx(stdscr) / 2) / 4), getcury(stdscr) - (getcury(stdscr) / 4), ch - '0');
                    
                    int x = 0, y = 0;

                    if (getcurx(stdscr) < 20) {
                        x = getcurx(stdscr) + 2;
                        if ((x + 2) % 8 == 0 && x > 0)
                            x += 2;
                        y = getcury(stdscr);
                    } else if (getcury(stdscr) < 10) {
                        y = getcury(stdscr) + 1;
                        if ((y + 1) % 4 == 0 && y > 0)
                            ++y; 
                    }

                    drawBoard(board);

                    move(y, x);
                }
                break;
        }
    }
}
