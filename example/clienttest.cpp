#include "clienttest.h"
#include "qsocketioclient.h"
#include <QtCore/QJsonDocument>
#include <QtCore/QJsonObject>
#include <QtCore/QJsonArray>

#define function(args...) [=](args)
ClientTest::ClientTest(QObject *parent) : QObject(parent),
    conn()
{
    connect(&conn, SIGNAL(connected(QString)), this, SLOT(onConnected(QString)));
    connect(&conn, SIGNAL(JSONMessageReceived(QJsonDocument)), this, SLOT(onMessage(QJsonDocument)));

     //conn.open(QUrl("http://ims.tbkf.net/?channel=1"), QString("tbim-1"));
     conn.open(QUrl("http://192.168.3.187:8001/?channel=1"), QString("tbim-1"));

     //客服邀请
     conn.on("invite", function(QJsonArray data) {
                 qDebug() << "servicer invite" << data;
                 QVariantMap service;
                 service["service_name"] = data[0].toString();
                 conn.emitMessage("set_service", QVariantList() << service);
     });




     //收到消息
     conn.on("invite", function(QJsonArray data) {
                 qDebug() << "servicer invite" << data;
                 QVariantMap service;
                 service["service_name"] = data[0].toString();
                 conn.emitMessage("set_service", QVariantList() << service);
     });

}

ClientTest::~ClientTest()
{

}

void ClientTest::onConnected(QString endpoint)
{
    qDebug() << "onConnected" << endpoint;
    QVariantList package;
    QVariantMap loginParams({
                                {"plat_id", 1},
                                {"channel_id",1},
                                {"username", ""},
                                {"password", ""},
                                {"usertype", "guester"},
                                {"proto_ver", 1},
                            });
    conn.emitMessage(QString("login"), QVariantList() << loginParams);
}
void ClientTest::onMessage(QJsonDocument document)
{
    qDebug() << "onMessage" << document;
    if (document.isObject()) {
        QJsonObject object = document.object();
        QJsonValue value = object["message"];
        if (!value.isUndefined()) {
            QString vs = value.toString();
            /*
             * {
                        from_user: fromuser,
                        to_user: touser,
                        client_time: clientTime,
                        format: 'json',
                        message: {
                            style: style,
                            message: text
                        }
                    }
                    */
            QVariantMap content;
            QVariantMap reply;
            reply["from_user"] = object["to_user"].toString();
            reply["to_user"] = object["from_user"].toString();
            reply["format"] = "html";


            content["message"] = QString("haha ");
           reply["message"] = content;
            conn.sendMessage(reply);

        }
    }

}
