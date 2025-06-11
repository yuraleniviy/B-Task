#ifndef DRAGTREEWIDGET_H
#define DRAGTREEWIDGET_H

#include <QMainWindow>
#include <QObject>
#include <QTreeWidget>
#include <QWidget>
#include <QMimeData>
#include <QDropEvent>
#include <QDrag>
#include <QTimer>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class DragTreeWidget : public QTreeWidget
{
    Q_OBJECT
public:
    DragTreeWidget();
    QTreeWidgetItem *currentMovingItem;
    QTimer *singleClickTimer;  // Таймер для одиночного клика

protected:

    void mousePressEvent(QMouseEvent *event) override
    {
        currentMovingItem = itemAt(event->position().toPoint());  // Получаем элемент, на котором произошло нажатие
        if (!currentMovingItem) return;

        QVariantList itemData = currentMovingItem->data(0, Qt::UserRole).toList();
        bool isFolder = itemData[0].toBool();
        if (!isFolder) {
            QDrag *drag = new QDrag(this);
            QMimeData *mimeData = new QMimeData;

            const QByteArray zaglushka;
            mimeData->setData("application/x-task-item", zaglushka);

            drag->setMimeData(mimeData);
            drag->exec();

            QRect checkboxRect = visualItemRect(currentMovingItem);  // Получаем прямоугольник, в котором отрисован элемент
            checkboxRect = QRect(checkboxRect.left() - 5, checkboxRect.top(), 35, checkboxRect.height()); // Модифицируем под размеры чекбокса
            if (checkboxRect.contains(event->position().toPoint())) {  // Проверяем, попал ли клик в область чекбокса
                if (currentMovingItem->checkState(0) == Qt::Unchecked) {
                    currentMovingItem->setCheckState(0, Qt::Checked);
                } else {
                    currentMovingItem->setCheckState(0, Qt::Unchecked);
                }
            } else {
                // Если клик не по чекбоксу, запускаем таймер для одиночного клика и инициируем перетаскивание
                if (!singleClickTimer->isActive()) {
                    singleClickTimer->start();  // Запускаем таймер для одиночного клика
                }
            }
        }

        QTreeWidget::mousePressEvent(event);
    }

    void mouseDoubleClickEvent(QMouseEvent *event) override
    {
        singleClickTimer->stop();

        if (currentMovingItem) {
            qDebug() << "Double click on item";
            emit itemDoubleClicked(currentMovingItem, 0);
        }
    }


    void dragEnterEvent(QDragEnterEvent *event) override
    {
        if (event->mimeData()->hasFormat("application/x-task-item")) {
            event->acceptProposedAction();
            currentMovingItem = itemAt(event->position().toPoint());  // Получаем текущий элемент
            qDebug() << "drag Enter";
        } else {
            event->ignore();  // Если формат не подходит, игнорируем событие
        }
    }

    void dragMoveEvent(QDragMoveEvent *event) override {
        //qDebug() << "dragMove event";
        event->acceptProposedAction();
    }


    void dropEvent(QDropEvent *event) override
    {
        //qDebug() << "drop event";
        QTreeWidgetItem *targetItem = itemAt(event->position().toPoint());  // Получаем элемент, на который был сброшен объект
         if (!targetItem || !currentMovingItem) return;

        // Проверяем, что элемент - папка
        QVariantList folderData = targetItem->data(0, Qt::UserRole).toList();
        bool isFolder = folderData[0].toBool();

        if (isFolder) {
            int folderId = folderData[1].toInt();  // Получаем ID папки

            QVariantList taskData = currentMovingItem->data(0, Qt::UserRole).toList();
            int taskId = taskData[1].toInt();  // Получаем ID задачи

            qDebug() << "drop event" << taskId << " on " << folderId;

            if (currentMovingItem->parent()) {
                currentMovingItem->parent()->removeChild(currentMovingItem);
            }

            targetItem->addChild(currentMovingItem);
            targetItem->setExpanded(true);

            moveTaskToNewFolder(taskId, folderId);

            this->update();
        }

        event->acceptProposedAction();
    }

    void moveTaskToNewFolder(int taskId, int folderId)
    {
        QSqlQuery query;
        query.prepare("UPDATE tasks SET task_folder_id = :folder_id WHERE task_id = :task_id");
        query.bindValue(":folder_id", folderId);
        query.bindValue(":task_id", taskId);
        if (!query.exec()) {
            qDebug() << "Ошибка при обновлении задачи в базе данных: " << query.lastError().text();
        } else {
            //qDebug() << "Задача успешно перемещена в новую папку!";
        }
        emit taskDraggedToFolder();
    }

signals:
    void taskDraggedToFolder();
};

#endif // DRAGTREEWIDGET_H
