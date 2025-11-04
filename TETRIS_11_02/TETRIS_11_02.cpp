#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;

static const int W = 10; // 보드 가로
static const int H = 20; // 보드 세로

// 2x2 블록 클래스
class Piece {
public:
    int x = W / 2 - 1; // 좌상단 x
    int y = 0;         // 좌상단 y
    int size = 2;      // 2x2
};

// Windows 10+ 콘솔에서 ANSI VT 기능 켜기
bool enableVT() {
    HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
    if (hOut == INVALID_HANDLE_VALUE) return false;
    DWORD mode = 0;
    if (!GetConsoleMode(hOut, &mode)) return false;
    if ((mode & ENABLE_VIRTUAL_TERMINAL_PROCESSING) == 0) {
        if (!SetConsoleMode(hOut, mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING)) return false;
    }
    return true;
}

// 한 프레임 출력 (커서 홈으로만 이동하여 덮어쓰기)
void drawFrame(const vector<string>& buf, bool locked) {
    cout << "\x1b[H"; // Home
    cout << "+" << string(W, '-') << "+\n";
    for (int r = 0; r < H; ++r) {
        cout << "|" << buf[r] << "|\n";
    }
    cout << "+" << string(W, '-') << "+\n";
    cout.flush();
}

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    enableVT(); // VT 활성화

    // 초기 화면 준비: 전체 지우기 1회 + 커서 숨김
    cout << "\x1b[2J\x1b[H"; // Clear + Home (초기 1회만)
    cout << "\x1b[?25l";     // Hide cursor

    // '.'=빈칸, '#'=고정된 블록
    vector<string> board(H, string(W, '.'));

    Piece piece;                     // 하나의 블록
    const int fallMs = 400;          // 중력 주기(ms)
    auto lastFall = chrono::steady_clock::now();
    bool locked = false;             // 바닥에 저장(고정) 완료 여부

    while (true) {
        auto now = chrono::steady_clock::now();

        // 중력: 일정 주기로 y 증가 (바닥 저장만 처리, 다른 충돌 X)
        if (!locked && chrono::duration_cast<chrono::milliseconds>(now - lastFall).count() >= fallMs) {
            piece.y += 1;

            // 바닥을 넘었으면 바닥에 "저장" 후 더 이상 움직이지 않음
            if (piece.y + piece.size > H) {
                piece.y = H - piece.size; // 바닥에 맞춰 위치 보정
                for (int dy = 0; dy < piece.size; ++dy) {
                    for (int dx = 0; dx < piece.size; ++dx) {
                        int xx = piece.x + dx;
                        int yy = piece.y + dy;
                        if (0 <= xx && xx < W && 0 <= yy && yy < H) {
                            board[yy][xx] = '#'; // 보드에 저장
                        }
                    }
                }
                locked = true; // 저장 완료 → 더 이상 낙하하지 않음
            }

            lastFall = now;
        }

        // 렌더 버퍼: 보드 복사 후, 아직 저장 전이면 현재 피스만 겹쳐서 보여주기
        vector<string> buf = board;
        if (!locked) {
            for (int dy = 0; dy < piece.size; ++dy) {
                for (int dx = 0; dx < piece.size; ++dx) {
                    int xx = piece.x + dx;
                    int yy = piece.y + dy;
                    if (0 <= xx && xx < W && 0 <= yy && yy < H) {
                        buf[yy][xx] = '#';
                    }
                }
            }
        }

        drawFrame(buf, locked);
        this_thread::sleep_for(chrono::milliseconds(16));
    }

    // 도달 불가 코드지만, 안전하게 커서 복원 예시
    cout << "\x1b[?25h"; // Show cursor
    return 0;
}