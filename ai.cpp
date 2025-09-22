#ifndef AI_H
#define AI_H

#include <vector>
#include <climits>
#include <algorithm>
#include <random>
#include <chrono>

class AI {
public:
    enum class Difficulty {
        BEGINNER = 1,
        EASY = 2,
        MEDIUM = 4,
        HARD = 6,
        EXPERT = 8
    };
    
    enum class PlayStyle {
        AGGRESSIVE,
        DEFENSIVE,
        BALANCED,
        POSITIONAL
    };
    
    struct MoveEvaluation {
        int row;
        int col;
        int score;
        int depth;
        bool isWinning;
        bool isBlocking;
        
        MoveEvaluation() : row(-1), col(-1), score(INT_MIN), depth(0), 
                          isWinning(false), isBlocking(false) {}
        
        MoveEvaluation(int r, int c, int s = 0) : row(r), col(c), score(s), depth(0),
                                                 isWinning(false), isBlocking(false) {}
    };
    
    struct ThinkingStats {
        int nodesEvaluated;
        int pruningCount;
        int maxDepthReached;
        double timeElapsed;
        
        ThinkingStats() : nodesEvaluated(0), pruningCount(0), maxDepthReached(0), timeElapsed(0.0) {}
    };

private:
    int aiPlayer;
    int humanPlayer;
    Difficulty difficulty;
    PlayStyle playStyle;
    int maxDepth;
    int maxCandidates;
    mutable ThinkingStats lastStats;
    mutable std::mt19937 rng;

public:
    AI(int aiPlayerNum = 2, Difficulty diff = Difficulty::MEDIUM, PlayStyle style = PlayStyle::BALANCED) 
        : aiPlayer(aiPlayerNum), difficulty(diff), playStyle(style),
          rng(std::chrono::steady_clock::now().time_since_epoch().count()) {
        
        humanPlayer = (aiPlayer == 1) ? 2 : 1;
        updateParameters();
    }
    
    MoveEvaluation findBestMove(const std::vector<std::vector<int>>& board) {
        lastStats = ThinkingStats();
        auto startTime = std::chrono::high_resolution_clock::now();
        
        if (isEmpty(board)) {
            return getOpeningMove(board);
        }
        
        MoveEvaluation specialMove = handleSpecialSituations(board);
        if (specialMove.row >= 0) {
            return specialMove;
        }
        
        std::vector<MoveEvaluation> candidates = generateCandidateMoves(board);
        if (candidates.empty()) {
            return getRandomMove(board);
        }
        
        sortMoves(candidates, board);
        
        MoveEvaluation bestMove;
        for (auto& candidate : candidates) {
            std::vector<std::vector<int>> tempBoard = board;
            tempBoard[candidate.row][candidate.col] = aiPlayer;
            
            int score = minimax(tempBoard, maxDepth - 1, false, INT_MIN, INT_MAX,
                               candidate.row, candidate.col);
            
            if (score > bestMove.score) {
                bestMove = MoveEvaluation(candidate.row, candidate.col, score);
            }
        }
        
        auto endTime = std::chrono::high_resolution_clock::now();
        lastStats.timeElapsed = std::chrono::duration<double>(endTime - startTime).count();
        
        return bestMove;
    }
    
    std::vector<MoveEvaluation> getTopMoves(const std::vector<std::vector<int>>& board, int count = 5) {
        std::vector<MoveEvaluation> candidates = generateCandidateMoves(board);
        
        for (auto& candidate : candidates) {
            std::vector<std::vector<int>> tempBoard = board;
            tempBoard[candidate.row][candidate.col] = aiPlayer;
            
            candidate.score = minimax(tempBoard, std::min(maxDepth, 4), false, 
                                     INT_MIN, INT_MAX, candidate.row, candidate.col);
        }
        
        std::sort(candidates.begin(), candidates.end(),
                  [](const MoveEvaluation& a, const MoveEvaluation& b) {
                      return a.score > b.score;
                  });
        
        candidates.resize(std::min(count, static_cast<int>(candidates.size())));
        return candidates;
    }
    
    void setDifficulty(Difficulty diff) {
        difficulty = diff;
        updateParameters();
    }
    
    void setPlayStyle(PlayStyle style) {
        playStyle = style;
    }
    
    Difficulty getDifficulty() const { return difficulty; }
    PlayStyle getPlayStyle() const { return playStyle; }
    const ThinkingStats& getLastThinkingStats() const { return lastStats; }
    int getAIPlayer() const { return aiPlayer; }
    int getHumanPlayer() const { return humanPlayer; }

private:
    void updateParameters() {
        maxDepth = static_cast<int>(difficulty);
        
        switch (difficulty) {
            case Difficulty::BEGINNER:
                maxCandidates = 8;
                break;
            case Difficulty::EASY:
                maxCandidates = 12;
                break;
            case Difficulty::MEDIUM:
                maxCandidates = 16;
                break;
            case Difficulty::HARD:
                maxCandidates = 20;
                break;
            case Difficulty::EXPERT:
                maxCandidates = 25;
                break;
        }
    }
    
    bool isEmpty(const std::vector<std::vector<int>>& board) {
        for (const auto& row : board) {
            for (int cell : row) {
                if (cell != 0) return false;
            }
        }
        return true;
    }
    
    MoveEvaluation getOpeningMove(const std::vector<std::vector<int>>& board) {
        int size = board.size();
        return MoveEvaluation(size / 2, size / 2, 1000);
    }
    
    MoveEvaluation handleSpecialSituations(const std::vector<std::vector<int>>& board) {
        int size = board.size();
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] == 0) {
                    if (isWinningThreat(board, i, j, aiPlayer)) {
                        return MoveEvaluation(i, j, 1000000);
                    }
                    
                    if (isWinningThreat(board, i, j, humanPlayer)) {
                        return MoveEvaluation(i, j, 999999);
                    }
                }
            }
        }
        
        return MoveEvaluation();
    }
    
    std::vector<MoveEvaluation> generateCandidateMoves(const std::vector<std::vector<int>>& board) {
        std::vector<MoveEvaluation> candidates;
        std::vector<MoveEvaluation> criticalMoves = getCriticalMoves(board);
        std::vector<MoveEvaluation> neighborMoves = getNeighborMoves(board);
        
        candidates.insert(candidates.end(), criticalMoves.begin(), criticalMoves.end());
        
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
        
        if (candidates.size() > maxCandidates) {
            candidates.resize(maxCandidates);
        }
        
        return candidates;
    }
    
    std::vector<MoveEvaluation> getCriticalMoves(const std::vector<std::vector<int>>& board) {
        std::vector<MoveEvaluation> criticalMoves;
        int size = board.size();
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] == 0) {
                    MoveEvaluation move(i, j, 0);
                    
                    if (isWinningThreat(board, i, j, aiPlayer)) {
                        move.score = 1000000;
                        move.isWinning = true;
                        criticalMoves.push_back(move);
                    }
                    else if (isWinningThreat(board, i, j, humanPlayer)) {
                        move.score = 999999;
                        move.isBlocking = true;
                        criticalMoves.push_back(move);
                    }
                }
            }
        }
        
        return criticalMoves;
    }
    
    std::vector<MoveEvaluation> getNeighborMoves(const std::vector<std::vector<int>>& board) {
        std::vector<MoveEvaluation> neighborMoves;
        int size = board.size();
        std::vector<std::vector<bool>> visited(size, std::vector<bool>(size, false));
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] != 0) {
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
    
    void sortMoves(std::vector<MoveEvaluation>& moves, const std::vector<std::vector<int>>& board) {
        for (auto& move : moves) {
            move.score = quickEvaluateMove(board, move.row, move.col, aiPlayer);
        }
        
        std::sort(moves.begin(), moves.end(),
                  [](const MoveEvaluation& a, const MoveEvaluation& b) {
                      return a.score > b.score;
                  });
    }
    
    int quickEvaluateMove(const std::vector<std::vector<int>>& board, int row, int col, int player) {
        return evaluatePosition(board, row, col, player);
    }
    
    int minimax(std::vector<std::vector<int>>& board, int depth, bool isMaximizing,
                int alpha, int beta, int lastRow = -1, int lastCol = -1) {
        
        lastStats.nodesEvaluated++;
        lastStats.maxDepthReached = std::max(lastStats.maxDepthReached, maxDepth - depth);
        
        if (isTerminalState(board, lastRow, lastCol) || depth <= 0) {
            return evaluateBoard(board);
        }
        
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
                
                board[move.row][move.col] = 0;
                
                maxEval = std::max(maxEval, eval);
                alpha = std::max(alpha, eval);
                
                if (beta <= alpha) {
                    lastStats.pruningCount++;
                    break;
                }
            }
            return maxEval;
            
        } else {
            int minEval = INT_MAX;
            
            for (const auto& move : moves) {
                board[move.row][move.col] = humanPlayer;
                
                int eval = minimax(board, depth - 1, true, alpha, beta,
                                  move.row, move.col);
                
                board[move.row][move.col] = 0;
                
                minEval = std::min(minEval, eval);
                beta = std::min(beta, eval);
                
                if (beta <= alpha) {
                    lastStats.pruningCount++;
                    break;
                }
            }
            return minEval;
        }
    }
    
    bool isTerminalState(const std::vector<std::vector<int>>& board, int lastRow, int lastCol) {
        if (lastRow >= 0 && lastCol >= 0) {
            return checkWin(board, lastRow, lastCol, board[lastRow][lastCol]);
        }
        return false;
    }
    
    bool checkWin(const std::vector<std::vector<int>>& board, int row, int col, int player) {
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
    
    bool isWinningThreat(const std::vector<std::vector<int>>& board, int row, int col, int player) {
        if (row < 0 || col < 0 || row >= board.size() || col >= board[0].size() || board[row][col] != 0) {
            return false;
        }
        
        std::vector<std::vector<int>> tempBoard = board;
        tempBoard[row][col] = player;
        
        return checkWin(tempBoard, row, col, player);
    }
    
    int evaluateBoard(const std::vector<std::vector<int>>& board) {
        int aiScore = evaluatePlayerPosition(board, aiPlayer);
        int humanScore = evaluatePlayerPosition(board, humanPlayer);
        
        int baseScore = aiScore - humanScore;
        return evaluateWithStyle(board, baseScore);
    }
    
    int evaluatePlayerPosition(const std::vector<std::vector<int>>& board, int player) {
        int totalScore = 0;
        int size = board.size();
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] == 0 && hasAdjacentPieces(board, i, j, 2)) {
                    totalScore += evaluatePosition(board, i, j, player);
                }
            }
        }
        
        return totalScore;
    }
    
    int evaluateWithStyle(const std::vector<std::vector<int>>& board, int baseScore) {
        switch (playStyle) {
            case PlayStyle::AGGRESSIVE:
                return baseScore + evaluatePatterns(board, aiPlayer) / 2;
            case PlayStyle::DEFENSIVE:
                return baseScore - evaluatePatterns(board, humanPlayer) / 2;
            case PlayStyle::POSITIONAL:
                return baseScore + evaluateCenterControl(board, aiPlayer);
            case PlayStyle::BALANCED:
            default:
                return baseScore;
        }
    }
    
    int evaluatePosition(const std::vector<std::vector<int>>& board, int row, int col, int player) {
        int score = 0;
        int directions[4][2] = {{0, 1}, {1, 0}, {1, 1}, {1, -1}};
        
        for (int dir = 0; dir < 4; dir++) {
            int dx = directions[dir][0];
            int dy = directions[dir][1];
            
            int count = countInLine(board, row, col, dx, dy, player);
            if (count >= 5) score += 100000;
            else if (count == 4) score += 10000;
            else if (count == 3) score += 1000;
            else if (count == 2) score += 100;
            else if (count == 1) score += 10;
        }
        
        return score;
    }
    
    int evaluatePatterns(const std::vector<std::vector<int>>& board, int player) {
        int score = 0;
        int size = board.size();
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] == player) {
                    score += evaluatePosition(board, i, j, player);
                }
            }
        }
        
        return score;
    }
    
    int evaluateCenterControl(const std::vector<std::vector<int>>& board, int player) {
        int score = 0;
        int size = board.size();
        int centerRow = size / 2;
        int centerCol = size / 2;
        
        for (int dr = -3; dr <= 3; dr++) {
            for (int dc = -3; dc <= 3; dc++) {
                int r = centerRow + dr;
                int c = centerCol + dc;
                
                if (r >= 0 && r < size && c >= 0 && c < size && board[r][c] == player) {
                    int distance = std::abs(dr) + std::abs(dc);
                    score += (4 - distance) * 10;
                }
            }
        }
        
        return score;
    }
    
    int countInLine(const std::vector<std::vector<int>>& board, int row, int col, int dx, int dy, int player) {
        int size = board.size();
        int count = 0;
        
        if (row >= 0 && row < size && col >= 0 && col < size && board[row][col] == player) {
            count = 1;
        }
        
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
    
    bool hasAdjacentPieces(const std::vector<std::vector<int>>& board, int row, int col, int radius) {
        int size = board.size();
        
        for (int dr = -radius; dr <= radius; dr++) {
            for (int dc = -radius; dc <= radius; dc++) {
                if (dr == 0 && dc == 0) continue;
                
                int nr = row + dr;
                int nc = col + dc;
                
                if (nr >= 0 && nr < size && nc >= 0 && nc < size && board[nr][nc] != 0) {
                    return true;
                }
            }
        }
        
        return false;
    }
    
    MoveEvaluation getRandomMove(const std::vector<std::vector<int>>& board) {
        int size = board.size();
        std::vector<MoveEvaluation> availableMoves;
        
        for (int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                if (board[i][j] == 0) {
                    availableMoves.emplace_back(i, j);
                }
            }
        }
        
        if (availableMoves.empty()) {
            return MoveEvaluation();
        }
        
        std::uniform_int_distribution<int> dist(0, availableMoves.size() - 1);
        return availableMoves[dist(rng)];
    }
};

#endif // AI_H
