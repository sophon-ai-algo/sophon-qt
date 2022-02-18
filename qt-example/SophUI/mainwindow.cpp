#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{

    ui->setupUi(this);

    //set font
    QFont ft;
    ft.setPointSize(80);
    ui->INFO->setFont(ft);
    ui->INFO_2->setFont(ft);
    ui->MAC_LABEL_2->setFont(ft);
    ui->MAC_LABEL_3->setFont(ft);
    ui->MAC_LABEL_4->setFont(ft);
    ui->MAC_LABEL_5->setFont(ft);
    ui->MAC_LABEL_6->setFont(ft);
    ui->MAC_LABEL_7->setFont(ft);
    ui->MAC_LABEL_8->setFont(ft);
    ui->MAC_LABEL_9->setFont(ft);

    ui->wan_button->setFont(ft);
    ui->lan_button->setFont(ft);

    ui->info_detail->setFont(ft);
    ui->info_detail_2->setFont(ft);
    ui->ip_detail->setFont(ft);

    QFont ft1;
    ft1.setPointSize(60);
    ui->wan_ip->setFont(ft1);
    ui->wan_net->setFont(ft1);
    ui->wan_gate->setFont(ft1);
    ui->wan_dns->setFont(ft1);
    ui->lan_ip->setFont(ft1);
    ui->lan_net->setFont(ft1);
    ui->lan_gate->setFont(ft1);
    ui->lan_dns->setFont(ft1);


    /* 读取version */
    QString version_str = executeLinuxCmd("bm_version");
    if(!version_str.isEmpty())
    {
        //QStringList version_str_list = version_str.split("\n");
        Sophon_Version = version_str;
    }
    else
        Sophon_Version = "beta";

    version_str = executeLinuxCmd("bm_get_basic_info");
    if(!version_str.isEmpty())
    {
        //QStringList version_str_list = version_str.split("\n");
        info_str = version_str;
    }
    else
        info_str = "beta";

    qDebug() << Sophon_Version;

    /* 时间显示 */
    time_clock = new QTimer(this);
    connect(time_clock, SIGNAL(timeout()), this, SLOT(_show_current_time()));
    time_clock->start(1000);

    ip_clock = new QTimer(this);
    connect(ip_clock, SIGNAL(timeout()), this, SLOT(_show_ip()));
    ip_clock->start(10000);

    connect(ui->wan_button, SIGNAL(clicked()), this,SLOT(_wan_button_click_cb()));

    connect(ui->lan_button, SIGNAL(clicked()), this, SLOT(_lan_button_click_cb()));

    QRegExp rx("\\b(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\b");
    ui->wan_ip->setValidator(new QRegExpValidator(rx, this));
    ui->wan_net->setValidator(new QRegExpValidator(rx, this));
    ui->wan_gate->setValidator(new QRegExpValidator(rx, this));
    ui->wan_dns->setValidator(new QRegExpValidator(rx, this));

    ui->lan_ip->setValidator(new QRegExpValidator(rx, this));
    ui->lan_net->setValidator(new QRegExpValidator(rx, this));
    ui->lan_gate->setValidator(new QRegExpValidator(rx, this));
    ui->lan_dns->setValidator(new QRegExpValidator(rx, this));

    _show_ip();
    ui->info_detail->setText(info_str +"\n");

    ui->info_detail_2->setText("Version:\n"+Sophon_Version +"\n");


}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::_get_ip_info(QNetworkInterface interface)
{
    QString ip_str;
    QString netmask_str;
    QString gateway_str;
    QString dns_str;

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

    result_str.append("IP"+ ip_str+"\nNETMASK: "+netmask_str+"\nMAC: "+mac_str );
    if(interface.name() == "eth0")
    {
        wan_mac = "WAN \n            MAC:              " + mac_str+ "\n";
        wan_ip =  "            IP:                    " +ip_str+ "\n";
        wan_netmask =  "            NETMASK:    " +netmask_str+ "\n";
    }
    else if(interface.name() == "eth1")
    {
        lan_mac =  "LAN \n            MAC:              " +mac_str + "\n";
        lan_ip =  "            IP:                    " +ip_str+ "\n";
        lan_netmask =  "            NETMASK:    " +netmask_str+ "\n";
    }
    return result_str;
}

void MainWindow::_show_ip()
{
    QString eth0_str;
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces())
    {
        eth0_str = _get_ip_info(netInterface);
        qDebug() << eth0_str;

    }
    ui->ip_detail->setText(wan_mac+wan_ip+wan_netmask+lan_mac+lan_ip+lan_netmask);
}

void MainWindow::_show_current_time()
{
    QDateTime *date_time = new QDateTime(QDateTime::currentDateTime());
    ui->TIME_LABLE->setText(tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font><br/>").arg(date_time->toString("hh:mm:ss"))
              + tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font>").arg(date_time->toString("yyyy-MM-dd ddd")));
    delete date_time;
}


QString MainWindow::executeLinuxCmd(QString strCmd)
{
    QProcess p;
    p.start("bash", QStringList() <<"-c" << strCmd);
    p.waitForFinished();
    QString strResult = p.readAllStandardOutput();
    qDebug() << strResult  + strCmd;
    return strResult;
}

void MainWindow::_wan_button_click_cb()
{
    QString set_str;
    bool flag = false;

    MyMessageBox msgBox;
   // msgBox.setFixedSize(640*4,480*4);//设置MessageBox的大小

    QFont ft1;
    ft1.setPointSize(80);
    msgBox.setFont(ft1);
    msgBox.setWindowTitle("WARNING!");
    msgBox.setText("Question?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);


    if(ui->wan_ip->text().isEmpty() && ui->wan_net->text().isEmpty() && ui->wan_gate->text().isEmpty() && ui->wan_dns->text().isEmpty())
    {
        qDebug() << QStringLiteral("Input exmpty will set to dhcp!");

        msgBox.setInformativeText("WAN将会设置成动态IP模式");
        int ret = msgBox.exec();
        if( ret == QMessageBox::Yes)
        {
            qDebug() << QStringLiteral("set ip to dhcp!");
            /* set_ip */
            executeLinuxCmd("bm_set_ip_auto eth0");
            flag = true;
        }
    }
    else
    {
        QString _ip = ui->wan_ip->text();
        QString _netmask = ui->wan_net->text();
        QString _gateway = ui->wan_gate->text();
        QString _dns = ui->wan_dns->text();

        set_str = "设置IP为" +_ip +"\n设置NETMASK为" + _netmask + "\n设置GATEWAY为"  +_gateway +"\n设置DNS为" + _dns;

        msgBox.setInformativeText(set_str);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Yes)
        {
            qDebug() << QStringLiteral("set ip to static!");
            /* set_ip */
            executeLinuxCmd("bm_set_ip eth0 " + _ip + " " +_netmask + " " + _gateway+ " " + _dns);
            flag = true;
        }
    }
    if (flag == true)
    {
        ui->wan_ip->setText("");
        ui->wan_net->setText("");
        ui->wan_gate->setText("");
        ui->wan_dns->setText("");
    }

}
void MainWindow::_lan_button_click_cb()
{
    QString set_str;
    bool flag = false;

    MyMessageBox msgBox;
   // msgBox.setFixedSize(640*4,480*4);//设置MessageBox的大小

    QFont ft1;
    ft1.setPointSize(80);
    msgBox.setFont(ft1);
    msgBox.setWindowTitle("WARNING!");
    msgBox.setText("Question?");
    msgBox.setStandardButtons(QMessageBox::Yes);
    msgBox.addButton(QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);

    if(ui->lan_ip->text().isEmpty() && ui->lan_net->text().isEmpty() && ui->lan_gate->text().isEmpty() && ui->lan_dns->text().isEmpty())
    {
        qDebug() << QStringLiteral("Input exmpty will set to dhcp!");
        msgBox.setInformativeText("LAN将会设置成动态IP模式");
        int ret = msgBox.exec();
        if( ret == QMessageBox::Yes)
        {
            qDebug() << QStringLiteral("set ip to dhcp!");
            /* set_ip */
            executeLinuxCmd("bm_set_ip_auto eth1");
            flag = true;
        }
    }
    else
    {
        QString _ip = ui->lan_ip->text();
        QString _netmask = ui->lan_net->text();
        QString _gateway = ui->lan_gate->text();
        QString _dns = ui->lan_dns->text();

        set_str = "设置IP为" +_ip +"\n设置NETMASK为" + _netmask + "\n设置GATEWAY为"  +_gateway +"\n设置DNS为" + _dns;

        //int ret = MyMessageBox::warning(this, QStringLiteral("warning!"), set_str, QMessageBox::Cancel | QMessageBox::Ok);
        msgBox.setInformativeText(set_str);
        int ret = msgBox.exec();
        if( ret == QMessageBox::Yes)
        {
            qDebug() << QStringLiteral("set ip to static!");
            /* set_ip */
            executeLinuxCmd("bm_set_ip eth1 " + _ip + " " +_netmask + " " + _gateway+ " " + _dns);
            flag = true;
        }
    }
    if (flag == true)
    {
        ui->lan_ip->setText("");
        ui->lan_net->setText("");
        ui->lan_gate->setText("");
        ui->lan_dns->setText("");
    }

}
