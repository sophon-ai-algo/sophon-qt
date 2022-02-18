#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QMouseEvent>
#include <QSizePolicy>
#include <QRect>
#include <QDebug>
#include <QQueue>
#include <QThread>
#include <QMutex>
#include <QTimer>
#include <QLabel>
#include <QtNetwork/QNetworkRequest>
#include <QtNetwork/QNetworkAccessManager>
#include <QtNetwork/QNetworkReply>
#include <QUrl>
#include <QJsonObject>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonArray>
#include <QPainter>
#include <QLayout>
#include <QLayoutItem>
#include <QFormLayout>
#include <QMenu>
#include <QComboBox>
#include <QVariant>
#include <QListWidget>
#include <QListWidgetItem>
#include <QStandardItem>
#include <QLineEdit>
#include <QWebSocket>
#include <QProcess>
#include <QNetworkInterface>
#include <QDateTime>
#include <QDialog>
#include <QCheckBox>
#include <QList>
#include <sys/time.h>
#include <sys/stat.h>
#include <algorithm>
#include <time.h>
#include <QMessageBox>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    QString executeLinuxCmd(QString strCmd);
    QString _get_ip_info(QNetworkInterface interface);

public slots:

    void _show_current_time();
    void _wan_button_click_cb();
    void _lan_button_click_cb();
    void _show_ip();


private:
    Ui::MainWindow *ui;
    QString Sophon_Version;
    QString info_str;

    QTimer * time_clock;
    QTimer * ip_clock;

    QString wan_ip;
    QString wan_mac;
    QString wan_netmask;
    QString lan_ip;
    QString lan_mac;
    QString lan_netmask;

};

class MyMessageBox : public QMessageBox {
protected:
void showEvent(QShowEvent* event) {
QMessageBox::showEvent(event);
qDebug() << "set dialg size";
//setFixedSize(800*2, 600*2);
}
};
#endif // MAINWINDOW_H
