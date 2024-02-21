#include <iostream>
#include <algorithm>
#include <iterator>
#include <random>
#include <fstream>
#include <string>
#include <locale.h>

// Apple uses an older version of NCurses, so you must #define a macro to be 1 for support
#ifdef __APPLE__
    #define _XOPEN_SOURCE_EXTENDED 1
#endif
#include <ncurses.h>

// Makes colors and status more clear to reference (e.g. Colors::Bad is the same as 1)
enum Colors { Bad = 1, Good = 2, Fixed = 3 };
enum Status { UserInput, UserSolve, Generate, Solve, Solved, Error };

// Define the Board and related methods for solving, testing unique, etc.
class Board {
    private:
        int _board[9][9] = {}; // The board itself represented as a 2d Array
        int *_fixed = new int[0]; // A list of indexes to the elements of the array which are fixed and cannot be edited
        int _numFixed = 0; // The number of fixed elements (the sized of _fixed)
    
    public:
        // Default: do nothing
        Board() {}
        
        // Copy constructor: necessary to prevent double free of _fixed
        Board(const Board &copy) : _numFixed(copy._numFixed) {
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    _board[i][j] = copy[i][j];
        
            // create a new _fixed and copy the elements of the original into it
            _fixed = new int[_numFixed]; 
            for (int i = 0; i < _numFixed; ++i)
                _fixed[i] = copy._fixed[i]; 
        }

        // Construct a board from a string (deserialize it)
        Board(std::string serialized) {
            for (int i = 0; i < 81; ++i)
                _board[i / 9][i % 9] = serialized[i] - '0';
        }

        // Delete _fixed so we don't leak memory
        ~Board() {
            delete[] _fixed;
        }

        // Overload operator[] to allow for easy indexing of the array (returns a constant; you must modify the board via `play()` not using direct modification)
        const int* operator[](const unsigned int r) const {
            return _board[r];
        }

        // Taking in a row and a column, returns a boolean describing if the element at that position is fixed
        bool fixed(int r, int c) {
            if (std::find(_fixed, _fixed + _numFixed, r * 9 + c) != _fixed + _numFixed)
                return true;

            return false;
        }

        // Fix the board as it is (prevent editing of non-zero elements in the future)
        bool fix() {
            // Don't fix a board if it's not valid or has no solution
            if (!validate() || unique() < 1)
                return false;

            // Get rid of old _fixed
            delete[] _fixed;

            // Set the number of fixed to the number of non-zero elements and create a new array to store this
            _numFixed = count();
            _fixed = new int[_numFixed];

            // loop through and add all of the elements to be fixed to _fixed
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

        // Play a move if it's not overlapping a fixed piece 
        bool play(int r, int c, int num) {
            if (fixed(r, c))
                return false;
                
            _board[r][c] = num > 0 && num < 10 ? num : 0;

            return canMove(r, c, num);
        }

        // retun true if the number of non-zero numbers is 81 (a full board)
        bool full() {
            return count() == 81;
        }

        // Count the number of non-zero numbers in the board
        int count() {
            int count = 0;

            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    if (_board[i][j] > 0 && _board[i][j] < 10)
                        ++count; 

            return count;
        }

        // Rotate a board 0, 1, 2, or 3 times
        void rotate(int times) {
            if (times == 0)
                return;

            Board old = *this; // Calls copy constructor

            switch (times) { // Using equations for transformations: 90 deg: (x, y) -> (y, -x); 180 deg: (x, y) -> (-x, -y); 270 deg: (x, y) -> (-y, x)
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

        // Reflect across axis 0, 1, 2, or 3 (no reflection, y-axis, x-axis, both
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

        // Validate a board by checking if any numbers repeat per row, column, and group
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

        // Check more efficiently if moving to a specific row and column is valid
        bool canMove(int r, int c, int num) {
            if (num == 0) // Can always change to 0
                return true;

            int row[9]; 
            int col[9];
            int group[9];

            int oldVal = _board[r][c]; // To reset the board afterward
            _board[r][c] = 0; // We don't want any current values at the location messing up the calculations

            std::copy(std::begin(_board[r]), std::end(_board[r]), std::begin(row));

            int g = ((r / 3) * 3 + c / 3);
            for (int j = 0; j < 9; ++j) {
                col[j] = _board[j][c];
                group[j] = _board[(g / 3) * 3 + j / 3][(g % 3) * 3 + j % 3];
            }

            // Check now if anything in the row, column, or group is equal to the inputted number
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

        // Recursively solve via smart backtracking using a row and column to solve from
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

        // Wrap the underlying solve function to only solve valid boards
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

        // Returns the number of solutions by recursively finding them; returns 0, 1, or 2 (2 simply means there are at least 2 solutions)
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

        // Generate a new board using the seeds file
        void generate(std::string file = "seeds.dat") {
            std::ifstream binary_file(file, std::ios::in | std::ios::binary); // Open the file

            // To generate better random numbers
            std::random_device dev;
            std::mt19937 rng(dev());

            // Run through and for each line make the result equal to the current line if a random number from 0 to the number of lines is less than one (ensures a result always gets picked)
            std::string line, result;
            for(std::size_t i = 0; std::getline(binary_file, line); ++i) {
                std::uniform_int_distribution<> dist(0, i);
                if (dist(rng) < 1)
                    result = line;
            }
            binary_file.close(); // Close file

            // Shuffle around which number is which
            int options[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
            std::shuffle(std::begin(options) + 1, std::end(options), rng); // don't shuffle 0
            for (int i = 0; i < 81; ++i)
                _board[i / 9][i % 9] = options[result[i] - '0'];

            // Rotate and reflect the board
            std::uniform_int_distribution<> dist(0, 3);
            rotate(dist(rng));
            reflect(dist(rng));

            // Fix it to complete the function
            fix();
        }

        // Static method to generate a seed for a puzzle because generation times are so long (generates a board) 
        static Board generateSeed() {     
            Board board;

            // Create a list of pointers to individual elements of the board
            int *shuffledBoard[81] = {};
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    shuffledBoard[i * 9 + j] = &board._board[i][j]; 

            // Randomly shuffle that list of pointers
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(std::begin(shuffledBoard), std::end(shuffledBoard), g);

            // Create a list of options to move so that they can be shuffled to make the algorithm somewhat random
            int options[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            // Iterate through all cells
            for (int i = 0; i < 81; ++i) {
                // Shuffle the list of options
                std::shuffle(std::begin(options), std::end(options), g);

                // Iterate through all the options for the selected element
                for (int k = 0; k < 9; ++k) {
                    // If you can move to the current cell with the kth option to move, do so 
                    if (board.canMove((shuffledBoard[i] - &(board._board[0][0])) / 9, (shuffledBoard[i] - &(board._board[0][0])) % 9, options[k])) {
                        *(shuffledBoard[i]) = options[k];
                        
                        // Get the number of solutions remaining
                        int numSolutions = board.unique();
                        
                        if (numSolutions == 1) { // We are finished if there is 1 and only 1 solution
                            board.fix();
                            return board;
                        } else if (numSolutions > 1) { // If there is more than one, let's keep going
                            break;
                        } else {
                            *(shuffledBoard[i]) = 0; // If there are less than one, we messed up
                        }
                    }
                }
            }

            // Fix the board and return (this code shouldn't really ever be ran because it implies we filled the entire board or something went wrong)
            board.fix();
            return board; 
        }

        // Serialize the board using handy operator<< notation; simply put every element one after another with no spacing or formatting
        friend std::ostream& operator<<(std::ostream& os, const Board& board) {
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    os << board._board[i][j];

            return os;
        }
};

class Game {
    private:
        Board _board;
        std::string _seeds = "seeds.dat";
        MEVENT _event;
        int _status = Status::UserInput;

    public:
        Game(std::string seeds = "seeds.dat") : _seeds(seeds) {
            setlocale(LC_ALL, "");
            setlocale(LC_NUMERIC,"C");

            initscr();
            keypad(stdscr, TRUE);
            noecho();
            cbreak();

            mousemask(ALL_MOUSE_EVENTS, NULL);

            start_color();
            init_pair(Colors::Bad, COLOR_RED, COLOR_BLACK);
            init_pair(Colors::Good, COLOR_GREEN, COLOR_BLACK);
            init_pair(Colors::Fixed, COLOR_BLUE, COLOR_BLACK);

            initDisplay();
            updateTUI();

            move(0, 0);
        }

        ~Game() {
            endwin();
        }

        void setCursor(int r, int c) {
            move(r + (r / 3), c * 2 + (c / 3) * 2);
        }

        void updateTUI() {
            int x = getcurx(stdscr), y = getcury(stdscr);

            mvprintw(0, 50, "                  ");

            switch (_status) {
                case Status::UserInput:
                    mvprintw(0, 34, "%s", "User Input        ");
                    break;
                case Status::UserSolve:
                    mvprintw(0, 34, "%s", "User Solving      ");
                    break;
                case Status::Generate:
                    mvprintw(0, 34, "%s", "Generating...     ");
                    break;
                case Status::Solve:
                    mvprintw(0, 34, "%s", "Solving...        ");
                    break;
                case Status::Solved:
                    attron(COLOR_PAIR(Colors::Good));
                    mvprintw(0, 34, "%s", "DONE              ");
                    attroff(COLOR_PAIR(Colors::Good));
                    break;
                case Status::Error:
                    attron(COLOR_PAIR(Colors::Bad));
                    mvprintw(0, 34, "%s", "ERROR             ");
                    attroff(COLOR_PAIR(Colors::Bad));
                    break;
                default:
                    attron(COLOR_PAIR(Colors::Bad));
                    mvprintw(0, 34, "%s", "INVALID STATUS    ");
                    attroff(COLOR_PAIR(Colors::Bad));
            }

            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    setCursor(i, j);
                    
                    if (_board.fixed(i, j)) {
                        attron(COLOR_PAIR(Colors::Fixed));
                        printw("%i", _board[i][j]);
                        attroff(COLOR_PAIR(Colors::Fixed));
                    } else if (!_board.canMove(i, j, _board[i][j])) {
                        attron(COLOR_PAIR(Colors::Bad));
                        printw("%i", _board[i][j]);
                        attroff(COLOR_PAIR(Colors::Bad));
                    } else {
                        printw("%i", _board[i][j]);
                    }
                }
            }

            move(y, x);
            refresh();
        }

        void initDisplay() {
            attron(COLOR_PAIR(Colors::Fixed));
            mvprintw(0, 26, "Status: ");
            mvprintw(2, 26, "Keybinds:");
            mvprintw(3, 30, "Movement: ");
            mvprintw(4, 30, "Generate: ");
            mvprintw(5, 30, "Finish Initial Input: ");
            mvprintw(6, 30, "Solve: ");
            mvprintw(7, 30, "Input: ");
            mvprintw(8, 30, "Hint: ");
            attroff(COLOR_PAIR(Colors::Fixed));

            mvprintw(3, 40, "Arrow Keys");
            mvprintw(4, 40, "G");
            mvprintw(5, 52, "F");
            mvprintw(6, 37, "S");
            mvprintw(7, 37, "Backspace | [0-9]");
            mvprintw(8, 36, "H");

            for (int i = 1; i < 12; ++i) {
                for (int j = 1; j < 12; ++j) {
                    if (i % 4 == 0) {
                        if (j % 4 == 0)
                            mvprintw(i - 1, (j - 1) * 2, "%ls", L"┼─");
                        else
                            mvprintw(i - 1, (j - 1) * 2, "%ls", L"──");
                    } else if (j % 4 == 0) {
                            mvprintw(i - 1, (j - 1) * 2, "%ls", L"│");
                    }
                }
            }
        }

        void loop() {
            for (int ch = getch(); ch != 'q'; ch = getch()) {
                switch (ch) {
                    case KEY_MOUSE:
                        if (getmouse(&_event) != OK)
                            break;

                        if (!(_event.bstate & BUTTON1_CLICKED))
                            break;

                        {
                            int x = _event.x, y = _event.y;
                            if ((y <= 10 && x <= 20) && (y + 1) % 4 != 0 && (x + 2) % 8 != 0 && x % 2 == 0)
                                move(y, x);
                        }

                        break;
                    case KEY_UP:
                        if (getcury(stdscr) > 0) {
                            move(getcury(stdscr) - 1, getcurx(stdscr));
                            if ((getcury(stdscr) + 1) % 4 == 0)
                                move(getcury(stdscr) - 1, getcurx(stdscr));
                        } else {
                            move(10, getcurx(stdscr));
                        }
                        break;
                    case KEY_RIGHT:
                        if (getcurx(stdscr) < 20) {
                            move(getcury(stdscr), getcurx(stdscr) + 2);
                            if ((getcurx(stdscr) + 2) % 8 == 0)
                                move(getcury(stdscr), getcurx(stdscr) + 2);
                        } else {
                            move(getcury(stdscr), 0);
                        }
                        break;
                    case KEY_DOWN:
                        if (getcury(stdscr) < 10) {
                            move(getcury(stdscr) + 1, getcurx(stdscr));
                            if ((getcury(stdscr) + 1) % 4 == 0)
                                move(getcury(stdscr) + 1, getcurx(stdscr));
                        } else {
                            move(0, getcurx(stdscr));
                        }
                        break;
                    case KEY_LEFT:
                        if (getcurx(stdscr) > 0) {
                            move(getcury(stdscr), getcurx(stdscr) - 2);
                            if ((getcurx(stdscr) + 2) % 8 == 0)
                                move(getcury(stdscr), getcurx(stdscr) - 2);
                        } else {
                            move(getcury(stdscr), 20);
                        }
                        break;
                    case 'f':
                        if (!_board.fix())
                            break;

                        _status = Status::UserInput;
                        updateTUI();
                        break;
                    case 'g':
                        _status = Status::Generate;
                        updateTUI();

                        _board.generate(_seeds);

                        if (_board.unique() == 1)
                            _status = Status::UserInput;
                        else
                            _status = Status::Error;

                        updateTUI();
                        move(0, 0);
                        break;
                    case 's':
                        _status = Status::Solve;
                        updateTUI();

                        _board.solve();

                        if (_board.full() && _board.validate())
                            _status = Status::Solved;
                        else
                            _status = Status::Error;
                        updateTUI();

                        break;
                    case 'h':
                        if (_status == Status::Solved)
                            break;

                        {
                            int x = getcurx(stdscr), y = getcury(stdscr);

                            attron(COLOR_PAIR(Colors::Fixed));
                            mvprintw(0, 50, "Hint: ");
                            attroff(COLOR_PAIR(Colors::Fixed));

                            if (_board.unique() < 1) {
                                attron(COLOR_PAIR(Colors::Bad));
                                mvprintw(0, 56, "NOT SOLVABLE");
                                attroff(COLOR_PAIR(Colors::Bad));

                                move(y, x);
                                break;
                            }

                            Board solution = _board;
                            solution.fix();
                            solution.solve();

                            std::random_device dev;
                            std::mt19937 rng(dev());
                            std::uniform_int_distribution<> dist(0, 80);

                            int index = dist(rng);
                            while (_board[index / 9][index % 9] != 0) {
                                if (index < 80)
                                    ++index;
                                else
                                    index = 0;
                            }

                            attron(COLOR_PAIR(Colors::Good));
                            mvprintw(0, 56, "SOLVABLE");
                            attroff(COLOR_PAIR(Colors::Good));

                            _board.play(index / 9, index % 9, solution[index / 9][index % 9]);

                            if (_board.validate() && _board.full())
                                _status = Status::Solved;
                            updateTUI();

                            attron(COLOR_PAIR(Colors::Good));
                            mvprintw((index / 9) + ((index / 9) / 3), ((index % 9) + ((index % 9) / 3)) * 2, "%i", _board[index / 9][index % 9]);
                            attroff(COLOR_PAIR(Colors::Good));

                            move(getcury(stdscr), getcurx(stdscr) - 1);
                        }
                        break;
                    default: // Handle numbers
                        if ((ch == 127 || ch == KEY_BACKSPACE || ch == 8 || ('0' <= ch && '9' >= ch)) && getcurx(stdscr) % 2 == 0 && getcurx(stdscr) <= 20 && getcury(stdscr) <= 10) {
                            int x = getcurx(stdscr), y = getcury(stdscr);

                            _board.play(y - (y / 4), (x / 2) - ((x / 2) / 4), '0' <= ch && '9' >= ch ? ch - '0' : 0);

                            if (ch <= '9' && ch > '0' && _board.canMove(y - (y / 4), (x / 2) - ((x / 2) / 4), ch - '0')) {
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
                                } while ((_board.full() || _board[y - (y / 4)][(x / 2) - ((x / 2) / 4)] != 0) && _board.fixed(y - (y / 4), (x / 2) - ((x / 2) / 4)));
                            }

                            if (_board.validate() && _board.full())
                                _status = Status::Solved;
                            updateTUI();

                            move(y, x);
                        }
                        break;
                }
            }
        }
};

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

    Game game(seedsFile);
    game.loop();
    
    return 0;
}
