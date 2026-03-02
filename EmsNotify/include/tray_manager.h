#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QDateTime>

class QMenu;
class QMenuBar;
class QMainWindow;
class QNetworkAccessManager;
class QTimer;
class QLabel;
class Updater;

class TrayManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayManager(QObject *parent = nullptr);

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void updateTimer();

private:
    //////////////////////////////////////////////////////////
    // Setup
    //////////////////////////////////////////////////////////
    void setupUI();
    void setupTray();
    void setupMenuBar();

    //////////////////////////////////////////////////////////
    // Dialogs
    //////////////////////////////////////////////////////////
    void openSettings();
    void openAbout();

    //////////////////////////////////////////////////////////
    // Business Logic
    //////////////////////////////////////////////////////////
    void checkEmployeeId();
    void callCheckInTimeApi(const QString &employeeId);
    bool parseCheckInTime(const QString &timeString);
    void calculateRemainingTime();

    //////////////////////////////////////////////////////////
    // Timer
    //////////////////////////////////////////////////////////
    void startTimer();
    void handleFinished();
    void toggleWindow();

private:
    //////////////////////////////////////////////////////////
    // Members
    //////////////////////////////////////////////////////////
    QSystemTrayIcon *trayIcon = nullptr;
    QMenu *trayMenu = nullptr;
    QNetworkAccessManager *networkManager = nullptr;
    QTimer *timer = nullptr;
    Updater *updater = nullptr;

    QMainWindow *mainWindow = nullptr;
    QLabel *timeLabel = nullptr;
    QLabel *checkInLabel = nullptr;
    QLabel *statusLabel = nullptr;

    int remainingSeconds = 0;
    QDateTime checkInDateTime;
    QString checkInTimeStr;
};