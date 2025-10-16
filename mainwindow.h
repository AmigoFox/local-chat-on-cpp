#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTcpServer>
#include <QTcpSocket>
#include <QUdpSocket>
#include <QTimer>
#include <QList>
#include <QMap>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // UI actions
    void sendMessage();
    void readMessage();
    void handleNewConnection();
    void saveNick();
    void clearChat();

    // Discovery / UDP
    void onUdpRead();
    void broadcastPresence();
    void becomeServer();

private:
    Ui::MainWindow *ui;

    // TCP
    QTcpServer *server;
    QTcpSocket *socket;                 // клиентский сокет (если мы клиент)
    QList<QTcpSocket*> clients;         // список подключённых клиентов (если мы сервер)
    QMap<QTcpSocket*, QString> clientNicks;
    QTimer *broadcastTimer;

    bool isServer = false;
    bool isClient = false;

    // UDP discovery
    QUdpSocket *udpSocket;
    QTimer *discoveryTimer;

    // state
    QString nick;

    // helpers
    void startClientDiscovery();
    void connectToServer(const QString &ip, quint16 port);
    void broadcastMessage(QTcpSocket* sender, const QString& msg);
    void appendToChatFile(const QString &msg);
};

#endif // MAINWINDOW_H
