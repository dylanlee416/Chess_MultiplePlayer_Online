#ifndef CHATPANEL_H
#define CHATPANEL_H

#include <QLineEdit>
#include <QPushButton>
#include <QTextEdit>
#include <QWidget>

class ChatPanel : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPanel(QWidget *parent = nullptr);

    void connectSignalsAndSlots();
    // Method to send a message
    void sendMessage(const QString &message);

    // Method to receive and display a message
    void receiveMessage(const QString &message);

    // Method to clear chat history
    void clearChatHistory();

private:
    QTextEdit *chatHistory;  // TextEdit to display chat history
    QLineEdit *messageInput; // Input field for typing messages
    QPushButton *sendButton; // Button to send messages

    void initializeUI(); // Method to initialize the UI elements

private slots:
    void onSendMessageClicked();

signals:
    void messageSent(const QString &message); // Signal emitted when a message is sent
};

#endif // CHATPANEL_H
