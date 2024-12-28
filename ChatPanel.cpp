#include "chatpanel.h"
#include <QHBoxLayout>
#include <QVBoxLayout>

ChatPanel::ChatPanel(QWidget *parent)
    : QWidget(parent)
{
    initializeUI();
    connectSignalsAndSlots();
}

void ChatPanel::initializeUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    chatHistory = new QTextEdit(this);
    chatHistory->setReadOnly(true);

    QHBoxLayout *inputLayout = new QHBoxLayout;
    messageInput = new QLineEdit(this);
    sendButton = new QPushButton("Send", this);
    inputLayout->addWidget(messageInput);
    inputLayout->addWidget(sendButton);

    mainLayout->addWidget(chatHistory);
    mainLayout->addLayout(inputLayout);
}

void ChatPanel::connectSignalsAndSlots()
{
    connect(sendButton, &QPushButton::clicked, this, &ChatPanel::onSendMessageClicked);
}

void ChatPanel::sendMessage(const QString &message)
{
    if (!message.isEmpty()) {
        emit messageSent(message);
        chatHistory->append("You: " + message);
        messageInput->clear();
    }
}

void ChatPanel::receiveMessage(const QString &message)
{
    chatHistory->append("Opponent: " + message);
}

void ChatPanel::clearChatHistory()
{
    chatHistory->clear();
}

void ChatPanel::onSendMessageClicked()
{
    sendMessage(messageInput->text());
}
