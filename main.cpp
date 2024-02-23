// All code from the namespace `std` is part of the C++ standard library
// made publically available by an ISO working group
#include <iostream>
#include <algorithm>
#include <iterator>
#include <random>
#include <fstream>
#include <string>

// Apple uses an older version of NCurses, so you must #define a macro to be 1 for support
#ifdef __APPLE__
    #define _XOPEN_SOURCE_EXTENDED 1
#endif

// A publically available API with C++ bindings to create a TUI using escape characters
// Initially released in 1993 and updated since then primarily by the following:
// Thomas E. Dickey, Juergen Pfeifer, Eric S Raymond, Alexander V Lukyanov, Philippe Blain,
// Sven Verdoolaege, and Nicolas Boulenguez (among others)
// More information may be found here: https://invisible-island.net/ncurses/ncurses.faq.html
#include <ncurses.h>
#include <locale.h>

// Makes colors and status more clear to reference (e.g. Colors::Bad is the same as 1)
enum Colors { Bad = 1, Good = 2, Fixed = 3 };
enum Status { UserInput, UserSolve, Generate, Solve, Solved, Error };

// Define the Board and related methods for solving, testing unique, etc.
class Board {
    private:
        int _board[9][9] = {}; // The board itself represented as a 2d Array
        int *_fixed = new int[0]; // A list of indexes to the uneditable elements of the array
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

        // Assignment operator (similar to copy constructor except assigns an existing Board)
        Board& operator=(const Board& copy) {
            _numFixed = copy._numFixed;
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    _board[i][j] = copy[i][j];

            // create a new _fixed and copy the elements of the original into it
            delete[] _fixed;
            _fixed = new int[_numFixed];
            for (int i = 0; i < _numFixed; ++i)
                _fixed[i] = copy._fixed[i];

            return *this;
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

        // Reset the board
        void clear() {
            delete[] _fixed;

            _numFixed = 0;
            _fixed = new int[_numFixed];

            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    _board[i][j] = 0;
        }

        // Overload operator[] to allow for easy indexing of the array 
        // Returns a constant: you must modify the board via `play()`
        const int* operator[](const unsigned int r) const {
            return _board[r];
        }

        // Taking in a row and col, returns a boolean describing if that cell is fixed
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

            // Set _numFixed to the number of non-zero elements & create a new array
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

            switch (times) {
                case 1: // 90 deg: (x, y) -> (y, -x)
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[j][8 - i];
                    break;
                case 2: // 180 deg: (x, y) -> (-x, -y)
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[8 - i][8 - j];
                    break;
                case 3: // 270 deg: (x, y) -> (-y, x)
                    for (int i = 0; i < 9; ++i)
                        for (int j = 0; j < 9; ++j)
                            _board[i][j] = old[8 - j][i];
                    break;
            }
        }

        // Reflect across axis 0, 1, 2, or 3 (no reflection, y-axis, x-axis, both)
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
            _board[r][c] = 0; // Clear current value bc it will be overridden

            std::copy(std::begin(_board[r]), std::end(_board[r]), std::begin(row));

            int g = ((r / 3) * 3 + c / 3);
            for (int j = 0; j < 9; ++j) {
                col[j] = _board[j][c];
                group[j] = _board[(g / 3) * 3 + j / 3][(g % 3) * 3 + j % 3];
            }

            // Check if anything in the row, col, or group is equal to the given number
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
                // Return a solution if we are on the bottom right corner of the board
                if (row == 8)
                    return true;

                // If out of bounds by going too far to the right, wrap around to the next row
                ++row;
                col = 0;
            }
            
            // If there's already a number, jump to the next cell
            if (_board[row][col] > 0)
                return solve(row, col + 1);

            // Iterate through all options for numbers
            for (int i = 1; i < 10; ++i) {
                // If a number works, set it, recurse `solve()`, and reset if no solution
                if (canMove(row, col, i)) {
                    _board[row][col] = i;

                    if (solve(row, col + 1))
                        return true;

                    _board[row][col] = 0;
                }   
            }

            return false; // no solution
        }

        // Wrap the underlying solve function to only solve valid boards
        bool solve() {
            // Reset anything that isn't fixed
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    if (_board[i][j] > 0 && !fixed(i, j))
                        _board[i][j] = 0;

            // Don't solve an impossible board
            if (!validate())
                return false;
            if (full())
                return true;

            return solve(0, 0);
        }

        // Returns the number of solutions by recursively finding them
        // Returns 0, 1, or 2 (2 simply means there are at least 2 solutions)
        int unique(int row = 0, int col = 0, int num = 0) {
            if (col > 8) {
                // If out of bounds on the bottom right corner, we found one more solution
                if (row == 8)
                    return num + 1;

                // If out of bounds on the right, go to the next row
                ++row;
                col = 0;
            }

            // If we find a fixed element, skip it
            if (_board[row][col] > 0)
                return unique(row, col + 1, num);

            // Iterate through all options
            for (int i = 1; i < 10; ++i) {
                // If we find a valid move, go there and then check for unique solutions
                if (canMove(row, col, i)) {
                    _board[row][col] = i;

                    num = unique(row, col + 1, num);

                    _board[row][col] = 0;

                    if (num > 1)
                        return num;
                }   
            }

            return num; // Return however many we found
        }

        // Generate a new board using the seeds file
        void generate(std::string file) {
            std::ifstream binary_file(file, std::ios::in | std::ios::binary); // Open the file

            // To generate better random numbers
            std::random_device dev;
            std::mt19937 rng(dev());

            std::string line, result;

            // If there are no seeds, generate a new seed
            if (binary_file.peek() == std::ifstream::traits_type::eof()) {
                *this = generateSeed();
                return;
            }

            // For each line randomly decide if it should be the result
            // Randomly select a number from 0 to the line number
            // If the num is less than 1, set the result to this line (ensures a result)
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

        // Static method to generate a seed for a puzzle (generates a board) 
        static Board generateSeed() {     
            Board board;

            // Create a list of pointers to individual elements of the board
            int shuffledBoard[81] = {};
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    shuffledBoard[i * 9 + j] = i * 9 + j; 

            // Randomly shuffle that list of pointers
            std::random_device rd;
            std::mt19937 g(rd());
            std::shuffle(std::begin(shuffledBoard), std::end(shuffledBoard), g);

            // Create a list of options to move to randomly shuffle
            int options[9] = {1, 2, 3, 4, 5, 6, 7, 8, 9};

            // Iterate through all cells
            for (int i = 0; i < 81; ++i) {
                // Shuffle the list of options
                std::shuffle(std::begin(options), std::end(options), g);

                // Iterate through all the options for the selected element
                for (int k = 0; k < 9; ++k) {
                    // If you can move to the current cell with the kth option to move, do so 
                    if (board.canMove(shuffledBoard[i] / 9, shuffledBoard[i] % 9, options[k])) {
                        board._board[shuffledBoard[i] / 9][shuffledBoard[i] % 9] = options[k];
                        
                        // Get the number of solutions remaining
                        int numSolutions = board.unique();
                        
                        if (numSolutions == 1) { // Return if 1 and only 1 solution
                            board.fix();
                            return board;
                        } else if (numSolutions > 1) { // If there is more than one, continue
                            break;
                        } else { // If there are less than one, we messed up
                            board._board[shuffledBoard[i] / 9][shuffledBoard[i] % 9] = 0; 
                        }
                    }
                }
            }

            // Fix the board and return
            board.fix();
            return board; 
        }

        // Serialize the board using handy operator<< notation
        // Simply put every element one after another with no spacing or formatting
        friend std::ostream& operator<<(std::ostream& os, const Board& board) {
            for (int i = 0; i < 9; ++i)
                for (int j = 0; j < 9; ++j)
                    os << board._board[i][j];

            return os;
        }
};

// Class which represents the user interface with the board
class Game {
    private:
        std::string _seeds = "seeds.dat"; // The file that the seeds are stored in
        Board _board; // The board itself
        int _status = Status::UserInput; // The current status of the game

        MEVENT _event; // Mouse Event handler

    public:
        // Default constructor that sets the locale & initializes the terminal using ncurses:
        // allows mouse events, creates the colors, and initializes the display
        Game(std::string seeds = "seeds.dat") : _seeds(seeds) {
            // Necessary for support of wide characters (MUST BE BEFORE `initscr()`)
            setlocale(LC_ALL, "");
            setlocale(LC_NUMERIC,"C");

            // Initialize the screen
            initscr();
            keypad(stdscr, TRUE); // Take input from special keys
            noecho(); // Don't display input on screen by default 
            cbreak(); // Don't buffer lines; make input immediately available

            // Capture mouse events
            mousemask(ALL_MOUSE_EVENTS, NULL);

            // Initialize colors
            start_color();
            init_pair(Colors::Bad, COLOR_RED, COLOR_BLACK);
            init_pair(Colors::Good, COLOR_GREEN, COLOR_BLACK);
            init_pair(Colors::Fixed, COLOR_BLUE, COLOR_BLACK);

            // Initialize display
            initDisplay();
            updateTUI();

            // Reset cursor
            move(0, 0);
        }

        // Safely exit ncurses
        ~Game() {
            endwin();
        }

        // Move the cursor to a specific row and column of the sudoku puzzle
        void setCursor(int r, int c) {
            move(r + (r / 3), c * 2 + (c / 3) * 2);
        }

        // Update the information provided to the user
        void updateTUI() {
            int x = getcurx(stdscr), y = getcury(stdscr); // to reset the cursor later

            // Clear any currently available hints
            mvprintw(0, 50, "                  ");

            // Change status
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

            // Print the board
            for (int i = 0; i < 9; ++i) {
                for (int j = 0; j < 9; ++j) {
                    setCursor(i, j);
                    
                    if (_board.fixed(i, j)) { // Fixed number
                        attron(COLOR_PAIR(Colors::Fixed));
                        printw("%i", _board[i][j]);
                        attroff(COLOR_PAIR(Colors::Fixed));
                    } else if (!_board.canMove(i, j, _board[i][j])) { // Invalid number
                        attron(COLOR_PAIR(Colors::Bad));
                        printw("%i", _board[i][j]);
                        attroff(COLOR_PAIR(Colors::Bad));
                    } else { // Normal number
                        printw("%i", _board[i][j]);
                    }
                }
            }

            // Reset cursor + refresh
            move(y, x);
            refresh();
        }

        // Initialize the display including the keybinds, board layout, etc.
        void initDisplay() {
            // Status label and keybind labels
            attron(COLOR_PAIR(Colors::Fixed));
            mvprintw(0, 26, "Status: ");
            mvprintw(2, 26, "Keybinds:");
            mvprintw(3, 30, "Movement: ");
            mvprintw(4, 30, "Input: ");
            mvprintw(5, 30, "Finish Initial Input: ");
            mvprintw(6, 30, "Generate: ");
            mvprintw(7, 30, "Solve: ");
            mvprintw(8, 30, "Hint: ");
            mvprintw(9, 30, "Reset: ");
            attroff(COLOR_PAIR(Colors::Fixed));

            // Actual keybinds themselves
            mvprintw(3, 40, "Arrow Keys");
            mvprintw(4, 37, "Backspace | [0-9]");
            mvprintw(5, 52, "F");
            mvprintw(6, 40, "G");
            mvprintw(7, 37, "S");
            mvprintw(8, 36, "H");
            mvprintw(9, 37, "R");

            // Grid
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

        // Base game loop that gets input from the user and performs a task based on it
        void loop() {
            for (int ch = getch(); ch != 'q'; ch = getch()) { // Until you type 'q', getch()
                switch (ch) {
                    case KEY_MOUSE:
                        if (getmouse(&_event) != OK)
                            break;

                        // Listen for left mouse button
                        if (!(_event.bstate & BUTTON1_CLICKED))
                            break;

                        { // Create a new scope to avoid skipping variable initialization
                            int x = _event.x, y = _event.y;
                            if (y <= 10 && (y + 1) % 4 != 0 &&
                                x <= 20 && (x + 2) % 8 != 0 && x % 2 == 0)
                                move(y, x); // Move if in bounds
                        }

                        break;
                    case KEY_UP: // Move up
                        if (getcury(stdscr) > 0) {
                            move(getcury(stdscr) - 1, getcurx(stdscr));
                            if ((getcury(stdscr) + 1) % 4 == 0)
                                move(getcury(stdscr) - 1, getcurx(stdscr));
                        } else {
                            move(10, getcurx(stdscr));
                        }
                        break;
                    case KEY_RIGHT: // Move right
                        if (getcurx(stdscr) < 20) {
                            move(getcury(stdscr), getcurx(stdscr) + 2);
                            if ((getcurx(stdscr) + 2) % 8 == 0)
                                move(getcury(stdscr), getcurx(stdscr) + 2);
                        } else {
                            move(getcury(stdscr), 0);
                        }
                        break;
                    case KEY_DOWN: // Move down
                        if (getcury(stdscr) < 10) {
                            move(getcury(stdscr) + 1, getcurx(stdscr));
                            if ((getcury(stdscr) + 1) % 4 == 0)
                                move(getcury(stdscr) + 1, getcurx(stdscr));
                        } else {
                            move(0, getcurx(stdscr));
                        }
                        break;
                    case KEY_LEFT: // Move left
                        if (getcurx(stdscr) > 0) {
                            move(getcury(stdscr), getcurx(stdscr) - 2);
                            if ((getcurx(stdscr) + 2) % 8 == 0)
                                move(getcury(stdscr), getcurx(stdscr) - 2);
                        } else {
                            move(getcury(stdscr), 20);
                        }
                        break;
                    case 'f': // Fix board in place
                        if (!_board.fix())
                            break;

                        // Set status + update
                        _status = Status::UserInput;
                        updateTUI();
                        break;
                    case 'g': // Generate board
                        _status = Status::Generate;
                        updateTUI();

                        _board.generate(_seeds);

                        // if not exactly one unique solution, show an error
                        if (_board.unique() == 1)
                            _status = Status::UserInput;
                        else
                            _status = Status::Error;

                        updateTUI();
                        move(0, 0);
                        break;
                    case 's': // solve
                        _status = Status::Solve;
                        updateTUI();

                        _board.solve();

                        // If board isn't solved, show an error
                        if (_board.full() && _board.validate())
                            _status = Status::Solved;
                        else
                            _status = Status::Error;
                        updateTUI();

                        break;
                    case 'h': // hint
                        if (_status == Status::Solved)
                            break;

                        { // Same thing; create new scope for variable initialization
                            int x = getcurx(stdscr), y = getcury(stdscr);

                            // Setup hint
                            attron(COLOR_PAIR(Colors::Fixed));
                            mvprintw(0, 50, "Hint: ");
                            attroff(COLOR_PAIR(Colors::Fixed));

                            // If there's less than one unique solution, hint unsolvable
                            if (_board.unique() < 1) {
                                attron(COLOR_PAIR(Colors::Bad));
                                mvprintw(0, 56, "NOT SOLVABLE");
                                attroff(COLOR_PAIR(Colors::Bad));

                                move(y, x);
                                break;
                            }

                            // Find the solution
                            Board solution = _board;
                            solution.fix();
                            solution.solve();

                            // Get a random index of the board
                            std::random_device dev;
                            std::mt19937 rng(dev());
                            std::uniform_int_distribution<> dist(0, 80);

                            // Get the closest empty blank near the selected index
                            int index = dist(rng);
                            while (_board[index / 9][index % 9] != 0) {
                                if (index < 80)
                                    ++index;
                                else
                                    index = 0;
                            }

                            // Board is solvable
                            attron(COLOR_PAIR(Colors::Good));
                            mvprintw(0, 56, "SOLVABLE");
                            attroff(COLOR_PAIR(Colors::Good));

                            // Play the index
                            _board.play(index / 9, index % 9, solution[index / 9][index % 9]);

                            // If this is the last number, change status to be solved
                            if (_board.validate() && _board.full())
                                _status = Status::Solved;
                            updateTUI(); // Update board + status

                            // Print the hint in green
                            attron(COLOR_PAIR(Colors::Good));
                            mvprintw((index / 9) + ((index / 9) / 3),
                                     ((index % 9) + ((index % 9) / 3)) * 2, 
                                     "%i", _board[index / 9][index % 9]);
                            attroff(COLOR_PAIR(Colors::Good));

                            // Move to the hint
                            move(getcury(stdscr), getcurx(stdscr) - 1);
                        }
                        break;
                    case 'r': // Reset board
                        _board.clear();
                        _status = Status::UserInput;
                        updateTUI();
                        break;
                    default: // Handle numbers
                        // If the input is a number or backspace, play the appropriate number
                        if (ch == 127 || ch == KEY_BACKSPACE || ch == 8 ||
                            ('0' <= ch && '9' >= ch)) {
                            int x = getcurx(stdscr), y = getcury(stdscr); // to move back

                            // Play the position
                            _board.play(y - (y / 4),
                                       (x / 2) - ((x / 2) / 4),
                                       '0' <= ch && '9' >= ch ? ch - '0' : 0);

                            // Move the cursor to the next available spot in the same grid
                            if ((ch <= '9' && ch > '0' && 
                                _board.canMove(y - (y / 4), (x / 2) - (x / 8), ch - '0')) ||
                                _board.fixed(y - (y / 4), (x / 2) - (x / 8))) {
                                // Move x & y position to the first cell in the grid square
                                x = (x / 4) * 4;
                                y = (y / 4) * 4;

                                // While the position isn't valid, progress it
                                while (!_board.full() && 
                                       (_board[y - (y / 4)][(x / 2) - ((x / 2) / 4)] != 0 || 
                                       _board.fixed(y - (y / 4), (x / 2) - ((x / 2) / 4)))) {
                                    // At the end of a grid
                                    if (((x + 2) % 8 == 6) && (y + 1) % 4 == 3) {
                                        // At end of row, move to the next row
                                        if (x == 20) {
                                            if (y < 10)
                                                y += 2;
                                            else // At the very end, move to the first cell
                                                y = 0;
                                            x = 0;
                                        } else { // else, move to the grid to the right
                                            x += 4;
                                            y -= 2;
                                        }
                                    } else if ((x + 2) % 8 < 6) { // Move inside grid
                                        x += 2;
                                    } else { // move to the next row within the grid
                                        x -= 4;
                                        ++y;
                                    }
                                }
                            }

                            // If this solves the board, update the TUI
                            if (_board.validate() && _board.full())
                                _status = Status::Solved;
                            else
                                _status = Status::UserInput;

                            updateTUI();

                            // Return to the original position
                            move(y, x);
                        }
                        break;
                }
            }
        }
};

// Program insertion point
int main(int argc, char **argv) {
    std::string seedsFile = "seeds.dat"; // default seeds file

    // Get any command-line arguments if there are any
    if (argc > 1) {
        // Return help menu if request (-h)
        if (std::find(argv, argv + argc, std::string("-h")) != argv + argc) {
            std::cout << "usage: sudoku [options]" << std::endl;
            std::cout << "options:" << std::endl;
            std::cout << "  sudoku -h               | prints this screen" << std::endl;
            std::cout << "  sudoku -s [file]        | sets the source for seeds to be [file]";
            std::cout << " (seeds.dat by default)" << std::endl;
            std::cout << "  sudoku -g [num]         | generates [num] seeds for sudoku puzzl";
            std::cout << "es (100 by default) and exports to seeds.dat" << std::endl;
            std::cout << "  sudoku -t               | tests the seeds file for seeds with un";
            std::cout << "ique solutions" << std::endl;
            return 0;
        }

        // Set the seeds file if requested (-s)
        char **s = std::find(argv, argv + argc, std::string("-s"));
        if (s != argv + argc)
            seedsFile = *(s + 1);

        // Generate seeds for sudoku puzzles if requested (-g)
        char **g = std::find(argv, argv + argc, std::string("-g"));
        if (g != argv + argc) {
            // Initialize screen
            setlocale(LC_ALL, "");
            initscr();
            noecho();
            cbreak();

            // Get the number of seeds to make; default to 100
            int num = (g + 1) == argv + argc ? 100 : std::stoi(*(g + 1));

            // Status
            mvaddstr(0, 0, "Generating seeds for sudoku puzzles:");
            mvprintw(1, 0, "[                    ] (0/%i)", num);
            refresh();

            // Open file for output
            std::ofstream binary_file(seedsFile, 
                                      std::ios::out | std::ios::binary | std::ios::app);

            // Generate the number of seeds requested
            for (int i = 0; i < num; ++i) {
                // Generate a board
                Board board = Board::generateSeed();

                // Add the serialized board to the file
                binary_file << board << std::endl;

                // Update the status on completing the number of seeds
                for (int j = 0; j < ((20 * (i + 1)) / num); ++j)
                    mvaddwstr(1, j + 1, L"▓");
                mvprintw(1, 23, "(%i/%i)", i + 1, num);
                refresh();
            }

            // Close the file and exit
            binary_file.close();
            endwin();
            return 0;
        }

        // Test the generated seeds if requested (-t)
        char **t = std::find(argv, argv + argc, std::string("-t"));
        if (t != argv + argc) {
            std::ifstream binary_file(*(t + 1), std::ios::in | std::ios::binary);

            std::string line;
            while (std::getline(binary_file, line)) {
                Board board(line);
                // If there isn't one unique solution to the puzzle, fail the test
                if (board.unique() != 1) {
                    binary_file.close();
                    std::cout << "FAILED" << std::endl;
                    return 1;
                }
            }

            // If nothing fails, pass the test
            binary_file.close();
            std::cout << "PASSED" << std::endl;
            return 0;
        }
    }

    // Initialize the game and loop
    Game game(seedsFile);
    game.loop();
    
    return 0;
}
