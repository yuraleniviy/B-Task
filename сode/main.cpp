#include "mainwindow.h"

#include <QApplication>
#include <QGraphicsBlurEffect>
#include <QLocalSocket>
#include <QLocalServer>
#include <QSharedMemory>

int main(int argc, char *argv[])
{
    const QString uniqueKey = "BTaskAppInstance";
    QLocalSocket socket;
    socket.connectToServer(uniqueKey);
    if (socket.waitForConnected(100)) {
        // Программа уже запущена — шлём сигнал и выходим
        socket.write("raise");
        socket.flush();
        socket.waitForBytesWritten(100);
        return 0;
    }

    // Новый экземпляр — запускаем сервер для прослушивания новых запусков
    QLocalServer server;
    server.removeServer(uniqueKey);
    if (!server.listen(uniqueKey)) {
        qDebug() << "Не удалось создать сервер";
        return 1;
    }

    QApplication a(argc, argv);

    QPalette palette;
    palette.setColor(QPalette::Window, QColor(247,247,239));
    palette.setColor(QPalette::WindowText, Qt::black);
    palette.setColor(QPalette::Base, QColor(230,230,217));
    palette.setColor(QPalette::Text, Qt::black);
    palette.setColor(QPalette::ButtonText, Qt::black);
    palette.setColor(QPalette::PlaceholderText, Qt::gray);
    palette.setColor(QPalette::Highlight, QColor(247,223,156));
    palette.setColor(QPalette::HighlightedText, QColor(152,111,0));
    //palette.setColor(QPalette::Disabled, QPalette::Button, Qt::gray);  // Цвет кнопок в состоянии disabled
    palette.setColor(QPalette::Disabled, QPalette::Text, Qt::gray);  // Текст кнопки в disabled
    palette.setColor(QPalette::Disabled, QPalette::WindowText, Qt::gray);  // Цвет текста в disabled
    a.setPalette(palette);

    a.setWindowIcon(QIcon(":/pictures/resources/B-Task-logo.svg"));

    MainWindow w;

    w.show();

    // Обрабатываем сигналы от новых запусков
    QObject::connect(&server, &QLocalServer::newConnection, [&]() {
        QLocalSocket *clientConnection = server.nextPendingConnection();
        QObject::connect(clientConnection, &QLocalSocket::readyRead, [=, &w]() {
            QByteArray data = clientConnection->readAll();
            if (data == "raise") {
                w.showNormal();
                w.raise();
                w.activateWindow();
            }
            clientConnection->disconnectFromServer();
        });
    });
    return a.exec();
}
