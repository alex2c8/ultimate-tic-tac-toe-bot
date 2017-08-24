/*******************************************************************************
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Abdulla Gaibullaev
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 ******************************************************************************/

#include <iostream>
#include <map>
#include <algorithm>
#include <sstream>
#include <time.h>

typedef std::pair<int, int> _move_t;

typedef __int64_t _score_t;

#define INF INT64_MAX

int winningPatterns[] = {
        0b111000000, 0b000111000, 0b000000111, // rows
        0b100100100, 0b010010010, 0b001001001, // cols
        0b100010001, 0b001010100 // diagonals
};

const _score_t MACRO_WIN_SCORE = 1000000;
const _score_t MICRO_WIN_SCORE = 1000;

std::map<int, _score_t> scoreAssign;

//  mapping between a bit's position and corresponding cell coordinates in matrix
std::vector<std::pair<int, int>> posPatterns(9);


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    elems.clear();
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


int stringToInt(const std::string &s) {
    std::istringstream ss(s);
    int result;
    ss >> result;
    return result;
}

/**
 * This class implements all IO operations.
 * Only one method must be realized:
 *
 *      > Bot::action
 *
 */
class Bot {

public:

    /**
     * Initialize your bot here.
     */
    Bot() {
        srand(static_cast<unsigned int>(time(0)));
        _field.resize(81);
        _macroboard.resize(9);

        //  init posPatterns
        for (int i = 0; i < 9; i++) {
            posPatterns[i].first = i % 3;
            posPatterns[i].second = i / 3;
        }
    }


    void loop() {
        std::string line;
        std::vector<std::string> command;
        command.reserve(256);

        while (std::getline(std::cin, line)) {
            processCommand(split(line, ' ', command));
        }
    }

public:

    /**
     * Implement this function.
     * type is always "move"
     *
     * return value must be position in x,y presentation
     *      (use std::make_pair(x, y))
     */
    std::pair<int, int> action(const std::string &type, int time) {
        Bot bot = (*this);
        std::vector<int> moves = getAvailableMoves();

        _move_t nextMove;

        //  make the first move in the center
        if (_move == 1)
            return _move_t(4, 4);


        //  if I have to move in an empty square, do it so the opponent's move
        //  will be in the same square -> better control
        if (isEmptySquare(moves)) {
            _move_t aux = convertToCoord(moves[0]);
            return _move_t(4 * (aux.first / 3), 4 * (aux.second / 3));
        }


        //  if there is a square that can be won directly
        //  that will lead to the end of the game in my favor
        if (multipleActiveSquares()) {
            nextMove = tryToWinGame(moves, _botId);

            //  if such a move exists
            if (nextMove.first != -1 && nextMove.second != -1)
                return nextMove;
        }


        //  get best move using minimax
        //TODO
        _score_t score = -INF;
        _score_t alpha = -INF;
        _score_t beta = INF;

        //  dynamically compute depth, based upon the number of available moves
//        int depth = getDepth((int) moves.size());
        int depth = 7;

        //  clone current field in order to be able to simulate moves
        Bot clone = Bot::clone(bot);

        //  for each available move
        for (int m : moves) {
            _move_t c_move = convertToCoord(m);

            //  simulate current move
            clone.simulateMove(c_move, _botId);

            //  calculate this move's score
            _score_t currentScore = minimax(clone, depth, alpha, beta, _opponentId);
            std::cerr << "current: " << c_move.first << " " << c_move.second << " " << currentScore << std::endl;

            //  undo this move
            clone.undoMove(c_move, bot);

            //  update general score and move
            if (currentScore > score) {
                score = currentScore;
                nextMove = c_move;
            }
        }


        std::cerr << "next: " << nextMove.first << " " << nextMove.second << " " << score << std::endl;
        return nextMove;
    }


    std::vector<int> getAvailableMoves() {
        std::vector<int> moves;

        for (int i = 0; i < 81; ++i) {
            int blockId = ((i / 27) * 3) + (i % 9) / 3;
            if (_macroboard[blockId] == -1 && _field[i] == 0) {
                moves.push_back(i);
            }
        }

        return moves;
    }

    bool isEmptySquare(std::vector<int> moves) {
        if (moves.size() != 9)
            return false;
        else {
            _move_t start = convertToCoord(moves[0]);
            _move_t end = convertToCoord(moves[8]);

            return start.first + 2 == end.first &&
                   start.second + 2 == end.second;
        }
    }

    _move_t convertToCoord(int m) {
        return _move_t(m % 9, m / 9);
    };

    int convertToInt(_move_t move) {
        return 9 * move.second + move.first;
    }

    //  used at final states: wins the entire game (without alpha-beta)
    _move_t tryToWinGame(std::vector<int> moves, int player) {
        int pattern = constructPattern(_macroboard, player);

        for (int wp : winningPatterns) {
            //  i know for sure that there is an optimal square
            std::pair<int, int> squarePos = getPos(pattern, wp);

            if (squarePos.first != -1 && squarePos.second != -1) {

                std::vector<int> auxsq = getSquareFromBoard(squarePos.first, squarePos.second, _field);
                int auxPattern = constructPattern(auxsq, player);

                for (int _wp : winningPatterns) {
                    //  i know for sure that there is an optimal move
                    std::pair<int, int> movePos = getPos(auxPattern, _wp);
                    if (movePos.first != -1 && movePos.second != -1) {

                        int x = 3 * squarePos.second + movePos.second;
                        int y = 3 * squarePos.first + movePos.first;


                        int _move = 9 * x + y;

                        if (std::find(moves.begin(), moves.end(), _move) != moves.end())
                            return convertToCoord(_move);
                    }
                }
            }

        }

        return _move_t(-1, -1);
    }


    /* minimax */

    //  dynamically compute depth
    //  depth is inversely proportional to the moves' size
    int getDepth(int movesSize) {
        if (_round < 18 && movesSize > 7)
            return 4;

        int depth = 7;


        if (movesSize == 5)
            depth = 8;

        else if (movesSize < 5)
            depth = 9;


        if (movesSize > 7 && movesSize <= 10)
            depth = 5;

        else if (movesSize > 10 && movesSize <= 17)
            depth = 4;

        else if (movesSize > 17 && movesSize <= 46)
            depth = 3;

        else if (movesSize > 46)
            depth = 2;

        if (_timePerMove < 4000 && depth > 3)
            depth = 3;

        else if (_timePerMove < 2000)
            depth = 1;

        return depth;
    }

    //  minimax + alpha-beta pruning
    _score_t minimax(Bot bot, int depth, _score_t alpha, _score_t beta, int player) {
        if (depth == 0 || bot.gameIsFinished()) {
            return bot.evaluate(_botId) - bot.evaluate(_opponentId);
        }

        _score_t score;

        //  clone current bot
        Bot clone = Bot::clone(bot);

        std::vector<int> moves = bot.getAvailableMoves();
        size_t movesSize = moves.size();

        //  compute depth
        if (movesSize > 7) {
            int newDepth;

            if (movesSize <= 10)
                newDepth = 5;

            else if (movesSize > 10 && movesSize <= 17)
                newDepth = 4;

            else if (movesSize > 17 && movesSize <= 46)
                newDepth = 3;

            else
                newDepth = 2;

            depth = (newDepth < depth) ? (newDepth) : (depth);
        }

        //  my turn
        if (player == _botId) {
            //  start pessimistic
            score = -INF;

            //  for each available move m
            for (int m : moves) {
                //  get the coordinates of the current move
                _move_t c_move = convertToCoord(m);

                //  simulate current move
                clone.simulateMove(c_move, _botId);

                //  calculate this move's score
                _score_t currentScore = minimax(clone, depth - 1, alpha, beta, _opponentId);

                //  undo current move
                clone.undoMove(c_move, bot);

                //  update scores
                if (currentScore > score) {
                    score = currentScore;
                    alpha = score;

                    //  pruning
                    if (alpha >= beta)
                        return score;
                }
            }
        }

            //  enemy's turn
        else {
            //  start pessimistic
            score = INF;

            //  for each available move
            for (int m : moves) {
                //  get coordinates of current move
                _move_t c_move = convertToCoord(m);

                //  simulate current enemy's move
                clone.simulateMove(c_move, _opponentId);

                //  calculate this move's score
                _score_t currentScore = minimax(clone, depth - 1, alpha, beta, _botId);

                //  undo current move
                clone.undoMove(c_move, bot);

                //  update scores
                if (currentScore < score) {
                    score = currentScore;
                    beta = score;

                    //  pruning
                    if (alpha >= beta)
                        return score;
                }
            }
        }

        return score;
    }

    //  TODO: improve
    void simulateMove(_move_t move, int player) {
        int x = move.first, y = move.second;

        int this_x = x / 3, this_y = y / 3;
        int sent_x = x % 3, sent_y = y % 3;

        int c_this = 3 * this_y + this_x;
        int c_sent = 3 * sent_y + sent_x;

        int c_move = convertToInt(move);

        //  put player on field
        _field[c_move] = player;

        //  get squares
        std::vector<int> thisSquare = getSquareFromBoard(this_x, this_y, _field);
        std::vector<int> sentSquare = getSquareFromBoard(sent_x, sent_y, _field);

        //  update macroboard
        if (isWinner(thisSquare, player))
            _macroboard[c_this] = player;
        else
            _macroboard[c_this] = 0;

        if (!squareIsDraw(sentSquare) && _macroboard[c_sent] <= 0) {
            _macroboard[c_sent] = -1;

            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    if (i == sent_x && j == sent_y)
                        continue;

                    if (!squareIsDraw(getSquareFromBoard(i, j, _field)) && _macroboard[3 * i + j] <= 0)
                        _macroboard[3 * i + j] = 0;
                }
            }
        }
        else {
            for (int i = 0; i < 3; i++) {
                for (int j = 0; j < 3; j++) {
                    std::vector<int> aux = getSquareFromBoard(i, j, _field);

                    if (_macroboard[3 * i + j] == 0 && !squareIsDraw(aux))
                        _macroboard[3 * i + j] = -1;
                }
            }
        }
    }

    void undoMove(_move_t move, Bot orig) {
        int m = convertToInt(move);

        //  reset field
        _field[m] = 0;

        //  reset macroboard
        for (int i = 0; i < 9; i++)
            _macroboard[i] = orig._macroboard[i];
    }


    /* heuristics */
    _score_t evaluate(int player) {
        int opponent = (player == 1) ? (2) : (1);

        if (isWinner(_macroboard, player))
            return MACRO_WIN_SCORE;

        else if (isWinner(_macroboard, opponent))
            return 0;

        std::vector<_score_t> scores(9);

        for (int i = 0; i < 3; i++) {
            for (int j = 0; j < 3; j++) {
                int k = 3 * i + j;

                //  if I won the square on (i,j) on the macroboard
                if (_macroboard[k] == player)
                    scores[k] = MICRO_WIN_SCORE;

                    //  if the opponent won the square (i,j) on the macroboard
                else if (_macroboard[k] == opponent)
                    scores[k] = 0;

                    //  otherwise, evaluate the square (i,j)
                else {
                    //  get the square identified by coordinates (i,j)
                    std::vector<int> aux = getSquareFromBoard(i, j, _field);
                    //  evaluate it
                    scores[k] = evaluateSquare(aux, player);
                }
            }
        }

        //  final score
        return evaluateMacro(scores);
    }

    _score_t evaluateMacro(std::vector<_score_t> macro) {
        _score_t score = 0;
        _score_t diag1 = 1, diag2 = 1;

        for (int i = 0; i < 3; i++) {
            _score_t line = 1, col = 1;

            for (int j = 0; j < 3; j++) {
                line *= macro[3 * i + j];
                col *= macro[3 * j + i];
            }

            score += line + col;
        }

        for (int i = 0; i < 3; i++) {
            diag1 *= macro[3 * i + i];
            diag2 *= macro[3 * (2 - i) + i];
        }

        score += diag1 + diag2;

        return score;
    }

    _score_t evaluateSquare(std::vector<int> square, int player) {
        int opponent = (player == 1) ? (2) : (1);

        //  init scoreAssigns
        scoreAssign.insert(std::pair<int, _score_t>(0, 1));
        scoreAssign.insert(std::pair<int, _score_t>(player, 10));
        scoreAssign.insert(std::pair<int, _score_t>(opponent, 0));

        _score_t score = 0;
        _score_t diag1 = 1, diag2 = 1;

        std::map<int, _score_t> mm;

        for (int i = 0; i < 3; i++) {
            _score_t line = 1, col = 1;

            for (int j = 0; j < 3; j++) {
                line *= scoreAssign[square[3 * i + j]];
                col *= scoreAssign[square[3 * j + i]];
            }

            score += line + col;
        }

        for (int i = 0; i < 3; i++) {
            diag1 *= scoreAssign[square[3 * i + i]];
            diag2 *= scoreAssign[square[3 * (2 - i) + i]];
        }

        score += diag1 + diag2;

        return score;
    }

    /* field */

    bool multipleActiveSquares() {
        int count = 0;

        for (int i = 0; i < 9; i++) {
            if (_macroboard[i] == -1)
                count++;
            if (count >= 2)
                return true;
        }

        return false;
    }

    //  retrieves the square at the position (x, y) from the board
    std::vector<int> getSquareFromBoard(int x, int y, std::vector<int> board) {
        std::vector<int> square(9);

        int k = 0;

        for (int i = y * 3; i < (y * 3 + 3); i++) {
            for (int j = x * 3; j < (x * 3 + 3); j++)
                square[k++] = board[9 * i + j];
        }

        return square;
    }

    bool gameIsFinished() {
        if (isWinner(_macroboard, _botId))
            return true;

        if (isWinner(_macroboard, _opponentId))
            return true;

        return isFull();
    }

    bool isFull() {
        if (getAvailableMoves().size() == 0)
            return true;

        return (std::find(_field.begin(), _field.end(), 0) == _field.end());
    }

    bool squareIsDraw(std::vector<int> square) {
        return (std::find(square.begin(), square.end(), 0) == square.end());
    }

    bool isDraw(std::vector<int> macro) {
        return getAvailableMoves().size() == 0;
    }

    /* clone */

    Bot clone(Bot from) {
        Bot clone;

        for (int i = 0; i < 81; i++)
            clone._field[i] = from._field[i];

        for (int i = 0; i < 9; i++)
            clone._macroboard[i] = from._macroboard[i];

        clone._round = from._round;
        clone._move = from._move;

        return clone;
    }


    /* utils */

    //  constructs a 9-bit pattern from square, for player
    int constructPattern(std::vector<int> cells, int player) {
        int pattern = 0b000000000; // 9-bit pattern for the 9 cells

        for (int i = 0; i < 9; i++) {
            if (cells[i] == player) {
                pattern |= (1 << (8 - i));
            }
        }

        return pattern;
    }

    // if one square matches a winning pattern, then 'player' wins the square
    bool isWinner(std::vector<int> cells, int player) {
        int pattern = 0b000000000; // 9-bit pattern for the 9 cells

        for (int i = 0; i < 9; i++) {
            if (cells[i] == player)
                pattern |= (1 << (8 - i));
        }

        for (int wp : winningPatterns) {
            if ((pattern & wp) == wp)
                return true;
        }

        return false;
    }

    //  returns the position of the square that, if won, can lead to the end of the game in my favor
    //  p(pattern) and wp(winning pattern) are 9-bit sequences
    std::pair<int, int> getPos(int p, int wp) {
        int diffs = countDiffs(p, wp);

        if (diffs == 2) {
            for (int i = 0; i < 9; i++) {
                int bit_p = (p >> (8 - i)) & 1;
                int bit_wp = (wp >> (8 - i)) & 1;

                //  if the current bit in the winning pattern is 1
                //  and the current bit in the table pattern is 0,
                //  then the position where they differ (i) refers to the square that can be won
                if ((bit_wp & 1) == 1 && (bit_p & 1) == 0) {
                    return posPatterns[i];
                }
            }

            return std::pair<int, int>(-1, -1);
        }

        return std::pair<int, int>(-1, -1);
    }

    //  count diffs between two bytes
    int countDiffs(int byte1, int byte2) {
        int diff = 0;

        for (int i = 0; i < 9; i++) {
            diff += ((byte1 >> i) & 1) & ((byte2 >> i) & 1);
        }

        return diff;
    }


    /* parsing */

    void processCommand(const std::vector<std::string> &command) {
        if (command[0] == "action") {
            auto point = action(command[1], stringToInt(command[2]));
            std::cout << "place_move " << point.first << " " << point.second << std::endl << std::flush;
        }
        else if (command[0] == "update") {
            update(command[1], command[2], command[3]);
        }
        else if (command[0] == "settings") {
            setting(command[1], command[2]);
        }
        else {
            debug("Unknown command <" + command[0] + ">.");
        }
    }

    void update(const std::string &player, const std::string &type, const std::string &value) {
        if (player != "game" && player != _myName) {
            // It's not my update!
            return;
        }

        if (type == "round") {
            _round = stringToInt(value);
        }
        else if (type == "move") {
            _move = stringToInt(value);
        }
        else if (type == "macroboard" || type == "field") {
            std::vector<std::string> rawValues;
            split(value, ',', rawValues);
            std::vector<int>::iterator choice = (type == "field" ? _field.begin() : _macroboard.begin());
            std::transform(rawValues.begin(), rawValues.end(), choice, stringToInt);
        }
        else {
            debug("Unknown update <" + type + ">.");
        }
    }

    void setting(const std::string &type, const std::string &value) {
        if (type == "timebank") {
            _timebank = stringToInt(value);
        }
        else if (type == "time_per_move") {
            _timePerMove = stringToInt(value);
        }
        else if (type == "player_names") {
            split(value, ',', _playerNames);
        }
        else if (type == "your_bot") {
            _myName = value;
        }
        else if (type == "your_botid") {
            _botId = stringToInt(value);
            _opponentId = (_botId == 1) ? (2) : (1);
        }
        else {
            debug("Unknown setting <" + type + ">.");
        }
    }

    void debug(const std::string &s) const {
        std::cerr << s << std::endl << std::flush;
    }

public:
    // static settings
    int _timebank;
    int _timePerMove;
    int _botId;
    int _opponentId;

    std::vector<std::string> _playerNames;
    std::string _myName;

    // dynamic settings
    int _round;
    int _move;
    std::vector<int> _macroboard;
    std::vector<int> _field;
};

/**
 * don't change this code.
 * See Bot::action method.
 **/
int main() {
    Bot bot;
    bot.loop();
//
//
//    int macro[3][3] = {
//            {2, -1, 2},
//            {2, 2,  -1},
//            {1, 1,  0}
//    };
//
//    int board[9][9] = {
//            {1, 1, 2, 2, 0, 0, 0, 0, 1},
//            {2, 2, 2, 0, 0, 2, 0, 2, 2},
//            {0, 0, 0, 1, 1, 0, 0, 0, 0},
//            {0, 1, 2, 2, 2, 2, 2, 1, 0},
//            {1, 0, 0, 2, 1, 2, 0, 1, 1},
//            {0, 0, 0, 2, 0, 2, 0, 1, 0},
//            {0, 0, 0, 2, 0, 0, 2, 2, 1},
//            {0, 1, 0, 0, 2, 0, 1, 1, 2},
//            {1, 0, 2, 1, 1, 1, 2, 1, 2}
//    };
//
//////////
//    for (int i = 0; i < 3; i++) {
//        for (int j = 0; j < 3; j++) {
//            bot._macroboard[3 * i + j] = macro[i][j];
//        }
//    }
//
//    for (int i = 0; i < 9; i++) {
//        for (int j = 0; j < 9; j++) {
//            bot._field[9 * i + j] = board[i][j];
//        }
//    }
//
//    bot._botId = 1;
//    bot._opponentId = 2;
////////
////////
////////
////////
////////
//////////    TEST
////////
//    std::vector<int> moves = bot.getAvailableMoves();
//////
//    _move_t m = bot.action("move", 1);
//    std::cout << m.first << " " << m.second << std::endl;

////
//    std::vector<int> s = bot.getSquareFromBoard(0, 2, bot._field);
//
//    for (auto i : s) {
//        std::cout << i << " ";
//    }
//    std::cout << "\n";
////
//////    -----
////
////
//////    PRINT
//////    std::cout << "MACRO ORIGINAL:\n";
//////    for (int i = 0; i < 3; i++) {
//////        for (int j = 0; j < 3; j++) {
//////            std::cout << clone._macroboard[3 * i + j] << " ";
//////        }
//////        std::cout << std::endl;
//////    }
//////
//////
//////    std::cout << "FIELD ORIGINAL:\n";
//////    for (int i = 0; i < 9; i++) {
//////        for (int j = 0; j < 9; j++) {
//////            std::cout << clone._field[9 * i + j] << " ";
//////        }
//////        std::cout << std::endl;
//////    }
//////
//////    Bot clone = bot;
//////
//////    std::cout << "MACRO CLONE:\n";
//////    for (int i = 0; i < 3; i++) {
//////        for (int j = 0; j < 3; j++) {
//////            std::cout << clone._macroboard[3 * i + j] << " ";
//////        }
//////        std::cout << std::endl;
//////    }
//////
//////
//////    std::cout << "FIELD CLONE:\n";
//////    for (int i = 0; i < 9; i++) {
//////        for (int j = 0; j < 9; j++) {
//////            std::cout << clone._field[9 * i + j] << " ";
//////        }
//////        std::cout << std::endl;
//////    }
////
//////    ----


    return 0;
}