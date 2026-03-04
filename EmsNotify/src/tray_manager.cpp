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
#include <QHBoxLayout>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QDialog>
#include <QDesktopServices>
#include <QDebug>
#include <QTimer>
#include <QMenu>
#include <QMenuBar>
#include <QMainWindow>
#include <QIcon>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QTextStream>

TrayManager::TrayManager(QObject *parent)
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
    mainWindow = new QMainWindow();
    mainWindow->setWindowIcon(QIcon(":/icons/clock.ico"));
    mainWindow->setWindowTitle("EMS Notify Status");
    mainWindow->resize(400, 300);

    auto *centralWidget = new QWidget();
    mainWindow->setCentralWidget(centralWidget);

    // Layout
    auto *layout = new QVBoxLayout(centralWidget);
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
        QMenuBar {
            background-color: #1E1E1E;
            color: #F0F0F0;
        }
        QMenuBar::item:selected {
            background-color: #3C3C3C;
        }
        QMenu {
            background-color: #2B2B2B;
            color: #F0F0F0;
            border: 1px solid #555;
        }
        QMenu::item:selected {
            background-color: #3C3C3C;
        }
    )");

    setupMenuBar();
}

//////////////////////////////////////////////////////////////
// TRAY SETUP
//////////////////////////////////////////////////////////////

void TrayManager::setupTray()
{
    trayIcon = new QSystemTrayIcon(QIcon(":/icons/clock.png"), this);
    trayMenu = new QMenu();

    auto *toggleAction = new QAction("Show / Hide", this);
    connect(toggleAction, &QAction::triggered, this, &TrayManager::toggleWindow);

    auto *quitAction = new QAction("Quit", this);
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

//////////////////////////////////////////////////////////////
// MENU BAR
//////////////////////////////////////////////////////////////

void TrayManager::setupMenuBar()
{
    QMenuBar *menuBar = mainWindow->menuBar();

    // Edit menu
    auto *editMenu = menuBar->addMenu("&Edit");
    auto *settingsAction = new QAction("&Settings", this);
    connect(settingsAction, &QAction::triggered, this, &TrayManager::openSettings);
    editMenu->addAction(settingsAction);

    editMenu->addSeparator();

    auto *startupAction = new QAction("&Run at Startup", this);
    startupAction->setCheckable(true);
    startupAction->setChecked(isStartupEnabled());
    connect(startupAction, &QAction::triggered, this, &TrayManager::toggleStartup);
    editMenu->addAction(startupAction);

    // About menu
    auto *aboutMenu = menuBar->addMenu("&About");
    auto *aboutAction = new QAction("&About EMS Notify", this);
    connect(aboutAction, &QAction::triggered, this, &TrayManager::openAbout);
    aboutMenu->addAction(aboutAction);
}

void TrayManager::openSettings()
{
    QSettings settings(QSettings::IniFormat,
                       QSettings::UserScope, "CSG", "EmsNotify");

    QDialog dialog(mainWindow);
    dialog.setWindowTitle("Settings");
    dialog.setWindowIcon(QIcon(":/icons/clock.ico"));
    dialog.setMinimumWidth(320);
    dialog.setStyleSheet(R"(
        QWidget  { background-color: #2B2B2B; color: #F0F0F0; font-family: "Segoe UI"; }
        QLineEdit { background-color: #3C3C3C; color: #F0F0F0; border: 1px solid #555;
                    border-radius: 3px; padding: 4px; }
        QPushButton { background-color: #3C3C3C; color: #F0F0F0; border: 1px solid #555;
                      border-radius: 3px; padding: 5px 14px; }
        QPushButton:hover { background-color: #505050; }
    )");

    auto *form = new QFormLayout(&dialog);
    form->setContentsMargins(16, 16, 16, 16);
    form->setSpacing(10);

    auto *employeeIdEdit = new QLineEdit(settings.value("employeeId").toString(), &dialog);
    auto *modeIdEdit = new QLineEdit(settings.value("modeId").toString(), &dialog);

    form->addRow("Employee ID:", employeeIdEdit);
    form->addRow("Mode ID:", modeIdEdit);

    auto *buttons = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, &dialog);
    form->addRow(buttons);

    connect(buttons, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted)
    {
        settings.setValue("employeeId", employeeIdEdit->text().trimmed());
        settings.setValue("modeId", modeIdEdit->text().trimmed());
    }
}

void TrayManager::openAbout()
{
    QDialog dialog(mainWindow);
    dialog.setWindowTitle("About EMS Notify");
    dialog.setWindowIcon(QIcon(":/icons/clock.ico"));
    dialog.setMinimumWidth(360);
    dialog.setStyleSheet(R"(
        QWidget    { background-color: #2B2B2B; color: #F0F0F0; font-family: "Segoe UI"; }
        QLabel     { color: #F0F0F0; }
        QLabel[hyperlink="true"] { color: #6EA8FE; }
        QPushButton { background-color: #3C3C3C; color: #F0F0F0; border: 1px solid #555;
                      border-radius: 3px; padding: 5px 14px; }
        QPushButton:hover { background-color: #505050; }
    )");

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 20);
    layout->setSpacing(8);

    auto *titleLabel = new QLabel("EMS Notify", &dialog);
    QFont titleFont;
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    titleLabel->setAlignment(Qt::AlignCenter);

    auto *versionLabel = new QLabel(
        "Version " + QCoreApplication::applicationVersion(), &dialog);
    versionLabel->setAlignment(Qt::AlignCenter);

    auto *builtWithLabel = new QLabel("Built with C++ and Qt", &dialog);
    builtWithLabel->setAlignment(Qt::AlignCenter);

    auto *repoLabel = new QLabel(
        "<a href=\"https://github.com/harsha-deep/EmsNotify\" "
        "style=\"color:#6EA8FE;\">github.com/harsha-deep/EmsNotify</a>",
        &dialog);
    repoLabel->setAlignment(Qt::AlignCenter);
    repoLabel->setOpenExternalLinks(true);
    repoLabel->setTextFormat(Qt::RichText);

    auto *checkUpdatesBtn = new QPushButton("Check for Updates", &dialog);
    connect(checkUpdatesBtn, &QPushButton::clicked, [&]()
            { QDesktopServices::openUrl(
                  QUrl("https://github.com/harsha-deep/EmsNotify/releases")); });

    auto *closeBtn = new QPushButton("Close", &dialog);
    connect(closeBtn, &QPushButton::clicked, &dialog, &QDialog::accept);

    layout->addWidget(titleLabel);
    layout->addWidget(versionLabel);
    layout->addSpacing(6);
    layout->addWidget(builtWithLabel);
    layout->addWidget(repoLabel);
    layout->addSpacing(10);
    layout->addWidget(checkUpdatesBtn);
    layout->addWidget(closeBtn);

    dialog.exec();
}

void TrayManager::toggleWindow()
{
    if (mainWindow->isVisible())
        mainWindow->hide();
    else
    {
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

    if (employeeId.isEmpty())
    {
        bool ok = false;

        employeeId = QInputDialog::getText(
            nullptr,
            "Employee ID",
            "Enter Employee ID:",
            QLineEdit::Normal,
            "",
            &ok);

        if (!ok || employeeId.trimmed().isEmpty())
        {
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

void TrayManager::callCheckInTimeApi(const QString &employeeId)
{
    QUrl url("https://smartcsg.karnataka.gov.in/ems/api/getCheckInTime");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,
                      "application/json");

    QJsonObject payload;
    payload["employeeId"] = employeeId;

    QNetworkReply *reply =
        networkManager->post(request,
                             QJsonDocument(payload).toJson());

    connect(reply, &QNetworkReply::finished, this, [=]()
            {

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

		reply->deleteLater(); });
}

bool TrayManager::parseCheckInTime(const QString &timeString)
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
    timer->start(1000);       // Update every second
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
    if (remainingSeconds < 0)
    {
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
    if (remainingSeconds <= 0)
    {
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
        8000);
}

//////////////////////////////////////////////////////////////
// TIMER
//////////////////////////////////////////////////////////////

bool TrayManager::isStartupEnabled() const
{
#ifdef Q_OS_WIN
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    return reg.contains("EmsNotify");
#else
    QString autostartFile = QDir::homePath() + "/.config/autostart/emsnotify.desktop";
    return QFile::exists(autostartFile);
#endif
}

void TrayManager::toggleStartup()
{
#ifdef Q_OS_WIN
    QSettings reg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Run",
                  QSettings::NativeFormat);
    if (isStartupEnabled())
        reg.remove("EmsNotify");
    else
        reg.setValue("EmsNotify", QCoreApplication::applicationFilePath().replace('/', '\\'));
#else
    QString autostartDir  = QDir::homePath() + "/.config/autostart";
    QString autostartFile = autostartDir + "/emsnotify.desktop";

    if (isStartupEnabled()) {
        QFile::remove(autostartFile);
    } else {
        QDir().mkpath(autostartDir);
        QFile f(autostartFile);
        if (f.open(QIODevice::WriteOnly | QIODevice::Text)) {
            QTextStream s(&f);
            s << "[Desktop Entry]\n"
              << "Type=Application\n"
              << "Name=EMS Notify\n"
              << "Exec=" << QCoreApplication::applicationFilePath() << "\n"
              << "Icon=emsnotify\n"
              << "Terminal=false\n"
              << "X-GNOME-Autostart-enabled=true\n";
        }
    }
#endif
}