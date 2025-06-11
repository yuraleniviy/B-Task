#include "task.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

int Task::taskCount = 1000;

Task::Task(QObject *parent)
    : QObject{parent}
{}

void Task::saveTaskToDB()
{
    QSqlQuery query;
    query.prepare("INSERT INTO tasks (task_id, task_name, task_status,"
                  "task_note, task_folder_id, task_priority, "
                  "task_duration, task_deadline, task_reminder) "
                  "VALUES (:id, :name, :status, :note, :folder_id,"
                  " :priority, :duration, :deadline, :reminder)");

    query.bindValue(":id", this->getTaskId());
    query.bindValue(":name", this->getTaskName());
    query.bindValue(":status", this->getTaskStatus());
    query.bindValue(":note", this->getTaskNote());
    query.bindValue(":folder_id", this->getTaskFolderId());
    query.bindValue(":priority", this->getTaskPriority() );
    query.bindValue(":duration", this->getTaskDuration());
    query.bindValue(":deadline", this->getTaskDeadline());
    query.bindValue(":reminder", this->getTaskReminder());

    if (!query.exec()) {
        qDebug() << "Ошибка при добавлении задачи: " << query.lastError().text();
    } else {
    //    qDebug() << "Задача успешно добавлена!";
    }
}
