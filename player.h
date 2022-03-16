#ifndef PLAYER_H
#define PLAYER_H


#include "chessboard.h"
#include <QString>

class Player;
class ChessBoard;
class Player
{
public:
    Player(int _color=1, int _spawn=0, int _target=3, QString _name="张三");
    ~Player();
    ChessBoard *parentChessBoard;
    int chess_num=10, color, spawn, target;
    QString name;
    Marble* chess[10];
    void addTo(ChessBoard *_parentChessBoard=0);
};

#endif // PLAYER_H
