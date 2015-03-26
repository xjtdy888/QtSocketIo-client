#include "qtransportwebsocket.h"
#include <QWebSocket>
QTransportWebSocket::QTransportWebSocket(QObject *parent):
    QTransport(parent),
    m_pWebSocket(new QWebSocket())
{
     connect(m_pWebSocket, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onError(QAbstractSocket::SocketError)));
     connect(m_pWebSocket, SIGNAL(textMessageReceived(QString)),
             this, SLOT(onMessage(QString)));
}

QTransportWebSocket::~QTransportWebSocket()
{
    delete m_pWebSocket;
}

bool QTransportWebSocket::doOpen()
{
    m_pWebSocket->open(m_requestUrl);
    return true;
}

bool QTransportWebSocket::doClose()
{
    m_pWebSocket->close();
    return true;
}

qint64 QTransportWebSocket::doSendTextMessage(const QString &message)
{
    qDebug()<< "QTransportWebSocket postMessage: " << message;
    return m_pWebSocket->sendTextMessage(message);
}

void QTransportWebSocket::onMessage(QString textMessage)
{
    qDebug()<< "QTransportWebSocket onMessage: " << textMessage;
    emit textMessageReceived(textMessage);
}

void QTransportWebSocket::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "Error occurred: " << error;
}
