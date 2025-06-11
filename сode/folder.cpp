#include "folder.h"
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

int Folder::folderCount = 0;

Folder::Folder(QObject *parent)
    : QObject{parent}
{
}

Folder::Folder(const QString name, const QString color, int status){
    this->setFolderId();
    this->setFolderName(name);
    this->setFolderColor(color);
    this->setFolderStatus(status);
}

Folder::Folder(int id, const QString name, const QString color, int status){
    this->setFolderId(id);
    this->setFolderName(name);
    this->setFolderColor(color);
    this->setFolderStatus(status);
}

void Folder::saveFolderToDB()
{
    QSqlQuery saveFolderQuery;
    saveFolderQuery.prepare("INSERT INTO folders (folder_id, folder_name, folder_color, folder_status) "
                  "VALUES (:id, :name, :color, :status)");

    saveFolderQuery.bindValue(":id", getFolderId());
    saveFolderQuery.bindValue(":name", getFolderName());
    saveFolderQuery.bindValue(":color", getFolderColor());
    saveFolderQuery.bindValue(":status", getFolderStatus());

    if (!saveFolderQuery.exec()) {
        qDebug() << "Ошибка при добавлении папка: " << saveFolderQuery.lastError().text();
    } else {
        qDebug() << "В БД сохранена папка с ID: " << this->getFolderId();
    }
}
