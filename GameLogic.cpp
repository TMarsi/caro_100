// GameLogic.cpp - Người 1: Kiểm tra thắng/thua
#ifndef GAMELOGIC_H
#define GAMELOGIC_H

#include <vector>
#include <algorithm>

enum class GameState {
    PLAYING,
    PLAYER1_WIN,  // X wins
    PLAYER2_WIN,  // O wins
    DRAW
};

class GameLogic {
private:
    static const int WIN_LENGTH = 5;  // Cần 5 quân liên tiếp để thắng
    
    // Directions for checking: horizontal, vertical, diagonal1, diagonal2
    static const int DIRECTIONS[4][2];
    
public:
    // Main win checking functions
    static bool checkWin(const std::vector<std::vector<int>>& board, int row, int col, int player);
    static GameState checkGameState(const std::vector<std::vector<int>>& board, int lastRow, int lastCol);
    
    // Utility functions for AI
    static int countConsecutive(const std::vector<std::vector<int>>& board, 
                               int row, int col, int dx, int dy, int player);
    static int evaluatePosition(const std::vector<std::vector<int>>& board, 
                               int row, int col, int player);
    static int evaluateLine(const std::vector<std::vector<int>>& board,
                           int row, int col, int dx, int dy, int player);
    
    // Pattern recognition
    static bool hasWinningThreat(const std::vector<std::vector<int>>& board, 
                                int row, int col, int player);
    static bool hasBlockingThreat(const std::vector<std::vector<int>>& board, 
                                 int row, int col, int player);
    
    // Board analysis
    static int getPatternScore(int consecutive, int openEnds);
    static bool isValidPosition(const std::vector<std::vector<int>>& board, int row, int col);
};

// Static member initialization
const int GameLogic::DIRECTIONS[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};

bool GameLogic::isValidPosition(const std::vector<std::vector<int>>& board, int row, int col) {
    int size = board.size();
    return row >= 0 && row < size && col >= 0 && col < size;
}

int GameLogic::countConsecutive(const std::vector<std::vector<int>>& board, 
                               int row, int col, int dx, int dy, int player) {
    int size = board.size();
    int count = 0;
    int x = row, y = col;
    
    while (isValidPosition(board, x, y) && board[x][y] == player) {
        count++;
        x += dx;
        y += dy;
    }
    
    return count;
}

bool GameLogic::checkWin(const std::vector<std::vector<int>>& board, int row, int col, int player) {
    if (!isValidPosition(board, row, col) || board[row][col] != player) {
        return false;
    }
    
    // Check all 4 directions
    for (int dir = 0; dir < 4; dir++) {
        int dx = DIRECTIONS[dir][0];
        int dy = DIRECTIONS[dir][1];
        
        // Count in positive direction
        int positiveCount = countConsecutive(board, row + dx, col + dy, dx, dy, player);
        
        // Count in negative direction  
        int negativeCount = countConsecutive(board, row - dx, col - dy, -dx, -dy, player);
        
        // Total count including current position
        int totalCount = positiveCount + negativeCount + 1;
        
        if (totalCount >= WIN_LENGTH) {
            return true;
        }
    }
    
    return false;
}

GameState GameLogic::checkGameState(const std::vector<std::vector<int>>& board, int lastRow, int lastCol) {
    int size = board.size();
    
    // Check if last move resulted in a win
    if (lastRow >= 0 && lastCol >= 0) {
        int lastPlayer = board[lastRow][lastCol];
        if (checkWin(board, lastRow, lastCol, lastPlayer)) {
            return (lastPlayer == 1) ? GameState::PLAYER1_WIN : GameState::PLAYER2_WIN;
        }
    }
    
    // Check for draw (board full)
    bool isFull = true;
    for (int i = 0; i < size && isFull; i++) {
        for (int j = 0; j < size && isFull; j++) {
            if (board[i][j] == 0) {
                isFull = false;
            }
        }
    }
    
    if (isFull) {
        return GameState::DRAW;
    }
    
    return GameState::PLAYING;
}

int GameLogic::evaluateLine(const std::vector<std::vector<int>>& board,
                           int row, int col, int dx, int dy, int player) {
    int size = board.size();
    
    // Count consecutive pieces in both directions
    int positiveCount = countConsecutive(board, row + dx, col + dy, dx, dy, player);
    int negativeCount = countConsecutive(board, row - dx, col - dy, -dx, -dy, player);
    int totalConsecutive = positiveCount + negativeCount + 1;
    
    // Count open ends
    int openEnds = 0;
    
    // Check positive end
    int posEndX = row + dx * (positiveCount + 1);
    int posEndY = col + dy * (positiveCount + 1);
    if (isValidPosition(board, posEndX, posEndY) && board[posEndX][posEndY] == 0) {
        openEnds++;
    }
    
    // Check negative end  
    int negEndX = row - dx * (negativeCount + 1);
    int negEndY = col - dy * (negativeCount + 1);
    if (isValidPosition(board, negEndX, negEndY) && board[negEndX][negEndY] == 0) {
        openEnds++;
    }
    
    return getPatternScore(totalConsecutive, openEnds);
}

int GameLogic::evaluatePosition(const std::vector<std::vector<int>>& board, 
                               int row, int col, int player) {
    if (!isValidPosition(board, row, col)) {
        return 0;
    }
    
    int totalScore = 0;
    
    // Evaluate all 4 directions
    for (int dir = 0; dir < 4; dir++) {
        int dx = DIRECTIONS[dir][0];
        int dy = DIRECTIONS[dir][1];
        totalScore += evaluateLine(board, row, col, dx, dy, player);
    }
    
    return totalScore;
}

int GameLogic::getPatternScore(int consecutive, int openEnds) {
    // Scoring based on consecutive pieces and open ends
    if (consecutive >= 5) {
        return 1000000;  // Win
    }
    
    if (consecutive == 4) {
        if (openEnds == 2) return 100000;  // Open four
        if (openEnds == 1) return 10000;   // Semi-open four
    }
    
    if (consecutive == 3) {
        if (openEnds == 2) return 10000;   // Open three
        if (openEnds == 1) return 1000;    // Semi-open three
    }
    
    if (consecutive == 2) {
        if (openEnds == 2) return 1000;    // Open two
        if (openEnds == 1) return 100;     // Semi-open two
    }
    
    if (consecutive == 1) {
        if (openEnds == 2) return 100;     // Open one
        if (openEnds == 1) return 10;      // Semi-open one
    }
    
    return 0;
}

bool GameLogic::hasWinningThreat(const std::vector<std::vector<int>>& board, 
                                int row, int col, int player) {
    // Temporary place the piece
    std::vector<std::vector<int>> tempBoard = board;
    tempBoard[row][col] = player;
    
    return checkWin(tempBoard, row, col, player);
}

bool GameLogic::hasBlockingThreat(const std::vector<std::vector<int>>& board, 
                                 int row, int col, int player) {
    int opponent = (player == 1) ? 2 : 1;
    
    // Check if placing here blocks opponent's winning move
    for (int dir = 0; dir < 4; dir++) {
        int dx = DIRECTIONS[dir][0];
        int dy = DIRECTIONS[dir][1];
        
        // Check both directions from this position
        for (int direction = -1; direction <= 1; direction += 2) {
            int checkX = row + dx * direction;
            int checkY = col + dy * direction;
            
            if (isValidPosition(board, checkX, checkY) && board[checkX][checkY] == opponent) {
                // Temporarily place opponent piece at current position
                std::vector<std::vector<int>> tempBoard = board;
                tempBoard[row][col] = opponent;
                
                if (checkWin(tempBoard, row, col, opponent)) {
                    return true;  // This move blocks opponent's win
                }
            }
        }
    }
    
    return false;
}

#endif // GAMELOGIC_H
