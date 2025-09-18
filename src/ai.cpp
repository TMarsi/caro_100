// AI.cpp - Pure AI Algorithm Library
// Người 1: Logic & AI - Thuật toán AI thuần túy không liên quan đồ họa
#ifndef AI_H
#define AI_H

#include <vector>
#include <climits>
#include <algorithm>
#include <random>
#include <chrono>
#include <memory>
#include <functional>

// Forward declarations (sẽ include các file khác)
class Board;
class GameLogic;

/**
 * @class AI
 * @brief Pure algorithm library for AI decision making
 * 
 * Cung cấp API hoàn chỉnh cho:
 * - Người 3 (GameController): Lấy AI moves
 * - Người 2 (Graphics): Hiển thị AI thinking process
 * - System: Configurable AI difficulty và behavior
 */
class AI {
public:
    // AI Difficulty levels
    enum class Difficulty {
        BEGINNER = 1,    // Depth 1, basic heuristics
        EASY = 2,        // Depth 2, some randomness
        MEDIUM = 4,      // Depth 4, good evaluation
        HARD = 6,        // Depth 6, advanced patterns
        EXPERT = 8       // Depth 8, maximum optimization
    };
    
    // AI Personality/Style
    enum class PlayStyle {
        AGGRESSIVE,      // Prioritize attack
        DEFENSIVE,       // Prioritize blocking
        BALANCED,        // Equal attack/defense
        POSITIONAL       // Focus on board control
    };
    
    // Move evaluation result
    struct MoveEvaluation {
        int row;
        int col;
        int score;
        int depth;
        bool isWinning;
        bool isBlocking;
        std::string reasoning;  // For debugging/display
        
        MoveEvaluation() : row(-1), col(-1), score(INT_MIN), depth(0), 
                          isWinning(false), isBlocking(false) {}
        
        MoveEvaluation(int r, int c, int s) : row(r), col(c), score(s), depth(0),
                                             isWinning(false), isBlocking(false) {}
    };
    
    // AI thinking statistics
    struct ThinkingStats {
        int nodesEvaluated;
        int pruningCount;
        int maxDepthReached;
        double timeElapsed;
        std::vector<MoveEvaluation> topMoves;
        
        ThinkingStats() : nodesEvaluated(0), pruningCount(0), maxDepthReached(0), timeElapsed(0.0) {}
    };

private:
    // AI configuration
    int aiPlayer;           // 1 or 2
    int humanPlayer;        // 1 or 2
    Difficulty difficulty;
    PlayStyle playStyle;
    
    // Algorithm parameters
    int maxDepth;
    int maxCandidates;
    double timeLimit;       // Max thinking time in seconds
    bool useIterativeDeepening;
    bool useTranspositionTable;
    
    // Statistics tracking
    mutable ThinkingStats lastStats;
    
    // Random number generator
    mutable std::mt19937 rng;
    
    // Transposition table (for advanced optimization)
    struct TranspositionEntry {
        uint64_t hash;
        int score;
        int depth;
        enum Type { EXACT, LOWER, UPPER } type;
    };
    
    mutable std::vector<TranspositionEntry> transpositionTable;
    static const size_t TABLE_SIZE = 1048576; // 1M entries

public:
    // === CONSTRUCTORS & CONFIGURATION ===
    
    /**
     * @brief Create AI with specified parameters
     * @param aiPlayerNum Player number for AI (1 or 2)
     * @param diff Difficulty level
     * @param style Play style
     */
    AI(int aiPlayerNum = 2, Difficulty diff = Difficulty::MEDIUM, 
       PlayStyle style = PlayStyle::BALANCED);
    
    // === MAIN AI INTERFACE ===
    
    /**
     * @brief Find best move for current board position
     * @param board Current board state
     * @return Best move evaluation
     */
    MoveEvaluation findBestMove(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Find best move with time limit
     * @param board Current board state
     * @param maxTimeMs Maximum thinking time in milliseconds
     * @return Best move within time limit
     */
    MoveEvaluation findBestMoveWithTime(const std::vector<std::vector<int>>& board, 
                                       int maxTimeMs);
    
    /**
     * @brief Get multiple top moves for analysis
     * @param board Current board state
     * @param count Number of top moves to return
     * @return Vector of top move evaluations
     */
    std::vector<MoveEvaluation> getTopMoves(const std::vector<std::vector<int>>& board, 
                                           int count = 5);
    
    // === CONFIGURATION ===
    
    void setDifficulty(Difficulty diff);
    void setPlayStyle(PlayStyle style);
    void setPlayerNumbers(int ai, int human);
    void setTimeLimit(double seconds) { timeLimit = seconds; }
    void setIterativeDeepening(bool enable) { useIterativeDeepening = enable; }
    
    // === GETTERS ===
    
    Difficulty getDifficulty() const { return difficulty; }
    PlayStyle getPlayStyle() const { return playStyle; }
    const ThinkingStats& getLastThinkingStats() const { return lastStats; }
    int getAIPlayer() const { return aiPlayer; }
    int getHumanPlayer() const { return humanPlayer; }
    
    // === ANALYSIS FUNCTIONS ===
    
    /**
     * @brief Evaluate board position for given player
     * @param board Board state
     * @param player Player to evaluate for
     * @return Position score
     */
    int evaluateBoardPosition(const std::vector<std::vector<int>>& board, int player);
    
    /**
     * @brief Quick evaluation of a specific move
     * @param board Board state
     * @param row Move row
     * @param col Move column
     * @param player Player making move
     * @return Move score
     */
    int quickEvaluateMove(const std::vector<std::vector<int>>& board,
                         int row, int col, int player);
    
    /**
     * @brief Check if position is critical (win/block)
     * @param board Board state
     * @param row Position row  
     * @param col Position column
     * @return true if position is critical
     */
    bool isCriticalPosition(const std::vector<std::vector<int>>& board,
                           int row, int col);

private:
    // === CORE ALGORITHMS ===
    
    /**
     * @brief Minimax algorithm with alpha-beta pruning
     */
    int minimax(std::vector<std::vector<int>>& board, int depth, bool isMaximizing,
                int alpha, int beta, int lastRow = -1, int lastCol = -1);
    
    /**
     * @brief Iterative deepening search
     */
    MoveEvaluation iterativeDeepening(const std::vector<std::vector<int>>& board,
                                     double timeLimit);
    
    /**
     * @brief Principal variation search (advanced)
     */
    int pvSearch(std::vector<std::vector<int>>& board, int depth, int alpha, int beta,
                bool isMaximizing, int lastRow = -1, int lastCol = -1);
    
    // === MOVE GENERATION ===
    
    /**
     * @brief Generate candidate moves intelligently
     */
    std::vector<MoveEvaluation> generateCandidateMoves(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Get moves near existing pieces
     */
    std::vector<MoveEvaluation> getNeighborMoves(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Find critical moves (wins/blocks)
     */
    std::vector<MoveEvaluation> getCriticalMoves(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Sort moves for better pruning
     */
    void sortMoves(std::vector<MoveEvaluation>& moves, 
                   const std::vector<std::vector<int>>& board);
    
    // === EVALUATION FUNCTIONS ===
    
    /**
     * @brief Comprehensive board evaluation
     */
    int evaluateBoard(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Evaluate based on play style
     */
    int evaluateWithStyle(const std::vector<std::vector<int>>& board, int baseScore);
    
    /**
     * @brief Pattern-based evaluation
     */
    int evaluatePatterns(const std::vector<std::vector<int>>& board, int player);
    
    /**
     * @brief Positional evaluation (center control, space)
     */
    int evaluatePosition(const std::vector<std::vector<int>>& board, int player);
    
    // === GAME-SPECIFIC HEURISTICS ===
    
    /**
     * @brief Opening book moves
     */
    MoveEvaluation getOpeningMove(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Handle special situations
     */
    MoveEvaluation handleSpecialSituations(const std::vector<std::vector<int>>& board);
    
    // === UTILITY FUNCTIONS ===
    
    /**
     * @brief Check if game is in terminal state
     */
    bool isTerminalState(const std::vector<std::vector<int>>& board, int lastRow, int lastCol);
    
    /**
     * @brief Get random move (fallback)
     */
    MoveEvaluation getRandomMove(const std::vector<std::vector<int>>& board);
    
    /**
     * @brief Update algorithm parameters based on difficulty
     */
    void updateParameters();
    
    // === TRANSPOSITION TABLE ===
    uint64_t hashBoard(const std::vector<std::vector<int>>& board) const;
    void storeInTable(uint64_t hash, int score, int depth, TranspositionEntry::Type type) const;
    bool probeTable(uint64_t hash, int depth, int alpha, int beta, int& score) const;
};

// === IMPLEMENTATION ===

AI::AI(int aiPlayerNum, Difficulty diff, PlayStyle style) 
    : aiPlayer(aiPlayerNum), difficulty(diff), playStyle(style),
      timeLimit(5.0), useIterativeDeepening(true), useTranspositionTable(true),
      rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
    
    humanPlayer = (aiPlayer == 1) ? 2 : 1;
    transpositionTable.resize(TABLE_SIZE);
    updateParameters();
}

void AI::setDifficulty(Difficulty diff) {
    difficulty = diff;
    updateParameters();
}

void AI::setPlayStyle(PlayStyle style) {
    playStyle = style;
}

void AI::setPlayerNumbers(int ai, int human) {
    aiPlayer = ai;
    humanPlayer = human;
}

void AI::updateParameters() {
    maxDepth = static_cast<int>(difficulty);
    
    switch (difficulty) {
        case Difficulty::BEGINNER:
            maxCandidates = 10;
            useIterativeDeepening = false;
            useTranspositionTable = false;
            break;
        case Difficulty::EASY:
            maxCandidates = 15;
            useIterativeDeepening = false;
            break;
        case Difficulty::MEDIUM:
            maxCandidates = 20;
            useIterativeDeepening = true;
            break;
        case Difficulty::HARD:
            maxCandidates = 25;
            useIterativeDeepening = true;
            break;
        case Difficulty::EXPERT:
            maxCandidates = 30;
            useIterativeDeepening = true;
            useTranspositionTable = true;
            break;
    }
}

AI::MoveEvaluation AI::findBestMove(const std::vector<std::vector<int>>& board) {
    // Reset statistics
    lastStats = ThinkingStats();
    auto startTime = std::chrono::high_resolution_clock::now();
    
    // Check for opening move
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
        return getOpeningMove(board);
    }
    
    // Check for special situations (immediate win/block)
    MoveEvaluation specialMove = handleSpecialSituations(board);
    if (specialMove.row >= 0) {
        return specialMove;
    }
    
    // Use iterative deepening or fixed depth search
    MoveEvaluation bestMove;
    if (useIterativeDeepening) {
        bestMove = iterativeDeepening(board, timeLimit);
    } else {
        std::vector<MoveEvaluation> candidates = generateCandidateMoves(board);
        if (candidates.empty()) {
            return getRandomMove(board);
        }
        
        sortMoves(candidates, board);
        
        int bestScore = INT_MIN;
        for (auto& candidate : candidates) {
            std::vector<std::vector<int>> tempBoard = board;
            tempBoard[candidate.row][candidate.col] = aiPlayer;
            
            int score = minimax(tempBoard, maxDepth - 1, false, INT_MIN, INT_MAX,
                               candidate.row, candidate.col);
            
            if (score > bestScore) {
                bestScore = score;
                bestMove = candidate;
                bestMove.score = score;
            }
        }
    }
    
    // Record statistics
    auto endTime = std::chrono::high_resolution_clock::now();
    lastStats.timeElapsed = std::chrono::duration<double>(endTime - startTime).count();
    
    return bestMove;
}

AI::MoveEvaluation AI::findBestMoveWithTime(const std::vector<std::vector<int>>& board, 
                                           int maxTimeMs) {
    double oldTimeLimit = timeLimit;
    timeLimit = maxTimeMs / 1000.0;
    
    MoveEvaluation result = findBestMove(board);
    
    timeLimit = oldTimeLimit;
    return result;
}

std::vector<AI::MoveEvaluation> AI::getTopMoves(const std::vector<std::vector<int>>& board, 
                                               int count) {
    std::vector<MoveEvaluation> candidates = generateCandidateMoves(board);
    
    // Evaluate all candidates
    for (auto& candidate : candidates) {
        std::vector<std::vector<int>> tempBoard = board;
        tempBoard[candidate.row][candidate.col] = aiPlayer;
        
        candidate.score = minimax(tempBoard, std::min(maxDepth, 4), false, 
                                 INT_MIN, INT_MAX, candidate.row, candidate.col);
    }
    
    // Sort by score
    std::sort(candidates.begin(), candidates.end(),
              [](const MoveEvaluation& a, const MoveEvaluation& b) {
                  return a.score > b.score;
              });
    
    // Return top moves
    candidates.resize(std::min(count, static_cast<int>(candidates.size())));
    return candidates;
}

int AI::minimax(std::vector<std::vector<int>>& board, int depth, bool isMaximizing,
                int alpha, int beta, int lastRow, int lastCol) {
    lastStats.nodesEvaluated++;
    lastStats.maxDepthReached = std::max(lastStats.maxDepthReached, maxDepth - depth);
    
    // Check terminal conditions
    if (isTerminalState(board, lastRow, lastCol) || depth <= 0) {
        return evaluateBoard(board);
    }
    
    // Generate moves
    std::vector<MoveEvaluation> moves = generateCandidateMoves(board);
    if (moves.empty()) {
        return evaluateBoard(board);
    }
    
    sortMoves(moves, board);
    
    if (isMaximizing) {
        int maxEval = INT_MIN;
        
        for (const auto& move : moves) {
            board[move.row][move.col] = aiPlayer;
            
            int eval = minimax(board, depth - 1, false, alpha, beta, 
                              move.row, move.col);
            
            board[move.row][move.col] = 0; // Undo
            
            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            
            if (beta <= alpha) {
                lastStats.pruningCount++;
                break; // Alpha-beta pruning
            }
        }
        return maxEval;
        
    } else {
        int minEval = INT_MAX;
        
        for (const auto& move : moves) {
            board[move.row][move.col] = humanPlayer;
            
            int eval = minimax(board, depth - 1, true, alpha, beta,
                              move.row, move.col);
            
            board[move.row][move.col] = 0; // Undo
            
            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            
            if (beta <= alpha) {
                lastStats.pruningCount++;
                break; // Alpha-beta pruning
            }
        }
        return minEval;
    }
}

std::vector<AI::MoveEvaluation> AI::generateCandidateMoves(const std::vector<std::vector<int>>& board) {
    std::vector<MoveEvaluation> candidates;
    
    // Get critical moves first (highest priority)
    std::vector<MoveEvaluation> criticalMoves = getCriticalMoves(board);
    candidates.insert(candidates.end(), criticalMoves.begin(), criticalMoves.end());
    
    // Get neighbor moves
    std::vector<MoveEvaluation> neighborMoves = getNeighborMoves(board);
    
    // Remove duplicates and combine
    for (const auto& move : neighborMoves) {
        bool isDuplicate = false;
        for (const auto& existing : candidates) {
            if (existing.row == move.row && existing.col == move.col) {
                isDuplicate = true;
                break;
            }
        }
        if (!isDuplicate) {
            candidates.push_back(move);
        }
    }
    
    // Limit candidates
    if (candidates.size() > maxCandidates) {
        candidates.resize(maxCandidates);
    }
    
    return candidates;
}

std::vector<AI::MoveEvaluation> AI::getCriticalMoves(const std::vector<std::vector<int>>& board) {
    std::vector<MoveEvaluation> criticalMoves;
    int size = board.size();
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                MoveEvaluation move(i, j, 0);
                
                // Check if this is a winning move
                if (GameLogic::isWinningThreat(board, i, j, aiPlayer)) {
                    move.score = 1000000;
                    move.isWinning = true;
                    move.reasoning = "Winning move";
                    criticalMoves.push_back(move);
                }
                // Check if this blocks opponent's win
                else if (GameLogic::isBlockingThreat(board, i, j, aiPlayer)) {
                    move.score = 999999;
                    move.isBlocking = true;
                    move.reasoning = "Blocking move";
                    criticalMoves.push_back(move);
                }
            }
        }
    }
    
    return criticalMoves;
}

std::vector<AI::MoveEvaluation> AI::getNeighborMoves(const std::vector<std::vector<int>>& board) {
    std::vector<MoveEvaluation> neighborMoves;
    int size = board.size();
    std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
    
    // Find all empty cells near existing pieces
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] != 0) {
                // Add neighbors of this piece
                for (int di = -2; di <= 2; di++) {
                    for (int dj = -2; dj <= 2; dj++) {
                        int ni = i + di;
                        int nj = j + dj;
                        
                        if (ni >= 0 && ni < size && nj >= 0 && nj < size &&
                            board[ni][nj] == 0 && !visited[ni][nj]) {
                            neighborMoves.emplace_back(ni, nj);
                            visited[ni][nj] = true;
                        }
                    }
                }
            }
        }
    }
    
    return neighborMoves;
}

int AI::evaluateBoard(const std::vector<std::vector<int>>& board) {
    // Base evaluation from GameLogic
    int aiScore = GameLogic::evaluateBoard(board, aiPlayer);
    int humanScore = GameLogic::evaluateBoard(board, humanPlayer);
    
    int baseScore = aiScore - humanScore;
    
    // Apply play style modifications
    return evaluateWithStyle(board, baseScore);
}

int AI::evaluateWithStyle(const std::vector<std::vector<int>>& board, int baseScore) {
    switch (playStyle) {
        case PlayStyle::AGGRESSIVE:
            // Bonus for attacking patterns
            return baseScore + evaluatePatterns(board, aiPlayer) / 2;
            
        case PlayStyle::DEFENSIVE:
            // Bonus for blocking opponent
            return baseScore - evaluatePatterns(board, humanPlayer) / 2;
            
        case PlayStyle::POSITIONAL:
            // Bonus for center control
            return baseScore + evaluatePosition(board, aiPlayer);
            
        case PlayStyle::BALANCED:
        default:
            return baseScore;
