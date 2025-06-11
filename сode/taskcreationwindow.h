#ifndef TASKCREATIONWINDOW_H
#define TASKCREATIONWINDOW_H

#include <QDialog>
#include "task.h"

namespace Ui {
class TaskCreationWindow;
}

class TaskCreationWindow : public QDialog
{
    Q_OBJECT

public:
    explicit TaskCreationWindow(QWidget *parent = nullptr);
    ~TaskCreationWindow();

    void setDefaultDesign();
    void loadFoldersToChoose();

private slots:
    void on_closeTaskButton_clicked();

    void on_saveTaskButton_clicked();

    void on_creationTaskIsDeadline_checkStateChanged(const Qt::CheckState &arg1);

    void on_creationTaskIsReminder_checkStateChanged(const Qt::CheckState &arg1);

signals:
    void newTaskCreated();

private:
    Ui::TaskCreationWindow *ui;
};

#endif // TASKCREATIONWINDOW_H
