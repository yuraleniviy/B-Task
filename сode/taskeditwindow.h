#ifndef TASKEDITWINDOW_H
#define TASKEDITWINDOW_H

#include <QDialog>
#include <QTreeWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class TaskEditWindow; }
QT_END_NAMESPACE

class TaskEditWindow : public QDialog {
    Q_OBJECT

public:
    explicit TaskEditWindow(QWidget *parent = nullptr);
    ~TaskEditWindow();
    void showAt(const QPoint &pos, const QString &text, QTreeWidgetItem *item);
    void loadFoldersToChoose();
    void setDefaultDesign();
    void addTaskInfoToDesign(QTreeWidgetItem *item);
    void setCurrentEditingTaskId(QTreeWidgetItem *item);

signals:
    void updateTaskBlock();
    void notificationChanged(int taskId);

private slots:
    void on_closeTaskButton_clicked();

    void on_editTaskIsDeadline_checkStateChanged(const Qt::CheckState &arg1);

    void on_editTaskIsReminder_checkStateChanged(const Qt::CheckState &arg1);

    void on_saveTaskButton_clicked();

    void on_deleteTaskButton_clicked();

private:
    Ui::TaskEditWindow *ui;
    QTreeWidgetItem *p_currentItem;
    int currentEditingTaskId;
};

#endif // TASKEDITWINDOW_H
