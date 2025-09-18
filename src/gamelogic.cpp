// GameLogic.cpp - Game Rules Engine
// Người 1: Logic & AI - Thuật toán kiểm tra luật chơi và điều kiện thắng/thua
#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>
#include <algorithm>
#include <array>

/**
 * @class GameLogic  
 * @brief Pure algorithm library for game rules validation
 * 
 * Cung cấp API hoàn chỉnh cho:
 * - Người 2 (Graphics): Hiển thị trạng thái game  
 * - Người 3 (GameController): Điều khiển luồng game
 * - AI system: Đánh giá position và pattern
 */
class GameLogic {
public:
    // Game constants
    static const int WIN_LENGTH = 5;  // Cần 5 quân liên tiếp để thắng
    static const int MAX_PLAYERS = 2;
    
    // Game state enumeration
    enum class GameState {
        PLAYING,      // Game đang diễn ra
        PLAYER1_WIN,  // Player 1 (X) thắng
        PLAYER2_WIN,  // Player 2 (O) thắng  
        DRAW          // Hòa (bàn cờ đầy)
    };
    
    // Move validation result
    enum class MoveResult {
        VALID,           // Nước đi hợp lệ
        OUT_OF_BOUNDS,   // Ngoài bàn cờ
        CELL_OCCUPIED,   // Ô đã có quân
        INVALID_PLAYER   // Player không hợp lệ
    };
    
    // Pattern types for AI evaluation
    enum class PatternType {
        NONE,
        SINGLE,          // 1 quân
        PAIR,            // 2 quân liên tiếp
        THREE_OPEN,      // 3 quân mở 2 đầu
        THREE_SEMI,      // 3 quân mở 1 đầu
        FOUR_OPEN,       // 4 quân mở 2 đầu (thắng chắc)
        FOUR_SEMI,       // 4 quân mở 1 đầu
        FIVE             // 5 quân (thắng)
    };
    
private:
    // Search directions: horizontal, vertical, diagonal1, diagonal2  
    static const std::array<std::pair<int, int>, 4> DIRECTIONS;
    
public:
    // === CORE GAME RULE VALIDATION ===
    
    /**
     * @brief Validate if move is legal according to game rules
     * @param board Game board state
     * @param row Target row
     * @param col Target column  
     * @param player Player making move (1 or 2)
     * @return MoveResult indicating validity
     */
    static MoveResult validateMove(const std::vector<std::vector<int>>& board, 
                                  int row, int col, int player);
    
    /**
     * @brief Check if game has ended and determine winner
     * @param board Current board state
     * @param lastRow Last move row (-1 if no moves)
     * @param lastCol Last move column (-1 if no moves) 
     * @return GameState indicating current game status
     */
    static GameState checkGameState(const std::vector<std::vector<int>>& board, 
                                   int lastRow = -1, int lastCol = -1);
    
    /**
     * @brief Check if specific position results in win
     * @param board Board state
     * @param row Position row
     * @param col Position column
     * @param player Player to check
     * @return true if position creates winning condition
     */
    static bool checkWinAtPosition(const std::vector<std::vector<int>>& board,
                                  int row, int col, int player);
    
    // === AI EVALUATION FUNCTIONS ===
    
    /**
     * @brief Evaluate position strength for AI decision making
     * @param board Board state
     * @param row Position row
     * @param col Position column
     * @param player Player to evaluate for
     * @return Score value (higher = better for player)
     */
    static int evaluatePosition(const std::vector<std::vector<int>>& board,
                               int row, int col, int player);
    
    /**
     * @brief Evaluate entire board for given player
     * @param board Board state  
     * @param player Player to evaluate
     * @return Overall board score for player
     */
    static int evaluateBoard(const std::vector<std::vector<int>>& board, int player);
    
    /**
     * @brief Get pattern type at specific position and direction
     * @param board Board state
     * @param row Starting row
     * @param col Starting column
     * @param dx Direction x component
     * @param dy Direction y component  
     * @param player Player to check
     * @return PatternType found
     */
    static PatternType getPattern(const std::vector<std::vector<int>>& board,
                                 int row, int col, int dx, int dy, int player);
    
    // === THREAT DETECTION ===
    
    /**
     * @brief Check if position creates immediate winning threat
     * @param board Board state
     * @param row Position row
     * @param col Position column
     * @param player Player to check
     * @return true if creates winning threat
     */
    static bool isWinningThreat(const std::vector<std::vector<int>>& board,
                               int row, int col, int player);
    
    /**
     * @brief Check if position blocks opponent's winning threat
     * @param board Board state
     * @param row Position row  
     * @param col Position column
     * @param player Player making defensive move
     * @return true if blocks opponent threat
     */
    static bool isBlockingThreat(const std::vector<std::vector<int>>& board,
                                int row, int col, int player);
    
    /**
     * @brief Find all immediate threats on board
     * @param board Board state
     * @param player Player to find threats for
     * @return Vector of threat positions
     */
    static std::vector<std::pair<int, int>> findThreats(const std::vector<std::vector<int>>& board,
                                                        int player);
    
    // === UTILITY FUNCTIONS ===
    
    /**
     * @brief Count consecutive pieces in direction from position
     * @param board Board state
     * @param row Starting row
     * @param col Starting column
     * @param dx Direction x
     * @param dy Direction y
     * @param player Player to count
     * @return Number of consecutive pieces
     */
    static int countConsecutive(const std::vector<std::vector<int>>& board,
                               int row, int col, int dx, int dy, int player);
    
    /**
     * @brief Count total pieces in line (both directions)
     * @param board Board state
     * @param row Center row
     * @param col Center column  
     * @param dx Direction x
     * @param dy Direction y
     * @param player Player to count
     * @return Total pieces in line including center
     */
    static int countInLine(const std::vector<std::vector<int>>& board,
                          int row, int col, int dx, int dy, int player);
    
    /**
     * @brief Convert GameState to string for logging/display
     */
    static std::string gameStateToString(GameState state);
    
    /**
     * @brief Convert PatternType to string for debugging
     */
    static std::string patternTypeToString(PatternType pattern);
    
    /**
     * @brief Get score value for pattern type
     */
    static int getPatternScore(PatternType pattern);

private:
    // === INTERNAL HELPERS ===
    static bool isValidPosition(const std::vector<std::vector<int>>& board, int row, int col);
    static int countOpenEnds(const std::vector<std::vector<int>>& board,
                            int row, int col, int dx, int dy, int consecutiveCount, int player);
    static PatternType classifyPattern(int consecutiveCount, int openEnds);
};

// === STATIC MEMBER DEFINITIONS ===

const std::array<std::pair<int, int>, 4> GameLogic::DIRECTIONS = {{
    {0, 1},   // Horizontal
    {1, 0},   // Vertical  
    {1, 1},   // Diagonal /
    {1, -1}   // Diagonal \
}};

// === IMPLEMENTATION ===

GameLogic::MoveResult GameLogic::validateMove(const std::vector<std::vector<int>>& board,
                                             int row, int col, int player) {
    // Check player validity
    if (player != 1 && player != 2) {
        return MoveResult::INVALID_PLAYER;
    }
    
    // Check bounds
    if (!isValidPosition(board, row, col)) {
        return MoveResult::OUT_OF_BOUNDS;
    }
    
    // Check if cell is empty
    if (board[row][col] != 0) {
        return MoveResult::CELL_OCCUPIED;
    }
    
    return MoveResult::VALID;
}

GameLogic::GameState GameLogic::checkGameState(const std::vector<std::vector<int>>& board,
                                              int lastRow, int lastCol) {
    // Check for win if last move is provided
    if (lastRow >= 0 && lastCol >= 0 && isValidPosition(board, lastRow, lastCol)) {
        int lastPlayer = board[lastRow][lastCol];
        if (lastPlayer != 0 && checkWinAtPosition(board, lastRow, lastCol, lastPlayer)) {
            return (lastPlayer == 1) ? GameState::PLAYER1_WIN : GameState::PLAYER2_WIN;
        }
    }
    
    // Check for draw (board full)
    bool boardFull = true;
    for (const auto& row : board) {
        for (int cell : row) {
            if (cell == 0) {
                boardFull = false;
                break;
            }
        }
        if (!boardFull) break;
    }
    
    if (boardFull) {
        return GameState::DRAW;
    }
    
    return GameState::PLAYING;
}

bool GameLogic::checkWinAtPosition(const std::vector<std::vector<int>>& board,
                                  int row, int col, int player) {
    if (!isValidPosition(board, row, col) || board[row][col] != player) {
        return false;
    }
    
    // Check all 4 directions
    for (const auto& [dx, dy] : DIRECTIONS) {
        int totalCount = countInLine(board, row, col, dx, dy, player);
        if (totalCount >= WIN_LENGTH) {
            return true;
        }
    }
    
    return false;
}

int GameLogic::evaluatePosition(const std::vector<std::vector<int>>& board,
                               int row, int col, int player) {
    if (!isValidPosition(board, row, col)) {
        return 0;
    }
    
    int totalScore = 0;
    
    // Evaluate all directions
    for (const auto& [dx, dy] : DIRECTIONS) {
        PatternType pattern = getPattern(board, row, col, dx, dy, player);
        totalScore += getPatternScore(pattern);
    }
    
    return totalScore;
}

int GameLogic::evaluateBoard(const std::vector<std::vector<int>>& board, int player) {
    int totalScore = 0;
    int size = board.size();
    
    // Evaluate all positions where player could potentially play
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                totalScore += evaluatePosition(board, i, j, player);
            }
        }
    }
    
    return totalScore;
}

GameLogic::PatternType GameLogic::getPattern(const std::vector<std::vector<int>>& board,
                                           int row, int col, int dx, int dy, int player) {
    // Count consecutive pieces in line
    int consecutiveCount = countInLine(board, row, col, dx, dy, player);
    
    // Count open ends
    int openEnds = countOpenEnds(board, row, col, dx, dy, consecutiveCount, player);
    
    return classifyPattern(consecutiveCount, openEnds);
}

bool GameLogic::isWinningThreat(const std::vector<std::vector<int>>& board,
                               int row, int col, int player) {
    if (!isValidPosition(board, row, col) || board[row][col] != 0) {
        return false;
    }
    
    // Simulate placing the piece
    std::vector<std::vector<int>> tempBoard = board;
    tempBoard[row][col] = player;
    
    return checkWinAtPosition(tempBoard, row, col, player);
}

bool GameLogic::isBlockingThreat(const std::vector<std::vector<int>>& board,
                                int row, int col, int player) {
    if (!isValidPosition(board, row, col) || board[row][col] != 0) {
        return false;
    }
    
    int opponent = (player == 1) ? 2 : 1;
    
    // Check if opponent would win by playing here
    std::vector<std::vector<int>> tempBoard = board;
    tempBoard[row][col] = opponent;
    
    return checkWinAtPosition(tempBoard, row, col, opponent);
}

std::vector<std::pair<int, int>> GameLogic::findThreats(const std::vector<std::vector<int>>& board,
                                                        int player) {
    std::vector<std::pair<int, int>> threats;
    int size = board.size();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                if (isWinningThreat(board, i, j, player)) {
                    threats.emplace_back(i, j);
                }
            }
        }
    }
    
    return threats;
}

int GameLogic::countConsecutive(const std::vector<std::vector<int>>& board,
                               int row, int col, int dx, int dy, int player) {
    int count = 0;
    int size = board.size();
    
    while (isValidPosition(board, row, col) && board[row][col] == player) {
        count++;
        row += dx;
        col += dy;
    }
    
    return count;
}

int GameLogic::countInLine(const std::vector<std::vector<int>>& board,
                          int row, int col, int dx, int dy, int player) {
    // Count in positive direction (not including center)
    int positiveCount = countConsecutive(board, row + dx, col + dy, dx, dy, player);
    
    // Count in negative direction (not including center)
    int negativeCount = countConsecutive(board, row - dx, col - dy, -dx, -dy, player);
    
    // Total including center (if it matches player)
    int centerCount = (isValidPosition(board, row, col) && board[row][col] == player) ? 1 : 0;
    
    return positiveCount + negativeCount + centerCount;
}

std::string GameLogic::gameStateToString(GameState state) {
    switch (state) {
        case GameState::PLAYING: return "PLAYING";
        case GameState::PLAYER1_WIN: return "PLAYER1_WIN";
        case GameState::PLAYER2_WIN: return "PLAYER2_WIN";
        case GameState::DRAW: return "DRAW";
        default: return "UNKNOWN";
    }
}

std::string GameLogic::patternTypeToString(PatternType pattern) {
    switch (pattern) {
        case PatternType::NONE: return "NONE";
        case PatternType::SINGLE: return "SINGLE";
        case PatternType::PAIR: return "PAIR";
        case PatternType::THREE_OPEN: return "THREE_OPEN";
        case PatternType::THREE_SEMI: return "THREE_SEMI";
        case PatternType::FOUR_OPEN: return "FOUR_OPEN";
        case PatternType::FOUR_SEMI: return "FOUR_SEMI";
        case PatternType::FIVE: return "FIVE";
        default: return "UNKNOWN";
    }
}

int GameLogic::getPatternScore(PatternType pattern) {
    switch (pattern) {
        case PatternType::NONE: return 0;
        case PatternType::SINGLE: return 1;
        case PatternType::PAIR: return 10;
        case PatternType::THREE_SEMI: return 100;
        case PatternType::THREE_OPEN: return 1000;
        case PatternType::FOUR_SEMI: return 10000;
        case PatternType::FOUR_OPEN: return 100000;
        case PatternType::FIVE: return 1000000;
        default: return 0;
    }
}

// === PRIVATE HELPER IMPLEMENTATIONS ===

bool GameLogic::isValidPosition(const std::vector<std::vector<int>>& board, int row, int col) {
    int size = board.size();
    return row >= 0 && row < size && col >= 0 && col < size;
}

int GameLogic::countOpenEnds(const std::vector<std::vector<int>>& board,
                            int row, int col, int dx, int dy, int consecutiveCount, int player) {
    int openEnds = 0;
    
    // Check positive direction end
    int posRow = row + dx * (consecutiveCount / 2 + 1);
    int posCol = col + dy * (consecutiveCount / 2 + 1);
    
    // Walk to actual end of consecutive sequence
    while (isValidPosition(board, posRow - dx, posCol - dy) && 
           board[posRow - dx][posCol - dy] == player) {
        posRow += dx;
        posCol += dy;
    }
    
    if (isValidPosition(board, posRow, posCol) && board[posRow][posCol] == 0) {
        openEnds++;
    }
    
    // Check negative direction end  
    int negRow = row - dx * (consecutiveCount / 2 + 1);
    int negCol = col - dy * (consecutiveCount / 2 + 1);
    
    // Walk to actual end of consecutive sequence
    while (isValidPosition(board, negRow + dx, negCol + dy) && 
           board[negRow + dx][negCol + dy] == player) {
        negRow -= dx;
        negCol -= dy;
    }
    
    if (isValidPosition(board, negRow, negCol) && board[negRow][negCol] == 0) {
        openEnds++;
    }
    
    return openEnds;
}

GameLogic::PatternType GameLogic::classifyPattern(int consecutiveCount, int openEnds) {
    if (consecutiveCount >= 5) {
        return PatternType::FIVE;
    }
    
    if (consecutiveCount == 4) {
        if (openEnds >= 2) return PatternType::FOUR_OPEN;
        if (openEnds == 1) return PatternType::FOUR_SEMI;
    }
    
    if (consecutiveCount == 3) {
        if (openEnds >= 2) return PatternType::THREE_OPEN;
        if (openEnds == 1) return PatternType::THREE_SEMI;
    }
    
    if (consecutiveCount == 2) {
        return PatternType::PAIR;
    }
    
    if (consecutiveCount == 1) {
        return PatternType::SINGLE;
    }
    
    return PatternType::NONE;
}

#endif // GAMELOGIC_H
