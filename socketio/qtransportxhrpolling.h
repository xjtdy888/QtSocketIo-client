#ifndef QTRANSPORTXHRPOLLING_H
#define QTRANSPORTXHRPOLLING_H
#include "qtransport.h"
#include <QNetworkReply>
#include <QLinkedList>
class QNetworkAccessManager;
class QObject;
class QTransportXhrPolling : public QTransport
{

Q_OBJECT
public:
    QTransportXhrPolling(QObject *parent=0);
    ~QTransportXhrPolling();

protected:
    virtual bool doOpen();
    virtual bool doClose();
    bool doPolling();
    QNetworkAccessManager *m_pNetworkAccessManager;

private slots:
    void replyFinished(QNetworkReply *reply);
    void networkError(QNetworkReply::NetworkError code);

private:
    //要发送的数据队列
    enum request_type{
        POLLING_GET_REQUEST,
        POLLING_POST_REQUEST
    };

    QLinkedList<QString> sendQueues;
    bool posting;   //当前是否正在POST数据
    bool opened;    //是否打开状态 只有打开状态才进行GET和POST

    //禁止对象复制
    QTransportXhrPolling(const QTransportXhrPolling &t);

    qint64 doPost();
    virtual qint64 doSendTextMessage(const QString &message);

    void pollingFinished(QNetworkReply *reply);
    void postFinished(QNetworkReply *reply);
};

#endif // QTRANSPORTXHRPOLLING_H
