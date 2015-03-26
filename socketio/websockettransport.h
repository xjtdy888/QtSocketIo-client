#ifndef WEBSOCKETTRANSPORT_H
#define WEBSOCKETTRANSPORT_H

#include <QObject>

class WebSocketTransport : public QObject
{
    Q_OBJECT
public:
    explicit WebSocketTransport(QObject *parent = 0);
    ~WebSocketTransport();

signals:

public slots:
};

#endif // WEBSOCKETTRANSPORT_H
