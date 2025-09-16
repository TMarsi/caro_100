// Board.cpp - Người 1: Quản lý bàn cờ (Hỗ trợ tới 100x100)
#ifndef BOARD_H
#define BOARD_H

#include <vector>
#include <iostream>
#include <iomanip>
#include <memory>
#include <unordered_set>
#include <algorithm>

class Board {
private:
    int size;  // Kích thước bàn cờ có thể thay đổi (15x15 đến 100x100)
    std::vector<std::vector<int>> board;   // 0: trống, 1: X (người), 2: O (AI)
    int moveCount;  // Đếm số nước đi
    
    // Optimization cho bàn cờ lớn
    std::vector<std::pair<int, int>> occupiedCells;  // Lưu vị trí có quân
    std::unordered_set<long long> activeRegions;     // Vùng hoạt động (để tối ưu AI)
    
    // Constants
    static const int MIN_SIZE = 15;
    static const int MAX_SIZE = 100;
    static const int REGION_SIZE = 10;  // Kích thước mỗi region để tối ưu
    
public:
    // Constructors
    Board(int boardSize = 15);
    Board(const Board& other);
    Board& operator=(const Board& other);
    
    // Destructor
    ~Board();
    
    // Size management
    bool resize(int newSize);
    int getSize() const { return size; }
    bool isValidSize(int size) const;
    
    // Getters
    int getCell(int row, int col) const;
    std::vector<std::vector<int>>& getBoard() { return board; }
    const std::vector<std::vector<int>>& getBoard() const { return board; }
    int getMoveCount() const { return moveCount; }
    
    // Game operations
    bool makeMove(int row, int col, int player);
    bool isValidMove(int row, int col) const;
    bool isFull() const;
    void reset();
    void reset(int newSize);
    
    // Memory optimization methods
    size_t getMemoryUsage() const;
    void optimizeMemory();
    
    // Display methods (có thể hiển thị một phần của bàn cờ lớn)
    void displayConsole() const;
    void displayRegion(int startRow, int startCol, int width, int height) const;
    void displayAroundLastMove(int lastRow, int lastCol, int radius = 10) const;
    
    // Utility functions
    bool isInBounds(int row, int col) const;
    std::vector<std::pair<int, int>> getEmptyCells() const;
    std::vector<std::pair<int, int>> getEmptyCellsInRegion(int startRow, int startCol, int endRow, int endCol) const;
    std::vector<std::pair<int, int>> getNeighborCells(int row, int col, int radius = 2) const;
    std::vector<std::pair<int, int>> getOccupiedCells() const { return occupiedCells; }
    
    // Region-based operations for large boards
    long long getRegionKey(int row, int col) const;
    std::vector<long long> getActiveRegions() const;
    std::vector<std::pair<int, int>> getCellsInRegion(long long regionKey) const;
    void addActiveRegion(int row, int col);
    
    // Statistics
    double getOccupancyRate() const;
    std::pair<int, int> getBoardCenter() const;
    std::pair<std::pair<int, int>, std::pair<int, int>> getActiveBounds() const;
};

// Implementation
Board::Board(int boardSize) : size(boardSize), moveCount(0) {
    if (!isValidSize(boardSize)) {
        size = 15;  // Default fallback
    }
    
    try {
        board.resize(size, std::vector<int>(size, 0));
        occupiedCells.reserve(size * size / 4);  // Reserve space for efficiency
    } catch (const std::bad_alloc& e) {
        // Handle memory allocation failure for very large boards
        size = 15;
        board.resize(size, std::vector<int>(size, 0));
        std::cerr << "Memory allocation failed for size " << boardSize 
                  << ", falling back to 15x15" << std::endl;
    }
}

Board::Board(const Board& other) 
    : size(other.size), board(other.board), moveCount(other.moveCount),
      occupiedCells(other.occupiedCells), activeRegions(other.activeRegions) {
}

Board& Board::operator=(const Board& other) {
    if (this != &other) {
        size = other.size;
        board = other.board;
        moveCount = other.moveCount;
        occupiedCells = other.occupiedCells;
        activeRegions = other.activeRegions;
    }
    return *this;
}

Board::~Board() {
    // Destructor - cleanup handled by vectors
}

bool Board::isValidSize(int size) const {
    return size >= MIN_SIZE && size <= MAX_SIZE;
}

bool Board::resize(int newSize) {
    if (!isValidSize(newSize)) {
        return false;
    }
    
    try {
        // Create new board
        std::vector<std::vector<int>> newBoard(newSize, std::vector<int>(newSize, 0));
        
        // Copy existing data if possible
        int copySize = std::min(size, newSize);
        for (int i = 0; i < copySize; i++) {
            for (int j = 0; j < copySize; j++) {
                newBoard[i][j] = board[i][j];
            }
        }
        
        // Update board
        board = std::move(newBoard);
        size = newSize;
        
        // Update occupied cells (remove out-of-bounds cells)
        occupiedCells.erase(
            std::remove_if(occupiedCells.begin(), occupiedCells.end(),
                          [this](const std::pair<int, int>& cell) {
                              return cell.first >= size || cell.second >= size;
                          }),
            occupiedCells.end()
        );
        
        // Recalculate active regions
        activeRegions.clear();
        for (const auto& cell : occupiedCells) {
            addActiveRegion(cell.first, cell.second);
        }
        
        return true;
        
    } catch (const std::bad_alloc& e) {
        std::cerr << "Failed to resize board to " << newSize << "x" << newSize << std::endl;
        return false;
    }
}

bool Board::isInBounds(int row, int col) const {
    return row >= 0 && row < size && col >= 0 && col < size;
}

int Board::getCell(int row, int col) const {
    if (!isInBounds(row, col)) {
        return -1;  // Out of bounds
    }
    return board[row][col];
}

bool Board::isValidMove(int row, int col) const {
    return isInBounds(row, col) && board[row][col] == 0;
}

bool Board::makeMove(int row, int col, int player) {
    if (!isValidMove(row, col)) {
        return false;
    }
    
    if (player != 1 && player != 2) {
        return false;  // Invalid player
    }
    
    board[row][col] = player;
    moveCount++;
    
    // Add to occupied cells
    occupiedCells.push_back({row, col});
    
    // Add to active regions
    addActiveRegion(row, col);
    
    return true;
}

bool Board::isFull() const {
    return moveCount >= (long long)size * size;
}

void Board::reset() {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            board[i][j] = 0;
        }
    }
    moveCount = 0;
    occupiedCells.clear();
    activeRegions.clear();
}

void Board::reset(int newSize) {
    if (resize(newSize)) {
        reset();
    }
}

size_t Board::getMemoryUsage() const {
    size_t usage = sizeof(*this);
    usage += board.size() * sizeof(std::vector<int>);
    usage += (long long)size * size * sizeof(int);
    usage += occupiedCells.capacity() * sizeof(std::pair<int, int>);
    usage += activeRegions.size() * sizeof(long long);
    return usage;
}

void Board::optimizeMemory() {
    // Shrink occupied cells vector to fit
    occupiedCells.shrink_to_fit();
    
    // Remove inactive regions (regions with no pieces)
    std::vector<long long> regionsToRemove;
    for (long long regionKey : activeRegions) {
        if (getCellsInRegion(regionKey).empty()) {
            regionsToRemove.push_back(regionKey);
        }
    }
    
    for (long long regionKey : regionsToRemove) {
        activeRegions.erase(regionKey);
    }
}

void Board::displayConsole() const {
    if (size > 30) {
        std::cout << "Board too large for full display (" << size << "x" << size << ")" << std::endl;
        std::cout << "Use displayAroundLastMove() or displayRegion() for partial view" << std::endl;
        std::cout << "Occupied cells: " << moveCount << "/" << (long long)size * size 
                  << " (" << std::fixed << std::setprecision(2) << getOccupancyRate() * 100 << "%)" << std::endl;
        return;
    }
    
    std::cout << "\n   ";
    for (int i = 0; i < size; i++) {
        std::cout << std::setw(3) << i % 100;  // Show last 2 digits for large boards
    }
    std::cout << "\n";
    
    for (int i = 0; i < size; i++) {
        std::cout << std::setw(2) << i % 100 << " ";
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) std::cout << " . ";
            else if (board[i][j] == 1) std::cout << " X ";
            else std::cout << " O ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void Board::displayRegion(int startRow, int startCol, int width, int height) const {
    int endRow = std::min(startRow + height, size);
    int endCol = std::min(startCol + width, size);
    
    std::cout << "\nRegion (" << startRow << "," << startCol << ") to (" 
              << endRow-1 << "," << endCol-1 << ")\n";
    
    std::cout << "   ";
    for (int j = startCol; j < endCol; j++) {
        std::cout << std::setw(3) << j % 100;
    }
    std::cout << "\n";
    
    for (int i = startRow; i < endRow; i++) {
        std::cout << std::setw(2) << i % 100 << " ";
        for (int j = startCol; j < endCol; j++) {
            if (board[i][j] == 0) std::cout << " . ";
            else if (board[i][j] == 1) std::cout << " X ";
            else std::cout << " O ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void Board::displayAroundLastMove(int lastRow, int lastCol, int radius) const {
    if (!isInBounds(lastRow, lastCol)) {
        displayRegion(0, 0, std::min(20, size), std::min(20, size));
        return;
    }
    
    int startRow = std::max(0, lastRow - radius);
    int startCol = std::max(0, lastCol - radius);
    int width = std::min(radius * 2 + 1, size - startCol);
    int height = std::min(radius * 2 + 1, size - startRow);
    
    displayRegion(startRow, startCol, width, height);
}

std::vector<std::pair<int, int>> Board::getEmptyCells() const {
    std::vector<std::pair<int, int>> emptyCells;
    
    // For large boards, this could be expensive, so we reserve space
    emptyCells.reserve((long long)size * size - moveCount);
    
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            if (board[i][j] == 0) {
                emptyCells.push_back({i, j});
            }
        }
    }
    
    return emptyCells;
}

std::vector<std::pair<int, int>> Board::getEmptyCellsInRegion(int startRow, int startCol, int endRow, int endCol) const {
    std::vector<std::pair<int, int>> emptyCells;
    
    // Clamp bounds
    startRow = std::max(0, startRow);
    startCol = std::max(0, startCol);
    endRow = std::min(size, endRow);
    endCol = std::min(size, endCol);
    
    for (int i = startRow; i < endRow; i++) {
        for (int j = startCol; j < endCol; j++) {
            if (board[i][j] == 0) {
                emptyCells.push_back({i, j});
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
            if (board[i][j] == 0 && (i != row || j != col)) {
                neighbors.push_back({i, j});
            }
        }
    }
    
    return neighbors;
}

long long Board::getRegionKey(int row, int col) const {
    int regionRow = row / REGION_SIZE;
    int regionCol = col / REGION_SIZE;
    return ((long long)regionRow << 32) | regionCol;
}

void Board::addActiveRegion(int row, int col) {
    // Add the region containing this cell and adjacent regions
    for (int dr = -1; dr <= 1; dr++) {
        for (int dc = -1; dc <= 1; dc++) {
            int r = row + dr * REGION_SIZE;
            int c = col + dc * REGION_SIZE;
            if (r >= 0 && r < size && c >= 0 && c < size) {
                activeRegions.insert(getRegionKey(r, c));
            }
        }
    }
}

std::vector<long long> Board::getActiveRegions() const {
    return std::vector<long long>(activeRegions.begin(), activeRegions.end());
}

std::vector<std::pair<int, int>> Board::getCellsInRegion(long long regionKey) const {
    std::vector<std::pair<int, int>> cells;
    
    int regionRow = (int)(regionKey >> 32);
    int regionCol = (int)(regionKey & 0xFFFFFFFF);
    
    int startRow = regionRow * REGION_SIZE;
    int endRow = std::min(size, startRow + REGION_SIZE);
    int startCol = regionCol * REGION_SIZE;
    int endCol = std::min(size, startCol + REGION_SIZE);
    
    for (int i = startRow; i < endRow; i++) {
        for (int j = startCol; j < endCol; j++) {
            if (board[i][j] != 0) {
                cells.push_back({i, j});
            }
        }
    }
    
    return cells;
}

double Board::getOccupancyRate() const {
    return (double)moveCount / ((long long)size * size);
}

std::pair<int, int> Board::getBoardCenter() const {
    return {size / 2, size / 2};
}

std::pair<std::pair<int, int>, std::pair<int, int>> Board::getActiveBounds() const {
    if (occupiedCells.empty()) {
        auto center = getBoardCenter();
        return {{center.first, center.second}, {center.first, center.second}};
    }
    
    int minRow = size, maxRow = -1;
    int minCol = size, maxCol = -1;
    
    for (const auto& cell : occupiedCells) {
        minRow = std::min(minRow, cell.first);
        maxRow = std::max(maxRow, cell.first);
        minCol = std::min(minCol, cell.second);
        maxCol = std::max(maxCol, cell.second);
    }
    
    return {{minRow, minCol}, {maxRow, maxCol}};
}

#endif // BOARD_H
