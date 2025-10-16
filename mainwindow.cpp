#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QFile>
#include <QTextStream>
#include <QCoreApplication>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QTimer>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
    ui(new Ui::MainWindow),
    server(new QTcpServer(this)),
    socket(nullptr),
    udpSocket(new QUdpSocket(this)),
    discoveryTimer(new QTimer(this)),
    broadcastTimer(new QTimer(this))
{
    ui->setupUi(this);
    setWindowTitle("Локальный чат");

    ui->modeLabel->setText("Режим: —");
    ui->modeLabel->setStyleSheet("font-weight: bold; color: gray;");

    // --- Загружаем ник ---
    QFile fileNick(QCoreApplication::applicationDirPath() + "/nick.txt");
    if (fileNick.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileNick);
        nick = in.readLine().trimmed();
        fileNick.close();
        ui->nickInput->setText(nick);
        if (!nick.isEmpty())
            ui->chatBox->append("🟢 Ник загружен: " + nick);
    }

    // --- Кнопки ---
    connect(ui->saveNickButton, &QPushButton::clicked, this, &MainWindow::saveNick);
    connect(ui->clearChatButton, &QPushButton::clicked, this, &MainWindow::clearChat);
    connect(ui->sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(ui->messageInput, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);

    // --- Загружаем историю ---
    QString chatFilePath = QCoreApplication::applicationDirPath() + "/chat.txt";
    QFile fileChat(chatFilePath);
    if (fileChat.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&fileChat);
        while (!in.atEnd()) ui->chatBox->append(in.readLine());
        fileChat.close();
    }

    // --- Клиент ищет сервер ---
    startClientDiscovery();

    // Если сервер не найден за 7 секунд — станем сервером
    connect(discoveryTimer, &QTimer::timeout, this, &MainWindow::becomeServer);
    discoveryTimer->setSingleShot(true);
    discoveryTimer->start(7000);
}

MainWindow::~MainWindow() {
    delete ui;
}

// === КЛИЕНТ: поиск сервера через UDP ===
void MainWindow::startClientDiscovery() {
    const quint16 discoveryPort = 45454;

    bool bound = udpSocket->bind(QHostAddress::AnyIPv4, discoveryPort,
                                 QUdpSocket::ShareAddress | QUdpSocket::ReuseAddressHint);
    if (!bound) {
        ui->chatBox->append("⚠️ Не удалось привязать UDP-порт для обнаружения.");
        return;
    }

    connect(udpSocket, &QUdpSocket::readyRead, this, &MainWindow::onUdpRead);
    ui->chatBox->append("📡 Поиск сервера в локальной сети...");
}

// === КЛИЕНТ: обработка пришедших UDP пакетов ===
void MainWindow::onUdpRead() {
    if (isServer) return; // 🔒 Если уже сервер — не слушаем пакеты

    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(int(udpSocket->pendingDatagramSize()));
        QHostAddress sender;
        quint16 senderPort = 0;
        udpSocket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

        QString msg = QString::fromUtf8(datagram).trimmed();
        if (msg.startsWith("SERVER:")) {
            if (isClient) return; // 🔒 Уже клиент — игнор

            QStringList parts = msg.split(":");
            quint16 port = 12345;
            if (parts.size() > 1) port = parts.at(1).toUShort();

            ui->chatBox->append("🟢 Найден сервер: " + sender.toString() + ":" + QString::number(port));

            isClient = true;
            discoveryTimer->stop();
            udpSocket->close();

            connectToServer(sender.toString(), port);

            ui->modeLabel->setText("Режим: Клиент");
            ui->modeLabel->setStyleSheet("color: blue; font-weight: bold;");
            return;
        }
    }
}


// === СТАНОВИМСЯ СЕРВЕРОМ ===
void MainWindow::becomeServer() {
    // Если уже кто-то стал клиентом — не становимся сервером
    if (isClient || isServer)
        return;

    discoveryTimer->stop();

    if (server->listen(QHostAddress::Any, 12345)) {
        isServer = true;

        connect(server, &QTcpServer::newConnection, this, &MainWindow::handleNewConnection);
        ui->chatBox->append("🟢 Сервер запущен (порт 12345).");
        ui->modeLabel->setText("Режим: Сервер");
        ui->modeLabel->setStyleSheet("color: green; font-weight: bold;");

        // Перестаем слушать UDP-пакеты (мы теперь сервер)
        udpSocket->close();

        // Запускаем broadcast через таймер
        connect(broadcastTimer, &QTimer::timeout, this, &MainWindow::broadcastPresence);
        broadcastTimer->start(2000); // каждые 2 секунды
    } else {
        ui->chatBox->append("⚠️ Не удалось запустить сервер (порт может быть занят).");
    }
}


// === СЕРВЕР: UDP broadcast (без вывода в UI) ===
void MainWindow::broadcastPresence() {
    QByteArray data = "SERVER:12345";
    udpSocket->writeDatagram(data, QHostAddress::Broadcast, 45454);
}

// === КЛИЕНТ: подключаемся к серверу ===
void MainWindow::connectToServer(const QString &ip, quint16 port) {
    if (socket) {
        socket->deleteLater();
        socket = nullptr;
    }

    socket = new QTcpSocket(this);
    connect(socket, &QTcpSocket::connected, [this]() {
        ui->chatBox->append("✅ Подключено к серверу!");
        if (!nick.isEmpty()) socket->write(nick.toUtf8() + "\n");
    });
    connect(socket, &QTcpSocket::readyRead, this, &MainWindow::readMessage);

    socket->connectToHost(ip, port);
}

// === СЕРВЕР: новый клиент подключился ===
void MainWindow::handleNewConnection() {
    QTcpSocket *newClient = server->nextPendingConnection();
    if (!newClient) return;

    clients.append(newClient);
    clientNicks[newClient] = "";

    connect(newClient, &QTcpSocket::readyRead, this, &MainWindow::readMessage);
    connect(newClient, &QTcpSocket::disconnected, [this, newClient]() {
        QString name = clientNicks.value(newClient, "Неизвестный");
        ui->chatBox->append("❌ Клиент отключился: " + name);
        clients.removeAll(newClient);
        clientNicks.remove(newClient);
        newClient->deleteLater();
    });

    ui->chatBox->append("🟢 Новый клиент: " + newClient->peerAddress().toString());
}

// === Чтение сообщений ===
void MainWindow::readMessage() {
    QTcpSocket* senderSocket = qobject_cast<QTcpSocket*>(sender());
    if (!senderSocket) return;

    while (senderSocket->canReadLine()) {
        QString msg = QString::fromUtf8(senderSocket->readLine()).trimmed();

        if (server->isListening() && clientNicks.value(senderSocket).isEmpty()) {
            clientNicks[senderSocket] = msg;
            ui->chatBox->append("🟢 Клиент представился как: " + msg);
            continue;
        }

        QString currentTime = QDateTime::currentDateTime().toString("HH:mm:ss");
        QString fullMsg = "[" + currentTime + "] " + msg;

        if (server->isListening()) {
            broadcastMessage(senderSocket, msg);
        } else {
            ui->chatBox->append(fullMsg);
        }

        appendToChatFile(fullMsg);
    }
}

// === СЕРВЕР: рассылаем сообщение всем клиентам ===
void MainWindow::broadcastMessage(QTcpSocket* sender, const QString& msg) {
    QString senderNick = sender ? clientNicks.value(sender, "Неизвестный") : (nick.isEmpty() ? "Сервер" : nick);
    QString currentTime = QDateTime::currentDateTime().toString("HH:mm:ss");
    QString fullMsg = "[" + currentTime + "] " + senderNick + ": " + msg;

    for (QTcpSocket* client : qAsConst(clients)) {
        if (client && client->isOpen()) {
            client->write(fullMsg.toUtf8() + "\n");
        }
    }

    ui->chatBox->append(fullMsg);
    appendToChatFile(fullMsg);
}

// === Отправка сообщения из UI ===
void MainWindow::sendMessage() {
    QString msg = ui->messageInput->text().trimmed();
    if (msg.isEmpty()) return;

    if (socket && socket->isOpen()) {
        // Клиент: отправляем только текст (без времени и ника, сервер сам добавит)
        socket->write(msg.toUtf8() + "\n");
    } else if (server->isListening()) {
        // Сервер: рассылаем всем
        broadcastMessage(nullptr, msg);
    } else {
        ui->chatBox->append("⚠️ Нет подключения.");
    }

    ui->messageInput->clear();
}


// === Сохранение ника ===
void MainWindow::saveNick() {
    nick = ui->nickInput->text().trimmed();
    if (nick.isEmpty()) return;

    QFile file(QCoreApplication::applicationDirPath() + "/nick.txt");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << nick;
        file.close();
    }
    ui->chatBox->append("🟢 Ник сохранён: " + nick);
}

// === Очистка чата ===
void MainWindow::clearChat() {
    QString chatFilePath = QCoreApplication::applicationDirPath() + "/chat.txt";
    QFile file(chatFilePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) file.close();
    ui->chatBox->clear();
    ui->chatBox->append("🟢 Чат очищен.");
}

// === Сохранение сообщений в файл ===
void MainWindow::appendToChatFile(const QString &msg) {
    QString chatFilePath = QCoreApplication::applicationDirPath() + "/chat.txt";
    QFile file(chatFilePath);
    if (file.open(QIODevice::Append | QIODevice::Text)) {
        QTextStream out(&file);
        out << msg << "\n";
        file.close();
    }
}
