#ifndef QTRANSPORT_H
#define QTRANSPORT_H

#include <QObject>
#include <QUrl>
class QTransport : public QObject
{
    Q_OBJECT
public:
    explicit QTransport(QObject *parent = 0);
    ~QTransport();

    qint64 sendTextMessage(const QString &message);
    bool open(const QUrl &url);
    bool close();
signals:
    void textMessageReceived(QString);
    void errorReceived(QString reason, QString advice);

protected:
    virtual bool doOpen() =0;
    virtual bool doClose()=0;
    virtual qint64 doSendTextMessage(const QString &message) =0;
    QUrl m_requestUrl;


};

#endif // QTRANSPORT_H
