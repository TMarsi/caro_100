// AI.cpp - Người 1: Trí tuệ nhân tạo (findBestMove)
#ifndef AI_H
#define AI_H

#include <vector>
#include <climits>
#include <algorithm>
#include <random>
#include <chrono>

// Forward declarations (sẽ include từ các file khác)
class Board;
class GameLogic;
enum class GameState;

struct Move {
    int row;
    int col;
    int score;
    
    Move() : row(-1), col(-1), score(INT_MIN) {}
    Move(int r, int c, int s = 0) : row(r), col(c), score(s) {}
};

enum class AILevel {
    EASY = 2,
    MEDIUM = 4, 
    HARD = 6
};

class AI {
private:
    int aiPlayer;           // 1 or 2
    int humanPlayer;        // 1 or 2  
    AILevel difficulty;
    std::mt19937 rng;
    
    // AI parameters
    static const int MAX_CANDIDATES = 20;
    static const int SEARCH_RADIUS = 2;
    
    // Scoring constants
    static const int WIN_SCORE = 1000000;
    static const int LOSE_SCORE = -1000000;
    
public:
    // Constructor
    AI(int aiPlayerNum = 2, AILevel level = AILevel::MEDIUM);
    
    // Main AI function
    Move findBestMove(const std::vector<std::vector<int>>& board);
    
    // Difficulty settings
    void setDifficulty(AILevel level) { difficulty = level; }
    AILevel getDifficulty() const { return difficulty; }
    
    // Player settings
    void setPlayers(int ai, int human) { aiPlayer = ai; humanPlayer = human; }
    
private:
    // Minimax algorithm
    int minimax(std::vector<std::vector<int>>& board, int depth, bool isMaximizing,
                int alpha, int beta, int lastRow = -1, int lastCol = -1);
    
    // Move generation and evaluation
    std::vector<Move> generateCandidateMoves(const std::vector<std::vector<int>>& board);
    std::vector<Move> getNeighborMoves(const std::vector<std::vector<int>>& board);
    std::vector<Move> getThreatMoves(const std::vector<std::vector<int>>& board);
    
    // Board evaluation
    int evaluateBoard(const std::vector<std::vector<int>>& board);
    int evaluatePlayerPosition(const std::vector<std::vector<int>>& board, int player);
    
    // Utility functions
    bool isGameOver(const std::vector<std::vector<int>>& board, int lastRow, int lastCol);
    Move getRandomMove(const std::vector<std::vector<int>>& board);
    Move getFirstMove(const std::vector<std::vector<int>>& board);
    
    // Move sorting for better alpha-beta pruning
    void sortMoves(std::vector<Move>& moves, const std::vector<std::vector<int>>& board);
    int quickEvaluateMove(const std::vector<std::vector<int>>& board, int row, int col, int player);
};

// Implementation
AI::AI(int aiPlayerNum, AILevel level) 
    : aiPlayer(aiPlayerNum), difficulty(level), 
      rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    humanPlayer = (aiPlayer == 1) ? 2 : 1;
}

Move AI::findBestMove(const std::vector<std::vector<int>>& board) {
    int size = board.size();
    
    // Check if board is empty - play center
    bool isEmpty = true;
    for (int i = 0; i < size && isEmpty; i++) {
        for (int j = 0; j < size && isEmpty; j++) {
            if (board[i][j] != 0) {
                isEmpty = false;
            }
        }
    }
    
    if (isEmpty) {
        return getFirstMove(board);
    }
    
    // Generate candidate moves
    std::vector<Move> candidates = generateCandidateMoves(board);
    
    if (candidates.empty()) {
        return getRandomMove(board);
    }
    
    // Check for immediate win
    for (const Move& move : candidates) {
        std::vector<std::vector<int>> tempBoard = board;
        tempBoard[move.row][move.col] = aiPlayer;
        
        if (GameLogic::checkWin(tempBoard, move.row, move.col, aiPlayer)) {
            return Move(move.row, move.col, WIN_SCORE);
        }
    }
    
    // Check for blocking opponent's win
    for (const Move& move : candidates) {
        std::vector<std::vector<int>> tempBoard = board;
        tempBoard[move.row][move.col] = humanPlayer;
        
        if (GameLogic::checkWin(tempBoard, move.row, move.col, humanPlayer)) {
            return Move(move.row, move.col, WIN_SCORE - 1);
        }
    }
    
    // Use minimax for deeper search
    Move bestMove;
    int maxDepth = static_cast<int>(difficulty);
    
    // Sort moves for better pruning
    sortMoves(candidates, board);
    
    for (Move& move : candidates) {
        std::vector<std::vector<int>> tempBoard = board;
        tempBoard[move.row][move.col] = aiPlayer;
        
        int score = minimax(tempBoard, maxDepth - 1, false, INT_MIN, INT_MAX, 
                           move.row, move.col);
        
        if (score > bestMove.score) {
            bestMove = Move(move.row, move.col, score);
        }
    }
    
    return bestMove;
}

int AI::minimax(std::vector<std::vector<int>>& board, int depth, bool isMaximizing,
                int alpha, int beta, int lastRow, int lastCol) {
    
    // Check terminal conditions
    if (lastRow >= 0 && lastCol >= 0) {
        if (GameLogic::checkWin(board, lastRow, lastCol, board[lastRow][lastCol])) {
            int winner = board[lastRow][lastCol];
            if (winner == aiPlayer) {
                return WIN_SCORE + depth;  // Win sooner is better
            } else {
                return LOSE_SCORE - depth; // Lose later is better
            }
        }
    }
    
    if (depth == 0) {
        return evaluateBoard(board);
    }
    
    std::vector<Move> moves = generateCandidateMoves(board);
    if (moves.empty()) {
        return evaluateBoard(board);
    }
    
    // Sort moves for better pruning
    sortMoves(moves, board);
    
    if (isMaximizing) {
        int maxEval = INT_MIN;
        
        for (const Move& move : moves) {
            board[move.row][move.col] = aiPlayer;
            
            int eval = minimax(board, depth - 1, false, alpha, beta, 
                             move.row, move.col);
            
            board[move.row][move.col] = 0;  // Undo move
            
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            
            if (beta <= alpha) {
                break;  // Alpha-beta pruning
            }
        }
        return maxEval;
        
    } else {
        int minEval = INT_MAX;
        
        for (const Move& move : moves) {
            board[move.row][move.col] = humanPlayer;
            
            int eval = minimax(board, depth - 1, true, alpha, beta,
                             move.row, move.col);
            
            board[move.row][move.col] = 0;  // Undo move
            
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            
            if (beta <= alpha) {
                break;  // Alpha-beta pruning
            }
        }
        return minEval;
    }
}

std::vector<Move> AI::generateCandidateMoves(const std::vector<std::vector<int>>& board) {
    std::vector<Move> candidates;
    
    // Get threat moves (high priority)
    std::vector<Move> threats = getThreatMoves(board);
    candidates.insert(candidates.end(), threats.begin(), threats.end());
    
    // Get neighbor moves
    std::vector<Move> neighbors = getNeighborMoves(board);
    candidates.insert(candidates.end(), neighbors.begin(), neighbors.end());
    
    // Remove duplicates and limit candidates
    std::sort(candidates.begin(), candidates.end(), 
              [](const Move& a, const Move& b) {
                  if (a.row != b.row) return a.row < b.row;
                  return a.col < b.col;
              });
    
    candidates.erase(std::unique(candidates.begin(), candidates.end(),
                                [](const Move& a, const Move& b) {
                                    return a.row == b.row && a.col == b.col;
                                }), candidates.end());
    
    // Limit number of candidates for performance
    if (candidates.size() > MAX_CANDIDATES) {
        candidates.resize(MAX_CANDIDATES);
    }
    
    return candidates;
}

std::vector<Move> AI::getNeighborMoves(const std::vector<std::vector<int>>& board) {
    std::vector<Move> neighbors;
    int size = board.size();
    std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] != 0) {
                // Add neighbors of existing pieces
                for (int di = -SEARCH_RADIUS; di <= SEARCH_RADIUS; di++) {
                    for (int dj = -SEARCH_RADIUS; dj <= SEARCH_RADIUS; dj++) {
                        int ni = i + di;
                        int nj = j + dj;
                        
                        if (ni >= 0 && ni < size && nj >= 0 && nj < size && 
                            board[ni][nj] == 0 && !visited[ni][nj]) {
                            neighbors.push_back(Move(ni, nj));
                            visited[ni][nj] = true;
                        }
                    }
                }
            }
        }
    }
    
    return neighbors;
}

std::vector<Move> AI::getThreatMoves(const std::vector<std::vector<int>>& board) {
    std::vector<Move> threats;
    int size = board.size();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                // Check if this move creates a threat for AI
                if (GameLogic::hasWinningThreat(board, i, j, aiPlayer)) {
                    threats.push_back(Move(i, j, WIN_SCORE));
                }
                // Check if this move blocks opponent's threat
                else if (GameLogic::hasBlockingThreat(board, i, j, aiPlayer)) {
                    threats.push_back(Move(i, j, WIN_SCORE - 1));
                }
            }
        }
    }
    
    return threats;
}

int AI::evaluateBoard(const std::vector<std::vector<int>>& board) {
    int aiScore = evaluatePlayerPosition(board, aiPlayer);
    int humanScore = evaluatePlayerPosition(board, humanPlayer);
    
    return aiScore - humanScore;
}

int AI::evaluatePlayerPosition(const std::vector<std::vector<int>>& board, int player) {
    int totalScore = 0;
    int size = board.size();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                totalScore += GameLogic::evaluatePosition(board, i, j, player);
            }
        }
    }
    
    return totalScore;
}

void AI::sortMoves(std::vector<Move>& moves, const std::vector<std::vector<int>>& board) {
    for (Move& move : moves) {
        move.score = quickEvaluateMove(board, move.row, move.col, aiPlayer);
    }
    
    std::sort(moves.begin(), moves.end(), 
              [](const Move& a, const Move& b) {
                  return a.score > b.score;
              });
}

int AI::quickEvaluateMove(const std::vector<std::vector<int>>& board, int row, int col, int player) {
    return GameLogic::evaluatePosition(board, row, col, player);
}

Move AI::getFirstMove(const std::vector<std::vector<int>>& board) {
    int size = board.size();
    return Move(size / 2, size / 2);  // Center of board
}

Move AI::getRandomMove(const std::vector<std::vector<int>>& board) {
    int size = board.size();
    std::vector<Move> availableMoves;
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                availableMoves.push_back(Move(i, j));
            }
        }
    }
    
    if (availableMoves.empty()) {
        return Move();  // No moves available
    }
    
    std::uniform_int_distribution<int> dist(0, availableMoves.size() - 1);
    return availableMoves[dist(rng)];
}

#endif // AI_H
