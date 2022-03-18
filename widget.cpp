#include "widget.h"
#include "ui_widget.h"
#include "chessboard.h"
#include <QPixmap>
#include <QPalette>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , playerNum(2)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);
    ui->label->setVisible(false);
    setWindowTitle("中国跳棋 By panjd123");
    this->setMinimumSize(QSize(800, 625));
    this->setMaximumSize(QSize(800, 625));
    QPixmap bkgnd(":/images/background.png");
    bkgnd = bkgnd.scaled(this->size(),Qt::IgnoreAspectRatio);
    QPalette palatte;
    palatte.setBrush(QPalette::Window, bkgnd);
    this->setPalette(palatte);

    chessBoard = new ChessBoard(this,playerNum);

    btnSetPlayerNum = new QPushButton(this);
    btnSetPlayerNum->setGeometry(30,360,100,30);
    btnSetPlayerNum->setText("SetPlayerNum");
    btnSetPlayerNum->setCursor(Qt::PointingHandCursor);
    connect(this->btnSetPlayerNum,&QPushButton::clicked,this,&Widget::on_btnSetPlayerNum_clicked);

    labelPlayerNum = new QLabel(this);
    labelPlayerNum->setGeometry(30,410,30,30);
    labelPlayerNum->setText("人数:");

    editPlayerNum = new QLineEdit(this);
    editPlayerNum->setGeometry(60,410,70,30);
//    editPlayerNum->setText("2");
    editPlayerNum->setAlignment(Qt::AlignHCenter);
    editPlayerNum->setClearButtonEnabled(true);
    editPlayerNum->setPlaceholderText("1,2,3,4,6");
}

Widget::~Widget()
{
    delete ui;
    delete chessBoard;
}

void Widget::paintEvent(QPaintEvent *)
{
    return ;
    QPainter painter(this);
    painter.setPen(QColor(Qt::black));
    painter.setBrush(QBrush(Qt::yellow));
    int x=693,y=190,r=12;
    painter.drawEllipse(249-r,y-r,2*r,2*r);
    painter.drawEllipse(x-r,y-r,2*r,2*r);
    painter.drawEllipse(471-r,312-r,2*r,2*r);
}

void Widget::on_btnSetPlayerNum_clicked()
{
    qDebug()<<chessBoard;
    if(this->chessBoard!=NULL){
        delete this->chessBoard;
        this->chessBoard=NULL;
    }
//    SetPlayerNumWindow wi=SetPlayerNumWindow();
//    wi.show();
    if(editPlayerNum->text().toInt()){
        playerNum = editPlayerNum->text().toInt();
    }
    chessBoard = new ChessBoard(this,playerNum);
    chessBoard->show();
    qDebug()<<chessBoard;
}
