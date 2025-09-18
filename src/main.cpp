#include <iostream>
#include <iomanip>
#include <vector>
#include <string>
#include <chrono>
#include <sstream>
#include <cassert>

class Board {
private:
    int size;
    std::vector<std::vector<int>> grid;
    int moveCount;
    int lastMoveRow, lastMoveCol, lastPlayer;

public:
    Board(int s = 15) : size(s), moveCount(0), lastMoveRow(-1), lastMoveCol(-1), lastPlayer(-1) {
        grid.assign(size, std::vector<int>(size, 0));
    }
    
    int getSize() const { return size; }
    int getMoveCount() const { return moveCount; }
    bool isEmpty() const { return moveCount == 0; }
    
    int getCell(int row, int col) const {
        if (row < 0 || row >= size || col < 0 || col >= size) return -1;
        return grid[row][col];
    }
    
    bool makeMove(int row, int col, int player) {
        if (row < 0 || row >= size || col < 0 || col >= size || grid[row][col] != 0) {
            return false;
        }
        if (player != 1 && player != 2) return false;
        
        grid[row][col] = player;
        moveCount++;
        lastMoveRow = row;
        lastMoveCol = col;
        lastPlayer = player;
        return true;
    }
    
    bool undoLastMove() {
        if (lastMoveRow < 0 || lastMoveCol < 0) return false;
        
        grid[lastMoveRow][lastMoveCol] = 0;
        moveCount--;
        lastMoveRow = lastMoveCol = lastPlayer = -1;
        return true;
    }
    
    void reset() {
        for (auto& row : grid) {
            std::fill(row.begin(), row.end(), 0);
        }
        moveCount = 0;
        lastMoveRow = lastMoveCol = lastPlayer = -1;
    }
    
    bool resize(int newSize) {
        if (newSize < 15 || newSize > 100) return false;
        
        std::vector<std::vector<int>> newGrid(newSize, std::vector<int>(newSize, 0));
        int copySize = std::min(size, newSize);
        
        for (int i = 0; i < copySize; i++) {
            for (int j = 0; j < copySize; j++) {
                newGrid[i][j] = grid[i][j];
            }
        }
        
        grid = std::move(newGrid);
        size = newSize;
        return true;
    }
    
    std::tuple<int, int, int> getLastMove() const {
        return std::make_tuple(lastMoveRow, lastMoveCol, lastPlayer);
    }
    
    double getOccupancyRate() const {
        return static_cast<double>(moveCount) / (size * size);
    }
    
    size_t getMemoryUsage() const {
        return sizeof(*this) + size * size * sizeof(int);
    }
};

class GameLogic {
public:
    enum class GameState { PLAYING, PLAYER1_WIN, PLAYER2_WIN, DRAW };
    enum class MoveResult { VALID, OUT_OF_BOUNDS, CELL_OCCUPIED, INVALID_PLAYER };
    
    static MoveResult validateMove(const std::vector<std::vector<int>>& board, int row, int col, int player) {
        if (player != 1 && player != 2) return MoveResult::INVALID_PLAYER;
        
        int size = board.size();
        if (row < 0 || row >= size || col < 0 || col >= size) {
            return MoveResult::OUT_OF_BOUNDS;
        }
        
        if (board[row][col] != 0) return MoveResult::CELL_OCCUPIED;
        
        return MoveResult::VALID;
    }
    
    static bool checkWin(const std::vector<std::vector<int>>& board, int row, int col, int player) {
        if (row < 0 || col < 0 || board[row][col] != player) return false;
        
        int size = board.size();
        int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        
        for (int dir = 0; dir < 4; dir++) {
            int dx = directions[dir][0];
            int dy = directions[dir][1];
            int count = 1;
            
            int x = row + dx, y = col + dy;
            while (x >= 0 && x < size && y >= 0 && y < size && board[x][y] == player) {
                count++;
                x += dx;
                y += dy;
            }
            
            x = row - dx, y = col - dy;
            while (x >= 0 && x < size && y >= 0 && y < size && board[x][y] == player) {
                count++;
                x -= dx;
                y -= dy;
            }
            
            if (count >= 5) return true;
        }
        
        return false;
    }
    
    static GameState checkGameState(const std::vector<std::vector<int>>& board, int lastRow, int lastCol) {
        if (lastRow >= 0 && lastCol >= 0) {
            int lastPlayer = board[lastRow][lastCol];
            if (checkWin(board, lastRow, lastCol, lastPlayer)) {
                return (lastPlayer == 1) ? GameState::PLAYER1_WIN : GameState::PLAYER2_WIN;
            }
        }
        
        bool isFull = true;
        for (const auto& row : board) {
            for (int cell : row) {
                if (cell == 0) {
                    isFull = false;
                    break;
                }
            }
            if (!isFull) break;
        }
        
        return isFull ? GameState::DRAW : GameState::PLAYING;
    }
    
    static std::string gameStateToString(GameState state) {
        switch (state) {
            case GameState::PLAYING: return "PLAYING";
            case GameState::PLAYER1_WIN: return "PLAYER1_WIN";
            case GameState::PLAYER2_WIN: return "PLAYER2_WIN";
            case GameState::DRAW: return "DRAW";
            default: return "UNKNOWN";
        }
    }
    
    static std::string moveResultToString(MoveResult result) {
        switch (result) {
            case MoveResult::VALID: return "VALID";
            case MoveResult::OUT_OF_BOUNDS: return "OUT_OF_BOUNDS";
            case MoveResult::CELL_OCCUPIED: return "CELL_OCCUPIED";
            case MoveResult::INVALID_PLAYER: return "INVALID_PLAYER";
            default: return "UNKNOWN";
        }
    }
};

class AI {
public:
    enum class Difficulty { EASY = 2, MEDIUM = 4, HARD = 6 };
    
    struct MoveEvaluation {
        int row, col, score;
        MoveEvaluation(int r = -1, int c = -1, int s = 0) : row(r), col(c), score(s) {}
    };

private:
    int aiPlayer, humanPlayer;
    Difficulty difficulty;

public:
    AI(int ai = 2, Difficulty diff = Difficulty::MEDIUM) 
        : aiPlayer(ai), difficulty(diff), humanPlayer(ai == 1 ? 2 : 1) {}
    
    MoveEvaluation findBestMove(const std::vector<std::vector<int>>& board) {
        int size = board.size();
        
        bool isEmpty = true;
        for (const auto& row : board) {
            for (int cell : row) {
                if (cell != 0) {
                    isEmpty = false;
                    break;
                }
            }
            if (!isEmpty) break;
        }
        
        if (isEmpty) {
            return MoveEvaluation(size / 2, size / 2, 1000);
        }
        
        std::vector<MoveEvaluation> candidates = generateCandidates(board);
        if (candidates.empty()) {
            return MoveEvaluation();
        }
        
        MoveEvaluation bestMove;
        int maxDepth = static_cast<int>(difficulty);
        
        for (auto& candidate : candidates) {
            std::vector<std::vector<int>> tempBoard = board;
            tempBoard[candidate.row][candidate.col] = aiPlayer;
            
            if (GameLogic::checkWin(tempBoard, candidate.row, candidate.col, aiPlayer)) {
                return MoveEvaluation(candidate.row, candidate.col, 1000000);
            }
            
            int score = minimax(tempBoard, maxDepth - 1, false, -999999, 999999);
            if (score > bestMove.score) {
                bestMove = MoveEvaluation(candidate.row, candidate.col, score);
            }
        }
        
        return bestMove;
    }

private:
    std::vector<MoveEvaluation> generateCandidates(const std::vector<std::vector<int>>& board) {
        std::vector<MoveEvaluation> candidates;
        int size = board.size();
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] == 0) {
                    bool hasNearby = false;
                    for (int di = -2; di <= 2 && !hasNearby; di++) {
                        for (int dj = -2; dj <= 2 && !hasNearby; dj++) {
                            int ni = i + di, nj = j + dj;
                            if (ni >= 0 && ni < size && nj >= 0 && nj < size && board[ni][nj] != 0) {
                                hasNearby = true;
                            }
                        }
                    }
                    if (hasNearby) {
                        candidates.push_back(MoveEvaluation(i, j));
                    }
                }
            }
        }
        
        return candidates;
    }
    
    int minimax(std::vector<std::vector<int>>& board, int depth, bool isMaximizing, int alpha, int beta) {
        if (depth <= 0) {
            return evaluateBoard(board);
        }
        
        auto candidates = generateCandidates(board);
        if (candidates.empty()) return evaluateBoard(board);
        
        if (isMaximizing) {
            int maxEval = -999999;
            for (const auto& move : candidates) {
                board[move.row][move.col] = aiPlayer;
                
                if (GameLogic::checkWin(board, move.row, move.col, aiPlayer)) {
                    board[move.row][move.col] = 0;
                    return 999999 + depth;
                }
                
                int eval = minimax(board, depth - 1, false, alpha, beta);
                board[move.row][move.col] = 0;
                
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                
                if (beta <= alpha) break;
            }
            return maxEval;
        } else {
            int minEval = 999999;
            for (const auto& move : candidates) {
                board[move.row][move.col] = humanPlayer;
                
                if (GameLogic::checkWin(board, move.row, move.col, humanPlayer)) {
                    board[move.row][move.col] = 0;
                    return -999999 - depth;
                }
                
                int eval = minimax(board, depth - 1, true, alpha, beta);
                board[move.row][move.col] = 0;
                
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                
                if (beta <= alpha) break;
            }
            return minEval;
        }
    }
    
    int evaluateBoard(const std::vector<std::vector<int>>& board) {
        int aiScore = 0, humanScore = 0;
        int size = board.size();
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] != 0) {
                    int score = evaluatePosition(board, i, j, board[i][j]);
                    if (board[i][j] == aiPlayer) {
                        aiScore += score;
                    } else {
                        humanScore += score;
                    }
                }
            }
        }
        
        return aiScore - humanScore;
    }
    
    int evaluatePosition(const std::vector<std::vector<int>>& board, int row, int col, int player) {
        int score = 0;
        int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        
        for (int dir = 0; dir < 4; dir++) {
            int dx = directions[dir][0];
            int dy = directions[dir][1];
            
            int count = countLine(board, row, col, dx, dy, player);
            if (count >= 5) score += 100000;
            else if (count == 4) score += 10000;
            else if (count == 3) score += 1000;
            else if (count == 2) score += 100;
            else score += 10;
        }
        
        return score;
    }
    
    int countLine(const std::vector<std::vector<int>>& board, int row, int col, int dx, int dy, int player) {
        int size = board.size();
        int count = 1;
        
        int x = row + dx, y = col + dy;
        while (x >= 0 && x < size && y >= 0 && y < size && board[x][y] == player) {
            count++;
            x += dx;
            y += dy;
        }
        
        x = row - dx, y = col - dy;
        while (x >= 0 && x < size && y >= 0 && y < size && board[x][y] == player) {
            count++;
            x -= dx;
            y -= dy;
        }
        
        return count;
    }
};

class GameTester {
private:
    static int totalTests, passedTests;

public:
    static void assert_test(bool condition, const std::string& testName) {
        totalTests++;
        if (condition) {
            passedTests++;
            std::cout << "✅ " << testName << std::endl;
        } else {
            std::cout << "❌ " << testName << std::endl;
        }
    }
    
    static void print_summary() {
        std::cout << "\nTest Results: " << passedTests << "/" << totalTests 
                  << " (" << (100.0 * passedTests / totalTests) << "%)" << std::endl;
    }
    
    static void test_board() {
        std::cout << "\nTesting Board..." << std::endl;
        
        Board board(15);
        assert_test(board.getSize() == 15, "Board size");
        assert_test(board.isEmpty(), "Empty board");
        assert_test(board.getMoveCount() == 0, "Initial move count");
        
        assert_test(board.makeMove(7, 7, 1), "Valid move");
        assert_test(!board.makeMove(7, 7, 2), "Duplicate move");
        assert_test(!board.makeMove(-1, 5, 1), "Out of bounds");
        assert_test(!board.makeMove(5, 5, 3), "Invalid player");
        
        assert_test(board.getMoveCount() == 1, "Move count after move");
        assert_test(!board.isEmpty(), "Non-empty board");
        assert_test(board.getCell(7, 7) == 1, "Cell value");
        
        assert_test(board.undoLastMove(), "Undo move");
        assert_test(board.getMoveCount() == 0, "Move count after undo");
        assert_test(board.getCell(7, 7) == 0, "Cell after undo");
        
        assert_test(board.resize(25), "Resize board");
        assert_test(board.getSize() == 25, "Size after resize");
    }
    
    static void test_gamelogic() {
        std::cout << "\nTesting GameLogic..." << std::endl;
        
        Board board(15);
        auto& grid = board;
        
        auto result = GameLogic::validateMove({{0}}, 0, 0, 1);
        assert_test(result == GameLogic::MoveResult::VALID, "Valid move validation");
        
        board.makeMove(7, 5, 1);
        board.makeMove(7, 6, 1);
        board.makeMove(7, 7, 1);
        board.makeMove(7, 8, 1);
        board.makeMove(7, 9, 1);
        
        std::vector<std::vector<int>> testGrid(15, std::vector<int>(15, 0));
        for (int i = 0; i < 5; i++) {
            testGrid[7][5 + i] = 1;
        }
        
        assert_test(GameLogic::checkWin(testGrid, 7, 7, 1), "Win detection");
        
        auto state = GameLogic::checkGameState(testGrid, 7, 9);
        assert_test(state == GameLogic::GameState::PLAYER1_WIN, "Game state win");
    }
    
    static void test_ai() {
        std::cout << "\nTesting AI..." << std::endl;
        
        AI ai(2, AI::Difficulty::MEDIUM);
        Board board(15);
        board.makeMove(7, 7, 1);
        
        std::vector<std::vector<int>> grid(15, std::vector<int>(15, 0));
        grid[7][7] = 1;
        
        auto move = ai.findBestMove(grid);
        assert_test(move.row >= 0 && move.col >= 0, "AI move generation");
        assert_test(move.row < 15 && move.col < 15, "AI move bounds");
        
        board.reset();
        auto centerMove = ai.findBestMove(board.grid);
        assert_test(centerMove.row == 7 && centerMove.col == 7, "AI center opening");
    }
    
    static void benchmark() {
        std::cout << "\nBenchmarking..." << std::endl;
        
        auto start = std::chrono::high_resolution_clock::now();
        
        Board board(50);
        for (int i = 0; i < 1000; i++) {
            board.makeMove(i % 50, (i * 7) % 50, (i % 2) + 1);
        }
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        
        std::cout << "1000 moves on 50x50 board: " << duration.count() << "ms" << std::endl;
        std::cout << "Memory usage: " << board.getMemoryUsage() / 1024 << " KB" << std::endl;
    }
};

class ConsoleGame {
private:
    Board board;
    AI ai;
    bool vsAI;
    int currentPlayer;

public:
    ConsoleGame() : board(15), ai(2), vsAI(false), currentPlayer(1) {}
    
    void displayBoard() {
        int size = board.getSize();
        if (size > 20) {
            std::cout << "Board " << size << "x" << size << " too large to display" << std::endl;
            std::cout << "Moves: " << board.getMoveCount() << std::endl;
            return;
        }
        
        std::cout << "\n   ";
        for (int i = 0; i < size; i++) {
            std::cout << std::setw(3) << i;
        }
        std::cout << std::endl;
        
        for (int i = 0; i < size; i++) {
            std::cout << std::setw(2) << i << " ";
            for (int j = 0; j < size; j++) {
                int cell = board.getCell(i, j);
                if (cell == 0) std::cout << " · ";
                else if (cell == 1) std::cout << " X ";
                else std::cout << " O ";
            }
            std::cout << std::endl;
        }
    }
    
    void play() {
        std::cout << "Caro Game" << std::endl;
        std::cout << "1. Player vs Player" << std::endl;
        std::cout << "2. Player vs AI" << std::endl;
        
        int choice;
        std::cin >> choice;
        vsAI = (choice == 2);
        
        while (true) {
            displayBoard();
            
            auto [lastRow, lastCol, lastPlayer] = board.getLastMove();
            auto state = GameLogic::checkGameState(board.grid, lastRow, lastCol);
            
            if (state != GameLogic::GameState::PLAYING) {
                std::cout << "Game Over: " << GameLogic::gameStateToString(state) << std::endl;
                break;
            }
            
            if (currentPlayer == 1 || !vsAI) {
                std::cout << "Player " << currentPlayer << " (" 
                         << (currentPlayer == 1 ? "X" : "O") << ") move: ";
                int row, col;
                std::cin >> row >> col;
                
                if (board.makeMove(row, col, currentPlayer)) {
                    std::cout << "Move accepted" << std::endl;
                } else {
                    std::cout << "Invalid move" << std::endl;
                    continue;
                }
            } else {
                std::cout << "AI thinking..." << std::endl;
                auto move = ai.findBestMove(board.grid);
                
                if (move.row >= 0 && board.makeMove(move.row, move.col, currentPlayer)) {
                    std::cout << "AI played: " << move.row << " " << move.col << std::endl;
                }
            }
            
            currentPlayer = (currentPlayer == 1) ? 2 : 1;
        }
    }
};

int GameTester::totalTests = 0;
int GameTester::passedTests = 0;

int main() {
    std::cout << "Caro Game Core Library Test" << std::endl;
    std::cout << "===========================" << std::endl;
    
    std::cout << "1. Run Tests" << std::endl;
    std::cout << "2. Play Game" << std::endl;
    std::cout << "3. Run Benchmark" << std::endl;
    
    int choice;
    std::cin >> choice;
    
    switch (choice) {
        case 1:
            GameTester::test_board();
            GameTester::test_gamelogic();
            GameTester::test_ai();
            GameTester::print_summary();
            break;
            
        case 2: {
            ConsoleGame game;
            game.play();
            break;
        }
        
        case 3:
            GameTester::benchmark();
            break;
            
        default:
            std::cout << "Invalid choice" << std::endl;
            break;
    }
    
    return 0;
}
