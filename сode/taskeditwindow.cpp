#include "taskeditwindow.h"
#include "ui_taskeditwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <qcalendarwidget.h>
#include <QScreen>


TaskEditWindow::TaskEditWindow(QWidget *parent)
    : QDialog(parent), ui(new Ui::TaskEditWindow), p_currentItem(nullptr) {
    ui->setupUi(this);
    ui->editTaskName->setFocus();
    setModal(true);
    setDefaultDesign();
}

TaskEditWindow::~TaskEditWindow() {
    delete ui;
}

void TaskEditWindow::showAt(const QPoint &pos, const QString &text, QTreeWidgetItem *item) {
    setCurrentEditingTaskId(item);
    addTaskInfoToDesign(item);

    QSize windowSize = size();
    QPoint topLeft = pos;

    QScreen *screen = QGuiApplication::screenAt(pos);
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }

    QRect screenGeometry = screen->availableGeometry();
    QRect targetRect(topLeft, windowSize + QSize(0, 20));

    if (screenGeometry.contains(targetRect)) {
        move(topLeft);
    } else {
        int x = pos.x();
        int y = pos.y() - windowSize.height();
        move(QPoint(x, y));
    }
    show();
    ui->editTaskName->setFocus();
}

void TaskEditWindow::loadFoldersToChoose()
{
    ui->editTaskFolder->clear();
    QSqlQuery folderQuery("SELECT folder_id, folder_name, folder_color FROM folders");
    while (folderQuery.next()) {
        QVariant folderId;
        folderId = folderQuery.value(0).toInt();
        QString folderColor = folderQuery.value(2).toString();
        if(folderQuery.value(1).toString() == "inbox"){ui->editTaskFolder->addItem(QIcon(":/pictures/resources/inbox.svg"),folderQuery.value(1).toString(), folderId);}
        else{
            if(folderColor == "white"){ui->editTaskFolder->addItem(QIcon(":/pictures/resources/color-white.svg"),folderQuery.value(1).toString(), folderId);}
            if(folderColor == "red"){ui->editTaskFolder->addItem(QIcon(":/pictures/resources/color-red.svg"),folderQuery.value(1).toString(), folderId);}
            if(folderColor == "green"){ui->editTaskFolder->addItem(QIcon(":/pictures/resources/color-green.svg"),folderQuery.value(1).toString(), folderId);}
            if(folderColor == "blue"){ui->editTaskFolder->addItem(QIcon(":/pictures/resources/color-blue.svg"),folderQuery.value(1).toString(), folderId);}
        }
    }
}

void TaskEditWindow::setDefaultDesign()
{
    ui->editTaskPriority->clear();
    ui->editTaskPriority->addItem("низкий");
    ui->editTaskPriority->addItem(QIcon(":/pictures/resources/green-exc.svg"),"средний");
    ui->editTaskPriority->addItem(QIcon(":/pictures/resources/red-exc.svg"),"высокий");
    ui->editTaskDuration->clear();
    ui->editTaskDuration->addItem("30 минут");
    ui->editTaskDuration->addItem("1-2 часа");
    ui->editTaskDuration->addItem(">2 часов");

    loadFoldersToChoose();

    ui->editTaskDeadline->setDate(QDate::currentDate().addDays(7));
    QDateTime inThreeDays = QDateTime::currentDateTime().addDays(3);
    QTime seventeen(17, 0);
    inThreeDays.setTime(seventeen);
    ui->editTaskReminder->setDateTime(inThreeDays);
    ui->editTaskReminder->setMinimumDateTime(QDateTime::currentDateTime().addSecs(-3600));

    ui->editTaskIsDeadline->setChecked(false);
    ui->editTaskIsReminder->setChecked(false);

    QCalendarWidget *calendarDeadline = ui->editTaskDeadline->calendarWidget();
    QCalendarWidget *calendarNotification = ui->editTaskReminder->calendarWidget();

    QTextCharFormat todayFormat;
    todayFormat.setForeground(QBrush(Qt::red));
    calendarDeadline->setDateTextFormat(QDate::currentDate(), todayFormat);
    calendarNotification->setDateTextFormat(QDate::currentDate(), todayFormat);
}

void TaskEditWindow::addTaskInfoToDesign(QTreeWidgetItem *item)
{
    QVariantList taskData = item->data(0, Qt::UserRole).toList();
    int taskId = taskData.value(1).toInt();

    QSqlQuery query;
    query.prepare("SELECT * FROM tasks WHERE task_id = :id");
    query.bindValue(":id", taskId);
    setDefaultDesign();

    if (query.exec()) {
        if (query.next()) {
            ui->editTaskName->setText(query.value("task_name").toString());
            ui->editTaskNote->setPlainText(query.value("task_note").toString());
            int targetFolderIndex = -1; //not found
            int targetFolderId = query.value("task_folder_id").toInt();
            qDebug() << "id: " << targetFolderId;
            for (int i = 0; i < ui->editTaskFolder->count(); i++){
                if (ui->editTaskFolder->itemData(i).toInt() == targetFolderId) {
                    targetFolderIndex = i;
                    qDebug() << "index: " << targetFolderIndex;
                    break;
                }
            }
            if (targetFolderIndex != -1) {
                ui->editTaskFolder->setCurrentIndex(targetFolderIndex);
            }
            ui->editTaskPriority->setCurrentIndex(query.value("task_priority").toInt());
            ui->editTaskDuration->setCurrentIndex(query.value("task_duration").toInt());
            if (!query.value("task_deadline").isNull()){
                ui->editTaskIsDeadline->setChecked(true);
                ui->editTaskDeadline->setDate(query.value("task_deadline").toDate());
            }
            if (!query.value("task_reminder").isNull()){
                ui->editTaskIsReminder->setChecked(true);
                ui->editTaskReminder->setDateTime(query.value("task_reminder").toDateTime());
            }
        }
    } else {
        qDebug() << "Query error:" << query.lastError().text();
    }
}

void TaskEditWindow::setCurrentEditingTaskId(QTreeWidgetItem *item)
{
    QVariantList taskData = item->data(0, Qt::UserRole).toList();
    currentEditingTaskId = taskData[1].toInt();
}

void TaskEditWindow::on_closeTaskButton_clicked()
{
    close();
}

void TaskEditWindow::on_editTaskIsDeadline_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1){
        ui->editTaskDeadline->setEnabled(true);
    }
    else{
        ui->editTaskDeadline->setEnabled(false);
    }
}

void TaskEditWindow::on_editTaskIsReminder_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1){
        ui->editTaskReminder->setEnabled(true);
    }
    else{
        ui->editTaskReminder->setEnabled(false);
    }
}

void TaskEditWindow::on_saveTaskButton_clicked()
{
    QSqlQuery updateTaskQuery;
    updateTaskQuery.prepare("UPDATE tasks SET task_name = :name,"
                "task_note = :note, task_folder_id = :folder_id,"
                "task_priority = :priority, "
                "task_duration = :duration, "
                "task_deadline = :deadline, "
                "task_reminder = :reminder "
                "WHERE task_id = :id;");

    updateTaskQuery.bindValue(":name", ui->editTaskName->text());
    updateTaskQuery.bindValue(":note", ui->editTaskNote->toPlainText());
    updateTaskQuery.bindValue(":folder_id", ui->editTaskFolder->itemData(ui->editTaskFolder->currentIndex()).toInt());
    updateTaskQuery.bindValue(":priority", ui->editTaskPriority->currentIndex());
    updateTaskQuery.bindValue(":duration", ui->editTaskDuration->currentIndex());

    if (ui->editTaskIsDeadline->checkState()){
        updateTaskQuery.bindValue(":deadline", ui->editTaskDeadline->date());
    }
    else{
        updateTaskQuery.bindValue(":deadline", QVariant());
    }

    if (ui->editTaskIsReminder->checkState()){
        updateTaskQuery.bindValue(":reminder", ui->editTaskReminder->dateTime());
    }
    else{
        updateTaskQuery.bindValue(":reminder", QVariant());
    }
    updateTaskQuery.bindValue(":id", currentEditingTaskId);


    if (!updateTaskQuery.exec()) {
        qDebug() << "Ошибка при изменении задачи: " << updateTaskQuery.lastError().text();
    } else {
    //    qDebug() << "Задача успешно изменена!";
    }

    emit updateTaskBlock();
    emit notificationChanged(currentEditingTaskId);

    close();
}


void TaskEditWindow::on_deleteTaskButton_clicked()
{
    QSqlQuery deleteTaskQuery;
    deleteTaskQuery.prepare("DELETE FROM tasks WHERE task_id = :id;");

    deleteTaskQuery.bindValue(":id", currentEditingTaskId);
    if (!deleteTaskQuery.exec()) {
        qDebug() << "Ошибка при удалении задачи: " << deleteTaskQuery.lastError().text();
    } else {
    //    qDebug() << "Задача успешно удалена!";
    }
    emit updateTaskBlock();
    this->close();
}

