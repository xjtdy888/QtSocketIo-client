#include "qtransport.h"

QTransport::QTransport(QObject *parent) : QObject(parent)
{

}

QTransport::~QTransport()
{

}


bool QTransport::open(const QUrl &url)
{
    this->m_requestUrl = url;
    return this->doOpen();
}

bool QTransport::close()
{
    return this->doClose();
}

qint64 QTransport::sendTextMessage(const QString &message)
{
    doSendTextMessage(message);
}
