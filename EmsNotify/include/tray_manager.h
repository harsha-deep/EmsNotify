#pragma once

#include <QObject>
#include <QSystemTrayIcon>

class QMenu;
class QNetworkAccessManager;
class QTimer;
class QWidget;
class QLabel;

class TrayManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayManager(QObject* parent = nullptr);

private slots:
    void onTrayIconActivated(QSystemTrayIcon::ActivationReason reason);
    void updateTimer();

private:
    //////////////////////////////////////////////////////////
    // Setup
    //////////////////////////////////////////////////////////
    void setupUI();
    void setupTray();

    //////////////////////////////////////////////////////////
    // Business Logic
    //////////////////////////////////////////////////////////
    void checkEmployeeId();
    void callApi(const QString& employeeId);
    bool parseTimeString(const QString& timeString);

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
    QSystemTrayIcon*        trayIcon            = nullptr;
    QMenu*                  trayMenu            = nullptr;
    QNetworkAccessManager*  networkManager      = nullptr;
    QTimer*                 timer               = nullptr;

    QWidget*                mainWindow          = nullptr;
    QLabel*                 timeLabel           = nullptr;
    QLabel*                 statusLabel         = nullptr;

    int                     remainingSeconds    = 0;
};