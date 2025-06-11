#ifndef FOLDEREDITWINDOW_H
#define FOLDEREDITWINDOW_H

#include <QDialog>
#include <QTreeWidgetItem>

namespace Ui {
class FolderEditWindow;
}

class FolderEditWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FolderEditWindow(QWidget *parent = nullptr);
    void showAt(const QPoint &pos, const QString &text, QTreeWidgetItem *item);
    void addFolderInfoToDesign(QTreeWidgetItem *item);
    void setCurrentEditingFolderId(QTreeWidgetItem *item);
    ~FolderEditWindow();

signals:
    void updateTaskBlock();

private slots:
    void on_folderEditSave_clicked();
    void on_folderEditClose_clicked();
    void on_folderEditDelete_clicked();

private:
    Ui::FolderEditWindow *ui;
    int currentEditingFolderId;
};

#endif // FOLDEREDITWINDOW_H
