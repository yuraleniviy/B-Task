#include "calendar.h"

#include <QPainter>

Calendar::Calendar(QWidget *parent)
    : QCalendarWidget(parent)
{}

void Calendar::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
{
    // change color for dates before current date
    if (date < QDate::currentDate())
    {
        painter->save();
        // set color for the text
        painter->setPen(QColor(64, 64, 64));
        // draw text with new color
        painter->drawText(rect, Qt::TextSingleLine | Qt::AlignCenter, QString::number(date.day()));

        // here you can draw anything you want

        painter->restore();
    } else {
        // draw cell in standard way
        QCalendarWidget::paintCell(painter, rect, date);
    }
}
