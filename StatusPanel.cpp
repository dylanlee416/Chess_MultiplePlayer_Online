#include "statuspanel.h"
#include <QHBoxLayout>
#include <QLabel>
#include <QMessageBox>
#include <QVBoxLayout>

#include "chessboard.h"

StatusPanel::StatusPanel(bool _playerColor, QWidget *parent)
    : QWidget(parent)
    , playerColor(_playerColor)
    , isReady(0)
    , whiteTime(0)
    , blackTime(0)
{
    initializeUI();

    // Timer to update the clocks every second
    gameTimer = new QTimer(this);
    connect(gameTimer, &QTimer::timeout, this, &StatusPanel::updateClocks);
}

// 定义一个函数来创建一个表示灯的QWidget
QWidget* StatusPanel::createStatusLight(bool isConnected) {
    QWidget* light = new QWidget(this);
    light->setFixedSize(10, 10); // 设置灯的大小
    light->setStyleSheet(isConnected ?
                             "background-color: green; border-radius: 5px;" :
                             "background-color: red; border-radius: 5px;");
    return light;
}

void StatusPanel::initializeUI()
{
    // Create LCD displays for both players' clocks
    whiteClock = new QLCDNumber(this);
    blackClock = new QLCDNumber(this);

    // Set digit count to accommodate hours and minutes (hh:mm:ss format)
    whiteClock->setDigitCount(8); // hh:mm:ss format
    blackClock->setDigitCount(8); // hh:mm:ss format

    blackClock->setSegmentStyle(QLCDNumber::Filled);
    blackClock->setSegmentStyle(QLCDNumber::Filled);

    // Set default styles for clocks
    whiteClock->setStyleSheet(
        "background-color: green; color: black; font-size: 24px;"); // Default style
    blackClock->setStyleSheet(
        "background-color: gray; color: white; font-size: 24px;"); // Default style

    // Create the time selector dropdown
    timeSelector = new QComboBox(this);
    // timeSelector->addItem("4 seconds", 4);
    timeSelector->addItem("5 minutes", 300);
    timeSelector->addItem("10 minutes", 600);
    timeSelector->addItem("15 minutes", 900);
    timeSelector->addItem("1 hour", 3600);
    timeSelector->addItem("2 hours", 7200);
    timeSelector->addItem("3 hours", 10800);
    timeSelector->addItem("6 hours", 21600);
    timeSelector->addItem("12 hours", 43200);
    timeSelector->addItem("24 hours", 86400);

    // Initialize clocks to 05:00
    int initialTime = timeSelector->itemData(0).toInt(); // 获取第一个选项的时间（秒）

    whiteTime = initialTime; // 重置白棋计时器
    blackTime = initialTime; // 重置黑棋计时器

    // 更新计时器显示
    updateClockDisplay(); // 将白棋和黑棋的初始时间更新到 UI

    // Create the start button
    readyButton = new QPushButton("Ready", this);
    readyButton->setFixedSize(100, 30); // 设置按钮的固定宽度为100，高度为50
    connect(readyButton, &QPushButton::clicked, this, &StatusPanel::readyForGame);

    // Create the start button
    startButton = new QPushButton("Start", this);
    startButton->setFixedSize(100, 30); // 设置按钮的固定宽度为100，高度为50
    connect(startButton, &QPushButton::clicked, this, &StatusPanel::startGame);

    // Create the move history text edit
    moveHistory = new QTextEdit(this);
    moveHistory->setReadOnly(true); // Make the move history read-only

    // 在你的布局中添加这个灯
    QHBoxLayout *whiteClockLayout = new QHBoxLayout();
    whiteLight = createStatusLight(false); // 假设连接是可通的
    whiteClockLayout->addWidget(whiteLight);
    whiteClockLayout->addWidget(new QLabel("White Player", this));
    whiteClockLayout->addStretch(); // Stretchable space between label and clock
    whiteClockLayout->addWidget(whiteClock);

    QHBoxLayout *blackClockLayout = new QHBoxLayout();
    blackLight = createStatusLight(false); // 假设连接不可通
    blackClockLayout->addWidget(blackLight);
    blackClockLayout->addWidget(new QLabel("Black Player", this));
    blackClockLayout->addStretch(); // Stretchable space between label and clock
    blackClockLayout->addWidget(blackClock);

    // Create a horizontal layout for the time selector
    QHBoxLayout *timeSelectorLayout = new QHBoxLayout();
    QLabel *timeLabel = new QLabel("Select Time:", this);
    timeSelectorLayout->addWidget(timeLabel);
    timeSelectorLayout->addWidget(timeSelector);

    // Create a vertical layout for the overall panel
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Add the time selector layout at the top
    mainLayout->addLayout(timeSelectorLayout);

    // Add the clock layout
    if (playerColor == true)
        mainLayout->addLayout(blackClockLayout);
    else
        mainLayout->addLayout(whiteClockLayout);
    // Add the black clock layout

    // Add the move history text edit
    mainLayout->addWidget(moveHistory);

    // Add the clock layout
    if (playerColor != true)
        mainLayout->addLayout(blackClockLayout);
    else
        mainLayout->addLayout(whiteClockLayout);

    // 创建布局以将按钮居中
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->addStretch(); // 添加可伸缩空间到左侧

    if (playerColor) {
        buttonLayout->addWidget(startButton); // 添加开始按钮
        readyButton->hide();
    } else {
        buttonLayout->addWidget(readyButton); // 添加准备按钮
        startButton->hide();
    }

    buttonLayout->addStretch(); // 添加可伸缩空间到右侧

    // Add the button layout to the main vertical layout
    mainLayout->addLayout(buttonLayout);

    // Set the main layout to the StatusPanel
    setLayout(mainLayout);
}

void StatusPanel::readyForGame()
{
    readyButton->setEnabled(false);
    readyButton->setText("");

    emit sentReadyInfoToServer();
}

void StatusPanel::startGame()
{
    if (!isReady) {
        // 如果startButton不可用，弹出一个信息框
        QMessageBox::information(this, "INFO", "Other player doesn't ready!");
        return; // 直接返回，避免执行后续的游戏开始逻辑
    }

    if (chessBoard)
        chessBoard->startGame();

    int selectedTime = timeSelector->currentData().toInt();
    initialClock(selectedTime);
    emit setClientClcok(selectedTime);

    // Disable the start button and time selector
    startButton->setEnabled(false);
    startButton->setText("");
    timeSelector->setEnabled(false);
}

void StatusPanel::enableStartButton()
{
    isReady = 1;
    startButton->setEnabled(true);
}

void StatusPanel::synClockAndStartGame(int selectedTime)
{
    if (chessBoard)
        chessBoard->startGame();

    initialClock(selectedTime);
    readyButton->setEnabled(false);
    readyButton->setText("");
    timeSelector->setEnabled(false);
}

void StatusPanel::initialClock(int selectedTime)
{
    whiteTime = blackTime = selectedTime;

    // Update the clocks
    updateClockDisplay();

    // Start the timer
    gameTimer->start(1000);
}

void StatusPanel::updateClockDisplay()
{
    // Function to format the display string
    auto formatTime = [](int seconds) -> QString {
        if (seconds >= 3600) {
            // Display in hh:mm:ss format
            return QString("%1:%2:%3")
                .arg(seconds / 3600, 2, 10, QChar('0'))
                .arg((seconds % 3600) / 60, 2, 10, QChar('0'))
                .arg(seconds % 60, 2, 10, QChar('0'));
        } else {
            // Display in mm:ss format if hours are zero
            return QString("%1:%2")
                .arg(seconds / 60, 2, 10, QChar('0'))
                .arg(seconds % 60, 2, 10, QChar('0'));
        }
    };

    // Update the display with the current time values
    whiteClock->display(formatTime(whiteTime));
    blackClock->display(formatTime(blackTime));
}

void StatusPanel::updateClocks()
{
    if (!chessBoard->getIsGaming())
        return;

    // Decrease time for the current player
    if (chessBoard->getIsCurrentWhite()) {
        whiteTime--;
        if (whiteTime < 0)
            whiteTime = 0; // Ensure time does not go negative

        // Update the display with the current time values
        updateClockDisplay();

        // Change color and background if the remaining time is low (e.g., less than 10 seconds)
        if (whiteTime <= 10) {
            whiteClock->setStyleSheet(
                "background-color: red; color: white; font-size: 24px;"); // Emphasize with red background and white text
        } else {
            whiteClock->setStyleSheet(
                "background-color: green; color: black; font-size: 24px;"); // Emphasize with green background and black text
        }
        blackClock->setStyleSheet(
            "background-color: gray; color: white; font-size: 24px;"); // Default style
    } else {
        blackTime--;
        if (blackTime < 0)
            blackTime = 0; // Ensure time does not go negative

        // Update the display with the current time values
        updateClockDisplay();

        // Change color and background if the remaining time is low (e.g., less than 10 seconds)
        if (blackTime <= 10) {
            blackClock->setStyleSheet(
                "background-color: red; color: white; font-size: 24px;"); // Emphasize with red background and white text
        } else {
            blackClock->setStyleSheet(
                "background-color: green; color: black; font-size: 24px;"); // Emphasize with green background and black text
        }
        whiteClock->setStyleSheet(
            "background-color: gray; color: white; font-size: 24px;"); // Default style
    }

    // Switch turns if time runs out and determine the result
    if (whiteTime == 0 || blackTime == 0) {
        gameTimer->stop(); // Stop the timer when time runs out

        // Determine the result of the game
        if (whiteTime == 0 && blackTime > 0) {
            // White loses due to timeout
            showTimeOutMessage(true); // Pass true since it's white's time that ran out
        } else if (blackTime == 0 && whiteTime > 0) {
            // Black loses due to timeout
            showTimeOutMessage(false); // Pass false since it's black's time that ran out
        } else if (whiteTime == 0 && blackTime == 0) {
            // Both players run out of time, it's a draw
            showGameOverMessage("It's a draw!");
        }
    }
}

void StatusPanel::showTimeOutMessage(bool whiteTurn)
{
    // 创建消息框
    QMessageBox msgBox;
    msgBox.setWindowTitle("Time's Up!");

    // 根据当前轮次设置文本 (例如，白方时间耗尽，黑方获胜)
    msgBox.setText(whiteTurn ? "White's time is up. Black wins!"
                             : "Black's time is up. White wins!");
    msgBox.setWindowIcon(QIcon(":/images/chess_icon.jpg"));
    msgBox.setIcon(QMessageBox::NoIcon); // 去除图标

    // 调整字体大小并设置居中对齐
    msgBox.setStyleSheet("QLabel { font-size: 14px; font-weight: bold; text-align: left; } "
                         "QPushButton { font-size: 10px; }");

    msgBox.setStandardButtons(QMessageBox::Ok);

    // 设置 QLabel 对齐方式为居中
    QLabel *label = msgBox.findChild<QLabel *>();
    if (label) {
        label->setAlignment(Qt::AlignCenter);
    }

    // 添加 spacer 来调整消息框大小
    QSpacerItem *spacer = new QSpacerItem(200, 150, QSizePolicy::Minimum, QSizePolicy::Expanding);
    QGridLayout *layout = (QGridLayout *) msgBox.layout();
    layout->addItem(spacer, layout->rowCount(), 0, 1, layout->columnCount());

    // 显示消息框
    msgBox.exec();

    chessBoard->timeRunOut();
}

void StatusPanel::showGameOverMessage(const QString &message)
{
    // Display a message box or update a label to show the game result
    QMessageBox::information(this, "Game Over", message);
}

void StatusPanel::addMoveToHistory(const QString &move, int step)
{
    moveHistory->insertPlainText(move); // 插入文本，不自动换行
    if (step % 2)
        moveHistory->append(QString());
}

void StatusPanel::getClockTime(int clockTime)
{
    initialClock(clockTime);
}
