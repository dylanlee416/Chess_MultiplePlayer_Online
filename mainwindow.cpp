#include "mainwindow.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QIcon>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QRegularExpression>
#include <QScreen>
#include <QTextEdit>
#include <QVBoxLayout>
#include <QWidget>
#include "NetworkClient.h"
#include "NetworkServer.h"
#include "chatpanel.h"
#include "chessboard.h"
#include "statuspanel.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , server(nullptr)
    , client(nullptr)
{
    selectedWidgets();
}

MainWindow::~MainWindow()
{
    if (server) {
        server->stopServer();
        delete server;
    }
    if (client) {
        delete client;
    }
}

void MainWindow::selectedWidgets()
{
    // 创建一个中央窗口部件
    QWidget *centralWidget = new QWidget(this);
    this->setCentralWidget(centralWidget);

    // 设置窗口大小
    this->setFixedSize(200, 100);

    // 创建垂直布局
    QVBoxLayout *layout = new QVBoxLayout(centralWidget);

    // 创建一个下拉列表来选择server或者client
    modeSelector = new QComboBox(this);
    modeSelector->addItem("Server");
    modeSelector->addItem("Client");
    layout->addWidget(modeSelector);

    // 创建一个文本输入框用于输入IP地址
    ipInput = new QLineEdit(this);
    ipInput->setPlaceholderText("Enter IP Address");
    ipInput->setText("172.16.20.118");
    layout->addWidget(ipInput);

    // 创建一个确认按钮
    QPushButton *confirmButton = new QPushButton("Confirm", this);
    layout->addWidget(confirmButton);

    // 可以在这里添加信号槽连接以处理确认按钮的点击事件
    // 例如:
    // connect(confirmButton, &QPushButton::clicked, this, &MainWindow::onConfirmClicked);

    // 为了更好地布局，我们可以添加一些伸缩空间
    layout->addStretch(1);

    // 设置中央部件的布局
    centralWidget->setLayout(layout);

    connect(confirmButton, &QPushButton::clicked, this, &MainWindow::startGame);
};

void MainWindow::startGame()
{
    if (modeSelector->currentText() == "Server") {
        playerColor = true;
        placeWidgets();
        serverCreated();
    } else if (modeSelector->currentText() == "Client") {
        QString ipAddress = ipInput->text();
        // 使用正则表达式验证IP地址格式
        QRegularExpression ipRegex("^((25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(25[0-5]|2[0-4]["
                                   "0-9]|[01]?[0-9][0-9]?)$");
        QRegularExpressionMatch match = ipRegex.match(ipAddress);
        if (match.hasMatch()) {
            playerColor = false;
            placeWidgets();
            clientCreated(ipAddress);
        } else {
            QMessageBox::warning(this,
                                 "Warning",
                                 "Please enter a valid IPv4 address to connect as a Client.");
        }
    } else {
        QMessageBox::warning(this, "Warning", "Please select either Server or Client mode.");
    }
}

void MainWindow::placeWidgets()
{
    delete modeSelector;
    delete ipInput;

    // Create a central widget
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    // Instantiate the ChessBoard, StatusPanel, and ChatPanel
    chessBoard = new ChessBoard(this);
    chatPanel = new ChatPanel(this);
    statusPanel = new StatusPanel(playerColor, this);

    // Connect ChessBoard and StatusPanel
    chessBoard->setStatusPanel(statusPanel);
    statusPanel->setChessBoard(chessBoard);

    // Create a horizontal layout to hold the chessboard, status panel, and chat panel
    QHBoxLayout *mainLayout = new QHBoxLayout(centralWidget);

    // Chessboard Layout
    QVBoxLayout *chessBoardLayout = new QVBoxLayout;
    chessBoardLayout->addWidget(chessBoard);
    mainLayout->addLayout(chessBoardLayout);

    // Status Panel Layout
    QVBoxLayout *statusLayout = new QVBoxLayout;
    statusLayout->addWidget(statusPanel);
    mainLayout->addLayout(statusLayout);

    // Chat Panel Layout
    QVBoxLayout *chatLayout = new QVBoxLayout;
    chatLayout->addWidget(chatPanel);
    mainLayout->addLayout(chatLayout);

    // Set stretch factors for layouts
    mainLayout->setStretch(0, 3); // Chessboard gets 3 parts of the space
    mainLayout->setStretch(1, 1); // Status panel gets 1 part of the space
    mainLayout->setStretch(2, 1); // Chat panel gets 1 part of the space

    // Set the layout to the central widget
    centralWidget->setLayout(mainLayout);

    // Calculate the new minimum width and height based on the chessboard size
    int newWidth = chessBoard->minimumSize().width() * 1.5;
    int newHeight = chessBoard->minimumSize().height();

    // Set the fixed size of the MainWindow
    setFixedSize(newWidth * 1.2, newHeight);

    setWindowIcon(QIcon(":/images/chess_icon.jpg"));
    chessBoard->initial(playerColor);

    // 将窗口居中
    if (QGuiApplication::primaryScreen()) {
        QRect screenGeometry = QGuiApplication::primaryScreen()->availableGeometry();
        int x = (screenGeometry.width() - width()) / 2;
        int y = (screenGeometry.height() - height()) / 2;
        move(x, y);
    }
}

void MainWindow::serverCreated()
{
    // Create the NetworkServer
    server = new NetworkServer(5010, this);

    // Connect server signals to appropriate slots
    connect(server, &NetworkServer::clientConnected, this, &MainWindow::onConnected);
    connect(server, &NetworkServer::clientChatDataReceived, this, &MainWindow::onDataReceived);
    connect(server,
            &NetworkServer::connectionStatusChanged,
            this,
            &MainWindow::onConnectionStatusChanged);
    connect(chessBoard,
            &ChessBoard::moveMessageSent,
            server,
            &NetworkServer::sendMoveMessageToClient);
    connect(server, &NetworkServer::clientMoveReceived, chessBoard, &ChessBoard::moveByOpponent);
    connect(server,
            &NetworkServer::clientReadyInfoReceived,
            statusPanel,
            &StatusPanel::enableStartButton);
    connect(statusPanel,
            &StatusPanel::setClientClcok,
            server,
            &NetworkServer::sendClockInfoToClient);

    connect(chatPanel, &ChatPanel::messageSent, this, &MainWindow::onSendMessageClicked);
}

void MainWindow::clientCreated(const QString &host)
{
    // Create the NetworkClient
    // const QString &host = "172.16.20.118";
    // const QString &host = "172.16.21.13";
    // const QString &host = "127.0.0.1";

    client = new NetworkClient(host, 5010, this);

    connect(client, &NetworkClient::serverConnected, this, &MainWindow::onConnected);
    connect(client, &NetworkClient::serverChatDataReceived, this, &MainWindow::onDataReceived);
    connect(client,
            &NetworkClient::connectionStatusChanged,
            this,
            &MainWindow::onConnectionStatusChanged);
    connect(chessBoard,
            &ChessBoard::moveMessageSent,
            client,
            &NetworkClient::sendMoveMessageToServer);
    connect(client, &NetworkClient::serverMoveReceived, chessBoard, &ChessBoard::moveByOpponent);
    connect(client,
            &NetworkClient::startGameAndSetClock,
            statusPanel,
            &StatusPanel::synClockAndStartGame);
    connect(statusPanel,
            &StatusPanel::sentReadyInfoToServer,
            client,
            &NetworkClient::sentReadyInfoToServer);

    connect(chatPanel, &ChatPanel::messageSent, this, &MainWindow::onSendMessageClicked);
}

void MainWindow::onConnected(const QString &ipAddress, quint16 port)
{
    if (server) qDebug() << "(server) Client connected from ip:" << ipAddress << " and port:" << port;
    if (client) qDebug() << "(client) Server connected from ip:" << ipAddress << " and port:" << port;
}

void MainWindow::onDataReceived(const QByteArray &data)
{
    QString message = QString::fromUtf8(data);
    chatPanel->receiveMessage(message);
}

void MainWindow::onConnectionStatusChanged(bool connected)
{
    if (server)
    {
        if (connected) {
            qDebug() << "(server) Server is now connected to Client";
            statusPanel->setBlackLightOn();
        } else {
            qDebug() << "(server) Server is no longer connected to any Client";
            statusPanel->setBlackLightOff();
        }
    }

    if (client)
    {
        if (connected) {
            qDebug() << "(client) Client is now connected to Server";
            statusPanel->setWhiteLightOn();
        } else {
            qDebug() << "(client) Client is no longer connected to any Server";
            statusPanel->setWhiteLightOff();
        }
    }
}

void MainWindow::onSendMessageClicked(const QString &message)
{
    sendMessage(message.toUtf8());
}

void MainWindow::sendMessage(const QByteArray &message)
{
    if (server) {
        server->sendMessageToClient(message);
    }

    if (client) {
        client->sendMessageToServer(message);
    }
}
