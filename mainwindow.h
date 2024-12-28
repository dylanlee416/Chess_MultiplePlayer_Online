#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "NetworkClient.h"
#include "NetworkServer.h"

#include <QLineEdit>
#include <QMainWindow>
#include <QPushButton>

class ChessBoard;
class StatusPanel;
class ChatPanel;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnected(const QString &host, quint16 port);
    void onDataReceived(const QByteArray &data);
    void onConnectionStatusChanged(bool connected);
    void onSendMessageClicked(const QString &message);

private:
    bool playerColor;

    void sendMessage(const QByteArray &message);

    void startGame();
    void selectedWidgets();
    void placeWidgets();
    void serverCreated();
    void clientCreated(const QString &host);

    ChessBoard *chessBoard;
    StatusPanel *statusPanel;
    ChatPanel *chatPanel;
    QComboBox *modeSelector;
    QLineEdit *ipInput;
    NetworkServer *server;
    NetworkClient *client;
};

#endif // MAINWINDOW_H
