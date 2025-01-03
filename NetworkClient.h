#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QString>
#include <QTcpSocket>
#include <QTimer>

class NetworkClient : public QObject
{
    Q_OBJECT

public:
    // 通过服务端ip以及端口进行客户端的构造
    explicit NetworkClient(const QString &host, quint16 port, QObject *parent = nullptr);
    ~NetworkClient();

    // 发送信息给服务端
    void sendMessageToServer(const QByteArray &message,
                             bool moveInfo = false,
                             bool readyInfo = false);
    // 发送准备信息给服务器端
    void sentReadyInfoToServer();

signals:
    // 客户端连通状态改变
    void connectionStatusChanged(bool connected);
    // 服务器端信息接收
    void serverChatDataReceived(const QByteArray &data);
    // 连接服务器端时发送信号
    void serverConnected(const QString &host, quint16 port);
    // 接受服务器端移动数据
    void serverMoveReceived(int startRow, int startCol, int endRow, int endCol, QString pieceType);
    // 开始游戏以及时钟同步信号接受
    void startGameAndSetClock(int clockTime);

private slots:
    // 连通状态发送
    void onConnected();
    // 接受数据并解析逻辑
    void onReadyRead();
    // 断联状态逻辑
    void onDisconnected();
    // 错误发生
    void onError();
    // 检查连通状态
    void checkConnectionStatus();

public slots:
    // 发送移动信息给服务器端
    void sendMoveMessageToServer(
        int startRow, int startCol, int endRow, int endCol, QString pieceType);

private:
    // 客户端socket
    QTcpSocket *socket;
    // 连通状态检测时钟倒计时
    QTimer *connectionMonitorTimer;
    // 连接ip
    QString host;
    // 通信端口
    quint16 port;
    // 上一次连接状态
    bool m_lastConnectionState;

    // 客户端终端DEBUG前缀
    static const char *CLIENT_PREFIX;
};

#endif // NETWORKCLIENT_H
