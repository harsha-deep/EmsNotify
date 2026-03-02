#include "auto_updater.h"
#include "config.h"

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QDesktopServices>
#include <QUrl>
#include <QDebug>

Updater::Updater(QObject *parent)
    : QObject(parent),
      networkManager(new QNetworkAccessManager(this))
{
    connect(networkManager, &QNetworkAccessManager::finished,
            this, &Updater::onReplyFinished);
}

void Updater::checkForUpdates(bool silent)
{
    m_silent = silent;

    QNetworkRequest request(QUrl(QString(UPDATE_CHECK_URL)));
    // GitHub API requires a User-Agent header
    request.setRawHeader("User-Agent", "EmsNotify-Updater");
    request.setRawHeader("Accept", "application/vnd.github+json");

    networkManager->get(request);
    qDebug() << "Checking for updates at:" << UPDATE_CHECK_URL;
}

void Updater::onReplyFinished(QNetworkReply *reply)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError)
    {
        qDebug() << "Update check failed:" << reply->errorString();
        if (!m_silent)
        {
            QMessageBox::warning(nullptr, "Update Check Failed",
                                 "Could not reach update server:\n" + reply->errorString());
        }
        return;
    }

    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
    if (doc.isNull() || !doc.isObject())
    {
        qDebug() << "Invalid JSON in update response";
        return;
    }

    const QJsonObject root = doc.object();
    const QString remoteTag = root.value("tag_name").toString(); // e.g. "v1.0.2"
    const QString releaseUrl = root.value("html_url").toString();

    // Strip leading 'v' if present
    const QString remoteVersion = remoteTag.startsWith('v')
                                      ? remoteTag.mid(1)
                                      : remoteTag;
    const QString localVersion = QString(APP_VERSION);

    qDebug() << "Local version:" << localVersion
             << " Remote version:" << remoteVersion;

    if (isNewerVersion(remoteVersion, localVersion))
    {
        const auto btn = QMessageBox::information(
            nullptr,
            "Update Available",
            QString("A new version is available: %1\n"
                    "You are on: %2\n\n"
                    "Open the releases page?")
                .arg(remoteVersion, localVersion),
            QMessageBox::Yes | QMessageBox::No);

        if (btn == QMessageBox::Yes)
            QDesktopServices::openUrl(QUrl(releaseUrl));
    }
    else if (!m_silent)
    {
        QMessageBox::information(nullptr, "No Updates",
                                 "You are already on the latest version (" + localVersion + ").");
    }
}

bool Updater::isNewerVersion(const QString &remote, const QString &local)
{
    // Compare semantic versions: MAJOR.MINOR.PATCH
    const auto split = [](const QString &v)
    {
        QList<int> parts;
        for (const QString &p : v.split('.'))
            parts << p.toInt();
        while (parts.size() < 3)
            parts << 0;
        return parts;
    };

    const QList<int> r = split(remote);
    const QList<int> l = split(local);

    for (int i = 0; i < 3; ++i)
    {
        if (r[i] > l[i])
            return true;
        if (r[i] < l[i])
            return false;
    }
    return false;
}