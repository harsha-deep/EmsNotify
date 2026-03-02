#pragma once

#include <QObject>
#include <QString>

class QNetworkAccessManager;
class QNetworkReply;

class Updater : public QObject
{
    Q_OBJECT

public:
    explicit Updater(QObject *parent = nullptr);

    // Call this to trigger an update check
    void checkForUpdates(bool silent = true);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    bool isNewerVersion(const QString &remote, const QString &local);

    QNetworkAccessManager *networkManager = nullptr;
    bool m_silent = true;
};