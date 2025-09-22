/**
 * =====================================================================================
 * Board.cpp - Core Game State Management Library
 * =====================================================================================
 * 
 * NGƯỜI 1: LOGIC & AI - Module quản lý trạng thái bàn cờ
 * 
 * CHỨC NĂNG CHÍNH:
 * ✅ Biểu diễn và quản lý trạng thái bàn cờ 15x15 đến 100x100
 * ✅ Cung cấp API hoàn chỉnh cho Graphics và GameController
 * ✅ Tối ưu hiệu năng cho AI search với region-based optimization
 * ✅ Quản lý memory thông minh cho bàn cờ lớn
 * ✅ Validation và error handling đầy đủ
 * 
 * API CHO CÁC MODULE KHÁC:
 * - Graphics Module (Người 2): getCell(), getGrid(), getActiveBounds()
 * - GameController (Người 3): makeMove(), isValidMove(), getLastMove()
 * - AI System: getNeighborCells(), getActiveRegions(), optimized search
 * 
 * THUẬT TOÁN VÀ TỐI ƯU:
 * - Region-based search: O(active_area) thay vì O(n²)
 * - Memory optimization cho bàn cờ lớn
 * - Smart candidate generation cho AI
 * =====================================================================================
 */

#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <memory>
#include <unordered_set>
#include <algorithm>
#include <stdexcept>
#include <tuple>
#include <string>

class Board {
public:
    // ================== CONSTANTS & ENUMS ==================
    
    static const int MIN_SIZE = 15;
    static const int MAX_SIZE = 100;
    static const int DEFAULT_SIZE = 15;
    
    enum CellState {
        EMPTY = 0,
        PLAYER1 = 1,  // X
        PLAYER2 = 2   // O
    };
    
private:
    // ================== CORE DATA STRUCTURES ==================
    
    int size;                                    // Kích thước bàn cờ
    std::vector<std::vector<int>> grid;          // Ma trận bàn cờ
    int moveCount;                               // Số nước đi đã thực hiện
    
    // Performance optimization structures
    std::vector<std::pair<int, int>> occupiedCells;     // Cache các ô có quân
    std::unordered_set<long long> activeRegions;        // Vùng hoạt động (10x10 regions)
    static const int REGION_SIZE = 10;
    
    // Move tracking
    int lastMoveRow, lastMoveCol, lastPlayer;
    std::vector<std::tuple<int, int, int>> moveHistory;  // History cho undo

public:
    // ================== CONSTRUCTORS & LIFECYCLE ==================
    
    /**
     * @brief Constructor với kích thước tùy chỉnh
     * @param boardSize Kích thước bàn cờ (MIN_SIZE đến MAX_SIZE)
     */
    explicit Board(int boardSize = DEFAULT_SIZE);
    
    Board(const Board& other);
    Board& operator=(const Board& other);
    Board(Board&& other) noexcept;
    Board& operator=(Board&& other) noexcept;
    ~Board() = default;

    // ================== SIZE MANAGEMENT ==================
    
    /**
     * @brief Thay đổi kích thước bàn cờ
     * @param newSize Kích thước mới
     * @return true nếu thành công
     */
    bool resize(int newSize);
    
    int getSize() const noexcept { return size; }
    bool isValidSize(int size) const noexcept;
    
    // ================== CORE GAME STATE ACCESS ==================
    
    /**
     * @brief Lấy trạng thái ô tại vị trí (row, col)
     * @return EMPTY, PLAYER1, PLAYER2, hoặc -1 nếu out of bounds
     */
    int getCell(int row, int col) const noexcept;
    
    /**
     * @brief Lấy reference chỉ đọc tới toàn bộ grid (cho Graphics)
     */
    const std::vector<std::vector<int>>& getGrid() const noexcept { return grid; }
    
    /**
     * @brief Lấy reference có thể chỉnh sửa (cho AI calculations)
     */
    std::vector<std::vector<int>>& getGrid() noexcept { return grid; }
    
    int getMoveCount() const noexcept { return moveCount; }

    // ================== MOVE OPERATIONS ==================
    
    /**
     * @brief Thực hiện nước đi
     * @param row Hàng (0-based)
     * @param col Cột (0-based)
     * @param player ID người chơi (PLAYER1 hoặc PLAYER2)
     * @return true nếu nước đi hợp lệ
     */
    bool makeMove(int row, int col, int player);
    
    /**
     * @brief Kiểm tra nước đi có hợp lệ không
     */
    bool isValidMove(int row, int col) const noexcept;
    
    /**
     * @brief Hoàn tác nước đi cuối (cho AI search)
     */
    bool undoLastMove();
    
    /**
     * @brief Hoàn tác nhiều nước đi
     */
    bool undoMoves(int count);

    // ================== GAME STATE QUERIES ==================
    
    /**
     * @brief Lấy thông tin nước đi cuối
     * @return {row, col, player} hoặc {-1, -1, -1} nếu chưa có nước đi
     */
    std::tuple<int, int, int> getLastMove() const noexcept;
    
    /**
     * @brief Lấy lịch sử các nước đi
     */
    const std::vector<std::tuple<int, int, int>>& getMoveHistory() const noexcept { return moveHistory; }
    
    bool isFull() const noexcept;
    bool isEmpty() const noexcept;
    
    void reset();
    void reset(int newSize);

    // ================== UTILITY FOR AI & GRAPHICS ==================
    
    bool isInBounds(int row, int col) const noexcept;
    
    /**
     * @brief Lấy tất cả ô trống (CHỈ dùng cho bàn cờ nhỏ)
     * WARNING: O(n²) complexity
     */
    std::vector<std::pair<int, int>> getEmptyCells() const;
    
    /**
     * @brief Lấy ô trống trong vùng cụ thể (OPTIMIZED)
     */
    std::vector<std::pair<int, int>> getEmptyCellsInRegion(
        int startRow, int startCol, int endRow, int endCol) const;
    
    /**
     * @brief Lấy các ô láng giềng trong bán kính (cho AI)
     * @param row Tâm
     * @param col Tâm  
     * @param radius Bán kính tìm kiếm
     * @return Vector các vị trí trống gần đó
     */
    std::vector<std::pair<int, int>> getNeighborCells(int row, int col, int radius = 2) const;
    
    /**
     * @brief Lấy tất cả vị trí có quân (CACHED)
     */
    const std::vector<std::pair<int, int>>& getOccupiedCells() const noexcept { return occupiedCells; }

    // ================== REGION-BASED OPTIMIZATION ==================
    
    /**
     * @brief Lấy các vùng hoạt động cho AI search
     */
    std::vector<long long> getActiveRegions() const;
    
    /**
     * @brief Lấy bounding box của vùng chơi hiện tại
     * @return {{minRow, minCol}, {maxRow, maxCol}}
     */
    std::pair<std::pair<int, int>, std::pair<int, int>> getActiveBounds() const;
    
    /**
     * @brief Lấy tâm bàn cờ
     */
    std::pair<int, int> getBoardCenter() const noexcept;
    
    /**
     * @brief Lấy các ô trong region cụ thể
     */
    std::vector<std::pair<int, int>> getCellsInRegion(long long regionKey) const;

    // ================== STATISTICS & METRICS ==================
    
    /**
     * @brief Tỷ lệ lấp đầy bàn cờ (0.0 - 1.0)
     */
    double getOccupancyRate() const noexcept;
    
    /**
     * @brief Ước tính memory usage (bytes)
     */
    size_t getMemoryUsage() const noexcept;
    
    /**
     * @brief Tối ưu memory (gọi định kỳ)
     */
    void optimizeMemory();
    
    /**
     * @brief Kiểm tra tính toàn vẹn dữ liệu
     */
    bool validateState() const noexcept;

private:
    // ================== INTERNAL HELPERS ==================
    
    long long getRegionKey(int row, int col) const noexcept;
    void addActiveRegion(int row, int col);
    void removeOccupiedCell(int row, int col);
    void updateActiveRegions();
    void initializeBoard();
};

// =====================================================================================
// IMPLEMENTATION
// =====================================================================================

Board::Board(int boardSize) 
    : size(boardSize), moveCount(0), lastMoveRow(-1), lastMoveCol(-1), lastPlayer(-1) {
    
    if (!isValidSize(boardSize)) {
        size = DEFAULT_SIZE;
    }
    
    initializeBoard();
}

Board::Board(const Board& other) 
    : size(other.size), grid(other.grid), moveCount(other.moveCount),
      occupiedCells(other.occupiedCells), activeRegions(other.activeRegions),
      lastMoveRow(other.lastMoveRow), lastMoveCol(other.lastMoveCol),
      lastPlayer(other.lastPlayer), moveHistory(other.moveHistory) {
}

Board& Board::operator=(const Board& other) {
    if (this != &other) {
        size = other.size;
        grid = other.grid;
        moveCount = other.moveCount;
        occupiedCells = other.occupiedCells;
        activeRegions = other.activeRegions;
        lastMoveRow = other.lastMoveRow;
        lastMoveCol = other.lastMoveCol;
        lastPlayer = other.lastPlayer;
        moveHistory = other.moveHistory;
    }
    return *this;
}

Board::Board(Board&& other) noexcept
    : size(other.size), grid(std::move(other.grid)), moveCount(other.moveCount),
      occupiedCells(std::move(other.occupiedCells)), activeRegions(std::move(other.activeRegions)),
      lastMoveRow(other.lastMoveRow), lastMoveCol(other.lastMoveCol),
      lastPlayer(other.lastPlayer), moveHistory(std::move(other.moveHistory)) {
    
    other.size = DEFAULT_SIZE;
    other.moveCount = 0;
    other.lastMoveRow = other.lastMoveCol = other.lastPlayer = -1;
}

Board& Board::operator=(Board&& other) noexcept {
    if (this != &other) {
        size = other.size;
        grid = std::move(other.grid);
        moveCount = other.moveCount;
        occupiedCells = std::move(other.occupiedCells);
        activeRegions = std::move(other.activeRegions);
        lastMoveRow = other.lastMoveRow;
        lastMoveCol = other.lastMoveCol;
        lastPlayer = other.lastPlayer;
        moveHistory = std::move(other.moveHistory);
        
        other.size = DEFAULT_SIZE;
        other.moveCount = 0;
        other.lastMoveRow = other.lastMoveCol = other.lastPlayer = -1;
    }
    return *this;
}

void Board::initializeBoard() {
    try {
        grid.assign(size, std::vector<int>(size, EMPTY));
        occupiedCells.reserve(size * size / 4);  // Reserve 25% capacity
        moveHistory.reserve(size * size);
    } catch (const std::bad_alloc&) {
        throw std::runtime_error("Failed to allocate memory for board of size " + std::to_string(size));
    }
}

bool Board::isValidSize(int size) const noexcept {
    return size >= MIN_SIZE && size <= MAX_SIZE;
}

bool Board::resize(int newSize) {
    if (!isValidSize(newSize)) {
        return false;
    }
    
    try {
        std::vector<std::vector<int>> newGrid(newSize, std::vector<int>(newSize, EMPTY));
        
        // Copy existing data
        int copySize = std::min(size, newSize);
        for (int i = 0; i < copySize; i++) {
            for (int j = 0; j < copySize; j++) {
                newGrid[i][j] = grid[i][j];
            }
        }
        
        grid = std::move(newGrid);
        size = newSize;
        
        // Clean up out-of-bounds data
        occupiedCells.erase(
            std::remove_if(occupiedCells.begin(), occupiedCells.end(),
                          [this](const std::pair<int, int>& cell) {
                              return !isInBounds(cell.first, cell.second);
                          }),
            occupiedCells.end()
        );
        
        updateActiveRegions();
        return true;
        
    } catch (const std::bad_alloc&) {
        return false;
    }
}

bool Board::isInBounds(int row, int col) const noexcept {
    return row >= 0 && row < size && col >= 0 && col < size;
}

int Board::getCell(int row, int col) const noexcept {
    if (!isInBounds(row, col)) {
        return -1;  // Out of bounds
    }
    return grid[row][col];
}

bool Board::isValidMove(int row, int col) const noexcept {
    return isInBounds(row, col) && grid[row][col] == EMPTY;
}

bool Board::makeMove(int row, int col, int player) {
    // Validate input
    if (!isValidMove(row, col) || (player != PLAYER1 && player != PLAYER2)) {
        return false;
    }
    
    // Update board state
    grid[row][col] = player;
    moveCount++;
    
    // Update tracking
    lastMoveRow = row;
    lastMoveCol = col;
    lastPlayer = player;
    
    // Add to history
    moveHistory.emplace_back(row, col, player);
    
    // Update optimization structures
    occupiedCells.emplace_back(row, col);
    addActiveRegion(row, col);
    
    return true;
}

bool Board::undoLastMove() {
    if (moveHistory.empty()) {
        return false;
    }
    
    // Get last move
    auto [row, col, player] = moveHistory.back();
    
    // Revert board state
    grid[row][col] = EMPTY;
    moveCount--;
    
    // Remove from structures
    moveHistory.pop_back();
    removeOccupiedCell(row, col);
    
    // Update last move tracking
    if (moveHistory.empty()) {
        lastMoveRow = lastMoveCol = lastPlayer = -1;
    } else {
        auto [prevRow, prevCol, prevPlayer] = moveHistory.back();
        lastMoveRow = prevRow;
        lastMoveCol = prevCol;
        lastPlayer = prevPlayer;
    }
    
    updateActiveRegions();
    return true;
}

bool Board::undoMoves(int count) {
    for (int i = 0; i < count; i++) {
        if (!undoLastMove()) {
            return false;
        }
    }
    return true;
}

std::tuple<int, int, int> Board::getLastMove() const noexcept {
    return std::make_tuple(lastMoveRow, lastMoveCol, lastPlayer);
}

bool Board::isFull() const noexcept {
    return moveCount >= static_cast<long long>(size) * size;
}

bool Board::isEmpty() const noexcept {
    return moveCount == 0;
}

void Board::reset() {
    // Clear grid
    for (auto& row : grid) {
        std::fill(row.begin(), row.end(), EMPTY);
    }
    
    // Reset counters and tracking
    moveCount = 0;
    lastMoveRow = lastMoveCol = lastPlayer = -1;
    
    // Clear optimization structures
    occupiedCells.clear();
    activeRegions.clear();
    moveHistory.clear();
}

void Board::reset(int newSize) {
    if (resize(newSize)) {
        reset();
    }
}

std::vector<std::pair<int, int>> Board::getEmptyCells() const {
    std::vector<std::pair<int, int>> emptyCells;
    emptyCells.reserve(static_cast<long long>(size) * size - moveCount);
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (grid[i][j] == EMPTY) {
                emptyCells.emplace_back(i, j);
            }
        }
    }
    
    return emptyCells;
}

std::vector<std::pair<int, int>> Board::getEmptyCellsInRegion(
    int startRow, int startCol, int endRow, int endCol) const {
    
    std::vector<std::pair<int, int>> emptyCells;
    
    // Clamp bounds
    startRow = std::max(0, startRow);
    startCol = std::max(0, startCol);
    endRow = std::min(size, endRow);
    endCol = std::min(size, endCol);
    
    for (int i = startRow; i < endRow; i++) {
        for (int j = startCol; j < endCol; j++) {
            if (grid[i][j] == EMPTY) {
                emptyCells.emplace_back(i, j);
            }
        }
    }
    
    return emptyCells;
}

std::vector<std::pair<int, int>> Board::getNeighborCells(int row, int col, int radius) const {
    std::vector<std::pair<int, int>> neighbors;
    
    int startRow = std::max(0, row - radius);
    int endRow = std::min(size, row + radius + 1);
    int startCol = std::max(0, col - radius);
    int endCol = std::min(size, col + radius + 1);
    
    for (int i = startRow; i < endRow; i++) {
        for (int j = startCol; j < endCol; j++) {
            if (grid[i][j] == EMPTY && (i != row || j != col)) {
                neighbors.emplace_back(i, j);
            }
        }
    }
    
    return neighbors;
}

std::vector<long long> Board::getActiveRegions() const {
    return std::vector<long long>(activeRegions.begin(), activeRegions.end());
}

std::pair<std::pair<int, int>, std::pair<int, int>> Board::getActiveBounds() const {
    if (occupiedCells.empty()) {
        auto center = getBoardCenter();
        return {{center.first, center.second}, {center.first, center.second}};
    }
    
    int minRow = size, maxRow = -1;
    int minCol = size, maxCol = -1;
    
    for (const auto& [row, col] : occupiedCells) {
        minRow = std::min(minRow, row);
        maxRow = std::max(maxRow, row);
        minCol = std::min(minCol, col);
        maxCol = std::max(maxCol, col);
    }
    
    return {{minRow, minCol}, {maxRow, maxCol}};
}

std::pair<int, int> Board::getBoardCenter() const noexcept {
    return {size / 2, size / 2};
}

std::vector<std::pair<int, int>> Board::getCellsInRegion(long long regionKey) const {
    std::vector<std::pair<int, int>> cells;
    
    int regionRow = static_cast<int>(regionKey >> 32);
    int regionCol = static_cast<int>(regionKey & 0xFFFFFFFF);
    
    int startRow = regionRow * REGION_SIZE;
    int endRow = std::min(size, startRow + REGION_SIZE);
    int startCol = regionCol * REGION_SIZE;
    int endCol = std::min(size, startCol + REGION_SIZE);
    
    for (int i = startRow; i < endRow; i++) {
        for (int j = startCol; j < endCol; j++) {
            if (grid[i][j] != EMPTY) {
                cells.emplace_back(i, j);
            }
        }
    }
    
    return cells;
}

double Board::getOccupancyRate() const noexcept {
    return static_cast<double>(moveCount) / (static_cast<long long>(size) * size);
}

size_t Board::getMemoryUsage() const noexcept {
    size_t usage = sizeof(*this);
    usage += grid.size() * sizeof(std::vector<int>);
    usage += static_cast<long long>(size) * size * sizeof(int);
    usage += occupiedCells.capacity() * sizeof(std::pair<int, int>);
    usage += activeRegions.size() * sizeof(long long);
    usage += moveHistory.capacity() * sizeof(std::tuple<int, int, int>);
    return usage;
}

void Board::optimizeMemory() {
    occupiedCells.shrink_to_fit();
    moveHistory.shrink_to_fit();
    updateActiveRegions();
}

bool Board::validateState() const noexcept {
    // Check move count consistency
    int actualMoves = 0;
    for (const auto& row : grid) {
        for (int cell : row) {
            if (cell != EMPTY) actualMoves++;
        }
    }
    
    return actualMoves == moveCount && 
           occupiedCells.size() == static_cast<size_t>(moveCount) &&
           moveHistory.size() == static_cast<size_t>(moveCount);
}

// ================== PRIVATE HELPERS ==================

long long Board::getRegionKey(int row, int col) const noexcept {
    int regionRow = row / REGION_SIZE;
    int regionCol = col / REGION_SIZE;
    return (static_cast<long long>(regionRow) << 32) | static_cast<unsigned int>(regionCol);
}

void Board::addActiveRegion(int row, int col) {
    // Add current region and adjacent regions
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            int r = row + dr * REGION_SIZE;
            int c = col + dc * REGION_SIZE;
            if (isInBounds(r, c)) {
                activeRegions.insert(getRegionKey(r, c));
            }
        }
    }
}

void Board::removeOccupiedCell(int row, int col) {
    auto it = std::find(occupiedCells.begin(), occupiedCells.end(), std::make_pair(row, col));
    if (it != occupiedCells.end()) {
        occupiedCells.erase(it);
    }
}

void Board::updateActiveRegions() {
    activeRegions.clear();
    for (const auto& [row, col] : occupiedCells) {
        addActiveRegion(row, col);
    }
}

#endif // BOARD_H
