#include "auto_updater.h"

#include <QCoreApplication>
#include <QDesktopServices>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QFileDevice>
#include <QIcon>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QMessageBox>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QProcess>
#include <QPushButton>
#include <QStandardPaths>
#include <QUrl>
#include <QVBoxLayout>

static const char *kApiUrl =
    "https://api.github.com/repos/harsha-deep/EmsNotify/releases/latest";
static const char *kReleasesUrl =
    "https://github.com/harsha-deep/EmsNotify/releases";

AutoUpdater::AutoUpdater(QNetworkAccessManager *nam, QObject *parent)
    : QObject(parent), m_nam(nam)
{
}

void AutoUpdater::setParentWidget(QWidget *widget)
{
    m_parentWidget = widget;
}

void AutoUpdater::checkForUpdates(bool silent)
{
    QNetworkRequest req{QUrl(kApiUrl)};
    req.setRawHeader("Accept", "application/vnd.github+json");
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, silent]() {
        if (reply->error() != QNetworkReply::NoError) {
            if (!silent)
                QMessageBox::warning(m_parentWidget, "Update Check Failed",
                                     "Could not reach GitHub:\n" + reply->errorString());
            reply->deleteLater();
            return;
        }
        onLatestReleaseReady(reply->readAll(), silent);
        reply->deleteLater();
    });
}

// Returns > 0 if a is newer than b, 0 if equal, < 0 if older.
// Strips a leading 'v' from either string before comparing.
int AutoUpdater::compareVersions(const QString &a, const QString &b)
{
    auto strip = [](const QString &s) {
        return s.startsWith('v') ? s.mid(1) : s;
    };
    const QStringList pa = strip(a).split('.');
    const QStringList pb = strip(b).split('.');
    const int len = qMax(pa.size(), pb.size());
    for (int i = 0; i < len; ++i) {
        const int va = i < pa.size() ? pa[i].toInt() : 0;
        const int vb = i < pb.size() ? pb[i].toInt() : 0;
        if (va != vb)
            return va - vb;
    }
    return 0;
}

void AutoUpdater::onLatestReleaseReady(const QByteArray &data, bool silent)
{
    const QJsonDocument doc = QJsonDocument::fromJson(data);
    if (!doc.isObject()) {
        if (!silent)
            QMessageBox::warning(m_parentWidget, "Update Check Failed",
                                 "Unexpected response from GitHub.");
        return;
    }

    const QJsonObject root = doc.object();
    const QString tagName = root["tag_name"].toString();
    const QString currentVersion = QCoreApplication::applicationVersion();

    if (compareVersions(tagName, currentVersion) <= 0) {
        if (!silent)
            QMessageBox::information(m_parentWidget, "Up To Date",
                                     "You are on the latest version (" + currentVersion + ").");
        return;
    }

    // Find the platform-specific download asset
    QUrl downloadUrl;
    const QJsonArray assets = root["assets"].toArray();
    for (const QJsonValue &val : assets) {
        const QJsonObject asset = val.toObject();
        const QString name = asset["name"].toString();
#ifdef Q_OS_WIN
        if (name.endsWith(".exe", Qt::CaseInsensitive)) {
            downloadUrl = QUrl(asset["browser_download_url"].toString());
            break;
        }
#elif defined(Q_OS_LINUX)
        if (name.endsWith(".AppImage")) {
            downloadUrl = QUrl(asset["browser_download_url"].toString());
            break;
        }
#endif
    }

    QDialog dialog(m_parentWidget);
    dialog.setWindowTitle("Update Available");
    dialog.setWindowIcon(QIcon(":/icons/clock.ico"));
    dialog.setMinimumWidth(360);
    dialog.setStyleSheet(R"(
        QWidget     { background-color: #2B2B2B; color: #F0F0F0; font-family: "Segoe UI"; }
        QPushButton { background-color: #3C3C3C; color: #F0F0F0; border: 1px solid #555;
                      border-radius: 3px; padding: 5px 14px; }
        QPushButton:hover { background-color: #505050; }
    )");

    auto *layout = new QVBoxLayout(&dialog);
    layout->setContentsMargins(20, 20, 20, 16);
    layout->setSpacing(10);

    auto *heading = new QLabel("A new version is available!", &dialog);
    QFont f;
    f.setBold(true);
    f.setPointSize(11);
    heading->setFont(f);
    heading->setAlignment(Qt::AlignCenter);

    auto *info = new QLabel(
        QString("Current: %1    \u2192    Latest: %2").arg(currentVersion, tagName),
        &dialog);
    info->setAlignment(Qt::AlignCenter);

    auto *downloadBtn = new QPushButton(
        downloadUrl.isValid() ? "Download && Install" : "View Releases", &dialog);
    auto *laterBtn = new QPushButton("Later", &dialog);

    layout->addWidget(heading);
    layout->addWidget(info);
    layout->addSpacing(6);
    layout->addWidget(downloadBtn);
    layout->addWidget(laterBtn);

    connect(laterBtn, &QPushButton::clicked, &dialog, &QDialog::reject);

    if (downloadUrl.isValid()) {
        connect(downloadBtn, &QPushButton::clicked, &dialog, [&, downloadUrl, tagName]() {
            dialog.accept();
            downloadAndInstall(downloadUrl, tagName);
        });
    } else {
        connect(downloadBtn, &QPushButton::clicked, &dialog, [&]() {
            QDesktopServices::openUrl(QUrl(kReleasesUrl));
            dialog.accept();
        });
    }

    dialog.exec();
}

void AutoUpdater::downloadAndInstall(const QUrl &url, const QString &version)
{
    QNetworkRequest req{url};
    req.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                     QNetworkRequest::NoLessSafeRedirectPolicy);

    auto *statusBox = new QMessageBox(
        QMessageBox::Information,
        "Downloading Update",
        "Downloading version " + version + "...\nPlease wait.",
        QMessageBox::NoButton,
        m_parentWidget);
    statusBox->setAttribute(Qt::WA_DeleteOnClose);
    statusBox->show();

    QNetworkReply *reply = m_nam->get(req);
    connect(reply, &QNetworkReply::finished, this, [this, reply, statusBox, version]() {
        statusBox->close();

        if (reply->error() != QNetworkReply::NoError) {
            QMessageBox::warning(m_parentWidget, "Download Failed",
                                 reply->errorString() + "\n\nOpening releases page instead.");
            QDesktopServices::openUrl(QUrl(kReleasesUrl));
            reply->deleteLater();
            return;
        }

        const QByteArray data = reply->readAll();
        reply->deleteLater();

#ifdef Q_OS_WIN
        const QString path =
            QDir::temp().filePath("EmsNotify_Update_" + version + ".exe");
#elif defined(Q_OS_LINUX)
        const QString path =
            QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) +
            "/EmsNotify_" + version + ".AppImage";
#else
        QDesktopServices::openUrl(QUrl(kReleasesUrl));
        return;
#endif

        QFile file(path);
        if (!file.open(QIODevice::WriteOnly)) {
            QMessageBox::critical(m_parentWidget, "Error",
                                  "Could not save update to:\n" + path);
            return;
        }
        file.write(data);
        file.close();

#ifdef Q_OS_WIN
        if (!QProcess::startDetached(path, {})) {
            QMessageBox::critical(m_parentWidget, "Error",
                                  "Could not launch installer:\n" + path);
            return;
        }
        QCoreApplication::quit();
#elif defined(Q_OS_LINUX)
        file.setPermissions(QFileDevice::ReadOwner  | QFileDevice::WriteOwner |
                            QFileDevice::ExeOwner   | QFileDevice::ReadGroup  |
                            QFileDevice::ExeGroup   | QFileDevice::ReadOther  |
                            QFileDevice::ExeOther);
        QMessageBox::information(
            m_parentWidget, "Download Complete",
            "Update saved to:\n" + path +
            "\n\nClose EMS Notify and run the new AppImage to complete the update.");
#endif
    });
}
