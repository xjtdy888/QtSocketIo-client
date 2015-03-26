#include "qtransportxhrpolling.h"
#include <QNetworkAccessManager>
#include <QUrl>
#include <QDateTime>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QVariant>

QTransportXhrPolling::QTransportXhrPolling(QObject *parent):
    QTransport(parent),
    m_pNetworkAccessManager(new QNetworkAccessManager()),
    sendQueues(),
    posting(false),
    opened(false)
{
    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));
}

QTransportXhrPolling::~QTransportXhrPolling()
{
    delete m_pNetworkAccessManager;
}

bool QTransportXhrPolling::doOpen()
{
    opened = true;
    return doPolling();
}
bool QTransportXhrPolling::doClose()
{
    opened = false;
    sendQueues.clear(); //清除所有未发数据
    return true;
}


bool QTransportXhrPolling::doPolling()
{
    QUrl url(QStringLiteral("%1&t=%2")
                    .arg(m_requestUrl.toString())
                    .arg(QString::number(QDateTime::currentMSecsSinceEpoch())));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html"));
    request.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("*/*"));
    request.setRawHeader(QByteArrayLiteral("Connection"), QByteArrayLiteral("close"));

    request.setAttribute(QNetworkRequest::User, QVariant(POLLING_GET_REQUEST));

    QNetworkReply *reply = m_pNetworkAccessManager->get(request);

    //connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),
     //       this, SLOT(networkError(QNetworkReply::NetworkError)));
    return true;

}

qint64 QTransportXhrPolling::doSendTextMessage(const QString &message)
{
    sendQueues.push_back(message);
    if (posting == false) {
        return doPost();
    }
    return 0;
}

qint64 QTransportXhrPolling::doPost()
{
    if (sendQueues.size() == 0) {
        posting = false;
        return 0;
    }

    QUrl url(QStringLiteral("%1&t=%2")
                    .arg(m_requestUrl.toString())
                    .arg(QString::number(QDateTime::currentMSecsSinceEpoch())));

    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html"));
    request.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("*/*"));
    request.setRawHeader(QByteArrayLiteral("Connection"), QByteArrayLiteral("close"));
    request.setAttribute(QNetworkRequest::User, QVariant(POLLING_POST_REQUEST));

    QString msg = sendQueues.front();

    QByteArray postData;
    postData.append(msg);
    qDebug() << "XHRPolling doPost" << msg;
    posting = true;
    m_pNetworkAccessManager->post(request, postData);
    return msg.length();
}

void QTransportXhrPolling::replyFinished(QNetworkReply *reply)
{
    QNetworkRequest request = reply->request();
    request_type type = (request_type)request.attribute(QNetworkRequest::User).toInt();
    switch (type) {
        case POLLING_GET_REQUEST:
        {
            pollingFinished(reply);
            break;
        }
        case POLLING_POST_REQUEST:
        {
            postFinished(reply);
            break;
        }
    }
    reply->deleteLater();
}

void QTransportXhrPolling::pollingFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<< "XHRPolling pollingFinished() status=" << status;
    switch (status)
    {
        case 200:
        {
            //everything allright
            QString payload = QString::fromUtf8(reply->readAll());
            qDebug()<< "XHRPolling payload: " << payload;
            emit textMessageReceived(payload);
            if (opened) {
                doPolling();
            }
            break;
        }

        case 401:	//unauthorized
        {
            //the server refuses to authorize the client to connect,
            //based on the supplied information (eg: Cookie header or custom query components).
            qDebug() << "Error:" << reply->readAll();
            break;
        }

        case 500:	//internal server error
        {
            qDebug() << "Error:" << reply->readAll();
            break;
        }

        case 404:	//Not Found
        {
            qDebug() << "Error: " << reply->readAll();
        }

        case 503:	//service unavailable
        {
            //the server refuses the connection for any reason (e.g. overload)
            qDebug() << "Error:" << reply->readAll();
            break;
        }

        default:
        {
        }
    }
}

void QTransportXhrPolling::postFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug()<< "XHRPolling postFinished() status=" << status;
    switch (status)
    {
        case 200:
        {
            sendQueues.pop_front();
            if (opened) {
                doPost();
            }
            break;
        }
    }
}


void QTransportXhrPolling::networkError(QNetworkReply::NetworkError code)
{
    qDebug() << "QTransportXhrPolling::networkError code =" << code;

}
