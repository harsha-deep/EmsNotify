#include "tray_manager.h"

#include <QApplication>
#include <QAction>
#include <QInputDialog>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrl>
#include <QMessageBox>

#include <iostream>

TrayManager::TrayManager(QObject* parent)
    : QObject(parent)
{
    networkManager = new QNetworkAccessManager(this);
    timer = new QTimer(this);

    setupTray();
    checkEmployeeId();
}

void TrayManager::setupTray()
{
    //trayIcon = new QSystemTrayIcon(QIcon::fromTheme("clock"), this);
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/clock.png"), this);

	std::cout << trayIcon->geometry().width() << "x" << trayIcon->geometry().height() << std::endl;

    trayMenu = new QMenu();

    QAction* quitAction = new QAction("Quit");
    connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

    trayMenu->addAction(quitAction);

    trayIcon->setContextMenu(trayMenu);
    trayIcon->setToolTip("EmsNotify Running");
    trayIcon->show();
}

void TrayManager::checkEmployeeId()
{
    QSettings settings("CSG", "EmsNotify");

    QString employeeId = settings.value("employeeId").toString();

    if (employeeId.isEmpty()) {
        bool ok;
        employeeId = QInputDialog::getText(nullptr,
            "Employee ID",
            "Enter Employee ID:",
            QLineEdit::Normal,
            "",
            &ok);

        if (!ok || employeeId.isEmpty()) {
            QMessageBox::critical(nullptr, "Error", "Employee ID required!");
            qApp->quit();
            return;
        }

        settings.setValue("employeeId", employeeId);
    }

    callApi(employeeId);
}

void TrayManager::callApi(const QString& employeeId)
{
    QUrl url("https://smartcsg.karnataka.gov.in/ems/api/getElapsedTime");

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["employeeId"] = employeeId;

    QNetworkReply* reply =
        networkManager->post(request, QJsonDocument(json).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]() {
        if (reply->error() != QNetworkReply::NoError) {
            trayIcon->showMessage("EmsNotify",
                "Failed to fetch time from server.",
                QSystemTrayIcon::Warning,
                5000);
            reply->deleteLater();
            return;
        }

        QByteArray response = reply->readAll();
        QString timeString = QString(response).trimmed();

        QStringList parts = timeString.split(":");
        if (parts.size() == 3) {
            int hours = parts[0].toInt();
            int minutes = parts[1].toInt();
            int seconds = parts[2].toInt();

            totalSeconds = hours * 3600 + minutes * 60 + seconds;
            startTimer();
        }

        reply->deleteLater();
        });
}

void TrayManager::startTimer()
{
    connect(timer, &QTimer::timeout, this, [=]() mutable {

        totalSeconds++;

        int remaining = (9 * 3600) - totalSeconds;

        if (remaining <= 0) {
            trayIcon->showMessage("EmsNotify",
                "You are done for the day.",
                QSystemTrayIcon::Information,
                8000);
            timer->stop();
            return;
        }

        int hrs = remaining / 3600;
        int mins = (remaining % 3600) / 60;

        trayIcon->setToolTip(
            QString("Remaining: %1h %2m")
            .arg(hrs)
            .arg(mins)
        );
        });

    timer->start(1000);
}