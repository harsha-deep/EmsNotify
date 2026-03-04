#pragma once

#include <QObject>
#include <QUrl>

class QNetworkAccessManager;
class QWidget;

class AutoUpdater : public QObject
{
    Q_OBJECT

public:
    explicit AutoUpdater(QNetworkAccessManager *nam, QObject *parent = nullptr);
    void setParentWidget(QWidget *widget);
    void checkForUpdates(bool silent = false);

private:
    void onLatestReleaseReady(const QByteArray &data, bool silent);
    void downloadAndInstall(const QUrl &url, const QString &version);
    static int compareVersions(const QString &a, const QString &b);

    QNetworkAccessManager *m_nam;
    QWidget *m_parentWidget = nullptr;
};
