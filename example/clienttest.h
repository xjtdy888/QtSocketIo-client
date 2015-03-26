#ifndef CLIENTTEST_H
#define CLIENTTEST_H

#include <QObject>
#include <QJsonArray>
#include <QtCore/QJsonDocument>
#include "qsocketioclient.h"
class ClientTest : public QObject
{
    Q_OBJECT
public:
    explicit ClientTest(QObject *parent = 0);
    ~ClientTest();



signals:

public slots:
    void onConnected(QString endpoint);
    void onMessage(QJsonDocument message);


private:
    QSocketIoClient conn;


};

#endif // CLIENTTEST_H
