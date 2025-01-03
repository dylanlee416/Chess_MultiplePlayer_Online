#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QList>
#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>

class NetworkServer : public QObject
{
    Q_OBJECT

public:
    // 构造服务端
    explicit NetworkServer(quint16 port, QObject *parent = nullptr);
    ~NetworkServer();

    void stopServer();
    // 是否服务端在监听
    bool isListening() const;
    // 服务端端口号
    quint16 serverPort() const;
    // 发送信息给客户端
    void sendMessageToClient(const QByteArray &message,
                             bool moveInfo = false,
                             bool clockTime = false);
    // 发送开始同步信息给客户端
    void sendClockInfoToClient(int clockTime);
    // 启动服务器
    bool startServer(quint16 port);

signals:
    // 服务端接受数据
    void clientChatDataReceived(const QByteArray &data);
    // 服务端连通状态
    void clientConnected(const QString &ipAddress, quint16 port);
    // 服务端连通状态变更信号
    void connectionStatusChanged(bool connected);
    // 关闭服务端
    void serverStopped();
    // 服务端错误
    void serverError(const QString &error);
    // 接受客户端移动信息
    void clientMoveReceived(int startRow, int startCol, int endRow, int endCol, QString pieceType);
    // 接受客户端准备信号
    void clientReadyInfoReceived();

private slots:
    // 接受客户端准备信号
    void onConnected();
    // 数据接受并解析逻辑
    void onReadyRead();
    // 断联处理
    void onDisconnected();
    // 异常处理
    void onError();
    // 检测连通性
    void checkConnectionStatus();

public slots:
    // 发送移动数据给客户端
    void sendMoveMessageToClient(
        int startRow, int startCol, int endRow, int endCol, QString pieceType);

private:
    //服务器
    QTcpServer *server;
    // 连通状态计时器
    QTimer *connectionMonitorTimer;
    // 连通客户端
    QList<QTcpSocket *> clientSockets;
    // 连通端口
    quint16 port;
    // 上一次连通状态
    bool m_lastConnectionState;
    // 连通客户端
    QTcpSocket *m_connectedClient;

    // 服务器端信息终端DEBUG前缀
    static const char *SERVER_PREFIX;
};

#endif // NETWORKSERVER_H
