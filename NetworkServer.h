#ifndef NETWORKSERVER_H
#define NETWORKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>

class NetworkServer : public QObject
{
    Q_OBJECT

public:
    explicit NetworkServer(quint16 port, QObject *parent = nullptr);
    ~NetworkServer();

    void stopServer();
    bool isListening() const;
    quint16 serverPort() const;
    void sendMessageToClient(const QByteArray &message, bool moveInfo = false, bool clockTime = false);
    void sendClockInfoToClient(int clockTime);
    bool startServer(quint16 port);

signals:
    void clientChatDataReceived(const QByteArray &data);
    void clientConnected(const QString &ipAddress, quint16 port);
    void connectionStatusChanged(bool connected);
    void serverStopped();
    void serverError(const QString &error);
    void clientMoveReceived(int startRow, int startCol, int endRow, int endCol, QString pieceType);
    void clientReadyInfoReceived();

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void onError();
    void checkConnectionStatus();

public slots:
    void sendMoveMessageToClient(int startRow, int startCol, int endRow, int endCol, QString pieceType);

private:
    QTcpServer *server;
    QTimer *connectionMonitorTimer;
    QList<QTcpSocket *> clientSockets;
    quint16 port;
    bool m_lastConnectionState;

    static const char* SERVER_PREFIX;
};

#endif // NETWORKSERVER_H

