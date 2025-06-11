#ifndef CUSTOMCALENDARWIDGET_H
#define CUSTOMCALENDARWIDGET_H

#include <QCalendarWidget>
#include <QMainWindow>
#include <QObject>
#include <QWidget>
#include <QDate>
#include <QTextCharFormat>

class CustomCalendarWidget : public QCalendarWidget
{
    Q_OBJECT
public:
    explicit CustomCalendarWidget(QWidget *parent = nullptr);
    QMap<QDate, QString> dateToIconPath;

    void setIconForDate(const QDate &date, const QString &iconPath);
// protected:
//     void paintCell(QPainter *painter, const QRect &rect, const QDate &date) const;

private:
};

#endif // CUSTOMCALENDARWIDGET_H
