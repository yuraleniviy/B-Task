#include "foldereditwindow.h"
#include "ui_foldereditwindow.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>
#include <QMessageBox>
#include <QScreen>

FolderEditWindow::FolderEditWindow(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FolderEditWindow)
{
    ui->setupUi(this);

    ui->folderEditName->setFocus();

    setModal(true);
}

void FolderEditWindow::showAt(const QPoint &pos, const QString &text, QTreeWidgetItem *item)
{
    setCurrentEditingFolderId(item);
    addFolderInfoToDesign(item);

    QScreen *screen = QGuiApplication::primaryScreen();
    if (screen) {
        QRect screenGeometry = screen->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(screenGeometry.topLeft() + QPoint(x, y));
    }
    show();
    ui->folderEditName->setFocus();
}

void FolderEditWindow::addFolderInfoToDesign(QTreeWidgetItem *item)
{
    QSqlQuery query;
    query.prepare("SELECT * FROM folders WHERE folder_id = :id");
    query.bindValue(":id", currentEditingFolderId);

    if (query.exec()) {
        if (query.next()) {
            ui->folderEditName->setText(query.value("folder_name").toString());
            QString folderColorString = query.value("folder_color").toString();
            if (folderColorString == "white"){ ui->folderColorWhite->setChecked(true);}
            if (folderColorString == "red"){ ui->folderColorRed->setChecked(true);}
            if (folderColorString == "green"){ ui->folderColorGreen->setChecked(true);}
            if (folderColorString == "blue"){ ui->folderColorBlue->setChecked(true);}
        }
    } else {
        qDebug() << "Query error:" << query.lastError().text();
    }
}

void FolderEditWindow::setCurrentEditingFolderId(QTreeWidgetItem *item)
{
    QVariantList folderData = item->data(0, Qt::UserRole).toList();
    currentEditingFolderId = folderData[1].toInt();
}



FolderEditWindow::~FolderEditWindow()
{
    delete ui;
}

void FolderEditWindow::on_folderEditSave_clicked()
{
    QAbstractButton *checkedButton = ui->buttonGroup->checkedButton();
    QString newfolderColor;
    if (checkedButton->objectName() == "folderColorWhite") {newfolderColor = "white";};
    if (checkedButton->objectName() == "folderColorRed") {newfolderColor = "red";};
    if (checkedButton->objectName() == "folderColorGreen") {newfolderColor = "green";};
    if (checkedButton->objectName() == "folderColorBlue") {newfolderColor = "blue";};

    QSqlQuery updateFolderQuery;
    updateFolderQuery.prepare("UPDATE folders SET folder_name = :name,"
                            "folder_color = :color,"
                            "folder_status = :status "
                            "WHERE folder_id = :id;");
    updateFolderQuery.bindValue(":name", ui->folderEditName->text());
    updateFolderQuery.bindValue(":color", newfolderColor);
    updateFolderQuery.bindValue(":status", 1);
    updateFolderQuery.bindValue(":id", currentEditingFolderId);

    if (!updateFolderQuery.exec()) {
        qDebug() << "Ошибка при изменении папки: " << updateFolderQuery.lastError().text();
    } else {
        qDebug() << "Папка успешно изменена!";
    }

    emit updateTaskBlock();
    close();
}


void FolderEditWindow::on_folderEditClose_clicked()
{
    close();
}


void FolderEditWindow::on_folderEditDelete_clicked()
{
    QMessageBox msgBox;
    msgBox.setText("Осторожно!");
    msgBox.setInformativeText("При удалении папки будут удалены все задачи, которые в ней находятся. Продолжить?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::Cancel);
    msgBox.button(QMessageBox::Yes)->setText("Да");
    msgBox.button(QMessageBox::Cancel)->setText("Отмена!");
    msgBox.setIcon(QMessageBox::Warning);
    msgBox.setStyleSheet("QMessageBox {"
                         "   border: 2px solid grey;"
                         "   border-radius: 5px;"
                         "   font-family: Bahnschrift;"
                         "   font-size: 14px;"
                         "}"
                         "QPushButton {"
                         "   font-size: 14px;"
                         "   border-radius: 5px;"
                         "   padding: 10px 30px;"
                         "   font-family: Bahnschrift;"
                         "   border: 2px solid grey;"
                         "}"
                         "QPushButton:hover {"
                         "   background-color: #f0f0f0;"
                         "}"
                         "QMessageBox QLabel {"
                         "   font-size: 14px;"
                         "}");
    if(msgBox.exec() == QMessageBox::Yes){
        QSqlQuery deleteFolderQuery;
        deleteFolderQuery.prepare("DELETE FROM folders WHERE folder_id = :id;");

        deleteFolderQuery.bindValue(":id", currentEditingFolderId);
        if (!deleteFolderQuery.exec()) {
            qDebug() << "Ошибка при удалении папки: " << deleteFolderQuery.lastError().text();
        } else {
            qDebug() << "Папка успешно удалена!";
        }

        QSqlQuery deleteTasksQuery;
        deleteTasksQuery.prepare("DELETE FROM tasks WHERE task_folder_id = :id;");
        deleteTasksQuery.bindValue(":id", currentEditingFolderId);
        if (!deleteTasksQuery.exec()) {
            qDebug() << "Ошибка при удалении задач в удаляемой папке: " << deleteTasksQuery.lastError().text();
        } else {
            qDebug() << "Задачи успешно удалены!";
        }


        emit updateTaskBlock();
        close();
    }
}

