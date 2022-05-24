#include "clickableqlabel.h"


ClickableQLabel::ClickableQLabel(Widget *_parentWindow):
QLabel(_parentWindow){
;
}

void ClickableQLabel::mousePressEvent(QMouseEvent *event)
{
    emit clicked();
    QLabel::mousePressEvent(event);
}
