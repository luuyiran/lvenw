#include <string.h>         // memcpy
#include <stdbool.h>        // bool 
#include <math.h>           // sqrt 
#include <wchar.h>
#include <locale.h>         // fix printf wchar_t
#include <easyx.h>          // ui

// #define NDEBUG           // turn off debug
#include <assert.h>         // assert

#ifdef NDEBUG
#define LOG(fmt, ...)   
#else   //  printf log
#include <stdio.h>          
#define LOG(fmt, ...)  printf("[%s %s %d]\033[31m" fmt"\033[0m", __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#endif


#define PIECE_SIZE      64              // 棋子大小
#define BOARD_EDGE      40              // 棋盘距离边界
#define PIECE_RADIUS   (PIECE_SIZE / 2) // 棋子半径
#define BOARD_WIDTH    (PIECE_SIZE * 8 + 2 * BOARD_EDGE)
#define BOARD_HEIGHT   (PIECE_SIZE * 9 + 2 * BOARD_EDGE)

#define TEXT_HEIGHT    ((int)(((double)PIECE_SIZE) / sqrt(2) - 6))

// 棋盘范围
#define RANK_TOP        3
#define RANK_BOTTOM     12
#define FILE_LEFT       3
#define FILE_RIGHT      11

// 棋子编号
#define PIECE_KING        0    // 将
#define PIECE_ADVISOR     1    // 仕
#define PIECE_BISHOP      2    // 相
#define PIECE_KNIGHT      3    // 马
#define PIECE_ROOK        4    // 车
#define PIECE_CANNON      5    // 炮
#define PIECE_PAWN        6    // 兵

// 最大的生成走法数
#define MAX_GEN_MOVES  128 

// 判断棋子是否在棋盘中的数组
const char inBoard[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// 判断棋子是否在九宫的数组
const char inFort[256] = {
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// 判断步长是否符合特定走法的数组，1=帅(将)，2=仕(士)，3=相(象)
const char legalSpan[512] = {
                       0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 2, 1, 2, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0
};

// 根据步长判断马是否蹩腿的数组
const char knightPin[512] = {
                              0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,-16,  0,-16,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0, -1,  0,  0,  0,  1,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0, 16,  0, 16,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0
};

// 帅(将)的步长
const char kingDelta[4] = { -16, -1, 1, 16 };

// 仕(士)的步长
const char advisorDelta[4] = { -17, -15, 15, 17 };

// 马的步长，以帅(将)的步长 kingDelta 作为马腿
const char knightDelta[4][2] = { 
    {-33,-31},  // 向上走，马腿步长 -16
    {-18, 14},  // 向左走，马腿步长 -1
    {-14, 18},  // 向右走，马腿步长 1
    { 31, 33}   // 向下走，马腿步长 1
};

// 马被将军的步长，以仕(士)的步长 advisorDelta 作为马腿
const char knightCheckDelta[4][2] = { 
    {-33, -18},  // 马在将的左上，马腿距离将的步长 -17
    {-31, -14},  // 右上， 马腿步长 -15
    { 14,  31},  // 左下
    { 18,  33}  // 右下
};

// 棋盘初始设置 黑方 ID - 51 - 127, 红方 128 - 203
const char boardStartup[256] = {
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 20, 19, 18, 17, 16, 17, 18, 19, 20,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0, 21,  0,  0,  0,  0,  0, 21,  0,  0,  0,  0,  0,
  0,  0,  0, 22,  0, 22,  0, 22,  0, 22,  0, 22,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 14,  0, 14,  0, 14,  0, 14,  0, 14,  0,  0,  0,  0,
  0,  0,  0,  0, 13,  0,  0,  0,  0,  0, 13,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0, 12, 11, 10,  9,  8,  9, 10, 11, 12,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

// 棋子标签
const wchar_t* lables[32] = {
    L"烫", L"烫", L"烫", L"烫", L"烫", L"烫", L"烫", L"烫",
    L"將", L"士", L"象", L"馬", L"車", L"砲", L"卒", L"烫", // 8  - 14 红旗
    L"帥", L"仕", L"相", L"傌", L"俥", L"炮", L"兵", L"烫", // 16 - 22 黑棋
};

#define LABLE(id)  lables[pos.curboard[id]]


// 局面结构
typedef struct positionStruct {
    bool blackPlayer;           // 轮到谁走，0=红方，1=黑方
    char curboard[256];  // 棋盘上的棋子
} positionStruct;

positionStruct pos;  // 局面实例

#define _XY(i)   (BOARD_EDGE + i * PIECE_SIZE)
// 格子中心的 x 坐标，像素坐标，用于绘制棋盘和棋子
const int xCenter[256] = {
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(1),_XY(2),_XY(3),_XY(4),_XY(5),_XY(6),_XY(7),_XY(8), 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0
};

// 格子中心的 y 坐标，像素坐标，用于绘制棋盘和棋子
const int yCenter[256] = {
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,_XY(0),_XY(0),_XY(0),_XY(0),_XY(0),_XY(0),_XY(0),_XY(0),_XY(0), 0, 0, 0, 0,
 0, 0, 0,_XY(1),_XY(1),_XY(1),_XY(1),_XY(1),_XY(1),_XY(1),_XY(1),_XY(1), 0, 0, 0, 0,
 0, 0, 0,_XY(2),_XY(2),_XY(2),_XY(2),_XY(2),_XY(2),_XY(2),_XY(2),_XY(2), 0, 0, 0, 0,
 0, 0, 0,_XY(3),_XY(3),_XY(3),_XY(3),_XY(3),_XY(3),_XY(3),_XY(3),_XY(3), 0, 0, 0, 0,
 0, 0, 0,_XY(4),_XY(4),_XY(4),_XY(4),_XY(4),_XY(4),_XY(4),_XY(4),_XY(4), 0, 0, 0, 0,
 0, 0, 0,_XY(5),_XY(5),_XY(5),_XY(5),_XY(5),_XY(5),_XY(5),_XY(5),_XY(5), 0, 0, 0, 0,
 0, 0, 0,_XY(6),_XY(6),_XY(6),_XY(6),_XY(6),_XY(6),_XY(6),_XY(6),_XY(6), 0, 0, 0, 0,
 0, 0, 0,_XY(7),_XY(7),_XY(7),_XY(7),_XY(7),_XY(7),_XY(7),_XY(7),_XY(7), 0, 0, 0, 0,
 0, 0, 0,_XY(8),_XY(8),_XY(8),_XY(8),_XY(8),_XY(8),_XY(8),_XY(8),_XY(8), 0, 0, 0, 0,
 0, 0, 0,_XY(9),_XY(9),_XY(9),_XY(9),_XY(9),_XY(9),_XY(9),_XY(9),_XY(9), 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0,
 0, 0, 0,     0,     0,     0,     0,     0,     0,     0,     0,     0, 0, 0, 0, 0
};

// 窗口像素坐标
inline int X_CENTER(int id) { return xCenter[id]; }
inline int Y_CENTER(int id) { return yCenter[id]; }

// 判断棋子是否在棋盘中
inline bool IN_BOARD(int id) { return inBoard[id] != 0; }

// 判断棋子是否在九宫中
inline bool IN_FORT(int id) { return inFort[id] != 0; }

// 纵坐标，除以 16，即行数
inline int   Y(int id) { return id >> 4; }
inline int ROW(int id) { return id >> 4; }

// 横坐标，16 取余，获得本行偏移，即列数
inline int   X(int id) { return id & 0XF; }
inline int COL(int id) { return id & 0XF; }

// 根据纵坐和横坐标获得格子
inline int COORD_XY(int x, int y) { return x + (y << 4); }

// 纵坐标水平镜像
inline int FILE_FLIP(int x) { return 14 - x; }

// 横坐标垂直镜像
inline int RANK_FLIP(int y) { return 15 - y; }

// 格子水平镜像
inline int MIRROR_SQUARE(int id) {
    return COORD_XY(FILE_FLIP(Y(id)), X(id));
}

// 兵卒前进一步
inline int SQUARE_FORWARD(int id, int isBlack) { return id - 16 + (isBlack << 5); }

// 走法是否符合帅(将)的步长
inline bool KING_SPAN(int idSrc, int idDst) {
    return legalSpan[idDst - idSrc + 256] == 1;
}

// 走法是否符合仕(士)的步长
inline bool ADVISOR_SPAN(int idSrc, int idDst) {
    return legalSpan[idDst - idSrc + 256] == 2;
}

// 走法是否符合相(象)的步长
inline bool BISHOP_SPAN(int idSrc, int idDst) {
    return legalSpan[idDst - idSrc + 256] == 3;
}

// 相(象)眼的位置
inline int BISHOP_PIN(int idSrc, int idDst) { return (idSrc + idDst) >> 1; }

// 马腿的位置
inline int KNIGHT_PIN(int idSrc, int idDst) {
    return idSrc + knightPin[idDst - idSrc + 256];
}

// 是否未过河
inline bool HOME_HALF(int id, int isBlack) { return (id & 0x80) != (isBlack << 7); }

// 是否已过河
inline bool AWAY_HALF(int id, int isBlack) { return (id & 0x80) == (isBlack << 7); }

// 是否在河的同一边 黑方 ID - 51 - 127, 红方 128 - 255  0X80 = 128
inline bool SAME_HALF(int idSrc, int idDst) {
    return ((idSrc ^ idDst) & 0x80) == 0;
}

// 是否在同一行
inline bool SAME_RANK(int idSrc, int idDst) {
    return ((idSrc ^ idDst) & 0xf0) == 0;
}

// 是否在同一列
inline bool SAME_FILE(int idSrc, int idDst) {
    return ((idSrc ^ idDst) & 0x0f) == 0;
}

// 获得红黑标记(红子是8，黑子是16)
inline int SIDE_TAG(int isBlack) { return 8 + (isBlack << 3); }

// 获得对方红黑标记
inline int OPP_SIDE_TAG(int isBlack) { return 16 - (isBlack << 3); }

// 获得走法的起点
inline int SRC(int mv) { return mv & 255; }

// 获得走法的终点
inline int DST(int mv) { return mv >> 8; }

// 根据起点和终点获得走法
inline int MOVE(int idSrc, int idDst) { return idSrc + (idDst << 8); }

// 走法水平镜像
inline int MIRROR_MOVE(int mv) {
    return MOVE(MIRROR_SQUARE(SRC(mv)), MIRROR_SQUARE(DST(mv)));
}

void changeSide(positionStruct* pos) {  // 交换走子方
    pos->blackPlayer ^= 1;
}
void addPiece(positionStruct* pos, int id, int type) {  // 在棋盘上放一枚棋子
    pos->curboard[id] = type;
}
void delPiece(positionStruct* pos, int id) {  // 从棋盘上拿走一枚棋子
    pos->curboard[id] = 0;
}

// 搬一步棋的棋子
int movePiece(positionStruct* pos, int mv) {
    int idSrc, idDst, type, pcCaptured;
    idSrc = SRC(mv);
    idDst = DST(mv);
    pcCaptured = pos->curboard[idDst];
    delPiece(pos, idDst);
    type = pos->curboard[idSrc];
    delPiece(pos, idSrc);
    addPiece(pos, idDst, type);
    return pcCaptured;
}

// 撤消搬一步棋的棋子
void undoMovePiece(positionStruct* pos, int mv, int typeDst) {
    int idSrc, idDst, typeSrc;
    idSrc = SRC(mv);
    idDst = DST(mv);
    typeSrc = pos->curboard[idDst];
    delPiece(pos, idDst);
    addPiece(pos, idSrc, typeSrc);
    addPiece(pos, idDst, typeDst);
}

// 判断是否被将军
bool checked(positionStruct* pos) {
    int i, j, idSrc, idDst;
    int sideMask, pcOppSide, typeDst, nDelta;
    sideMask = SIDE_TAG(pos->blackPlayer);
    pcOppSide = OPP_SIDE_TAG(pos->blackPlayer);

    // 找到棋盘上的帅(将)，再做以下判断：
    for (idSrc = 51; idSrc <= 203; idSrc++) {
        if (pos->curboard[idSrc] != sideMask + PIECE_KING) {
            continue;
        }

        // 1. 判断是否被对方的兵(卒)将军，按兵的走法走一步看是否会碰上对方的兵
        if (pos->curboard[SQUARE_FORWARD(idSrc, pos->blackPlayer)] ==
            pcOppSide + PIECE_PAWN) {
            return true;
        }
        for (nDelta = -1; nDelta <= 1; nDelta += 2) {
            if (pos->curboard[idSrc + nDelta] == pcOppSide + PIECE_PAWN) {
                return true;
            }
        }

        // 2. 判断是否被对方的马将军(以仕(士)的步长当作马腿)
        for (i = 0; i < 4; i++) {
            // 从将的角度计算马腿
            if (pos->curboard[idSrc + advisorDelta[i]] != 0) {
                continue;
            }
            for (j = 0; j < 2; j++) {
                typeDst = pos->curboard[idSrc + knightCheckDelta[i][j]];
                if (typeDst == pcOppSide + PIECE_KNIGHT) {
                    return true;
                }
            }
        }

        // 3. 判断是否被对方的车或炮将军(包括将帅对脸)
        for (i = 0; i < 4; i++) {
            nDelta = kingDelta[i];
            idDst = idSrc + nDelta;
            while (IN_BOARD(idDst)) {
                typeDst = pos->curboard[idDst];
                if (typeDst != 0) {
                    if (typeDst == pcOppSide + PIECE_ROOK ||
                        typeDst == pcOppSide + PIECE_KING) {
                        return true;
                    }
                    break;
                }
                idDst += nDelta;
            }
            idDst += nDelta;
            while (IN_BOARD(idDst)) {
                int typeDst = pos->curboard[idDst];
                if (typeDst != 0) {
                    if (typeDst == pcOppSide + PIECE_CANNON) {
                        return true;
                    }
                    break;
                }
                idDst += nDelta;
            }
        }
        return false;
    }
    return false;
}

// 走棋动画用
void renderMove(positionStruct *pos, int mv, int typeDst);

// 走一步棋
bool makeMove(positionStruct* pos, int mv) {
    int typeDst;
    typeDst = movePiece(pos, mv);
    if (checked(pos)) {
        undoMovePiece(pos, mv, typeDst);
        return false;
    }
    // 渲染移动过程
    renderMove(pos, mv, typeDst);
    changeSide(pos);
    return true;
}

// 生成所有走法
int generateMoves(positionStruct* pos, int* mvs) {
    int i, j, nGenMoves, nDelta, idSrc, idDst;
    int sideMask, pcOppSide, typeSrc, typeDst;
    // 生成所有走法，需要经过以下几个步骤：

    nGenMoves = 0;
    sideMask = SIDE_TAG(pos->blackPlayer);
    pcOppSide = OPP_SIDE_TAG(pos->blackPlayer);
    for (idSrc = 0; idSrc < 256; idSrc++) {
        // 1. 找到一个本方棋子，再做以下判断：
        typeSrc = pos->curboard[idSrc];
        if ((typeSrc & sideMask) == 0) {
            continue;
        }

        // 2. 根据棋子确定走法
        switch (typeSrc - sideMask) {
        case PIECE_KING:
            for (i = 0; i < 4; i++) {
                idDst = idSrc + kingDelta[i];
                if (!IN_FORT(idDst)) {
                    continue;
                }
                typeDst = pos->curboard[idDst];
                // des 位置无子或者没有自己的棋子
                if ((typeDst & sideMask) == 0) {
                    mvs[nGenMoves] = MOVE(idSrc, idDst);
                    nGenMoves++;
                }
            }
            break;
        case PIECE_ADVISOR:
            for (i = 0; i < 4; i++) {
                idDst = idSrc + advisorDelta[i];
                if (!IN_FORT(idDst)) {
                    continue;
                }
                typeDst = pos->curboard[idDst];
                // des 位置无子或者没有自己的棋子
                if ((typeDst & sideMask) == 0) {
                    mvs[nGenMoves] = MOVE(idSrc, idDst);
                    nGenMoves++;
                }
            }
            break;
        case PIECE_BISHOP:
            for (i = 0; i < 4; i++) {
                idDst = idSrc + advisorDelta[i];
                // 1. 先验证象眼
                if (!(IN_BOARD(idDst) || !HOME_HALF(idDst, pos->blackPlayer) ||
                    pos->curboard[idDst] != 0)) {
                    continue;
                }
                // 2. 继续走一步，无需验证 IN_BOARD
                idDst += advisorDelta[i];
                typeDst = pos->curboard[idDst];
                if ((typeDst & sideMask) == 0) {
                    mvs[nGenMoves] = MOVE(idSrc, idDst);
                    nGenMoves++;
                }
            }
            break;
        case PIECE_KNIGHT:
            for (i = 0; i < 4; i++) {
                // 1. 看看马腿有没有棋子
                idDst = idSrc + kingDelta[i];
                if (pos->curboard[idDst] != 0) {
                    continue;
                }
                // 2. 每个马腿有两个方向
                for (j = 0; j < 2; j++) {
                    idDst = idSrc + knightDelta[i][j];
                    if (!IN_BOARD(idDst)) {
                        continue;
                    }
                    // 3. des 位置无子或者没有自己的棋子
                    typeDst = pos->curboard[idDst];
                    if ((typeDst & sideMask) == 0) {
                        mvs[nGenMoves] = MOVE(idSrc, idDst);
                        nGenMoves++;
                    }
                }
            }
            break;
        case PIECE_ROOK:
            for (i = 0; i < 4; i++) {
                nDelta = kingDelta[i];
                idDst = idSrc + nDelta;
                while (IN_BOARD(idDst)) {
                    typeDst = pos->curboard[idDst];
                    if (typeDst == 0) {
                        mvs[nGenMoves] = MOVE(idSrc, idDst);
                        nGenMoves++;
                    }
                    else {
                        if ((typeDst & pcOppSide) != 0) {
                            mvs[nGenMoves] = MOVE(idSrc, idDst);
                            nGenMoves++;
                        }
                        break;
                    }
                    idDst += nDelta;
                }
            }
            break;
        case PIECE_CANNON:
            for (i = 0; i < 4; i++) {
                nDelta = kingDelta[i];
                idDst = idSrc + nDelta;
                // 1. 按车的走法，不吃子走法
                while (IN_BOARD(idDst)) {
                    typeDst = pos->curboard[idDst];
                    if (typeDst == 0) {
                        mvs[nGenMoves] = MOVE(idSrc, idDst);
                        nGenMoves++;
                    }
                    else {
                        break;
                    }
                    idDst += nDelta;
                }
                idDst += nDelta;
                // 2. 看能否吃子
                while (IN_BOARD(idDst)) {
                    typeDst = pos->curboard[idDst];
                    if (typeDst != 0) {
                        if ((typeDst & pcOppSide) != 0) {
                            mvs[nGenMoves] = MOVE(idSrc, idDst);
                            nGenMoves++;
                        }
                        break;
                    }
                    idDst += nDelta;
                }
            }
            break;
        case PIECE_PAWN:
            // 1. 前进一步是否合法
            idDst = SQUARE_FORWARD(idSrc, pos->blackPlayer);
            if (IN_BOARD(idDst)) {
                typeDst = pos->curboard[idDst];
                if ((typeDst & sideMask) == 0) {
                    mvs[nGenMoves] = MOVE(idSrc, idDst);
                    nGenMoves++;
                }
            }
            // 2. 左右是否能走
            if (AWAY_HALF(idSrc, pos->blackPlayer)) {
                for (nDelta = -1; nDelta <= 1; nDelta += 2) {
                    idDst = idSrc + nDelta;
                    if (IN_BOARD(idDst)) {
                        typeDst = pos->curboard[idDst];
                        if ((typeDst & sideMask) == 0) {
                            mvs[nGenMoves] = MOVE(idSrc, idDst);
                            nGenMoves++;
                        }
                    }
                }
            }
            break;
        }
    }
    return nGenMoves;
}

// 判断走法是否合理
bool legalMove(positionStruct* pos, int mv) {
    int idSrc, idDst, sqPin;
    int sideMask, typeSrc, typeDst, nDelta;
    // 判断走法是否合法，需要经过以下的判断过程：

    // 1. 判断起始格是否有自己的棋子
    idSrc = SRC(mv);
    typeSrc = pos->curboard[idSrc];
    sideMask = SIDE_TAG(pos->blackPlayer);
    if ((typeSrc & sideMask) == 0) {
        return false;
    }

    // 2. 判断目标格是否有自己的棋子
    idDst = DST(mv);
    typeDst = pos->curboard[idDst];
    if ((typeDst & sideMask) != 0) {
        return false;
    }

    // 3. 根据棋子的类型检查走法是否合理
    switch (typeSrc - sideMask) {
    case PIECE_KING:
        return IN_FORT(idDst) && KING_SPAN(idSrc, idDst);
    case PIECE_ADVISOR:
        return IN_FORT(idDst) && ADVISOR_SPAN(idSrc, idDst);
    case PIECE_BISHOP:
        return SAME_HALF(idSrc, idDst) && BISHOP_SPAN(idSrc, idDst) &&
            pos->curboard[BISHOP_PIN(idSrc, idDst)] == 0;
    case PIECE_KNIGHT:
        sqPin = KNIGHT_PIN(idSrc, idDst);
        return sqPin != idSrc && pos->curboard[sqPin] == 0;
    case PIECE_ROOK:
    case PIECE_CANNON:
        if (SAME_RANK(idSrc, idDst)) {
            nDelta = (idDst < idSrc ? -1 : 1);
        }
        else if (SAME_FILE(idSrc, idDst)) {
            nDelta = (idDst < idSrc ? -16 : 16);
        }
        else {
            return false;
        }
        sqPin = idSrc + nDelta;
        while (sqPin != idDst && pos->curboard[sqPin] == 0) {
            sqPin += nDelta;
        }
        if (sqPin == idDst) {
            return typeDst == 0 || typeSrc - sideMask == PIECE_ROOK;
        }
        else if (typeDst != 0 && typeSrc - sideMask == PIECE_CANNON) {
            sqPin += nDelta;
            while (sqPin != idDst && pos->curboard[sqPin] == 0) {
                sqPin += nDelta;
            }
            return sqPin == idDst;
        }
        else {
            return false;
        }
    case PIECE_PAWN:
        if (AWAY_HALF(idDst, pos->blackPlayer) &&
            (idDst == idSrc - 1 || idDst == idSrc + 1)) {
            return true;
        }
        return idDst == SQUARE_FORWARD(idSrc, pos->blackPlayer);
    default:
        return false;
    }
}

// 判断是否被杀
bool isMate(positionStruct* pos) {
    int i, nGenMoveNum, pcCaptured;
    int mvs[MAX_GEN_MOVES];

    nGenMoveNum = generateMoves(pos, mvs);
    for (i = 0; i < nGenMoveNum; i++) {
        pcCaptured = movePiece(pos, mvs[i]);
        if (!checked(pos)) {
            undoMovePiece(pos, mvs[i], pcCaptured);
            return false;
        }
        else {
            undoMovePiece(pos, mvs[i], pcCaptured);
        }
    }
    return true;
}

void startup(positionStruct* pos) {  // 初始化棋盘
    pos->blackPlayer = false;
    memcpy(pos->curboard, boardStartup, 256);
}

/********************************************** 图形界面、鼠标输入 *******************************************************/
double moveX = 0;
double moveY = 0;
int idSelected = 0;

// 点击格子事件处理
void click(int id) {
    int type, mv;
    type = pos.curboard[id];

#ifndef NDEBUG
    for (int i = 0; i < 256; i++) {
        if (i == id)  printf("\033[31m%02d  \033[0m", pos.curboard[i]);
        else printf("%02d  ", pos.curboard[i]);
        if ((i + 1) % 16 == 0) printf("\n");
    }
#endif

    if ((type & SIDE_TAG(pos.blackPlayer)) != 0) {
        // 如果点击自己的子，那么直接选中该子
        idSelected = id;
        moveX = X_CENTER(id);
        moveY = Y_CENTER(id);
    }
    else if (idSelected != 0) {
        // 如果点击的不是自己的子，但有子选中了(一定是自己的子)，那么走这个子
        mv = MOVE(idSelected, id);
        if (legalMove(&pos, mv)) {
            if (makeMove(&pos, mv)) {
                idSelected = 0;
                if (isMate(&pos)) {
                    // 如果分出胜负，那么播放胜负的声音，并且弹出不带声音的提示框
                    MessageBoxW(NULL, L"祝贺你取得胜利！", L"厉害哇", MB_OKCANCEL);
                }
            }
            else {
                MessageBoxW(NULL, L"将军！", L"警告", MB_OKCANCEL);
            }
        }
    }

}

void init() {
    setlocale(LC_ALL, "");
    LOGFONT font;
#ifdef NDEBUG
    initgraph(BOARD_WIDTH, BOARD_HEIGHT);
#else 
    initgraph(BOARD_WIDTH, BOARD_HEIGHT, EW_SHOWCONSOLE);
#endif
    gettextstyle(&font);                           // 获取当前字体设置
    font.lfHeight = TEXT_HEIGHT;    // 设置字体高度
    _tcscpy_s(font.lfFaceName, _T("楷体"));        // 设置字体为“黑体”(高版本 VC 推荐使用 _tcscpy_s 函数)
    font.lfQuality = ANTIALIASED_QUALITY;          // 设置输出效果为抗锯齿  
    font.lfWeight = FW_HEAVY;
    settextstyle(&font);                        // 设置字体样式
    setlinestyle(PS_SOLID | PS_JOIN_ROUND, 2);
    BeginBatchDraw();
}

void drawLines(int row1, int col1, int row2, int col2) {
    line(BOARD_EDGE + row1 * PIECE_SIZE, BOARD_EDGE + col1 * PIECE_SIZE, \
        BOARD_EDGE + row2 * PIECE_SIZE, BOARD_EDGE + col2 * PIECE_SIZE);
}

void drawSelected() {
    int x, y, id = idSelected;
    if (id == 0) return;
    if (!pos.curboard[id])    return;

    // 获取中心点的像素坐标
    x = (int)moveX;
    y = (int)moveY;

    setlinecolor(BROWN);
    settextcolor(BLACK);
    // 填充色和字体背景色保持一致
    setfillcolor(0X6FEF00);
    setbkcolor(0X6FEF00);

    if (pos.curboard[id] & 8) {
        setlinecolor(RED);
        settextcolor(RED);
    }

    fillcircle(x, y, PIECE_RADIUS - 3);
    outtextxy(x - TEXT_HEIGHT / 2, y - TEXT_HEIGHT / 2, LABLE(id));
    circle(x, y, PIECE_RADIUS - 6);
    // 恢复背景颜色
    setbkcolor(BLACK);
}

void drawPiece(int id) {
    int x, y;
    if (id == idSelected)  return;   // selected ID 单独绘制
    if (!pos.curboard[id])    return;    // 位置空, 无棋子

    // 获取中心点的像素坐标
    x = X_CENTER(id);
    y = Y_CENTER(id);

    setlinecolor(BROWN);
    settextcolor(BLACK);
    setbkcolor(0x555555);
    setfillcolor(0x555555);

    if (pos.curboard[id] & 8) {
        setlinecolor(RED);
        settextcolor(RED);
        setbkcolor(0x116677);
        setfillcolor(0x116677);
    }

    fillcircle(x, y, PIECE_RADIUS - 3);
    outtextxy(x - TEXT_HEIGHT / 2, y - TEXT_HEIGHT / 2, LABLE(id));
    circle(x, y, PIECE_RADIUS - 6);

    setbkcolor(BLACK);
}

void render() {
    cleardevice();
    setlinecolor(YELLOW);
    // 10 条横线
    for (int i = 0; i < 10; i++)
        drawLines(0, i, 8, i);
    // 9 条竖线
    for (int i = 0; i < 9; i++) {
        if (i == 0 || i == 8)
            drawLines(i, 0, i, 9);
        else {
            drawLines(i, 0, i, 4);
            drawLines(i, 5, i, 9);
        }
    }
    // 九宫格
    drawLines(3, 0, 5, 2);
    drawLines(3, 2, 5, 0);
    drawLines(3, 7, 5, 9);
    drawLines(3, 9, 5, 7);
    // 绘制棋子
    for (int id = 51; id <= 203; id++)
        drawPiece(id);
    // 单独绘制 可以保证移动时在最上面显示
    drawSelected();
    FlushBatchDraw();
}



void renderMove(positionStruct *pos, int mv, int typeDst) {
    int idSrc, idDst;
    undoMovePiece(pos, mv, typeDst);

    idSrc = SRC(mv);
    idDst = DST(mv);

    moveX = X_CENTER(idSrc);
    moveY = Y_CENTER(idSrc);

    double dx = (double)X_CENTER(idDst) - X_CENTER(idSrc);
    double dy = (double)Y_CENTER(idDst) - Y_CENTER(idSrc);
    double len = sqrt(dx * dx + dy * dy);
    dx /= len; dy /= len;

    while (fabs(moveX - X_CENTER(idDst)) > 3 ||
           fabs(moveY - Y_CENTER(idDst)) > 3) {
        moveX += dx;
        moveY += dy;
        render();
    }

    movePiece(pos, mv);
}

// 通过鼠标点击的像素坐标获取索引号
int clikeId(int x, int y) {
    int row = RANK_TOP + abs(y - BOARD_EDGE) / PIECE_SIZE;
    int col = FILE_LEFT + abs(x - BOARD_EDGE) / PIECE_SIZE;
    int id = row * 16 + col;

    // 计算在哪个顶点
    for (int i = 0; i < 4; i++) {
        LOG("clike id[%d], row[%d], col[%d], %ls\n", id, ROW(id), COL(id), LABLE(id));
        int dx = x - X_CENTER(id);
        int dy = y - Y_CENTER(id);
        if (dx * dx + dy * dy < PIECE_RADIUS * PIECE_RADIUS)
            return id;
        if (i & 0X1) id += 15;
        else id += 1;
    }

    return -1;
}

void processInput() {
    int id = -1;
    ExMessage msg;
    do {
        // 阻塞等待玩家输入
        getmessage(&msg, EM_MOUSE);
    } while (!msg.lbutton || (id = clikeId(msg.x, msg.y)) == -1);

    //LOG("clike at[%d, %d]  id[%d]\n", msg.x, msg.y, id);
    LOG("clike id[%d], row[%d], col[%d], %ls\n", id, ROW(id), COL(id), LABLE(id));
    LOG("======= > id : %d\n", id);

    click(id);
}


void update() {

}

int main() {
    init();
    startup(&pos);
    while (1) {
        LOG("\n\nrender...\n");
        render();
        processInput();
        update();     // 人机对战，电脑做出反应
    }
    closegraph();
    return 0;
}

