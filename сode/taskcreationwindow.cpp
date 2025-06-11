#include "taskcreationwindow.h"
#include "ui_taskcreationwindow.h"
#include "task.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <qcalendarwidget.h>

TaskCreationWindow::TaskCreationWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::TaskCreationWindow)
{
    ui->setupUi(this);
    ui->creationTaskName->setFocus();
    setModal(true);
    setDefaultDesign();
}

TaskCreationWindow::~TaskCreationWindow()
{
    delete ui;
}

void TaskCreationWindow::on_closeTaskButton_clicked()
{
    close();
}

void TaskCreationWindow::on_saveTaskButton_clicked()
{
    int folderId = ui->creationTaskFolder->currentData().toInt();
    Task *p_newTask = new Task (ui->creationTaskName->text(),
                               ui->creationTaskNote->toPlainText(),
                               folderId,
                               ui->creationTaskPriority->currentIndex(),
                               ui->creationTaskDuration->currentIndex(),
                               ui->creationTaskDeadline->date(),
                               ui->creationTaskReminder->dateTime(),
                               ui->creationTaskIsDeadline->checkState(),
                               ui->creationTaskIsReminder->checkState());
    p_newTask->saveTaskToDB();
    //qDebug() << "id выбрано: " << folderId;
    emit newTaskCreated();
    close();
}

void TaskCreationWindow::on_creationTaskIsDeadline_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1){
        ui->creationTaskDeadline->setEnabled(true);
    }
    else{
        ui->creationTaskDeadline->setEnabled(false);
    }
}

void TaskCreationWindow::on_creationTaskIsReminder_checkStateChanged(const Qt::CheckState &arg1)
{
    if (arg1){
        ui->creationTaskReminder->setEnabled(true);
    }
    else{
        ui->creationTaskReminder->setEnabled(false);
    }
}

void TaskCreationWindow::setDefaultDesign()
{
    loadFoldersToChoose();
    ui->creationTaskFolder->setCurrentIndex(0);

    ui->creationTaskName->clear();
    ui->creationTaskNote->clear();
    ui->creationTaskPriority->clear();
    ui->creationTaskDuration->clear();

    ui->creationTaskPriority->addItem("низкий");
    ui->creationTaskPriority->addItem(QIcon(":/pictures/resources/green-exc.svg"),"средний");
    ui->creationTaskPriority->addItem(QIcon(":/pictures/resources/red-exc.svg"),"высокий");
    ui->creationTaskPriority->setCurrentIndex(1);

    ui->creationTaskDuration->addItem("30 минут");
    ui->creationTaskDuration->addItem("1-2 часа");
    ui->creationTaskDuration->addItem(">2 часов");
    ui->creationTaskDuration->setCurrentIndex(0);

    ui->creationTaskIsReminder->setChecked(false);
    ui->creationTaskIsDeadline->setChecked(false);

    ui->creationTaskDeadline->setDate(QDate::currentDate().addDays(7));
    QDateTime inThreeDays = QDateTime::currentDateTime().addDays(3);
    QTime seventeen(17, 0);
    inThreeDays.setTime(seventeen);
    ui->creationTaskReminder->setDateTime(inThreeDays);
    ui->creationTaskReminder->setMinimumDateTime(QDateTime::currentDateTime());

    QCalendarWidget *calendarDeadline = ui->creationTaskDeadline->calendarWidget();
    QCalendarWidget *calendarNotification = ui->creationTaskReminder->calendarWidget();

    QTextCharFormat todayFormat;
    todayFormat.setForeground(QBrush(Qt::red));
    calendarDeadline->setDateTextFormat(QDate::currentDate(), todayFormat);
    calendarNotification->setDateTextFormat(QDate::currentDate(), todayFormat);
}

void TaskCreationWindow::loadFoldersToChoose()
{
    ui->creationTaskFolder->clear();
    QSqlQuery folderQuery("SELECT folder_id, folder_name, folder_color FROM folders");
    while (folderQuery.next()) {
        QVariant folderId;
        folderId = folderQuery.value(0).toInt();
        QString folderColor = folderQuery.value(2).toString();
        if(folderQuery.value(1).toString() == "inbox"){ui->creationTaskFolder->addItem(QIcon(":/pictures/resources/inbox.svg"),folderQuery.value(1).toString(), folderId);}
        else{
            if(folderColor == "white"){ui->creationTaskFolder->addItem(QIcon(":/pictures/resources/color-white.svg"),folderQuery.value(1).toString(), folderId);}
            if(folderColor == "red"){ui->creationTaskFolder->addItem(QIcon(":/pictures/resources/color-red.svg"),folderQuery.value(1).toString(), folderId);}
            if(folderColor == "green"){ui->creationTaskFolder->addItem(QIcon(":/pictures/resources/color-green.svg"),folderQuery.value(1).toString(), folderId);}
            if(folderColor == "blue"){ui->creationTaskFolder->addItem(QIcon(":/pictures/resources/color-blue.svg"),folderQuery.value(1).toString(), folderId);}
        }
    }
}

