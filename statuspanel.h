#ifndef STATUSPANEL_H
#define STATUSPANEL_H

#include <QComboBox>
#include <QLCDNumber>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit> // For displaying move history
#include <QTimer>
#include <QWidget>

class ChessBoard;

class StatusPanel : public QWidget
{
    Q_OBJECT

public:
    explicit StatusPanel(bool _playColor, QWidget *parent = nullptr);
    void setChessBoard(ChessBoard *_chessBoard) { chessBoard = _chessBoard; }

    // Method to start the game clock
    void readyForGame();
    void startGame();
    void initialClock(int selectedTime);
    void enableStartButton();
    void synClockAndStartGame(int selectedTime);
    void getClockTime(int clockTime);
    void stopTimer() { gameTimer->stop(); }
    void switchTurns(); // Switch turns between players
    void addMoveHistoryToStatusPlane(QPair<QPoint, QPoint> move);
    void addMoveToHistory(const QString &move, int step); // Add a move to the history
    int getGameTime() { return timeSelector->currentData().toInt(); }

signals:
    void setClientClcok(int selectedTime);
    void sentReadyInfoToServer();

private:
    ChessBoard *chessBoard;
    bool playerColor;
    bool isReady;

    QLabel *statusLabel;      // Label to display game status information
    QComboBox *timeSelector;  // Dropdown for selecting time (5, 10, 15, 60 minutes)
    QTextEdit *moveHistory;   // TextEdit to display move history
    QPushButton *readyButton; // Button to ready the clock
    QPushButton *startButton; // Button to ready the clock

    int whiteTime;          // White player's remaining time (in seconds)
    int blackTime;          // Black player's remaining time (in seconds)
    QLCDNumber *whiteClock; // White player's clock display
    QLCDNumber *blackClock; // Black player's clock display

    QTimer *gameTimer; // Timer to control the game clock

    void initializeUI(); // Method to initialize the UI elements
    void updateClocks(); // Update the clocks every second
    void updateClockDisplay();
    void showGameOverMessage(const QString &message);
    void showTimeOutMessage(bool whiteTurn);

    // Helper function to set clock styles
    void setClockStyles();
};

#endif // STATUSPANEL_H
