#ifndef TASK_H
#define TASK_H

#include <QObject>
#include <qdatetime.h>

class Task : public QObject
{
    Q_OBJECT

private:
    int task_id;
    QString task_name;
    int task_status;
    QString task_note;
    int task_folder_id;
    int task_priority;
    int task_duration;
    QDate task_deadline;
    QDateTime task_reminder;
    bool task_is_deadline;
    bool task_is_reminder;

public:
    explicit Task(QObject *parent = nullptr);
    int static taskCount;

    // setters
    void setTaskId() {
        this->task_id = taskCount;
        taskCount++;
        // qDebug() << task_id;
    }
    void setTaskId(int task_id) {
        this->task_id = task_id;
        if (taskCount < task_id + 1){
            taskCount = task_id + 1;
        }
        // qDebug() << task_id;
    }

    void setTaskName(const QString& task_name) {
        if (task_name.trimmed().isEmpty()) {
            this->task_name = "новая задача";
        }
        else{
            this->task_name = task_name.trimmed();
        }
    }

    void setTaskStatus(int task_status) {this->task_status = task_status;}
    void setTaskNote(const QString& task_note) {this->task_note = task_note;}
    void setTaskFolderId(int task_folder_id) {this->task_folder_id = task_folder_id;}
    void setTaskPriority(int task_priority) {this->task_priority = task_priority;}
    void setTaskDuration(int task_duration) {this->task_duration = task_duration;}
    void setTaskIsDeadline(bool task_is_deadline){this->task_is_deadline = task_is_deadline;}
    void setTaskIsReminder(bool task_is_reminder){this->task_is_reminder = task_is_reminder;}

    void setTaskDeadline(const QDate& task_deadline) {
        if (this->task_is_deadline){
            this->task_deadline = task_deadline;
        }
    }
    void setTaskReminder(const QDateTime& task_reminder) {
        if (this->task_is_reminder){
            this->task_reminder = task_reminder;
        }
    }

    // getters
    int getTaskId() const {return task_id;}
    QString getTaskName() const {return task_name;}
    int getTaskStatus() const {return task_status;}
    QString getTaskNote() const {return task_note;}
    int getTaskFolderId() const {return task_folder_id;}
    int getTaskPriority() const {return task_priority;}
    int getTaskDuration() const {return task_duration;}
    QDate getTaskDeadline() const {return task_deadline;}
    QDateTime getTaskReminder() const {return task_reminder;}
    bool getTaskIsDeadline() {return task_is_deadline;}
    bool getTaskIsReminder() {return task_is_reminder;}

    //already id
    Task(int task_id, QString task_name, int task_status, QString task_note, int task_folder_id,
         int task_priority, int task_duration, QDate task_deadline, QDateTime task_reminder, bool task_is_deadline, bool task_is_reminder){

        this->setTaskId(task_id);
        this->setTaskName(task_name);
        this->setTaskStatus(task_status);
        this->setTaskNote(task_note);
        this->setTaskFolderId(task_folder_id);
        this->setTaskPriority(task_priority);
        this->setTaskDuration(task_duration);

        this->setTaskIsDeadline(task_is_deadline);
        this->setTaskIsReminder(task_is_reminder);
        this->setTaskDeadline(task_deadline);
        this->setTaskReminder(task_reminder);
    }

    //no id
    Task(QString task_name, QString task_note, int task_folder_id,
         int task_priority, int task_duration, QDate task_deadline, QDateTime task_reminder, bool task_is_deadline, bool task_is_reminder){

        this->setTaskId();
        this->setTaskName(task_name);
        this->setTaskStatus(0);
        this->setTaskNote(task_note);
        this->setTaskFolderId(task_folder_id);
        this->setTaskPriority(task_priority);
        this->setTaskDuration(task_duration);

        this->setTaskIsDeadline(task_is_deadline);
        this->setTaskIsReminder(task_is_reminder);
        this->setTaskDeadline(task_deadline);
        this->setTaskReminder(task_reminder);
    }

    void saveTaskToDB();

signals:
};

#endif // TASK_H
