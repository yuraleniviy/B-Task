#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTreeWidgetItem>
#include "task.h"
#include "folder.h"
#include <QDropEvent>
#include "dragtreewidget.h"
#include <qprogressbar.h>
#include <QDate>
#include <qsystemtrayicon.h>
#include "customcalendarwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QTreeWidgetItem *p_inboxItem;
    QTreeWidgetItem *p_selectedItem;
    QVector <Folder> folders;
    Folder *p_inboxFolder;
    QTreeWidgetItem* findTargetFolder(int targetId);
    DragTreeWidget *taskWidget;
    CustomCalendarWidget *calendarWidget;
    QLabel *statusBarSortText;
    QLabel *statusBarFilterText;
    QSystemTrayIcon *trayIcon;

    void initializeDatabase();
    void initializeLoadFoldersFromDatabase();
    void initializeLoadTasksFromDatabase();
    void initCreateTreeWidget();
    void initCalendarBlockDesign();
    void initTray();
    void checkReminders();

    void updateTaskBlock();
    void updateCalendarBlock();
    void updateTaskBlockFolders();
    void updateTaskBlockTasks();

    void makeConnections();
    void changeTaskStatus(QTreeWidgetItem* item);
    void makeCompletedTaskGrey(QTreeWidgetItem *item);
    void makeUndoneTaskBlack(QTreeWidgetItem *item);


    const QString inbox = "inbox";

    int static filterStatus;
    int static sortStatus;
    QDate filterStartDate;
    QDate filterEndDate;
    QDate selectedViaCalendarDate;
    int filterCheckedStatus;
    int filterPriority;
    bool updateUIInProgress = false;
    QVector<int> shownReminders;


private slots:
    void on_newFolderButton_clicked();
    void updateUI();
    void onCalendarDateClicked(const QDate &date);
    void on_notificationChanged(int taskId);
    void on_menuUpdateUi_triggered();
    void on_menuDeleteCompletedTasks_triggered();
    void on_menu_calendar_triggered(bool checked);
    void on_sortByDeadline_triggered();
    void on_sortByStatus_triggered();
    void on_sortByPriority_triggered();
    void on_filterByDeadline_triggered();
    void on_filterByDefault_triggered();
    void on_filterByStatus_triggered();
    void on_filterByPriority_triggered();
    void on_actionExportToJSON_triggered();
    void on_actionExportToCSV_triggered();

    void on_displayTasksOfSelectedDayButton_clicked();

    void on_backToTodayButton_clicked();

protected:
    void closeEvent(QCloseEvent *event) override;
private:
    Ui::MainWindow *ui;


};
#endif // MAINWINDOW_H
