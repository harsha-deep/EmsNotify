#pragma once

#include <QObject>
#include <QSystemTrayIcon>
#include <QMenu>
#include <QNetworkAccessManager>
#include <QTimer>

class TrayManager : public QObject
{
    Q_OBJECT

public:
    explicit TrayManager(QObject* parent = nullptr);

private:
    QSystemTrayIcon* trayIcon;
    QMenu* trayMenu;
    QNetworkAccessManager* networkManager;
    QTimer* timer;

    int totalSeconds = 0;

    void setupTray();
    void checkEmployeeId();
    void callApi(const QString& employeeId);
    void startTimer();
};