#include "qsocketioclient.h"
#include <QtWebSockets/QWebSocket>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QtCore/QString>
#include <QtCore/QStringList>
#include <QtCore/QDateTime>
#include <QtCore/QTimer>
#include <QtCore/QRegExp>
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>
#include <QtCore/QDebug>
#include <functional>
#include "qtransportwebsocket.h"
#include "qtransportxhrpolling.h"

QSocketIoClient::QSocketIoClient(QObject *parent) :
    QObject(parent),
    m_pTransport(NULL),
    m_pNetworkAccessManager(new QNetworkAccessManager()),
    m_requestUrl(),
    m_namespace("socket.io"),
    m_connectionTimeout(30000),
    m_heartBeatTimeout(20000),
    m_pHeartBeatTimer(new QTimer()),
    m_sessionId("")
{
    m_pHeartBeatTimer->setInterval(m_heartBeatTimeout);

   // connect(m_pTransport, SIGNAL(error(QAbstractSocket::SocketError)),
   //        this, SLOT(onError(QAbstractSocket::SocketError)));
   // connect(m_pTransport, SIGNAL(textMessageReceived(QString)),
   //         this, SLOT(onMessage(QString)));


    connect(m_pNetworkAccessManager, SIGNAL(finished(QNetworkReply*)),
            this, SLOT(replyFinished(QNetworkReply*)));

    connect(m_pHeartBeatTimer, SIGNAL(timeout()), this, SLOT(sendHeartBeat()));
}

QSocketIoClient::~QSocketIoClient()
{
    m_pHeartBeatTimer->stop();
    delete m_pHeartBeatTimer;
    delete m_pTransport;
    delete m_pNetworkAccessManager;
}

bool QSocketIoClient::open(const QUrl &url, const QString &ns)
{
    m_requestUrl = url;
    m_namespace = ns;
    QUrl requestUrl(QStringLiteral("http://%1:%2/%3/1/?t=%4")
                    .arg(url.host())
                    .arg(QString::number(url.port(80)))
                    .arg(ns)
                    .arg(QString::number(QDateTime::currentMSecsSinceEpoch())));
    QNetworkRequest request(requestUrl);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("text/html"));
    request.setRawHeader(QByteArrayLiteral("Accept"), QByteArrayLiteral("*/*"));
    request.setRawHeader(QByteArrayLiteral("Connection"), QByteArrayLiteral("close"));
    m_pNetworkAccessManager->get(request);
    return true;
}

void QSocketIoClient::onError(QAbstractSocket::SocketError error)
{
    qDebug() << "Error occurred: " << error;
}

void QSocketIoClient::onMessage(QString textMessage)
{
    Q_UNUSED(textMessage);
    parseMessage(textMessage);
}

void QSocketIoClient::sendHeartBeat()
{
    (void)m_pTransport->sendTextMessage(QStringLiteral("2::"));
}

void QSocketIoClient::replyFinished(QNetworkReply *reply)
{
    int status = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    //QString statusReason = reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
    qDebug()<< "handshake replyFinished() status=" << status;
    switch (status)
    {
        case 200:
        {
            //everything allright
            QString payload = QString::fromUtf8(reply->readAll());
            qDebug()<< "handshake payload: " << payload;
            QStringList handshakeReturn = payload.split(":");
            if (handshakeReturn.length() != 4)
            {
                qDebug() << "Not a valid handshake return";
            }
            else
            {
                QString sessionId = handshakeReturn[0];
                m_heartBeatTimeout = handshakeReturn[1].toInt() * 1000 - 500;
                m_connectionTimeout = handshakeReturn[2].toInt() * 1000;

                m_pHeartBeatTimer->setInterval(m_heartBeatTimeout);

                QStringList protocols = handshakeReturn[3].split(",");
                /*if (!protocols.contains("websocket"))
                {
                    qDebug() << "websockets not supported; so cannot continue";
                    return;
                }*/
                m_sessionId = sessionId;
                handshakeSucceeded(protocols);
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

void QSocketIoClient::handshakeSucceeded(QStringList protocols)
{
    //QUrl url(m_requestUrl.toString() + QStringLiteral("/socket.io/1/websocket/%1").arg(m_sessionId));
    QString p, schema;
    if (NULL == m_pTransport) {
        if (protocols.contains("websocket")) {
            qDebug() << "websockets supported; use websockets protocol transport";
            p = "websocket";
            schema = m_requestUrl.scheme() == "https" ? "wss" : "ws";
            m_pTransport = new QTransportWebSocket();
        }else if (protocols.contains("xhr-polling")) {
            qDebug() << "xhr-polling supported; use xhr-polling protocol transport";
            p = "xhr-polling";
            schema = m_requestUrl.scheme();
            m_pTransport = new QTransportXhrPolling();
        }else {
            qDebug() << "websockets and xhr-polling not supported; so cannot continue";
            return;
        }
    }

    QUrl url(QStringLiteral("%1://%2:%3/%4/1/%5/%6?%7")
             .arg(schema)
             .arg(m_requestUrl.host())
             .arg(QString::number(m_requestUrl.port(80)))
             .arg(m_namespace)
             .arg(p)
             .arg(m_sessionId)
             .arg(m_requestUrl.query())
             );
    qDebug() << qPrintable(p)  << "do connect" << qPrintable(url.toString());
    m_pTransport->open(url);

     //connect(m_pTransport, SIGNAL(error(QAbstractSocket::SocketError)),
     //       this, SLOT(onError(QAbstractSocket::SocketError)));
     connect(m_pTransport, SIGNAL(textMessageReceived(QString)),
             this, SLOT(onMessage(QString)));
}

void QSocketIoClient::ackReceived(int messageId, QJsonArray arguments)
{
    QAbstractCallback *callback = m_callbacks.value(messageId, Q_NULLPTR);
    if (callback) {
        (*callback)(arguments);
        delete callback;
    }
}

#include <functional>

void QSocketIoClient::eventReceived(QString message, QJsonArray arguments,
                                    bool mustAck, int messageId)
{
    if (m_subscriptions.contains(message)) {
        QJsonValue retVal;
        QAbstractCallback *callback = m_subscriptions[message];
        /*if (callback->hasReturnValue()) {
            retVal = (*callback)(arguments);
        } else {*/
            (*callback)(arguments);
        //}
        if (mustAck) {
            acknowledge(messageId, retVal);
        }
    }
}

void QSocketIoClient::parseMessage(const QString &message)
{
    QRegExp regExp("^([^:]+):([0-9]+)?(\\+)?:([^:]+)?:?([\\s\\S]*)?$", Qt::CaseInsensitive,
                   QRegExp::RegExp2);
    if (regExp.indexIn(message) != -1)
    {
        QStringList captured = regExp.capturedTexts();
        int messageType = captured.at(1).toInt();
        int messageId = captured.at(2).toInt();
        bool mustAck = (messageId != 0);
        bool autoAck = mustAck && captured.at(3).isEmpty();
        QString endpoint = captured.at(4);
        QString data = captured.at(5);

        if (autoAck)
        {
            acknowledge(messageId);
        }

        switch(messageType)
        {
            case 0:	//disconnect
            {
                Q_EMIT(disconnected(endpoint));
                break;
            }
            case 1: //connect
            {
                m_pHeartBeatTimer->start();
                Q_EMIT(connected(endpoint));
                break;
            }
            case 2:	//heartbeat
            {
                Q_EMIT(heartbeatReceived());
                break;
            }
            case 3:	//message
            {
                Q_EMIT(messageReceived(data));
                break;
            }
            case 4:	//json message
            {
                qDebug() << "JSON message received:" << data;
                QJsonParseError parseError;
                QJsonDocument document = QJsonDocument::fromJson(QByteArray(data.toLatin1()),
                                                                 &parseError);

                if (parseError.error != QJsonParseError::NoError)
                {
                    qDebug() << parseError.errorString();
                }
                else
                {
                   Q_EMIT(JSONMessageReceived(document));
                }

                break;
            }
            case 5: //event
            {
                QJsonParseError parseError;
                QJsonDocument document = QJsonDocument::fromJson(QByteArray(data.toLatin1()),
                                                                 &parseError);
                if (parseError.error != QJsonParseError::NoError)
                {
                    qDebug() << parseError.errorString();
                }
                else
                {
                    if (document.isObject())
                    {
                        QJsonObject object = document.object();
                        QJsonValue value = object["name"];
                        if (!value.isUndefined())
                        {
                            QString message = value.toString();
                            QJsonArray arguments;
                            QJsonValue argsValue = object["args"];
                            if (!argsValue.isUndefined() && !argsValue.isNull())
                            {
                                if (argsValue.isArray())
                                {
                                    arguments = argsValue.toArray();
                                }
                                else
                                {
                                    qWarning() << "Args argument is not an array";
                                    return;
                                }
                            }
                            eventReceived(message, arguments, mustAck && !autoAck, messageId);
                        }
                        else
                        {
                            qWarning() << "Invalid event received: no name";
                        }
                    }
                }
                break;
            }
            case 6:	//ack
            {
                QRegExp regExp("^([0-9]+)(\\+)?(.*)$", Qt::CaseInsensitive, QRegExp::RegExp2);
                if (regExp.indexIn(data) != -1)
                {
                    QJsonParseError parseError;
                    QJsonArray arguments;
                    int messageId = regExp.cap(1).toInt();
                    QString argumentsValue = regExp.cap(3);
                    if (!argumentsValue.isEmpty())
                    {
                        QJsonDocument doc = QJsonDocument::fromJson(argumentsValue.toLatin1(),
                                                                    &parseError);
                        if (parseError.error != QJsonParseError::NoError)
                        {
                            qWarning() << "JSONParseError:" << parseError.errorString();
                            return;
                        }
                        else
                        {
                            if (doc.isArray())
                            {
                                arguments = doc.array();
                            }
                            else
                            {
                                qWarning() << "Error: data of event is not an array";
                                return;
                            }
                        }
                    }
                    ackReceived(messageId, arguments);
                }
                break;
            }
            case 7:	//error
            {
                QStringList pieces = data.split("+");
                QString reason = pieces[0];
                QString advice;
                if (pieces.length() == 2)
                {
                    advice = pieces[1];
                }
                Q_EMIT(errorReceived(reason, advice));
                break;
            }
            case 8:	//noop
            {
                qDebug() << "Noop received" << data;
                break;
            }
        }
    }
}

QJsonDocument QSocketIoClient::package(const QVariant &value)
{
    if (value.canConvert<QVariantMap>()) {
        return QJsonDocument(QJsonObject::fromVariantMap(value.toMap()));
    } else if (value.canConvert<QVariantList>()) {
        return QJsonDocument(QJsonArray::fromVariantList(value.toList()));
    } else {
        QJsonArray ar;
        ar.append(QJsonValue::fromVariant(value));
        return QJsonDocument(ar);
    }
}

void QSocketIoClient::emitMessage(const QString &message, bool value)
{
    const QString m_endPoint;
    doEmitMessage(message, package(value), m_endPoint, true);
}

void QSocketIoClient::emitMessage(const QString &message, int value)
{
    const QString m_endPoint;
    doEmitMessage(message, package(value), m_endPoint, true);
}

void QSocketIoClient::emitMessage(const QString &message, double value)
{
    const QString m_endPoint;
    doEmitMessage(message, package(value), m_endPoint, true);
}

void QSocketIoClient::emitMessage(const QString &message, const QString &value)
{
    const QString m_endPoint;
    doEmitMessage(message, package(value), m_endPoint, true);
}

void QSocketIoClient::emitMessage(const QString &message, const QVariantList &arguments)
{
    const QString m_endPoint;
    doEmitMessage(message, package(arguments), m_endPoint, true);
}

void QSocketIoClient::emitMessage(const QString &message, const QVariantMap &arguments)
{
    const QString m_endPoint;
    doEmitMessage(message, package(arguments), m_endPoint, true);
}

void QSocketIoClient::sendMessage(const QString &message)
{
    doPostMessage(3, message, "", true, false);
}

void QSocketIoClient::sendMessage(const QVariantMap &value)
{
    QJsonDocument document = package(value);
    QString data = QString::fromUtf8(document.toJson(QJsonDocument::Compact));
    doPostMessage(4, data, "", true, false);
}

void QSocketIoClient::sendMessage(const QVariantList &value)
{
    QJsonDocument document = package(value);
    QString data = QString::fromUtf8(document.toJson(QJsonDocument::Compact));
    doPostMessage(4, data, "", true, false);
}

QString QSocketIoClient::sessionId() const
{
    return m_sessionId;
}

void QSocketIoClient::acknowledge(int messageId, const QJsonValue &retVal)
{
    QString msg = QStringLiteral("6:::") + QString::number(messageId);
    if (!retVal.isUndefined() && !retVal.isNull()) {
        QJsonDocument doc;
        if (retVal.isArray()) {
            doc.setArray(retVal.toArray());
        } else if (retVal.isObject()) {
            doc.setObject(retVal.toObject());
        } else {
            QJsonArray ar;
            ar.append(retVal);
            doc.setArray(ar);
        }
        msg.append(QStringLiteral("+") + QString::fromUtf8(doc.toJson(QJsonDocument::Compact)));
    }
    (void)m_pTransport->sendTextMessage(msg);
}

int QSocketIoClient::doEmitMessage(const QString &message, const QJsonDocument &document,
                                    const QString &endpoint, bool callbackExpected)
{
    /*static int id = 0;

    //const QString msg = QStringLiteral("5:%1%2:%3:{\"name\":\"%4\",\"args\":%5}")
      //      .arg(++id)
            const QString msg = QStringLiteral("5::%1:{\"name\":\"%2\",\"args\":%3}")
            .arg(endpoint)
            .arg(message)
            .arg(QString::fromUtf8(document.toJson(QJsonDocument::Compact)));
    (void)m_pTransport->sendTextMessage(msg);*/

    const QString msg = QStringLiteral("{\"name\":\"%1\",\"args\":%2}")
            .arg(message)
            .arg(QString::fromUtf8(document.toJson(QJsonDocument::Compact)));

    return doPostMessage(5, msg, endpoint, callbackExpected, false);
}


int QSocketIoClient::doPostMessage(int pack_type,
                                   const QString &message,
                                   const QString &endpoint,
                                   bool callbackExpected,
                                   bool mustAck)
{
    /**
    static int id = 0;
    const QString msg = QStringLiteral("5:%1%2:%3:{\"name\":\"%4\",\"args\":%5}")
            .arg(++id)
            .arg(callbackExpected ? QStringLiteral("+") : QStringLiteral(""))
            .arg(endpoint)
            .arg(message)
            .arg(QString::fromUtf8(document.toJson(QJsonDocument::Compact)));
            **/
    static int id = 0;
    callbackExpected = false;

    const QString ackstr = mustAck ? QStringLiteral("%1").arg(++id) : QStringLiteral("");
    const QString msg = QStringLiteral("%1:%2%3:%4:%5")
            .arg(pack_type)
            .arg(ackstr)
            .arg(endpoint)
            .arg(callbackExpected ? QStringLiteral("+") : QStringLiteral(""))
            .arg(message);
    (void)m_pTransport->sendTextMessage(msg);
    return id;
}
