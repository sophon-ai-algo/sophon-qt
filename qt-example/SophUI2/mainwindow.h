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
#ifdef ARM64
#include <opencv2/opencv.hpp>
#endif


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
#ifdef ARM64
using namespace cv;
#endif
QT_END_NAMESPACE
class MainWindow;

#define NAME_FONT_SIZE 40
//static QString DEFAULT_BOX_IP =  "192.168.1.180";
static QString DEFAULT_BOX_IP =  "127.0.0.1";
//static QString DEFAULT_BOX_IP =  "10.30.35.88";

enum MATRIX_TYPE {
    MATRIX_1 = 1,
    MATRIX_4 = 4,
    MATRIX_16 = 16,
};

class MyThread;
class EventThread;

class DetectEvent
{
public:
    //QString * t_str;

    QString event_id;
    QString image_id;
    QString name;
    int frame_id;
    QString face_id;
    float user_in_timestamp;
    float user_out_timestamp;
    double timestamp;
    QPixmap d_pixmap;
    QPixmap r_pixmap;
};

class EventWidget : public QWidget
{
    Q_OBJECT
public:
    EventWidget();
    ~EventWidget();
    QHBoxLayout * hbox;

    QLabel * d_label;
    QLabel * r_label;
    QLabel * t_label;

    QPixmap d_pixmap;
    QPixmap r_pixmap;

    QPixmap pframe;
    DetectEvent * ev;
};

class VideoInfo
{
public:
    QString vid;
    QString name;
    QString url;
    QString type;
};

struct ItemInfo
{
    int idx;
    QString str;
    QVariant userData;
    bool bChecked;
};

class CustLabel : public QLabel
{
    Q_OBJECT
public:

    CustLabel();
    ~CustLabel();
    QString label_name;
    void paintEvent(QPaintEvent *) override;
};

class VL_Dialog : public QDialog
{
    Q_OBJECT

public:
    VL_Dialog(MainWindow * win);
    ~VL_Dialog();

    MainWindow * main_win;
    QGridLayout * cboxs_layout;
    QListWidget * ch_list;

    QList<QCheckBox *> cbox_list;

    //填充所有添加的视频源
    void fill_channels();

public slots:
    void itemclick(QListWidgetItem * item);

private slots:
    void enter_click_cb();
    void close_click_cb();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    //通过api获取的视频源列表
    QList<VideoInfo *> vlist;

    struct timeval reply_ts,reply_td;
    //预览线程
    QList<MyThread *> video_thread;
    //解析event线程
    EventThread * event_thread;

    //预览label列表
    CustLabel * preview_label_list[16];

    QNetworkReply *reply;
    QNetworkRequest request;
    QNetworkAccessManager *manager;

    int prev_label = -1;

    int recv_type = 0;
    QString recv_data;
    QQueue<EventWidget *> ev_queue;
    QMutex ev_mutex;
    QFormLayout * flayout;

    int ev_total = 0;
    //版本信息
    QString Sophon_Version;

    QTimer * button_timer;
    QTimer * show_ip_timer;
    bool restart_flag = false;
    /* 每页第一个预览视频源的编号
     * 16宫格下 0
     * 4宫格下 0，4，8，12，
     * 1宫格下递增或递减
     */
    int preview_start_index = 0;

    //显示视频源选择界面的dialog
    VL_Dialog * video_list_dialog;
    //总体预览视频列表
    QStringList total_preview_list;
    //当前页面显示视频列表
    QStringList current_preview_list;

    void parse_recv_data(QString recv_str);
    QString executeLinuxCmd(QString strCmd);
    void show_ip();
    void clear_all_label();
    void get_device_list();
    void get_preview_video_list();
    void set_preview_video_list();
    void get_preview_info();
    void fill_vlist_to_combobox(int index);

    void stop_video_thread();
    void create_video_thread();
    void create_preview_labels();
    void clear_preview_labels();
    void set_current_preview_list(int start_index);
    void send_event_config();
    void Sleep(int msec);

public slots:

private slots:
    void _show_current_time();
    void _update(int num, QPixmap image, QString name, QString type);
    void _update_event(DetectEvent * detect_ev);

    void _onConnected();
    void _disConnected();
    void _onBinaryReceived(QByteArray message);

    void _onConnected_event();
    void _disConnected_event();
    void _onTextReceived_event(QString message);

    void _onConnected_cmd();
    void _disConnected_cmd();
    void _onTextReceived_cmd(QString message);

    void _reconnect_video_ws();
    void _reconnect_event_ws();
    void _reconnect_cmd_ws();

    void matrix_1_button_click_cb();
    void matrix_4_button_click_cb();
    void matrix_16_button_click_cb();
    void left_button_click_cb();
    void right_button_click_cb();
    void enter_button_click_cb();

    void mouseDoubleClickEvent(QMouseEvent *event);
    void button_timeout_callback();
    void show_ip_timeout_callback();

private:
    Ui::MainWindow *ui;
    QWebSocket m_video_webSocket;
    QWebSocket m_event_webSocket;
    QWebSocket m_cmd_webSocket;
    QUrl m_video_url;
    QUrl m_event_url;
    QUrl m_cmd_url;
    QTimer * event_config_timer;
    QTimer * time_clock;
    QTimer * video_ws_timer;
    QTimer * event_ws_timer;
    QTimer * cmd_ws_timer;
    int ws_event_config_flag = -1;
    QString face_api_url;
    QString device_api_url;

    int preview_port = 5678;
    QString preview_suffic = "/preview";

    MATRIX_TYPE current_matrix_type = MATRIX_4;
    MATRIX_TYPE prev_matrix_type = MATRIX_4;

    bool refresh_timeout = false;
    void ws_init();
    QString _get_ip_info(QNetworkInterface interface);
    QString _get_ip_info(QString device_name);
    void _parse_device_list(QString recv_str);
    int _get_current_preview_num();
    int _run_in_docker();
};

class EventThread : public QThread
{
    Q_OBJECT
public:
    EventThread(MainWindow * win);
    void closeThread();
    void show_event(QJsonObject rootobj);
    void parse_event_data(QString recv_str);

    QMutex mutex;
    QQueue<QString> ev_queue;

    struct timeval t_start,t_end;

signals:
    void sig_update_event(DetectEvent * ev);
    void sig_notify(int num, QPixmap image, QString name, QString type);

protected:
    virtual void run();

private:
    volatile bool isStop;
    MainWindow *w;
    QImage _icon;
};


class MyThread : public QThread
{
    Q_OBJECT
public:
    QMutex mutex;
    QQueue<QString> json_queue;
    QPixmap image_pixmap;
    QImage image;
    QString video_id;
    int preview_id;
    MyThread(MainWindow * win, int c);
    ~MyThread();
    void closeThread();
    int num;

    struct timeval t_start,t_end;
    QNetworkAccessManager *accessManager;
    QNetworkReply *my_reply;
    QNetworkRequest *my_request;

signals:
    void notify(int num, QPixmap image, QString name, QString type);
    //void signal_update_event(DetectEvent * detect_ev);
    //void signal_update_face(QByteArray image_data);

protected:
    virtual void run();

private:
    volatile bool isStop;
    MainWindow *w;
#ifdef ARM64
    Mat inMat;
#endif
};
#endif // MAINWINDOW_H
