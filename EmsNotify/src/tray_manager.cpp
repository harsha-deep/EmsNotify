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
#include <QVBoxLayout>
#include <QLabel>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QIcon>
#include <QDateTime>

TrayManager::TrayManager(QObject* parent)
	: QObject(parent),
	networkManager(new QNetworkAccessManager(this)),
	timer(new QTimer(this)),
	remainingSeconds(0)
{
	setupUI();
	setupTray();
	checkEmployeeId();

	connect(timer, &QTimer::timeout, this, &TrayManager::updateTimer);
}

//////////////////////////////////////////////////////////////
// UI SETUP
//////////////////////////////////////////////////////////////

void TrayManager::setupUI()
{
    mainWindow = new QWidget();
    mainWindow->setWindowIcon(QIcon(":/icons/clock.ico"));
    mainWindow->setWindowTitle("EMS Notify Status");
    mainWindow->resize(400, 300);

    // Layout
    auto* layout = new QVBoxLayout(mainWindow);
    layout->setAlignment(Qt::AlignCenter);

    // Check-in Label
    checkInLabel = new QLabel("Check-in time: Loading...", mainWindow);
    checkInLabel->setAlignment(Qt::AlignCenter);

    QFont checkInFont;
    checkInFont.setPointSize(11);
    checkInLabel->setFont(checkInFont);

    // Time Label
    timeLabel = new QLabel("Initializing...", mainWindow);
    timeLabel->setAlignment(Qt::AlignCenter);

    QFont timeFont;
    timeFont.setPointSize(12);
    timeFont.setBold(true);
    timeLabel->setFont(timeFont);

    // Status Label
    statusLabel = new QLabel("Status: Working", mainWindow);
    statusLabel->setAlignment(Qt::AlignCenter);

    QFont statusFont;
    statusFont.setPointSize(12);
    statusLabel->setFont(statusFont);

    layout->addWidget(checkInLabel);
    layout->addSpacing(10);
    layout->addWidget(timeLabel);
    layout->addSpacing(10);
    layout->addWidget(statusLabel);

    mainWindow->setStyleSheet(R"(
        QWidget {
            background-color: #2B2B2B;
            color: #F0F0F0;
            font-family: "Segoe UI";
        }
    )");
}

//////////////////////////////////////////////////////////////
// TRAY SETUP
//////////////////////////////////////////////////////////////

void TrayManager::setupTray()
{
	trayIcon = new QSystemTrayIcon(QIcon(":/icons/clock.png"), this);
	trayMenu = new QMenu();

	auto* toggleAction = new QAction("Show / Hide", this);
	connect(toggleAction, &QAction::triggered, this, &TrayManager::toggleWindow);

	auto* quitAction = new QAction("Quit", this);
	connect(quitAction, &QAction::triggered, qApp, &QApplication::quit);

	trayMenu->addAction(toggleAction);
	trayMenu->addSeparator();
	trayMenu->addAction(quitAction);

	trayIcon->setContextMenu(trayMenu);
	trayIcon->setToolTip("EMS Notify: Initializing...");
	trayIcon->show();

	connect(trayIcon, &QSystemTrayIcon::activated,
		this, &TrayManager::onTrayIconActivated);
}

void TrayManager::toggleWindow()
{
	if (mainWindow->isVisible())
		mainWindow->hide();
	else {
		mainWindow->showNormal();
		mainWindow->activateWindow();
	}
}

void TrayManager::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
	if (reason == QSystemTrayIcon::Trigger ||
		reason == QSystemTrayIcon::DoubleClick)
	{
		toggleWindow();
	}
}

//////////////////////////////////////////////////////////////
// EMPLOYEE ID
//////////////////////////////////////////////////////////////

void TrayManager::checkEmployeeId()
{
	QSettings settings(QSettings::IniFormat,
		QSettings::UserScope,
		"CSG",
		"EmsNotify");

	QString employeeId = settings.value("employeeId").toString();

	if (employeeId.isEmpty()) {
		bool ok = false;

		employeeId = QInputDialog::getText(
			nullptr,
			"Employee ID",
			"Enter Employee ID:",
			QLineEdit::Normal,
			"",
			&ok
		);

		if (!ok || employeeId.trimmed().isEmpty()) {
			QMessageBox::critical(nullptr,
				"Error",
				"Employee ID required!");
			qApp->quit();
			return;
		}

		settings.setValue("employeeId", employeeId);
	}

	callCheckInTimeApi(employeeId);
}

//////////////////////////////////////////////////////////////
// API Stuff
//////////////////////////////////////////////////////////////

void TrayManager::callCheckInTimeApi(const QString& employeeId)
{
	QUrl url("https://smartcsg.karnataka.gov.in/ems/api/getCheckInTime");
	QNetworkRequest request(url);
	request.setHeader(QNetworkRequest::ContentTypeHeader,
		"application/json");

	QJsonObject payload;
	payload["employeeId"] = employeeId;

	QNetworkReply* reply =
		networkManager->post(request,
			QJsonDocument(payload).toJson());

	connect(reply, &QNetworkReply::finished, this, [=]() {

		if (reply->error() != QNetworkReply::NoError) {
			qDebug() << "API Error:" << reply->errorString();
			trayIcon->showMessage("EMS Notify",
				"Failed to fetch check-in time.",
				QSystemTrayIcon::Warning);
			reply->deleteLater();
			return;
		}

		const QString timeString =
			QString(reply->readAll()).trimmed();

		qDebug() << "Check-in Time Response:" << timeString;

		if (!parseCheckInTime(timeString)) {
			qDebug() << "Invalid check-in time format received";
		}

		reply->deleteLater();
	});
}

bool TrayManager::parseCheckInTime(const QString& timeString)
{
	const QStringList parts = timeString.split(":");
	if (parts.size() != 3)
		return false;

	int hours = parts[0].toInt();
	int minutes = parts[1].toInt();
	int seconds = parts[2].toInt();

	// Store check-in time string
	checkInTimeStr = timeString;

	// Create today's date with check-in time
	QDate today = QDate::currentDate();
	QTime checkInTime(hours, minutes, seconds);
	checkInDateTime = QDateTime(today, checkInTime);

	// Update UI
	checkInLabel->setText("Check-in time: " + checkInTimeStr);

	// Start live timer
	startTimer();
	return true;
}

//////////////////////////////////////////////////////////////
// TIMER
//////////////////////////////////////////////////////////////

void TrayManager::startTimer()
{
	timer->stop();
	calculateRemainingTime(); // Initial calculation
	timer->start(1000); // Update every second
}

void TrayManager::calculateRemainingTime()
{
	// Get current time
	QDateTime now = QDateTime::currentDateTime();

	// Calculate completion time (check-in + 9 hours)
	QDateTime completionTime = checkInDateTime.addSecs(9 * 3600);

	// Calculate remaining seconds
	remainingSeconds = now.secsTo(completionTime);

	// If negative, set to 0
	if (remainingSeconds < 0) {
		remainingSeconds = 0;
	}
}

void TrayManager::updateTimer()
{
	// Recalculate remaining time based on current time
	calculateRemainingTime();

	int hrs = remainingSeconds / 3600;
	int mins = (remainingSeconds % 3600) / 60;
	int secs = remainingSeconds % 60;

	const QString formattedTime =
		QString("%1:%2:%3")
		.arg(hrs, 2, 10, QChar('0'))
		.arg(mins, 2, 10, QChar('0'))
		.arg(secs, 2, 10, QChar('0'));

	// Update UI
	timeLabel->setText("Time remaining: " + formattedTime);
	
	// Update tooltip (live updates when hovering)
	trayIcon->setToolTip("Remaining: " + formattedTime);

	// Check if finished
	if (remainingSeconds <= 0) {
		handleFinished();
	}
}

void TrayManager::handleFinished()
{
	timer->stop();

	timeLabel->setText("Time remaining: 00:00:00");

	statusLabel->setText("Status: Complete");

	trayIcon->setToolTip("EMS Notify: Complete!");
	trayIcon->showMessage(
		"EMS Notify",
		"You have completed your 9 hours.",
		QSystemTrayIcon::Information,
		8000
	);
}