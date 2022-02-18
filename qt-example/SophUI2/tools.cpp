#include "mainwindow.h"
#include "./ui_mainwindow.h"

int MainWindow::_run_in_docker()
{
    QFile file("/proc/1/cpuset");
    if(file.open(QIODevice::ReadOnly))
    {
        QByteArray _line = file.readLine();
        QString _line_str = _line;
        if(_line_str.contains("docker") || _line_str.contains("pod"))
        {
            qDebug() << "run in docker";
            return 1;
        }
    }
    return 0;
}


QString MainWindow::_get_ip_info(QNetworkInterface interface)
{
    QString ip_str;
    QString netmask_str;
    QString mac_str;
    QString device_name = interface.name();
    mac_str = interface.hardwareAddress().toUtf8();
    QList<QNetworkAddressEntry>addressList = interface.addressEntries();
    foreach(QNetworkAddressEntry _entry, addressList)
    {
        QHostAddress address = _entry.ip();
        if(address.protocol() == QAbstractSocket::IPv4Protocol)
        {
            ip_str = address.toString();
            netmask_str = _entry.netmask().toString();
        }
    }
    qDebug() << "mac: " << mac_str << " ip: " << ip_str << " netmask: " << netmask_str;
    QString result_str;
    result_str.append(ip_str);
    return result_str;
}
QString MainWindow::_get_ip_info(QString device_name)
{
    QString ip_str;
    QString netmask_str;
    QString mac_str;
    QString dname;
    QProcess process;
    process.start("curl http://172.17.0.1:8888/sophon/v2/system/getip -X POST");
    process.waitForFinished();
    QByteArray output = process.readAllStandardOutput();
    qDebug() << output;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(QString(output).toUtf8(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" --- get_ip_info json error");
    }

    if(jsonDoc.isObject())
    {
        QJsonArray data_array = jsonDoc.object().value("address_list").toArray();
        foreach (const QJsonValue & value, data_array)
        {
            dname = value.toObject().value("interface").toString();
            if(dname == device_name)
            {
                mac_str = value.toObject().value("mac").toString();
                ip_str = value.toObject().value("ip").toString();
                netmask_str = value.toObject().value("netmask").toString();
            }
        }
    }
    qDebug() << "mac: " << mac_str << " ip: " << ip_str << " netmask: " << netmask_str;
    QString result_str;
    result_str.append(ip_str);
    return result_str;
}
void MainWindow::show_ip()
{
    QString eth0_str;
    QString eth1_str;
#ifdef ARM64
    if(_run_in_docker())
    {
        eth0_str = "WAN: " + _get_ip_info("eth0");
        eth1_str = "LAN: " + _get_ip_info("eth1");
    }
    else {
        foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
        {
            //qDebug() << netInterface;
            if(netInterface.name() == "eth0")
            {
                eth0_str = "WAN: " + _get_ip_info(netInterface);
            }
            else if(netInterface.name() == "eth1")
            {
                eth1_str = "LAN: " + _get_ip_info(netInterface);
            }
        }
    }
#endif
    qDebug() << eth0_str;
    qDebug() << eth1_str;
    ui->ip_label->setText(tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font><br/>").arg(eth0_str)
              +tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font><br/>").arg(eth1_str));
    ui->ver_label->setText(tr("<font style = 'font-size:15px; color:#FFFFFF;'>%1</font><br/>").arg(Sophon_Version));
}

void MainWindow::_show_current_time()
{
    QDateTime *date_time = new QDateTime(QDateTime::currentDateTime());
    ui->time_label->setText(tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font><br/>").arg(date_time->toString("hh:mm:ss"))
              + tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font>").arg(date_time->toString("yyyy-MM-dd ddd")));
    delete date_time;
}

QString MainWindow::executeLinuxCmd(QString strCmd)
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();
    return strResult;
}
