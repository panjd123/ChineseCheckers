#include "serverwidget.h"
#include "ui_serverwidget.h"
#include "util.h"

ServerWidget::ServerWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ServerWidget)
{
    ui->setupUi(this);
    this->server = new NetworkServer(this);

    connect(this->server, &NetworkServer::receive, this, &ServerWidget::receiveData);
    //
}

int ServerWidget::receiveData(QTcpSocket *client, NetworkData data) {
    Room *room;
    bool flag;
    int result;
    QString ErrorCode;
    switch(data.op) {
    case OPCODE::MOVE_OP:
        this->ui->textBrowser->append("receive: MOVE_OP");
        room = roomList[0];
        if(room->isGameRunning() == false)
        {
            server->send(client, NetworkData(OPCODE::ERROR_OP, convertToQStr(ERRCODE::ROOM_NOT_RUNNING), ""));
            break;
        }
        result = room->w->chessBoard->serverMoveProcess(data.data1, data.data2);
       //Debug()<<"Why    "<<result; q
        switch(result) {
        case 1:
            for(auto i:room->players) {
                if(i->getSocket() == client) continue;
                server->send(i->getSocket(), data);
            }
            startTurn(room->w->chessBoard->activatedPlayer->name);
            break;
        case -1:
        case 0:
            ErrorCode = (result == 0)?"INVALID_MOVE":"OUTTURN_MOVE";
            server->send(client, NetworkData(OPCODE::ERROR_OP, ErrorCode, ""));
            break;
        default:
            qDebug() << "ERROR at server widget";
        }
        break;

    case OPCODE::JOIN_ROOM_OP:
        this->ui->textBrowser->append("receive: JOIN_ROOM_OP");
        if(invalidName(data.data2)) {   //check if username is absloutly unacceptable.
            server->send(client, NetworkData(OPCODE::ERROR_OP, "INVALID_JOIN", "invalid username. Please use another one"));
            return 0;
        }
        room = findRoom(data.data1);
        if(room == NULL) {  //A new room should be created if not found.
            room = new Room(data.data1);
            roomList.push_back(room);
        }
        flag = false;
        for(auto i:room->players) { //Look for a duplicate username
            if(i->name == data.data2) {
                flag = true;
                server->send(client, NetworkData(OPCODE::ERROR_OP, "INVALID_JOIN", "Duplicate username."));
                break;
            }
        }
        if(flag) break;
        for(auto i:room->players) {
            server->send(i->getSocket(), NetworkData(OPCODE::JOIN_ROOM_OP, data.data2, ""));
        }
        server->send(client, NetworkData(OPCODE::JOIN_ROOM_REPLY_OP, room->playerNameListStr(), room->playerStateListStr()));
        room->addPlayer(new ServerPlayer(data.data2, client));
        this->ui->textBrowser->append("send: JOIN_ROOM_REPLY_OP");
        break;
    case OPCODE::LEAVE_ROOM_OP:
        this->ui->textBrowser->append("receive: LEAVE_ROOM_OP");
        room = nullptr;
        for(auto r:roomList) {
            if (r->RoomID() == data.data1) {
                room = r;
                break;
            }
        }
        if(room == nullptr) server->send(client, NetworkData(OPCODE::ERROR_OP, "INVALID_REQ", "room name not found"));
        flag = false;
        for(auto i:room->players) {
            if(i->name == data.data2) {
                flag = true;
                room->removePlayer(i);
                for(auto j:room->players) {
                    server->send(j->getSocket(), NetworkData(OPCODE::LEAVE_ROOM_OP, data.data2, ""));
                }
                break;
            }
        }
        if(!flag) server->send(client, NetworkData(OPCODE::ERROR_OP, "NOT_IN_ROOM", ""));
        break;
    case OPCODE::PLAYER_READY_OP:
        this->ui->textBrowser->append("receive: PLAYER_READY_OP");
        room = roomList[0];
        for(auto i:room->players) {
            if(i->name == data.data1) i->Ready();
            server->send(i->getSocket(), NetworkData(OPCODE::PLAYER_READY_OP, data.data1, ""));
        }
        int N;
        N = room->players.size();
        if(N >= 2 && N != 5) {
            int count = 0;
            for(auto i:room->players)
                if (i->isReady()) count++;

            if(count == N) {
                this->ui->textBrowser->append("send: START_GAME_OP");
                QString data2;
                for(qsizetype i=0;i<room->players.size();i++)
                    data2+=getID(board::playerSpawn[room->players.size()][i])+" ";
                data2.chop(1);
                for(auto i:room->players) {
                    server->send(i->getSocket(), NetworkData(OPCODE::START_GAME_OP,
                        room->playerNameListStr(), data2));
                }
                room->changeGameState();
                connect(room->w->chessBoard, &ChessBoard::overtime, this, &ServerWidget::overtime);
                connect(room->w->chessBoard, &ChessBoard::endgame, this, &ServerWidget::endGame);
                connect(room->w->chessBoard, &ChessBoard::victory, this, &ServerWidget::sendVictory);
                connect(room->w->chessBoard, &ChessBoard::startTurn, this,&ServerWidget::startTurn);
                server->send(room->players[0]->getSocket(), NetworkData(OPCODE::START_TURN_OP, "", ""));
            }
        }
        break;
    default:
        server->send(client, NetworkData(OPCODE::ERROR_OP, convertToQStr(ERRCODE::INVALID_REQ), ""));
        break;
    }
    return 0;
}

ServerWidget::~ServerWidget()
{
    for(auto i:roomList) delete i;
    delete ui;
}

bool ServerWidget::invalidName(QString &name)
{
    if(name.size() > 20) return true;
    std::string s = name.toStdString();
    for(auto i:s)
        if (!isalnum(i) && i != '_')
             return true;
    return false;
}

Room* ServerWidget::findRoom (QString &roomName)
{
    // future plan: using a better search algorithm. Maybe a hash will work.
    // anyway, there's no need optimizing it currently...
    for (auto i:roomList) {
        if(i->RoomID() == roomName)
        {
            return i;
        }
    }
    return NULL;
}

void ServerWidget::__receiveCommand()
{
    QString words = ui->textEdit->toPlainText();
    ui->textEdit->clear();
    auto list = words.split(" ", Qt::SkipEmptyParts);
    auto &cmd = list[0];
    if(cmd == "help") {
        write("?");
    }else if (cmd == "size") {      //size <roomID=0>
        write("当前房间内人数:" + std::to_string(roomList[0]->players.size()));
    }else if (cmd == "start") {     //start
        auto room = roomList[0];
        QString p;
        switch(room->players.size()) {
        case 2:p = "A D"; break;
        case 3:p = "A C E"; break;
        case 4:p = "A B D E";break;
        case 6:p = "A B C D E F";break;
        }
        int k = 0;
        for(auto i:room->players) {
            i->startArea = p[2*k + 1]; k++;
            server->send(i->getSocket(), NetworkData(OPCODE::START_GAME_OP, room->playerNameListStr(), p));
            server->send(i->getSocket(), NetworkData(OPCODE::START_TURN_OP, i->startArea, time(NULL)));
        }
    }else if (cmd == "display") {
        this->roomList[0]->w->show();
    }
    return;
}

void ServerWidget::overtime(QString data) {
    auto room = roomList[0];
    for(auto i:room->players) {
        server->send(i->getSocket(), NetworkData(OPCODE::MOVE_OP, data, "-1"));
        startTurn(room->w->chessBoard->activatedPlayer->name);
    }
}

void ServerWidget::endGame(QString data)
{
    auto room = roomList[0];
    for(auto i:room->players) {
        server->send(i->getSocket(), NetworkData(OPCODE::END_GAME_OP, data, ""));
    }
}

void ServerWidget::sendVictory(QString name)
{
    auto room = roomList[0];
    room->changeGameState();
    for(auto i:room->players) {
        if(i->name == name)
            server->send(i->getSocket(), NetworkData(OPCODE::END_TURN_OP, "", ""));
    }
    delete room;
    roomList.pop_back();
}

void ServerWidget::startTurn(QString name)
{
    qDebug()<<"ting wo shuo xie xie ni"<<name;
    auto room = roomList[0];
    for(auto i:room->players) {
        if(i->name == name)
            server->send(i->getSocket(), NetworkData(OPCODE::START_TURN_OP, "", ""));
    }
}

void ServerWidget::write (std::string str) {
    this->ui->textBrowser->append(QString::fromStdString(str));
    return;
};

