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
    void toggleStartup();
    bool isStartupEnabled() const;

    //////////////////////////////////////////////////////////
    // Business Logic
    //////////////////////////////////////////////////////////
    void checkEmployeeId();
    void callApi(const QString &employeeId);
    void callCheckInTimeApi(const QString &employeeId);
    void callAttendanceApi(const QString &employeeId, const QString &modeId);
    bool parseTimeString(const QString &timeString);
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

    QMainWindow *mainWindow = nullptr;
    QLabel *timeLabel = nullptr;
    QLabel *checkInLabel = nullptr;
    QLabel *weekLabel = nullptr;

    int remainingSeconds = 0;
    QDateTime checkInDateTime;
    QString checkInTimeStr;
    QString checkInTime; // New member variable
};