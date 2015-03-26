#ifndef TRANSPORT_H
#define TRANSPORT_H

#include <QObject>

class Transport : public QObject
{
    Q_OBJECT
public:
    explicit Transport(QObject *parent = 0);
    ~Transport();

signals:

public slots:
};

#endif // TRANSPORT_H
