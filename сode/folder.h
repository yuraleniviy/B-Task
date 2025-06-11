#ifndef FOLDER_H
#define FOLDER_H

#include <QObject>
#include <qdebug.h>
#include <QMessageBox>

class Folder : public QObject
{
    Q_OBJECT
private:
    int folder_id;
    QString folder_name;
    QString folder_color;
    int folder_status;
public:
    explicit Folder(QObject *parent = nullptr);
    int static folderCount;

    // Сеттеры
    void setFolderId() {
        folder_id = folderCount;
        //qDebug() << "Добавлена папка с ID: " << folder_id;
        folderCount++;
    }
    void setFolderId(int id) {
        folder_id = id;
        //qDebug() << "Добавлена папка с ID: " << folder_id;
        if (folderCount < id + 1) {
            folderCount = id + 1;
        }
    }
    void setFolderName(const QString &name) { folder_name = name; }
    void setFolderColor(const QString &color) { folder_color = color; }
    void setFolderStatus(int status) { folder_status = status; }

    // Геттеры
    int getFolderId() const { return folder_id; }
    QString getFolderName() const { return folder_name; }
    QString getFolderColor() const { return folder_color; }
    int getFolderStatus() const { return folder_status; }


    Folder(const QString name, const QString color, int status);

    Folder(int id, const QString name, const QString color, int status);

    void saveFolderToDB();

signals:
};

#endif // FOLDER_H
