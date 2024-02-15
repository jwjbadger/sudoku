#include <iostream>
#include <algorithm>
#include <iterator>
#include <random>
#include <fstream>
#include <string>
#define _XOPEN_SOURCE_EXTENDED 1
#include "ncurses.h"
#include <locale.h>


class Board {
    private:
        int _board[9][9] = {};
        int *_fixed = new int[0];
        int _numFixed = 0;
    
    public:
        Board() {}

        Board(const Board &copy) : _numFixed(copy._numFixed) {
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    _board[i][j] = copy[i][j];
            
            _fixed = new int[_numFixed];
            for (int i = 0; i < _numFixed; ++i)
                _fixed[i] = copy._fixed[i]; 
        }

        Board(std::string serialized) {
            for (int i = 0; i < 81; ++i) {
                _board[i / 9][i % 9] = serialized[i] - '0';
            }
        }

        ~Board() {
            delete[] _fixed;
        }

        const int* operator[](const unsigned int r) const {
            return _board[r];
        }

        bool fixed(int r, int c) {
            if (std::find(_fixed, _fixed + _numFixed, r * 9 + c) != _fixed + _numFixed)
                return true;
            return false;
        }

        bool fix() {
            if (!validate() || unique() < 1)
                return false;

            delete[] _fixed;

            _numFixed = count();
            _fixed = new int[_numFixed];

            int index = 0;
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    if (_board[i][j] != 0) {
                        _fixed[index] = i * 9 + j;
                        ++index;
                    }
                }
            }

            return true;
        }

        bool play(int r, int c, int num) {
            if (fixed(r, c))
                return false;
                
            _board[r][c] = num > 0 && num < 10 ? num : 0;

            return canMove(r, c, num);
        }

        bool full() {
            return count() == 81;
        }

        int count() {
            int count = 0;

            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    if (_board[i][j] > 0 && _board[i][j] < 10)
                        ++count; 

            return count;
        }

        void rotate(int times) {
            if (times == 0)
                return;

            Board old = *this;

            switch (times) {
                case 1:
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[j][8 - i];
                    break;
                case 2:
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[8 - i][8 - j];
                    break;
                case 3:
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[8 - j][i];
                    break;
            }
        }

        void reflect(int axis) {
            if (axis == 0)
                return;

            Board old = *this;

            switch (axis) {
                case 1:
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[8 - i][j];
                    break;
                case 2:
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[i][8 - j];
                    break;
                case 3:
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[8 - i][8 - j];
                    break;
            }
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
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    if (_board[i][j] > 0 && !fixed(i, j))
                        _board[i][j] = 0;

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

        void generate(std::string file = "seeds.dat") {
            std::ifstream binary_file(file, std::ios::in | std::ios::binary);

            std::random_device dev;
            std::mt19937 rng(dev());

            std::string line, result;
            for(std::size_t i = 0; std::getline(binary_file, line); ++i) {
                std::uniform_int_distribution<> dist(0, i);
                if (dist(rng) < 1)
                    result = line;
            }
            binary_file.close();

            int options[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            std::shuffle(std::begin(options) + 1, std::end(options), rng);
            for (int i = 0; i < 81; ++i)
                _board[i / 9][i % 9] = options[result[i] - '0'];

            std::uniform_int_distribution<> dist(0, 3);
            rotate(dist(rng));
            reflect(dist(rng));

            fix();
        }

        static Board generateSeed() {     
            Board board;

            int *shuffledBoard[81] = {};

            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    shuffledBoard[i * 9 + j] = &board._board[i][j]; 

            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(std::begin(shuffledBoard), std::end(shuffledBoard), g);

            int options[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            for (int i = 0; i < 81; ++i) {
                std::random_device rd;
                std::mt19937 g(rd());
                std::shuffle(std::begin(options), std::end(options), g);

                for (int k = 0; k < 9; ++k) {
                   if (board.canMove((shuffledBoard[i] - &(board._board[0][0])) / 9, (shuffledBoard[i] - &(board._board[0][0])) % 9, options[k])) {
                        *(shuffledBoard[i]) = options[k];
                        
                        int numSolutions = board.unique();
                        
                        if (numSolutions == 1) {
                            board.fix();
                            return board;
                        } else if (numSolutions > 1) {
                            break;
                        } else {
                            *(shuffledBoard[i]) = 0;
                        }
                    }
                }
            }

            board.fix();
            return board; 
        }

        friend std::ostream& operator<<(std::ostream& os, const Board& board) {
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    os << board._board[i][j];
                }
            }

            return os;
        }

        operator const wchar_t*() const {
            static wchar_t ch[253] = L"";

            for (int i = 1; i < 12; ++i) {
                for (int j = 1; j < 12; ++j) {
                    if (i != 0 && i % 4 == 0) {
                        if (j != 0 && j % 4 == 0)
                            ch[(i - 1) * 23 + (j - 1) * 2] = L'┼';
                        else
                            ch[(i - 1) * 23 + (j - 1) * 2] = L'─';
                        ch[(i - 1) * 23 + (j - 1) * 2 + 1] = j == 11 ? L' ' : L'─';
                    } else {
                        if (j != 0 && j % 4 == 0) {
                            ch[(i - 1) * 23 + (j - 1) * 2] = L'│';
                            ch[((i - 1) * 23) + ((j - 1) * 2) + 1] = L' ';
                        } else {
                            ch[(i - 1) * 23 + (j - 1) * 2] = L'0' + _board[i - (i / 4) - 1][j - (j / 4) - 1];
                            ch[((i - 1) * 23) + ((j - 1) * 2) + 1] = L' ';
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
    int x = getcurx(stdscr), y = getcury(stdscr);

    for (int i = 0; i < 9; ++i) {
        for (int j = 0; j < 9; ++j) {
            move(i + (i / 3), j * 2 + (j / 3) * 2);
            
            if (board.fixed(i, j)) {
                attron(COLOR_PAIR(3));
                printw("%i", board[i][j]);
                attroff(COLOR_PAIR(3));
            } else if (!board.canMove(i, j, board[i][j])) {
                attron(COLOR_PAIR(1));
                printw("%i", board[i][j]);
                attroff(COLOR_PAIR(1));
            } else {
                printw("%i", board[i][j]);
            }
        }
    }

    move(y, x);
    refresh();
}

void drawStats(char status, char mode) {
    int x = getcurx(stdscr), y = getcury(stdscr);

    mvprintw(0, 50, "                    ");

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

    move(y, x);
    refresh();
}

int main(int argc, char **argv) {
    std::string seedsFile = "seeds.dat";

    if (argc > 1) {
        if (std::find(argv, argv + argc, std::string("-h")) != argv + argc) {
            std::cout << "usage: sudoku [options]" << std::endl;
            std::cout << "options:" << std::endl;
            std::cout << "  sudoku -g [num]         | generates [num] seeds for sudoku puzzles (100 by default) and exports to seeds.dat" << std::endl;
            std::cout << "  sudoku -t [file]        | tests [file] for seeds with unique solutions" << std::endl;
            std::cout << "  sudoku -s [file]        | sets the source for seeds to be [file] (seeds.dat by default)" << std::endl;
            return 0;
        }

        char **g = std::find(argv, argv + argc, std::string("-g"));
        if (g != argv + argc) {
            setlocale(LC_ALL, "");

            initscr();
            noecho();
            cbreak();

            int num = (g + 1) == argv + argc ? 100 : std::stoi(*(g + 1));

            mvaddstr(0, 0, "Generating seeds for sudoku puzzles:");
            mvprintw(1, 0, "[                    ] (0/%i)", num);
            refresh();

            std::ofstream binary_file("seeds.dat", std::ios::out | std::ios::binary | std::ios::app);
            for (int i = 0; i < num; ++i) {
                Board board = Board::generateSeed();

                binary_file << board << std::endl;

                for (int j = 0; j < ((20 * (i + 1)) / num); ++j)
                    mvaddwstr(1, j + 1, L"▓");
                mvprintw(1, 23, "(%i/%i)", i + 1, num);
                refresh();
            }
            binary_file.close();

            endwin();
            return 0;
        }

        char **t = std::find(argv, argv + argc, std::string("-t"));
        if (t != argv + argc) {
            std::ifstream binary_file(*(t + 1), std::ios::in | std::ios::binary);

            std::string line;
            while (std::getline(binary_file, line)) {
                Board board(line);
                if (board.unique() != 1) {
                    binary_file.close();
                    std::cout << "FAILED" << std::endl;
                    return 1;
                }
            }

            binary_file.close();
            std::cout << "PASSED" << std::endl;
            return 0;
        }

        char **s = std::find(argv, argv + argc, std::string("-s"));
        if (s != argv + argc) {
            seedsFile = *(s + 1);
        }
    }

    setlocale(LC_ALL, "");
    setlocale(LC_NUMERIC,"C");

    initscr();
    keypad(stdscr, TRUE);
    noecho();
    cbreak();

    mousemask(ALL_MOUSE_EVENTS, NULL);
    MEVENT event;

    start_color();
    init_pair(1, COLOR_RED, COLOR_BLACK);
    init_pair(2, COLOR_GREEN, COLOR_BLACK);
    init_pair(3, COLOR_BLUE, COLOR_BLACK);

    Board board;

    char mode = 'c';
    char status = 'i';

    mvprintw(0, 0, "%ls", static_cast<const wchar_t *>(board));

    attron(COLOR_PAIR(3));
    mvprintw(0, 26, "Status: ");
    mvprintw(1, 26, "Mode: ");
    mvprintw(3, 26, "Keybinds:");
    mvprintw(4, 30, "Movement: ");
    mvprintw(5, 30, "Switch edit/command mode: ");
    mvprintw(6, 30, "Generate: ");
    mvprintw(7, 30, "Finish Initial Input: ");
    mvprintw(8, 30, "Solve: ");
    mvprintw(9, 30, "Input: ");
    mvprintw(10, 30, "Hint: ");
    attroff(COLOR_PAIR(3));

    mvprintw(4, 40, "Arrow Keys");
    mvprintw(5, 56, "Enter");
    mvprintw(6, 40, "G");
    mvprintw(7, 52, "F");
    mvprintw(8, 37, "S");
    mvprintw(9, 37, "Backspace | [0-9]");
    mvprintw(10, 36, "H");

    drawStats(status, mode);
    move(0, 0);

    for (int ch = getch(); ch != 'q'; ch = getch()) {
        switch (ch) {
            case KEY_MOUSE:
                if (getmouse(&event) != OK)
                    break;

                if (event.bstate & BUTTON1_CLICKED) {
                    if ((event.y <= 10 && event.x <= 20) && (event.y == 0 || (event.y + 1) % 4 != 0) && (event.x == 0 || ((event.x + 2) % 8 != 0 && event.x % 2 == 0)))
                        move(event.y, event.x);
                }
                break;
            case KEY_UP:
                if (getcury(stdscr) > 0) {
                    move(getcury(stdscr) - 1, getcurx(stdscr));
                    if ((getcury(stdscr) + 1) % 4 == 0 && getcury(stdscr) > 0)
                        move(getcury(stdscr) - 1, getcurx(stdscr));
                } else {
                    move(10, getcurx(stdscr));
                }
                break;
            case KEY_RIGHT:
                if (getcurx(stdscr) < 20) {
                    move(getcury(stdscr), getcurx(stdscr) + 2);
                    if ((getcurx(stdscr) + 2) % 8 == 0 && getcurx(stdscr) > 0)
                        move(getcury(stdscr), getcurx(stdscr) + 2);
                } else {
                    move(getcury(stdscr), 0);
                }
                break;
            case KEY_DOWN:
                if (getcury(stdscr) < 10) {
                    move(getcury(stdscr) + 1, getcurx(stdscr));
                    if ((getcury(stdscr) + 1) % 4 == 0 && getcury(stdscr) > 0)
                        move(getcury(stdscr) + 1, getcurx(stdscr));
                } else {
                    move(0, getcurx(stdscr));
                }
                break;
            case KEY_LEFT:
                if (getcurx(stdscr) > 0) {
                    move(getcury(stdscr), getcurx(stdscr) - 2);
                    if ((getcurx(stdscr) + 2) % 8 == 0 && getcurx(stdscr) > 0)
                        move(getcury(stdscr), getcurx(stdscr) - 2);
                } else {
                    move(getcury(stdscr), 20);
                }
                break;
            case '\n':
            case KEY_ENTER: 
                if (mode == 'c') {
                    curs_set(2);
                    mode = 'e';
                    status = 'u';
                } else if (mode == 'e') {
                    curs_set(1);
                    mode = 'c';
                    if (board.validate() && board.full())
                        status = 'd';
                }

                drawStats(status, mode);
                break;
            case 'f':
                if (mode != 'c')
                    break;

                if (!board.fix())
                    break;

                status = 'u';

                drawBoard(board);
                drawStats(status, mode);

                break;
            case 'g':
                if (mode != 'c')
                    break;

                status = 'g';
                drawStats(status, mode);

                board.generate(seedsFile);

                if (board.unique() == 1)
                    status = 'u';
                else
                    status = 'E';

                drawBoard(board);
                drawStats(status, mode);

                move(0, 0);

                break;
            case 's':
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
            case 'h':
                if (mode == 'c') {
                    if (status == 'd')
                        break;

                    int x = getcurx(stdscr), y = getcury(stdscr);

                    attron(COLOR_PAIR(3));
                    mvprintw(0, 50, "Hint: ");
                    attroff(COLOR_PAIR(3));

                    if (board.unique() < 1) {
                        attron(COLOR_PAIR(1));
                        mvprintw(0, 56, "NOT SOLVABLE");
                        attroff(COLOR_PAIR(1));

                        move(y, x);
                        break;
                    }

                    Board solution = board;
                    solution.fix();
                    solution.solve();

                    std::random_device dev;
                    std::mt19937 rng(dev());
                    std::uniform_int_distribution<> dist(0, 80);

                    int index = dist(rng);
                    while (board[index / 9][index % 9] != 0) {
                        if (index < 80)
                            ++index;
                        else
                            index = 0;
                    }

                    board.play(index / 9, index % 9, solution[index / 9][index % 9]);
                    drawBoard(board);

                    attron(COLOR_PAIR(2));
                    mvprintw((index / 9) + ((index / 9) / 3), ((index % 9) + ((index % 9) / 3)) * 2, "%i", board[index / 9][index % 9]);
                    attroff(COLOR_PAIR(2));

                    attron(COLOR_PAIR(2));
                    mvprintw(0, 56, "SOLVABLE");
                    attroff(COLOR_PAIR(2));

                    if (board.validate() && board.full()) {
                        status = 'd';
                        drawStats(status, mode);
                    }

                    move(y, x);
                }
                break;
            default: // Handle numbers
                if ((ch == 127 || ch == KEY_BACKSPACE || ch == 8 || ('0' <= ch && '9' >= ch)) && getcurx(stdscr) % 2 == 0 && mode == 'e' && getcurx(stdscr) <= 20 && getcury(stdscr) <= 10) {
                    int x = getcurx(stdscr), y = getcury(stdscr);

                    board.play(y - (y / 4), (x / 2) - ((x / 2) / 4), '0' <= ch && '9' >= ch ? ch - '0' : 0);

                    if (ch <= '9' && ch > '0' && board.canMove(y - (y / 4), (x / 2) - ((x / 2) / 4), ch - '0')) {
                        do {
                            if (((x + 2) % 8 == 6) && (y + 1) % 4 == 3) {
                                if (x == 20) {
                                    if (y < 10)
                                        y += 2;
                                    else
                                        y = 0;
                                    x = 0;
                                } else {
                                    x += 4;
                                    y -= 2;
                                }
                            } else if ((x + 2) % 8 < 6) {
                                x += 2;
                            } else {
                                x -= 4;
                                ++y;
                            }
                        } while ((board.full() || board[y - (y / 4)][(x / 2) - ((x / 2) / 4)] != 0) && board.fixed(y - (y / 4), (x / 2) - ((x / 2) / 4)));
                    }

                    drawBoard(board);

                    if (board.validate() && board.full())
                        status = 'd';
                    drawStats(status, mode);

                    move(y, x);
                }
                break;
        }
    }

    endwin();
    return 0;
}