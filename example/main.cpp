#include "mainwindow.h"
#include <QApplication>
#include <QUrl>


#include "clienttest.h"



int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    ClientTest c;
   // QSocketIoClient *www = new QSocketIoClient();
    //www->open(QUrl("http://ims.tbkf.net/?channel=1"), QString("tbim-1"));

    //QObject::connect(www, SIGNAL(connected(QString)),

    //QTransportXhrPolling *poll = new QTransportXhrPolling();
   // poll->open(QUrl("http://www.baidu.com"));

    return a.exec();
}
