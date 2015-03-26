#ifndef XHRPOLLINGTRANSPORT_H
#define XHRPOLLINGTRANSPORT_H

#include <QObject>

class XhrPollingTransport : public QObject
{
    Q_OBJECT
public:
    explicit XhrPollingTransport(QObject *parent = 0);
    ~XhrPollingTransport();

signals:

public slots:
};

#endif // XHRPOLLINGTRANSPORT_H
