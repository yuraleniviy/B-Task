#ifndef CALENDAR_H
#define CALENDAR_H

#include <QCalendarWidget>

class Calendar : public QCalendarWidget
{
public:
    Calendar(QWidget *parent = nullptr);
    void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const override;

};

#endif // CALENDAR_H
