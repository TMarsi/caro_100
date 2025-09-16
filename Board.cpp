#include <iostream>
#include <vector>
#include <iomanip>
using namespace std;

class Board {
private:
    static const int SIZE = 15;  // Kích thước bàn cờ 15x15
    vector<vector<int>> board;   // 0: trống, 1: X, 2: O
    
public:
    Board() {
        board.resize(SIZE, vector<int>(SIZE, 0));
    }
    
    // Hiển thị bàn cờ
    void display() {
        cout << "\n   ";
        for (int i = 0; i < SIZE; i++) {
            cout << setw(3) << i;
        }
        cout << "\n";
        
        for (int i = 0; i < SIZE; i++) {
            cout << setw(2) << i << " ";
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == 0) cout << " . ";
                else if (board[i][j] == 1) cout << " X ";
                else cout << " O ";
            }
            cout << "\n";
        }
        cout << "\n";
    }
    
    // Đặt quân cờ
    bool makeMove(int row, int col, int player) {
        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) {
            return false;  // Ngoài bàn cờ
        }
        if (board[row][col] != 0) {
            return false;  // Ô đã có quân
        }
        
        board[row][col] = player;
        return true;
    }
    
    // Lấy giá trị tại vị trí
    int getCell(int row, int col) {
        if (row < 0 || row >= SIZE || col < 0 || col >= SIZE) {
            return -1;  // Ngoài bàn cờ
        }
        return board[row][col];
    }
    
    // Kiểm tra bàn cờ đầy
    bool isFull() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                if (board[i][j] == 0) return false;
            }
        }
        return true;
    }
    
    // Reset bàn cờ
    void reset() {
        for (int i = 0; i < SIZE; i++) {
            for (int j = 0; j < SIZE; j++) {
                board[i][j] = 0;
            }
        }
    }
    
    // Lấy kích thước bàn cờ
    int getSize() { return SIZE; }
    
    // Lấy toàn bộ bàn cờ (cho AI)
    vector<vector<int>>& getBoard() { return board; }
};
