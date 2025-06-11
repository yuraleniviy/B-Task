#include "customcalendarwidget.h"
#include <QPainter>

CustomCalendarWidget::CustomCalendarWidget(QWidget *parent)
    : QCalendarWidget(parent)
{
    setGridVisible(false);
    setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
    setStyleSheet(
        "QCalendarWidget{"
        "   background-color:white;"
        "   font-family: Bahnschrift;"
        "   font-size: 16px;"
        "}"
        "QCalendarWidget QTableView::item {"
        "    border-radius: 30px;"  // Закругление ячеек
        "}"
        "QCalendarWidget QTableView::item:selected {" // выделенный ден
        "   image: url(:/pictures/resources/calendar-selected-date.svg);"
        "   background-color: rgba(247,223,156,0);"
        "}"
        "QCalendarWidget QTableView:focus {"     // Убираем фокусную рамку
        "    outline: none;"
        "}"
        "QCalendarWidget QTableView:disabled{" // дни другого месяца
        "color: rgba(0, 0, 0,100);"
        "}"
        "QCalendarWidget QWidget#qt_calendar_navigationbar {" //верхнее меню
        "    background-color: rgb(247,223,156);"
        "    min-height: 40px;"
        "    border-radius: 5px;"
        "    border: 2px solid gray;"
        "}"
        "QCalendarWidget QToolButton {"                     //текст меню
        "    color: black;"
        "    font-size: 18px;"
        "border-radius: 20px;"
        "}"
        "#qt_calendar_nextmonth {"                      //стрелка вправо
        "   icon-size: 30px;"
        "   qproperty-icon: url(:/pictures/resources/right-arrow.svg);"
        "   border-radius: 5px;"
        "}"
        "#qt_calendar_prevmonth {"                     //стрелка влево
        "    icon-size: 30px;"
        "    qproperty-icon: url(:/pictures/resources/left-arrow.svg);"
        "    border-radius: 5px;"
        "}"
        "QCalendarWidget QMenu {"
        "   background-color: white;"
        "   border: 2px solid gray;"
        "   border-radius: 7px;"
        "   padding: 4px 4px;"
        "   font-family: Bahnschrift;"
        "   font-size: 16px;"
        "}"
        "QCalendarWidget QMenu::item:selected {"
        "   background-color: rgb(247,223,156);"
        "   border-radius: 5px;"
        "}"
        "QCalendarWidget QSpinBox {"
        "   background-color: rgb(247,223,156);"
        "   width: 80px;"
        "}"
        );

    // CALENDAR -> TODAY DESIGN
    QTextCharFormat todayFormat;
    todayFormat.setForeground(QBrush(Qt::red));
    setDateTextFormat(QDate::currentDate(), todayFormat);
}

void CustomCalendarWidget::setIconForDate(const QDate &date, const QString &iconPath)
{
    qDebug() << "set Icon";
    dateToIconPath[date] = iconPath;
    updateCell(date);
}

// void CustomCalendarWidget::paintCell(QPainter *painter, const QRect &rect, const QDate &date) const
// {
//     qDebug() << "paint cell";
//     // Вызываем стандартную отрисовку ячейки
//     QCalendarWidget::paintCell(painter, rect, date);

//     // Если для этой даты задана иконка — рисуем поверх
//     if (dateToIconPath.contains(date)) {
//         QPixmap icon(dateToIconPath.value(date));
//         if (!icon.isNull()) {
//             QSize iconSize(14, 14);
//             QPoint iconPos = rect.bottomRight() - QPoint(iconSize.width() + 2, iconSize.height() + 2);
//             painter->drawPixmap(iconPos, icon.scaled(iconSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
//         }
//     }
// }
