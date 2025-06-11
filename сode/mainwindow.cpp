#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <qtoolbox.h>
#include <QTreeWidgetItem>
#include <QTextCharFormat>
#include "taskeditwindow.h"
#include "taskcreationwindow.h"
#include "folder.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include "foldereditwindow.h"
#include <QDropEvent>
#include <QMimeData>
#include <QDrag>
#include "dragtreewidget.h"
#include <QHeaderView>
#include <QDialog>
#include <QScreen>
#include <QDateEdit>
#include <QCheckBox>
#include <QComboBox>
#include <qsystemtrayicon.h>
#include <qtimer.h>
#include <QFileDialog>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QTextStream>

int MainWindow::filterStatus = 0;
int MainWindow::sortStatus = 11;


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    initCreateTreeWidget();

    initializeDatabase();
    updateUI();
    initCalendarBlockDesign();
    initTray();
    selectedViaCalendarDate = QDate::currentDate();


    ui->calendarBlock->hide();
    resize(600, height());

    makeConnections();

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(screenGeometry.topLeft() + QPoint(x, y));
    }

    statusBarSortText = new QLabel("Сортировка: сначала срочные");
    statusBarFilterText = new QLabel("");
    statusBar()->addWidget(statusBarSortText);
    statusBar()->addWidget(statusBarFilterText);
    statusBar()->setStyleSheet(R"(
        QStatusBar{
            outline: none;
            border: none;
            background: transparent;
        }
        QStatusBar::item {
            border: none;
            margin-left: 20px;
            margin-bottom: 10px;
        }
        QLabel{
            font-family: Bahnschrift;
            font-size: 14px;
            font-weight: normal;
            padding-left: 20px;
            padding-bottom: 10px;
        }
        )");

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::makeConnections()
{
    TaskEditWindow *p_taskeditwindow = new TaskEditWindow(this);
    TaskCreationWindow *p_taskcreationwindow = new TaskCreationWindow(this);
    FolderEditWindow *p_foldereditwindow = new FolderEditWindow(this);

    connect(ui->newTaskButton, &QPushButton::clicked, this, [=]{
        p_taskcreationwindow->setDefaultDesign();
        p_taskcreationwindow->show();
    });

    connect(taskWidget, &QTreeWidget::itemDoubleClicked, this, [=](QTreeWidgetItem *p_item, int){ //opening task or folder edit window
        QVariantList data = p_item->data(0, Qt::UserRole).toList();
        bool isFolder = data[0].toBool();
        int folderId = data[1].toInt();
        if (folderId != 0){ //inbox менять нельзя
            if (isFolder){
                p_foldereditwindow->showAt(QCursor::pos(), p_item->text(0), p_item);
            }
            else{
                p_taskeditwindow->showAt(QCursor::pos(), p_item->text(0), p_item);
            }
        }
    });

    connect(p_taskcreationwindow, &TaskCreationWindow::newTaskCreated, this, &MainWindow::updateUI);

    connect(p_taskeditwindow, &TaskEditWindow::updateTaskBlock, this, &MainWindow::updateUI);

    connect(taskWidget, &QTreeWidget::itemChanged, this, &MainWindow::changeTaskStatus);

    connect(taskWidget, &DragTreeWidget::taskDraggedToFolder, this, &MainWindow::updateUI);

    connect(p_foldereditwindow, &FolderEditWindow::updateTaskBlock, this, &MainWindow::updateUI);

    connect(p_taskeditwindow, &TaskEditWindow::notificationChanged,this, &MainWindow::on_notificationChanged);

    connect(calendarWidget, &QCalendarWidget::clicked, this, &MainWindow::onCalendarDateClicked);

    //connect(calendarWidget, &QCalendarWidget::selectionChanged, this, &MainWindow::onCalendarDateClickedOrSelected);
 }

QTreeWidgetItem *MainWindow::findTargetFolder(int targetId)
{
    QTreeWidgetItem *p_targetFolderItem;
    for (int i = 0; i < taskWidget->topLevelItemCount(); i++) {
        QTreeWidgetItem *p_item = taskWidget->topLevelItem(i);

        QVariantList folderData = p_item->data(0, Qt::UserRole).toList();
        int folderId = folderData[1].toInt();
        if (folderId == targetId) {
            p_targetFolderItem = p_item;
            //qDebug() << "findTargetFolder нашел папку: " << folderId;
            return p_targetFolderItem;
        }
    }
    return nullptr;
}

void MainWindow::initializeDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("tasks-and-folders.db");
    if (!db.open()) {
        qDebug() << "База данных не подключилась: " << db.lastError().text();
    } else {
    //    qDebug() << "Успешное подключение к базе данных!";
    }

    QSqlQuery queryInit;
    //create tasks table
    QString createTableQuery = "CREATE TABLE IF NOT EXISTS tasks ("
                               "task_id INTEGER,"
                               "task_name TEXT,"
                               "task_status INTEGER,"
                               "task_note TEXT,"
                               "task_folder_id INTEGER,"
                               "task_priority INTEGER,"
                               "task_duration INTEGER,"
                               "task_deadline DATETIME,"
                               "task_reminder DATETIME)";

    if (!queryInit.exec(createTableQuery)) {
        qDebug() << "Ошибка при создании таблицы задач: " << queryInit.lastError().text();
    } else {
    //    qDebug() << "Таблица задач - успешно :)";
    }

    //create folders table
    QString createFolderQuery = "CREATE TABLE IF NOT EXISTS folders ("
                                "folder_id INTEGER,"
                                "folder_name TEXT,"
                                "folder_color TEXT,"
                                "folder_status INTEGER)";

    if (!queryInit.exec(createFolderQuery)) {
        qDebug() << "Ошибка при создании таблицы папок: " << queryInit.lastError().text();
    } else {
     //   qDebug() << "Таблица папок - успешно :)";
    }
}

void MainWindow::initCreateTreeWidget()
{
    taskWidget = new DragTreeWidget();
    ui->verticalLayout_2->addWidget(taskWidget);
    taskWidget->setColumnCount(2);
    taskWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    taskWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    taskWidget->header()->setStretchLastSection(false);
}


void MainWindow::initCalendarBlockDesign()
{
    calendarWidget = new CustomCalendarWidget;
    ui->calendarLayout->addWidget(calendarWidget);
}

void MainWindow::initTray()
{
    trayIcon = new QSystemTrayIcon(QIcon(":/pictures/resources/B-Task-logo.svg"), this);
    trayIcon->setToolTip("B-Task");

    QMenu *trayMenu = new QMenu(this);
    trayMenu->addAction("Открыть", this, &QWidget::showNormal);
    trayMenu->addAction("Выход", qApp, &QApplication::quit);
    trayIcon->setContextMenu(trayMenu);
    trayIcon->show();
    trayMenu->setStyleSheet(R"(
    QMenu {
        background-color: rgb(247, 247, 239);
        border: 1px solid gray;
        padding: 5px;
        font-family: Bahnschrift;
        font-size: 14px;
    }
    QMenu::item {
        padding: 5px 20px;
        background-color: transparent;
    }
    QMenu::item:selected {
        background-color: rgb(247, 223, 156);
    }
)");

    QTimer *reminderTimer = new QTimer(this);
    connect(reminderTimer, &QTimer::timeout, this, &MainWindow::checkReminders);
    reminderTimer->start(30000);

    connect(trayIcon, &QSystemTrayIcon::activated, this, [this](QSystemTrayIcon::ActivationReason reason) {
        if (reason == QSystemTrayIcon::Trigger) {
            this->showNormal();
            this->raise();
            this->activateWindow();
        }
    });
}

void MainWindow::checkReminders()
{
    qDebug() << "checking reminders";
    QSqlQuery remindersQuery;
    remindersQuery.prepare(R"(SELECT task_id, task_name FROM tasks
        WHERE task_reminder <= :now AND task_reminder >= :whileago AND task_status = 0)");
    remindersQuery.bindValue(":now", QDateTime::currentDateTime());
    remindersQuery.bindValue(":whileago", QDateTime::currentDateTime().addSecs(-60));
    if (remindersQuery.exec()) {
        while (remindersQuery.next()) {
            int taskId = remindersQuery.value(0).toInt();
            QString taskName = remindersQuery.value(1).toString();
            if (!shownReminders.contains(taskId)) {
                shownReminders.append(taskId);
                qDebug() << "notification: " << taskName << taskId;
                trayIcon->showMessage("Дела не ждут!!",
                                      taskName,
                                      QIcon(":/pictures/resources/B-Task-logo.svg"),
                                      5000);
            }
        }
    }
}

void MainWindow::updateTaskBlock()
{
    taskWidget->clear();

    updateTaskBlockFolders();

    updateTaskBlockTasks();

}

void MainWindow::updateCalendarBlock()
{
    qDebug() << "update calendar block";
    calendarWidget->setIconForDate(QDate(2025, 5, 15), ":/pictures/resources/bin.png");
    calendarWidget->update();
}

void MainWindow::updateTaskBlockFolders()
{
    p_inboxFolder = new Folder(0, inbox,"white",1);
    p_inboxItem = new QTreeWidgetItem(taskWidget,QStringList(p_inboxFolder->getFolderName()));
    QVariantList folderData;
    folderData << true << p_inboxFolder->getFolderId(); //true - folder, false - task
    p_inboxItem->setData(0,Qt::UserRole, folderData);
    p_inboxItem->setIcon(0, QIcon(":/pictures/resources/inbox.svg"));
    p_inboxItem->setSizeHint(0, QSize(100,40));
    p_inboxItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
    taskWidget->insertTopLevelItem(0,p_inboxItem);

    bool inboxAlreadyExists = false;
    QSqlQuery folderQuery("SELECT folder_id, folder_name, folder_color, folder_status FROM folders");
    while (folderQuery.next()) {
            int folderId = folderQuery.value(0).toInt();
            QString folderName = folderQuery.value(1).toString();
        if (folderName != inbox){
            QString folderColor = folderQuery.value(2).toString();
            int folderStatus = folderQuery.value(3).toInt();

            Folder *p_newFolder = new Folder(folderId, folderName, folderColor, folderStatus);

            QTreeWidgetItem *p_newFolderItem;

            p_newFolderItem = new QTreeWidgetItem(taskWidget, QStringList(folderName));

            QVariantList folderData;
            folderData << true << p_newFolder->getFolderId();
            p_newFolderItem->setData(0, Qt::UserRole, folderData);
            p_newFolderItem->setSizeHint(0, QSize(100,40));
            p_newFolderItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
            if (folderColor == "white"){
                p_newFolderItem->setIcon(0,QIcon(":/pictures/resources/color-white.svg"));
            }
            if (folderColor == "green"){
                p_newFolderItem->setIcon(0,QIcon(":/pictures/resources/color-green.svg"));
            }
            if (folderColor == "red"){
                p_newFolderItem->setIcon(0,QIcon(":/pictures/resources/color-red.svg"));
            }
            if (folderColor == "blue"){
                p_newFolderItem->setIcon(0,QIcon(":/pictures/resources/color-blue.svg"));
            }

            taskWidget->insertTopLevelItem(0, p_newFolderItem);

            delete p_newFolder;
        }
        else{
            inboxAlreadyExists = true;
        }
    }
    if (!inboxAlreadyExists){
        p_inboxFolder->saveFolderToDB();
    }

}

void MainWindow::updateTaskBlockTasks()
{
        /*sort and filter Status:

        sort
    11 - by deadline (default)
    12 - by status
    13 - by priority

        filter
    0 - no (default)
    21 - by deadline
    22 - by status
    23 - by priority
        */
    QSqlQuery taskQuery;
    QString taskQueryString = "SELECT task_id, task_name, task_status, task_note,"
                              " task_folder_id, task_priority, task_duration, task_deadline,"
                              " task_reminder FROM tasks";

    QString additionalTaskQueryString;
    switch (filterStatus){
    case 0:
        switch (sortStatus){
        case 11:
            additionalTaskQueryString = " ORDER BY task_deadline IS NULL, task_deadline ASC";
            break;
        case 12:
            additionalTaskQueryString = " ORDER BY task_status ASC";
            break;
        case 13:
            additionalTaskQueryString = " ORDER BY task_priority DESC";
            break;
        }
        break;
    case 21:
        if (filterStartDate.isValid() && filterEndDate.isValid()) {
            additionalTaskQueryString = QString(" WHERE task_deadline BETWEEN '%1' AND '%2' ")
                .arg(filterStartDate.toString("yyyy-MM-dd"))
                .arg(filterEndDate.toString("yyyy-MM-dd"));
            switch (sortStatus){
            case 11:
                additionalTaskQueryString += " ORDER BY task_deadline IS NULL, task_deadline ASC";
                break;
            case 12:
                additionalTaskQueryString += " ORDER BY task_status ASC";
                break;
            case 13:
                additionalTaskQueryString += " ORDER BY task_priority DESC";
                break;
            }
        }
        break;
    case 22:
        additionalTaskQueryString = QString(" WHERE task_status = %1")
                                        .arg(filterCheckedStatus);
        switch (sortStatus){
        case 11:
            additionalTaskQueryString += " ORDER BY task_deadline IS NULL, task_deadline ASC";
            break;
        case 12:
            additionalTaskQueryString += " ORDER BY task_status ASC";
            break;
        case 13:
            additionalTaskQueryString += " ORDER BY task_priority DESC";
            break;
        }
        break;
    case 23:
        additionalTaskQueryString = QString(" WHERE task_priority = %1")
                                        .arg(filterPriority);
        switch (sortStatus){
        case 11:
            additionalTaskQueryString += " ORDER BY task_deadline IS NULL, task_deadline ASC";
            break;
        case 12:
            additionalTaskQueryString += " ORDER BY task_status ASC";
            break;
        case 13:
            additionalTaskQueryString += " ORDER BY task_priority DESC";
            break;
        }
        break;
    }

    taskQuery = QSqlQuery(taskQueryString + additionalTaskQueryString);

    //QString warningMessage = "Не удалось найти папки с этими ID: ";
    while (taskQuery.next()) {
        // qDebug() << "----программа дошла cюда----";
        int task_id = taskQuery.value(0).toInt();
        QString taskName = taskQuery.value(1).toString();
        QString taskNote = taskQuery.value(3).toString();
        int taskStatus = taskQuery.value(2).toInt();
        int task_folder_id = taskQuery.value(4).toInt();
        int task_priority = taskQuery.value(5).toInt();
        int task_duration = taskQuery.value(6).toInt();
        QDate task_deadline = taskQuery.value(7).toDate();
        QDateTime task_reminder = taskQuery.value(8).toDateTime();

        Task *p_newTask = new Task (task_id,
                                   taskName,
                                   taskStatus,
                                   taskNote,
                                   task_folder_id,
                                   task_priority,
                                   task_duration,
                                   task_deadline,
                                   task_reminder,
                                   task_deadline.isValid(),
                                   task_reminder.isValid());
        QTreeWidgetItem *p_newTaskItem;
        QTreeWidgetItem *p_targetFolderItem;

        //qDebug() << "----программа дошла до findTargetFolder----";
        p_targetFolderItem = findTargetFolder(p_newTask->getTaskFolderId());

        //qDebug() << p_targetFolderItem->data(0, Qt::UserRole).toList();
        if (!p_targetFolderItem) {
            qDebug() << "Не найдена папка с ID:" << p_newTask->getTaskFolderId();
            //warningMessage = warningMessage + QString::number(p_newTask->getTaskFolderId()) + " " ;
            continue;
        }

        p_newTaskItem = new QTreeWidgetItem(p_targetFolderItem, QStringList(p_newTask->getTaskName()));
        QVariantList taskData;
        taskData << false << p_newTask->getTaskId();
        p_newTaskItem->setData(0, Qt::UserRole, taskData);
        p_newTaskItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable );

        //чекбокс
        if (p_newTask->getTaskStatus()){
            p_newTaskItem->setCheckState(0, Qt::Checked);
            makeCompletedTaskGrey(p_newTaskItem);
        }
        else{
            p_newTaskItem->setCheckState(0, Qt::Unchecked);
            makeUndoneTaskBlack(p_newTaskItem);
        }

        //приоритет
        switch(p_newTask->getTaskPriority()){
        case 1:
            p_newTaskItem->setIcon(0,QIcon(":/pictures/resources/green-exc.svg"));
            break;
        case 2:
            p_newTaskItem->setIcon(0,QIcon(":/pictures/resources/red-exc.svg"));
            break;
        }

        //дедлайн отображение
        if (!p_newTask->getTaskDeadline().isNull()){
            QLocale russian(QLocale::Russian);
            QString deadlineStr = russian.toString(p_newTask->getTaskDeadline(), "d MMM");
            p_newTaskItem->setText(1, deadlineStr);
            QFont dateFont;
            dateFont.setItalic(true);
            // if (p_newTask->getTaskStatus() == 1){
            //     //p_newTaskItem->setForeground(1, QBrush(Qt::gray));
            // }
            // else{
            //     if (p_newTask->getTaskDeadline() >= QDate::currentDate()){
            //         //p_newTaskItem->setForeground(1, QBrush(Qt::red));
            //     }
            //     else{
            //         //p_newTaskItem->setForeground(1, QBrush(Qt::red));
            //     }
            // }

            p_newTaskItem->setFont(1, dateFont);
            p_newTaskItem->setTextAlignment(1, Qt::AlignRight | Qt::AlignVCenter);
        }

        p_targetFolderItem->setExpanded(true);
    }
    //QMessageBox::warning(this, "Отсутствующие папки!", warningMessage);
}

void MainWindow::changeTaskStatus(QTreeWidgetItem *item)
{
    bool isChecked = item->checkState(0);
    QVariantList taskData = item->data(0, Qt::UserRole).toList();
    int taskId = taskData.value(1).toInt();

    QSqlQuery query;
    query.prepare("UPDATE tasks SET task_status = :status WHERE task_id = :id;");
    query.bindValue(":id", taskId);

    if (isChecked == true){
        query.bindValue(":status", 1);
        makeCompletedTaskGrey(item);
    }
    else{
        query.bindValue(":status", 0);
        makeUndoneTaskBlack(item);
    }

    query.exec();
}

void MainWindow::makeCompletedTaskGrey(QTreeWidgetItem *item)
{
    item->setForeground(0, QBrush(Qt::gray));
    item->setForeground(1, QBrush(Qt::gray));
    QFont font = item->font(0);
    font.setStrikeOut(true);
    item->setFont(0, font);
}

void MainWindow::makeUndoneTaskBlack(QTreeWidgetItem *item)
{
    QVariantList taskData = item->data(0, Qt::UserRole).toList();
    int taskId = taskData.value(1).toInt();
    QSqlQuery query;
    query.prepare("SELECT * FROM tasks WHERE task_id = :id;");
    query.bindValue(":id", taskId);

    if (query.exec() && query.next()) {
        QDate taskDeadline = query.value("task_deadline").toDate();
        if (taskDeadline <= QDate::currentDate()){
            item->setForeground(1, QBrush(Qt::red));
        }
        else{
            item->setForeground(1, QBrush(Qt::black));
        }
    }

    item->setForeground(0, QBrush(Qt::black));
    QFont font = item->font(0);
    font.setStrikeOut(false);
    item->setFont(0, font);
}

void MainWindow::on_newFolderButton_clicked()
{
    Folder *p_newFolder = new Folder(QString("новая папка"), "white", 1);

    p_newFolder->saveFolderToDB();

    QTreeWidgetItem *p_newFolderItem;
    p_newFolderItem = new QTreeWidgetItem(taskWidget, QStringList(p_newFolder->getFolderName()));

    QVariantList folderData;
    folderData << true << p_newFolder->getFolderId();
    p_newFolderItem->setData(0, Qt::UserRole, folderData);
    p_newFolderItem->setSizeHint(0, QSize(100,40));
    p_newFolderItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
    p_newFolderItem->setIcon(0,QIcon(":/pictures/resources/color-white.svg"));

    taskWidget->insertTopLevelItem(0, p_newFolderItem);

    //delete p_newFolder;
}

void MainWindow::updateUI()
{
    if (updateUIInProgress) {
        return;
    }

    updateUIInProgress = true;

    QDialog *loadingDialog = new QDialog(this, Qt::FramelessWindowHint);
    loadingDialog->setWindowTitle("Загрузка...");
    loadingDialog->setModal(true);
    loadingDialog->resize(300, 50);
    loadingDialog->setStyleSheet(
    "QDialog{"
    "  border: 2px solid gray;"
        "border-radius: 5px; "
    "}"
    );

    QProgressBar *progressBar = new QProgressBar(loadingDialog);
    progressBar->setRange(0, 0);
    progressBar->setTextVisible(true);
    progressBar->setStyleSheet(
        "QProgressBar {"
        "  border: 2px solid gray;"
        "  border-radius: 5px;"
        "  text-align: center;"
        "  background: rgb(247,247,239)"
        "}"
        "QProgressBar::chunk {"
        "  background-color: rgb(247,223,156);"
        "  width: 10px;"
        "}"
        );

    QVBoxLayout *layout = new QVBoxLayout(loadingDialog);
    layout->setAlignment(Qt::AlignCenter);
    layout->addWidget(progressBar);

    loadingDialog->show();

    QTimer::singleShot(500, this, [this, loadingDialog]() {
        updateTaskBlock();
        updateCalendarBlock();

        loadingDialog->close();
        loadingDialog->deleteLater();
        updateUIInProgress = false;
    });
}

void MainWindow::onCalendarDateClicked(const QDate &date)
{
    selectedViaCalendarDate = date;
    qDebug() << "selected date: " << selectedViaCalendarDate.toString();
}

void MainWindow::on_notificationChanged(int taskId)
{
    if (shownReminders.contains(taskId)){
        shownReminders.removeOne(taskId);
        qDebug() << "removed from shown: " << taskId;
    }
}



void MainWindow::on_menuUpdateUi_triggered()
{
    updateUI();
}


void MainWindow::on_menuDeleteCompletedTasks_triggered()
{
    QSqlQuery deleteCompletedTasksQuery;
    deleteCompletedTasksQuery.prepare("DELETE FROM tasks WHERE task_status = 1;");
    if (!deleteCompletedTasksQuery.exec()) {
        qDebug() << "Ошибка при удалении задач в удаляемой папке: " << deleteCompletedTasksQuery.lastError().text();
    } else {
     //   qDebug() << "Задачи успешно удалены!";
    }
    updateUI();
}




void MainWindow::on_menu_calendar_triggered(bool checked)
{
    if (checked){
        ui->calendarBlock->show();
        resize(1100, height());
    }
    else{
        ui->calendarBlock->hide();
        resize(600, height());
    }

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(screenGeometry.topLeft() + QPoint(x, y));
    }
}


void MainWindow::on_sortByDeadline_triggered()
{
    sortStatus = 11;
    statusBarSortText->setText("Сортировка: сначала срочные");
    updateUI();
}


void MainWindow::on_sortByStatus_triggered()
{
    sortStatus = 12;
    statusBarSortText->setText("Сортировка: сначала невыполненные");
    updateUI();
}


void MainWindow::on_sortByPriority_triggered()
{
    sortStatus = 13;
    statusBarSortText->setText("Сортировка: сначала важные");
    updateUI();
}

void MainWindow::on_filterByDeadline_triggered()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Фильтрация: выбор дат");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    QLabel *startLabel = new QLabel("Дата начала:");
    QDateEdit *startDateEdit = new QDateEdit(QDate::currentDate());
    startDateEdit->setCalendarPopup(true);

    QLabel *endLabel = new QLabel("Дата окончания:");
    QDateEdit *endDateEdit = new QDateEdit(QDate::currentDate());
    endDateEdit->setCalendarPopup(true);

    endDateEdit->setMinimumDate(QDate::currentDate());
    startDateEdit->setMaximumDate(QDate::currentDate());

    QPushButton *applyButton = new QPushButton("Применить");
    QPushButton *closeButton = new QPushButton("Закрыть");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(applyButton);

    QHBoxLayout *firstDateLayout = new QHBoxLayout;
    QHBoxLayout *lastDateLayout = new QHBoxLayout;
    firstDateLayout->addWidget(startLabel);
    firstDateLayout->addWidget(startDateEdit);
    lastDateLayout->addWidget(endLabel);
    lastDateLayout->addWidget(endDateEdit);

    mainLayout->addLayout(firstDateLayout);
    mainLayout->addLayout(lastDateLayout);;
    mainLayout->addLayout(buttonLayout);


    // Сигналы
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(applyButton, &QPushButton::clicked, [=]() {
        filterStartDate = startDateEdit->date();
        filterEndDate = endDateEdit->date();
        QLocale russian(QLocale::Russian);

        QString startStr = russian.toString(filterStartDate, "d MMM");
        QString endStr   = russian.toString(filterEndDate, "d MMM");

        statusBarFilterText->setText(QString("Фильтрация: с %1 по %2")
                                         .arg(startStr)
                                         .arg(endStr));

        filterStatus = 21;
        updateUI();
        dialog->accept();
    });
    connect(startDateEdit, &QDateEdit::dateChanged, [=](const QDate &start) {
        if (endDateEdit->date() < start) {
            endDateEdit->setDate(start);
        }
        endDateEdit->setMinimumDate(start);
    });

    connect(endDateEdit, &QDateEdit::dateChanged, [=](const QDate &end) {
        if (startDateEdit->date() > end) {
            startDateEdit->setDate(end);
        }
        startDateEdit->setMaximumDate(end);
    });

    dialog->resize(300,50);
    dialog->setStyleSheet(R"(
        QDialog{
            border: 2px solid gray;
            border-radius: 5px;
            text-align: center;
            background: rgb(247,247,239);
        }
        QLabel, QDateEdit {
            font-family: Bahnschrift;
            font-size: 16px;
        }
        QPushButton {
            background-color: rgb(230,230,217);
            border-radius: 5px;
            border: 2px solid gray;
            color: black;
            font-family: Bahnschrift;
            font-size: 16px;
            padding: 5px;
        }
        QPushButton:hover {
            background-color: rgb(247,223,156);
        }
        QCalendarWidget {
            font-family: Bahnschrift;
            font-size: 16px;
            background-color: white;
        }

        QTableView::item {
            border-radius: 30px;
        }

        QTableView::item:selected {
            image: url(:/pictures/resources/calendar-selected-date.svg);
            background-color: rgba(247,223,156,0);
        }

        QTableView:focus {
            outline: none;
        }

        QTableView:disabled {
            color: rgba(0, 0, 0, 100);
        }

        QCalendarWidget QAbstractItemView::item:enabled[today="true"] {
            color: red;
            font-weight: bold;
        }

        QWidget#qt_calendar_navigationbar {
            background-color: rgb(247,223,156);
            border-radius: 5px;
            border: 2px solid gray;
        }

        QToolButton {
            color: black;
            font-size: 18px;
            border-radius: 20px;
        }

        #qt_calendar_nextmonth {
            icon-size: 30px;
            qproperty-icon: url(:/pictures/resources/right-arrow.svg);
            border-radius: 5px;
        }

        #qt_calendar_prevmonth {
            icon-size: 30px;
            qproperty-icon: url(:/pictures/resources/left-arrow.svg);
            border-radius: 5px;
        }

        QMenu {
            background-color: white;
            border: 2px solid gray;
            border-radius: 7px;
            padding: 4px 4px;
            font-family: Bahnschrift;
            font-size: 16px;
        }

        QMenu::item:selected {
            background-color: rgb(247,223,156);
            border-radius: 5px;
        }

        QSpinBox {
            background-color: rgb(247,223,156);
            width: 80px;
        }

        )");

    dialog->exec();
}


void MainWindow::on_filterByDefault_triggered()
{
    filterStatus = 0;
    statusBarFilterText->setText("");
    updateUI();
}


void MainWindow::on_filterByStatus_triggered()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Фильтрация: выбор статуса");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // Создаем метки и чекбоксы
    QLabel *label = new QLabel("Отобразить задачи со статусом ");
    label->setWordWrap(true);
    QCheckBox *checkBox = new QCheckBox;

    // Создаем кнопки
    QPushButton *applyButton = new QPushButton("Применить");
    QPushButton *closeButton = new QPushButton("Закрыть");

    // Ставим кнопки в горизонтальный layout
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(applyButton);

    // Создаем layout для меток и чекбоксов
    QHBoxLayout *checkboxLayout = new QHBoxLayout;
    checkboxLayout->addWidget(label);
    checkboxLayout->addWidget(checkBox);

    // Добавляем все layout'ы в основной layout
    mainLayout->addLayout(checkboxLayout);
    mainLayout->addLayout(buttonLayout);

    // Сигналы
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(applyButton, &QPushButton::clicked, [=]() {
        // Получаем состояние чекбоксов
        if (checkBox->isChecked()){
            filterCheckedStatus = 1;
            statusBarFilterText->setText("Фильтрация: только выполненные");
        }
        else{
            filterCheckedStatus = 0;
            statusBarFilterText->setText("Фильтрация: только невыполненные");
        }
        filterStatus = 22;
        updateUI();
        dialog->accept();
    });

    dialog->setFixedSize(250, 120);  // Устанавливаем размер диалога
    dialog->setStyleSheet(R"(
    QDialog {
        border: 2px solid gray;
        border-radius: 5px;
        text-align: center;
        background: rgb(247,247,239);
    }
    QLabel, QCheckBox {
        font-family: Bahnschrift;
        font-size: 16px;
    }
    QPushButton {
        background-color: rgb(230,230,217);
        border-radius: 5px;
        border: 2px solid gray;
        color: black;
        font-family: Bahnschrift;
        font-size: 16px;
        padding: 5px;
    }
    QPushButton:hover {
        background-color: rgb(247,223,156);
    }
    QCheckBox::indicator:checked {
        image: url(:/pictures/resources/checkbox-done.svg);
    }
    QCheckBox::indicator:unchecked {
        image: url(:/pictures/resources/checkbox-undone.svg);
    }
)");

    dialog->exec();
}


void MainWindow::on_filterByPriority_triggered()
{
    QDialog *dialog = new QDialog(this);
    dialog->setWindowTitle("Фильтрация: по приоритету");

    QVBoxLayout *mainLayout = new QVBoxLayout(dialog);

    // Метка и спинбокс
    QLabel *label = new QLabel("Показать задачи с приоритетом:");
    label->setWordWrap(true);
    QComboBox *filterComboBox = new QComboBox;
    filterComboBox->addItem("низкий");
    filterComboBox->addItem(QIcon(":/pictures/resources/green-exc.svg"),"средний");
    filterComboBox->addItem(QIcon(":/pictures/resources/red-exc.svg"),"высокий");
    filterComboBox->setCurrentIndex(1);

    // Кнопки
    QPushButton *applyButton = new QPushButton("Применить");
    QPushButton *closeButton = new QPushButton("Закрыть");

    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    buttonLayout->addWidget(applyButton);

    // Layout для метки и спинбокса
    QHBoxLayout *spinBoxLayout = new QHBoxLayout;
    spinBoxLayout->addWidget(label);
    spinBoxLayout->addWidget(filterComboBox);

    // Собираем в основной layout
    mainLayout->addLayout(spinBoxLayout);
    mainLayout->addLayout(buttonLayout);

    // Сигналы
    connect(closeButton, &QPushButton::clicked, dialog, &QDialog::reject);
    connect(applyButton, &QPushButton::clicked, [=]() {
        filterPriority = filterComboBox->currentIndex();

        filterStatus = 23;
        statusBarFilterText->setText("Фильтрация: приоритет");
        updateUI();
        dialog->accept();
    });

    dialog->setFixedSize(300, 120);
    dialog->setStyleSheet(R"(
    QDialog {
        border: 2px solid gray;
        border-radius: 5px;
        text-align: center;
        background: rgb(247,247,239);
    }
    QLabel, QComboBox {
        font-family: Bahnschrift;
        font-size: 16px;
    }
    QPushButton {
        background-color: rgb(230,230,217);
        border-radius: 5px;
        border: 2px solid gray;
        color: black;
        font-family: Bahnschrift;
        font-size: 16px;
        padding: 5px;
    }
    QPushButton:hover {
        background-color: rgb(247,223,156);
    }
    QComboBox {
        width: 80px;
    }
)");

    dialog->exec();

}

void MainWindow::closeEvent(QCloseEvent *event) {
    hide();
    event->ignore();
}


void MainWindow::on_actionExportToJSON_triggered()
{
    QSqlQuery queryFolders("SELECT * FROM folders");
    QJsonArray foldersArray;
    while (queryFolders.next()) {
        QJsonObject folderJsonObject;
        folderJsonObject["id"] = queryFolders.value("folder_id").toInt();
        folderJsonObject["name"] = queryFolders.value("folder_name").toString();
        folderJsonObject["color"] = queryFolders.value("folder_color").toString();
        foldersArray.append(folderJsonObject);
    }

    QSqlQuery queryTasks("SELECT * FROM tasks");
    QJsonArray tasksArray;
    while (queryTasks.next()) {
        QJsonObject taskJsonObject;
        taskJsonObject["id"] = queryTasks.value("task_id").toInt();
        taskJsonObject["name"] = queryTasks.value("task_name").toString();
        taskJsonObject["status"] = queryTasks.value("task_status").toInt();
        taskJsonObject["note"] = queryTasks.value("task_note").toString();
        taskJsonObject["folder_id"] = queryTasks.value("task_folder_id").toInt();
        taskJsonObject["priority"] = queryTasks.value("task_priority").toInt();
        taskJsonObject["duration"] = queryTasks.value("task_duration").toInt();
        taskJsonObject["deadline"] = queryTasks.value("task_deadline").toString();
        taskJsonObject["reminder"] = queryTasks.value("task_reminder").toString();
        tasksArray.append(taskJsonObject);
    }

    QJsonObject root;
    root["folders"] = foldersArray;
    root["tasks"]   = tasksArray;

    QString filename = QFileDialog::getSaveFileName(this, "Сохранить всё в JSON", "", "JSON-файл (*.json)");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }
    file.write(QJsonDocument(root).toJson(QJsonDocument::Indented));
    file.close();
}


void MainWindow::on_actionExportToCSV_triggered()
{
    QString csv;
    csv += "Folders\n";
    csv += "folder_id,folder_name,folder_color,folder_status\n";
    QSqlQuery queryFolders("SELECT * FROM folders");
    while (queryFolders.next()) {
        QString name = queryFolders.value("folder_name").toString().replace("\"","\"\"");
        QString color= queryFolders.value("folder_color").toString().replace("\"","\"\"");
        csv += QString("%1,\"%2\",\"%3\n")
                   .arg(queryFolders.value("folder_id").toInt())
                   .arg(name)
                   .arg(color);
    }

    csv += "\nTasks\n";
    csv += "task_id,task_name,task_status,task_note,task_folder_id,task_priority,task_duration,task_deadline,task_reminder\n";
    QSqlQuery queryTasks("SELECT * FROM tasks");
    while (queryTasks.next()) {
        QString name  = queryTasks.value("task_name").toString().replace("\"","\"\"");
        QString note  = queryTasks.value("task_note").toString().replace("\"","\"\"");
        QString deadline    = queryTasks.value("task_deadline").toString();
        QString reminder   = queryTasks.value("task_reminder").toString();
        csv += QString("%1,\"%2\",%3,\"%4\",%5,%6,%7,%8,%9\n")
                   .arg(queryTasks.value("task_id").toInt())
                   .arg(name)
                   .arg(queryTasks.value("task_status").toInt())
                   .arg(note)
                   .arg(queryTasks.value("task_folder_id").toInt())
                   .arg(queryTasks.value("task_priority").toInt())
                   .arg(queryTasks.value("task_duration").toInt())
                   .arg(deadline)
                   .arg(reminder);
    }

    QString filename = QFileDialog::getSaveFileName(this, "Сохранить всё в CSV", "", "CSV-файл (*.csv)");
    if (filename.isEmpty()) return;

    QFile file(filename);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Ошибка", "Не удалось открыть файл для записи");
        return;
    }
    file.write("\xEF\xBB\xBF");
    QTextStream out(&file);
    out << csv;
    file.close();

}


void MainWindow::on_displayTasksOfSelectedDayButton_clicked()
{
    filterStartDate = selectedViaCalendarDate;
    filterEndDate = selectedViaCalendarDate;
    QLocale russian(QLocale::Russian);

    QString dateStr = russian.toString(filterStartDate, "d MMM");

    statusBarFilterText->setText(QString("Отображены задачи на %1")
                                     .arg(dateStr));

    filterStatus = 21;
    updateUI();
}


void MainWindow::on_backToTodayButton_clicked()
{
    calendarWidget->setSelectedDate(QDate::currentDate());
    selectedViaCalendarDate = QDate::currentDate();
    if (filterStatus == 21){
        filterStatus = 0;
        updateUI();
        statusBarFilterText->setText("");
    }
}

