#include "dragtreewidget.h"

DragTreeWidget::DragTreeWidget() {

    singleClickTimer = new QTimer(this);
    singleClickTimer->setSingleShot(true);  // Ожидаем только одно срабатывание
    singleClickTimer->setInterval(300);  // Задержка в 300 мс для различения кликов

    setDragEnabled(true);
    setAcceptDrops(true);
    setAnimated(true);
    setDragDropMode(QAbstractItemView::InternalMove);

    this->setHeaderHidden(true);
    //this->setWordWrap(false);

    this->setStyleSheet("QTreeWidget{"
        "   font-family: Bahnschrift;"
        "   font-size: 16px;"
        "   outline: none;"
        "}"
        "QTreeWidget::item {"
        "   outline: none; "
        "   border: none;"
        "}"
        "QTreeWidget::item:focus { "
        "   font-family: Bahnschrift; "
        "   font-size: 16px;"
        "   outline: none;"
        "   border: none;"
        "   background-color: rgb(247,223,156);"
        "}"
        "QTreeWidget::item:selected { "
        "   background-color: rgb(247,223,156);"
        "   outline: none;"
        "}"
       "QTreeWidget::item:hover { "
       "   background-color: rgb(247,223,156);"
       "}"
        "QTreeWidget::indicator:checked {"
        "	image: url(:/pictures/resources/checkbox-done.svg);"
        "}"
        "QTreeWidget::indicator:unchecked {"
        "	image: url(:/pictures/resources/checkbox-undone.svg);"
        "}"
    );

}
