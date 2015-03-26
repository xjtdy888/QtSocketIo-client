#ifndef QTRANSPORTWEBSOCKET_H
#define QTRANSPORTWEBSOCKET_H
#include "qtransport.h"
#include <QWebSocket>

class QTransportWebSocket : public QTransport
{
    Q_OBJECT
public:
    QTransportWebSocket(QObject *parent=0);
    ~QTransportWebSocket();

protected:
    virtual bool doOpen();
    virtual bool doClose();

private:
    QWebSocket *m_pWebSocket;

    QTransportWebSocket(const QTransportWebSocket &t);

    virtual qint64 doSendTextMessage(const QString &message);

private Q_SLOTS:
    void onError(QAbstractSocket::SocketError error);
    void onMessage(QString textMessage);
};

#endif // QTRANSPORTWEBSOCKET_H
