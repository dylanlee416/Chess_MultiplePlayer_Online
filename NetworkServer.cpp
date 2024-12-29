#include "NetworkServer.h"
#include <QDebug>
#include <QHostAddress>
#include <QTimer>

const char *NetworkServer::SERVER_PREFIX = "(server)";

NetworkServer::NetworkServer(quint16 _port, QObject *parent)
    : QObject(parent)
    , port(_port)
    , m_lastConnectionState(false)
    , m_connectedClient(nullptr)
{
    server = new QTcpServer(this);

    // Connect the newConnection signal to the onConnected slot
    connect(server, &QTcpServer::newConnection, this, &NetworkServer::onConnected);

    // Timer for monitoring connections
    connectionMonitorTimer = new QTimer(this);
    connect(connectionMonitorTimer, &QTimer::timeout, this, &NetworkServer::checkConnectionStatus);

    // Start the server immediately with the given port
    if (!startServer(port)) {
        emit serverError(server->errorString());
    } else {
        qDebug().noquote() << SERVER_PREFIX << "Server started on port" << server->serverPort();
    }
}

NetworkServer::~NetworkServer()
{
    stopServer();
    qDebug().noquote() << SERVER_PREFIX << "Server closed";
}

bool NetworkServer::isListening() const
{
    return server->isListening();
}

bool NetworkServer::startServer(quint16 port)
{
    if (!server->isListening()) {
        if (server->listen(QHostAddress::Any, port)) {
            qDebug().noquote() << SERVER_PREFIX << "Server is listening on port"
                               << server->serverPort();
            connectionMonitorTimer->start(5000); // Start checking connections
            emit connectionStatusChanged(true);
            return true;
        } else {
            emit serverError(server->errorString());
            return false;
        }
    }
    return true; // Server was already listening
}

void NetworkServer::stopServer()
{
    if (server->isListening()) {
        server->close();
        connectionMonitorTimer->stop();
        if (m_connectedClient) {
            m_connectedClient->disconnectFromHost();
            m_connectedClient->deleteLater();
            m_connectedClient = nullptr;
        }
        emit serverStopped();
        emit connectionStatusChanged(false);
        qDebug().noquote() << SERVER_PREFIX << "Server stopped";
    }
}

quint16 NetworkServer::serverPort() const
{
    return server->serverPort();
}

void NetworkServer::sendMessageToClient(const QByteArray &message, bool moveInfo, bool startInfo)
{
    QByteArray messageWithPrefix;

    if (moveInfo) {
        messageWithPrefix = "[MOVE]" + message;
    } else if (startInfo) {
        messageWithPrefix = "[START]" + message;
    } else {
        messageWithPrefix = "[MSG]" + message;
    }

    if (m_connectedClient && m_connectedClient->state() == QAbstractSocket::ConnectedState) {
        if (m_connectedClient->write(messageWithPrefix) != -1) {
            m_connectedClient->flush(); // Ensure the data is sent immediately
            qDebug().noquote() << SERVER_PREFIX << "Sent message to client"
                               << m_connectedClient->peerAddress().toString() << ":" << messageWithPrefix;
        } else {
            qDebug().noquote() << SERVER_PREFIX << "Failed to send message to client"
                               << m_connectedClient->peerAddress().toString();
        }
    }
}

void NetworkServer::onConnected()
{
    // Check if there's already a connected client
    if (m_connectedClient) {
        // If there's already a client, close the new connection
        QTcpSocket *newConnection = server->nextPendingConnection();
        newConnection->disconnectFromHost();
        newConnection->deleteLater();
        qDebug().noquote() << SERVER_PREFIX << "Rejected new connection because a client is already connected.";
        return;
    }

    // Get the new connection socket
    m_connectedClient = server->nextPendingConnection();

    // Connect socket signals to slots
    connect(m_connectedClient, &QTcpSocket::readyRead, this, &NetworkServer::onReadyRead);
    connect(m_connectedClient, &QTcpSocket::disconnected, this, &NetworkServer::onDisconnected);
    connect(m_connectedClient, &QTcpSocket::errorOccurred, this, &NetworkServer::onError);

    // Record and emit the IP address of the connected client
    QString ipAddress = m_connectedClient->peerAddress().toString();
    qDebug().noquote() << SERVER_PREFIX << "New connection from:" << ipAddress;
    emit clientConnected(ipAddress, port);
}

void NetworkServer::onReadyRead()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        QByteArray data = clientSocket->readAll();
        QString ipAddress = clientSocket->peerAddress().toString();

        if (data.startsWith("[MOVE]")) {
            // Handle move data
            data = data.mid(6); // Remove the prefix
            // Assuming 'data' is in the format: "startRow,startCol,endRow,endCol,pieceType"
            QStringList parts = QString(data).split(',');
            int startRow = parts[0].toInt();
            int startCol = parts[1].toInt();
            int endRow = parts[2].toInt();
            int endCol = parts[3].toInt();
            QString pieceType = parts[4];
            emit clientMoveReceived(startRow, startCol, endRow, endCol, pieceType);
            qDebug().noquote() << SERVER_PREFIX << "Move data received from client" << ipAddress
                               << ":" << data;
        } else if (data.startsWith("[MSG]")) {
            // Handle regular message
            data = data.mid(5); // Remove the prefix
            emit clientChatDataReceived(data);
            qDebug().noquote() << SERVER_PREFIX << "Chat message received from client" << ipAddress
                               << ":" << data;
        } else if (data.startsWith("[READY]")) {
            // Handle regular message
            emit clientReadyInfoReceived();
            qDebug().noquote() << SERVER_PREFIX << "READY INFO received from client" << ipAddress
                               << ":" << data;
        } else {
            qDebug().noquote() << SERVER_PREFIX << "Received invalid message from client"
                               << ipAddress << ":" << data;
        }
    }
}

void NetworkServer::onDisconnected()
{
    if (m_connectedClient) {
        QString ipAddress = m_connectedClient->peerAddress().toString();
        qDebug().noquote() << SERVER_PREFIX << "Client disconnected:" << ipAddress;
        m_connectedClient->deleteLater(); // Clean up the socket
        m_connectedClient = nullptr;
        emit connectionStatusChanged(false); // Update connection status
    }
}

void NetworkServer::onError()
{
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    qDebug().noquote() << SERVER_PREFIX << "Error occurred:" << clientSocket->errorString();
}

void NetworkServer::checkConnectionStatus()
{
    if (server->isListening()) {
        bool anyConnected = !clientSockets.isEmpty();
        if (m_lastConnectionState != anyConnected) {
            m_lastConnectionState = anyConnected;
            emit connectionStatusChanged(anyConnected);
            qDebug().noquote() << SERVER_PREFIX << "Connection status:"
                               << (anyConnected ? "Connected" : "No connections");
        }
    }
}

void NetworkServer::sendMoveMessageToClient(
    int startRow, int startCol, int endRow, int endCol, QString pieceType)
{
    // Format: [MOVE]startRow,startCol,endRow,endCol
    QString moveMessage = QString("%1,%2,%3,%4,%5")
                              .arg(startRow)
                              .arg(startCol)
                              .arg(endRow)
                              .arg(endCol)
                              .arg(pieceType);
    sendMessageToClient(moveMessage.toUtf8(), true);
}

void NetworkServer::sendClockInfoToClient(int clockTime)
{
    const QByteArray message = QString("%1").arg(clockTime).toUtf8();
    sendMessageToClient(message, 0, 1);
}
