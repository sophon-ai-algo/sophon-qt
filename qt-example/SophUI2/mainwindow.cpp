#include "mainwindow.h"
#include "./ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->page_layout->setSpacing(2);

    preview_start_index = 0;
    video_list_dialog = new VL_Dialog(this);

    connect(ui->matrix_1_button, SIGNAL(clicked()), this, SLOT(matrix_1_button_click_cb()));
    connect(ui->matrix_4_button, SIGNAL(clicked()), this, SLOT(matrix_4_button_click_cb()));
    connect(ui->matrix_16_button, SIGNAL(clicked()), this, SLOT(matrix_16_button_click_cb()));
    connect(ui->enter_button, SIGNAL(clicked()), this, SLOT(enter_button_click_cb()));
    connect(ui->left_button, SIGNAL(clicked()), this, SLOT(left_button_click_cb()));
    connect(ui->right_button, SIGNAL(clicked()), this, SLOT(right_button_click_cb()));

    manager = new QNetworkAccessManager(this);
    face_api_url = "http://" + DEFAULT_BOX_IP + ":5555/sophon/v2/face/";
    device_api_url = "http://" + DEFAULT_BOX_IP + ":5555/sophon/v2/face/";

    flayout = new QFormLayout();
    flayout->setGeometry(QRect(0, 0, 480, 720));
    ui->ev_list->setLayout(flayout);

    event_thread = new EventThread(this);
    connect(event_thread,SIGNAL(sig_update_event(DetectEvent *)),this,SLOT(_update_event(DetectEvent *)));
    connect(event_thread,SIGNAL(sig_notify(int, QPixmap, QString, QString)),this,SLOT(_update(int, QPixmap, QString, QString)));
    event_thread->start();


    /* 读取version */
#if 1
    QString version_str = executeLinuxCmd("bm_version");
    if(!version_str.isEmpty())
    {
        QStringList version_str_list = version_str.split("\n");
        Sophon_Version = version_str_list[1];
    }
    else
        Sophon_Version = "";
#endif
    qDebug() << Sophon_Version;

    /* 时间显示 */
    time_clock = new QTimer(this);
    connect(time_clock, SIGNAL(timeout()), this, SLOT(_show_current_time()));
    time_clock->start(1000);

    button_timer = new QTimer(this);
    connect(button_timer, SIGNAL(timeout()), this, SLOT(button_timeout_callback()));

    show_ip();
    show_ip_timer = new QTimer(this);
    connect(show_ip_timer, SIGNAL(timeout()), this, SLOT(show_ip_timeout_callback()));
    show_ip_timer->start(10 * 1000);
#if 1
    get_device_list();
    get_preview_video_list();
    get_preview_info();
    qDebug() << "total_preview_list: " << total_preview_list;
#endif
    ws_init();

    current_matrix_type = MATRIX_4;
    if(total_preview_list.size() == 1)
        current_matrix_type = MATRIX_1;
    else if(total_preview_list.size() <= 4)
        current_matrix_type = MATRIX_4;
    else
        current_matrix_type = MATRIX_16;
    set_current_preview_list(0);
    create_preview_labels();
    create_video_thread();
    send_event_config();

    printf("Mainwindow init done\n");
    restart_flag = true;
}

MainWindow::~MainWindow()
{
    delete ui;
}
void MainWindow::Sleep(int msec)
{
    QTime dieTime = QTime::currentTime().addMSecs(msec);
    while( QTime::currentTime() < dieTime )
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

void MainWindow::show_ip_timeout_callback()
{
    show_ip();
}
void MainWindow::button_timeout_callback()
{
    refresh_timeout = false;
    button_timer->stop();
}

void MainWindow::set_current_preview_list(int start_index)
{
    int diff = 0;
    int end = 0;
    qDebug() << "start_index : " << start_index;
    preview_start_index = start_index;
    qDebug() << "total_preview_list size : " << total_preview_list.size();
    qDebug() << "- 1 - preview_start_index: " << preview_start_index;
    switch(current_matrix_type)
    {
        case MATRIX_1:
            end = preview_start_index + 1;
        break;
        case MATRIX_4:
            diff = total_preview_list.size() - preview_start_index;
            if(diff <= 4)
            {
                end = preview_start_index + diff;
            }
            else {
                end = preview_start_index + 4;;
            }
        break;
        case MATRIX_16:
            end = total_preview_list.size();
        break;
    };

    if(total_preview_list.size() == 0)
        end = 0;

    qDebug() << "- 2 - preview_start_index: " << preview_start_index;
    qDebug() << "end: " << end;
    current_preview_list.clear();
    for(int i = preview_start_index; i < end; ++i)
    {
        current_preview_list.append(total_preview_list.at(i));
    }
    qDebug() << "current_preview_list: " << current_preview_list;
}

int MainWindow::_get_current_preview_num()
{
    return current_preview_list.size();
#if 0
    int num = 0;
    switch(current_matrix_type)
    {
        case MATRIX_1:
            num = 1;
        break;
        case MATRIX_4:
            num = 4;
        break;
        case MATRIX_16:
            num = 16;//current_preview_list.size();
        break;
    };
    return num;
#endif
}

void MainWindow::stop_video_thread()
{
    int num = _get_current_preview_num();
    qDebug() << "need stop " << num << " threads";
    foreach(MyThread * _thread, video_thread)
    {
        if(_thread->isRunning())
        {
            _thread->closeThread();
            _thread->quit();
            _thread->wait();
            _thread->mutex.lock();
            qDebug() << "_thrad.json_queue size: " << _thread->json_queue.size();
            for(int i = 0; i < _thread->json_queue.size(); ++i)
                _thread->json_queue.dequeue();
            _thread->mutex.unlock();

        }
        delete _thread;
    }
    video_thread.clear();
}
void MainWindow::create_video_thread()
{
    int num = _get_current_preview_num();
    qDebug() << "create " << num << " threads";
    for(int i = 0; i < num; ++i)
    {
        MyThread * _thread = new MyThread(this, i);
        connect(_thread,SIGNAL(notify(int, QPixmap, QString, QString)),this,SLOT(_update(int, QPixmap, QString, QString)));
        _thread->video_id = current_preview_list.at(i);
        qDebug() << i << ": " << _thread->video_id;
        video_thread.append(_thread);
        _thread->start();
    }
    qDebug() << "video_thread size: " << video_thread.size();

    qDebug() << "ws_event_config_flag: " << ws_event_config_flag;
}
void MainWindow::clear_preview_labels()
{
    QList<QLabel *> label_list = ui->stackedWidget->findChildren<QLabel *>(QString());
    qDebug() << label_list;

    Sleep(300);
    foreach (QLabel * item, label_list)
    {
        item->clear();
        item->setText(tr("<font style = 'font-size:60px; color:#FFFFFF;'> 无信号 </font><br/>"));
        delete item;
    }
}

void MainWindow::create_preview_labels()
{
    int row = 0;
    int col = 0;

    QRect layout_rect = ui->stackedWidget->geometry();
    for(int i = 0; i < current_matrix_type; ++i)
    {
        preview_label_list[i] = new CustLabel();
        preview_label_list[i]->setAlignment(Qt::AlignCenter);
        preview_label_list[i]->setText(tr("<font style = 'font-size:20px; color:#FFFFFF;'> 无信号 </font><br/>"));
    }
    if(current_matrix_type == MATRIX_1)
    {
        preview_label_list[0]->resize(layout_rect.width(), layout_rect.height());
        ui->page_layout->addWidget(preview_label_list[0], 0, 0);
    }
    else if(current_matrix_type == MATRIX_4)
    {
        preview_label_list[0]->resize(layout_rect.width() / 2, layout_rect.height() / 2);
        preview_label_list[1]->resize(layout_rect.width() / 2, layout_rect.height() / 2);
        preview_label_list[2]->resize(layout_rect.width() / 2, layout_rect.height() / 2);
        preview_label_list[3]->resize(layout_rect.width() / 2, layout_rect.height() / 2);
        ui->page_layout->addWidget(preview_label_list[0], 0, 0);
        ui->page_layout->addWidget(preview_label_list[1], 0, 1);
        ui->page_layout->addWidget(preview_label_list[2], 1, 0);
        ui->page_layout->addWidget(preview_label_list[3], 1, 1);
    }
    else if(current_matrix_type == MATRIX_16)
    {
        for(row = 0; row < 4; ++row)
        {
            for(col = 0; col < 4; ++col)
            {
                preview_label_list[row * 4 + col]->resize(layout_rect.width() / 4, layout_rect.height() / 4);
                ui->page_layout->addWidget(preview_label_list[row * 4 + col], row, col);
            }
        }
    }

}
void MainWindow::clear_all_label()
{
#if 0
    for(int i = 0; i < 16; ++i)
    {
        preview_label_list[i]->setText(tr("<font style = 'font-size:60px; color:#FFFFFF;'> 无信号 </font><br/>"));
    }
#endif
}

void MainWindow::matrix_1_button_click_cb()
{
    if(current_matrix_type == MATRIX_1)
        return;
    restart_flag = false;
    stop_video_thread();
    clear_preview_labels();
    current_matrix_type = MATRIX_1;
    set_current_preview_list(0);
    set_preview_video_list();
    create_preview_labels();
    create_video_thread();
    restart_flag = true;
    send_event_config();
}
void MainWindow::matrix_4_button_click_cb()
{
    if(current_matrix_type == MATRIX_4)
        return;
    restart_flag = false;
    stop_video_thread();
    clear_preview_labels();
    current_matrix_type = MATRIX_4;
    set_current_preview_list(0);
    set_preview_video_list();
    create_preview_labels();
    create_video_thread();
    restart_flag = true;
    send_event_config();
    prev_matrix_type = MATRIX_4;
}
void MainWindow::matrix_16_button_click_cb()
{
    if(current_matrix_type == MATRIX_16)
        return;
    restart_flag = false;
    stop_video_thread();
    clear_preview_labels();
    current_matrix_type = MATRIX_16;
    set_current_preview_list(0);
    set_preview_video_list();
    create_preview_labels();
    create_video_thread();
    restart_flag = true;
    send_event_config();
    prev_matrix_type = MATRIX_16;
}

void MainWindow::left_button_click_cb()
{
    if(refresh_timeout)
        return;
    refresh_timeout = true;
    button_timer->start(500);

    qDebug() << "left button preview_start_index: " << preview_start_index;
    if(total_preview_list.size()  == 0)
        return;
    else if(current_matrix_type == MATRIX_16)
        return;
    else if(current_matrix_type == MATRIX_1)
    {
        if(total_preview_list.size() == 1)
            return;
        preview_start_index = preview_start_index == 0 ? total_preview_list.size() - 1 : preview_start_index - 1;
    }
    else if(current_matrix_type == MATRIX_4)
    {
        if(total_preview_list.size() <= 4)
            return;
        preview_start_index = preview_start_index == 0 ? 0 : preview_start_index - 4;
    }
    restart_flag = false;
    stop_video_thread();
    clear_preview_labels();
    set_current_preview_list(preview_start_index);
    set_preview_video_list();
    create_preview_labels();
    create_video_thread();
    restart_flag = true;
    send_event_config();
}
void MainWindow::right_button_click_cb()
{
    if(refresh_timeout)
        return;
    refresh_timeout = true;
    button_timer->start(500);

    qDebug() << "right button preview_start_index: " << preview_start_index << "\n\n\n";
    if(total_preview_list.size()  == 0)
        return;
    else if(current_matrix_type == MATRIX_16)
        return;
    else if(current_matrix_type == MATRIX_1)
    {
        if(total_preview_list.size() == 1)
            return;
        preview_start_index = (preview_start_index + 1) >= total_preview_list.size() ? 0 : preview_start_index + 1;
    }
    else if(current_matrix_type == MATRIX_4)
    {
        if(total_preview_list.size() <= 4)
            return;
        preview_start_index = (preview_start_index + 4) >= total_preview_list.size() ? 0 : preview_start_index+4;
    }
    qDebug() << "right button preview_start_index: " << preview_start_index;
    restart_flag = false;
    stop_video_thread();
    clear_preview_labels();
    set_current_preview_list(preview_start_index);
    set_preview_video_list();
    create_preview_labels();
    create_video_thread();
    restart_flag = true;
    send_event_config();
}

void MainWindow::enter_button_click_cb()
{
    if(!video_list_dialog->isHidden())
        return;

    get_device_list();
    video_list_dialog->fill_channels();
    video_list_dialog->show();
#if 0
    QStringList vids;
    VideoInfo * vinfo = vlist.at(0);
    vids.append(vinfo->vid);
    vinfo = vlist.at(1);
    vids.append(vinfo->vid);
    set_preview_video_list(vids);
#endif
}

void MainWindow::fill_vlist_to_combobox(int index)
{
    (void)index;
#if 0
    int i = 0;
    for(i = 0; i < vlist.size(); ++i)
    {
        VideoInfo * vinfo = vlist.at(i);
    }
#endif
}


bool _check_event_xy_in_widge(int ev_x, int ev_y, QWidget * widget)
{
    QRect rect = widget->geometry();
    if(ev_x > rect.x() && ev_y > rect.y() && ev_x < (rect.x() + rect.width()) && ev_y < (rect.y() + rect.height()))
        return true;
    else
        return false;
}

void MainWindow::mouseDoubleClickEvent(QMouseEvent *event)
{
    int ev_x = event->x();
    int ev_y = event->y();
    bool in_range = false;

    qDebug() << event->x() << ", " << event->y();
    for(int i = 0; i < current_preview_list.size(); ++i)
    {
        if(_check_event_xy_in_widge(ev_x, ev_y, preview_label_list[i]))
        {
            in_range = true;
            prev_label = i;
            break;
        }
    }
    if(in_range)
    {
        //change preview video
        if(current_matrix_type == MATRIX_1)
        {
            if(prev_matrix_type == MATRIX_16)
            {
                matrix_16_button_click_cb();
            }
            else if(prev_matrix_type == MATRIX_4)
            {
                restart_flag = false;
                stop_video_thread();
                clear_preview_labels();
                current_matrix_type = MATRIX_4;
                set_current_preview_list(preview_start_index / 4 * 4);
                set_preview_video_list();
                create_preview_labels();
                create_video_thread();
                restart_flag = true;
                send_event_config();
            }

        }
        else if(current_matrix_type == MATRIX_4 || current_matrix_type == MATRIX_16)
        {
            restart_flag = false;
            stop_video_thread();
            clear_preview_labels();
            prev_matrix_type = current_matrix_type;
            current_matrix_type = MATRIX_1;
            set_current_preview_list(prev_label + preview_start_index);
            set_preview_video_list();
            create_preview_labels();
            create_video_thread();
            restart_flag = true;
            send_event_config();
        }
    }
    qDebug() << "prev_label: " << prev_label;
}

QString _get_video_id(QString str)
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(str.toUtf8(), &json_error));
    QString vid;

    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" !!!!!!!! get video id json error !!!!!!!!");
        return nullptr;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        vid = rootobj.value("video_id").toString();
    }
    return vid;
}
QString _get_event_id(QString str)
{
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(str.toUtf8(), &json_error));
    QString event_id;

    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" !!!!!!!! get event id json error !!!!!!!!");
        while(1);
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        QJsonObject frame = rootobj.value("events").toObject();

        event_id = rootobj.value("event_id").toString();
    }
    return event_id;
}

void MainWindow::_update(int num, QPixmap image, QString name, QString type)
{
    QRect rect;
    int width;
    int height;
    if(restart_flag == false)
        return;
    if(num >= current_matrix_type)
        return;
    if(ui->stackedWidget->currentIndex() == 0)
    {
        rect = preview_label_list[num]->geometry();
        width = rect.width();
        height = rect.height();
        if(type == "photo")
        {
            image = image.scaled(width, height, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        else
            image = image.scaled(width, height);
        preview_label_list[num]->setPixmap(image);
        preview_label_list[num]->label_name = name;
    }
}

CustLabel::CustLabel()
{
    label_name = "";
}

CustLabel::~CustLabel()
{

}
void CustLabel::paintEvent(QPaintEvent * event)
{
    int text_x = 10;
    QFont ft;
    ft.setPixelSize(40);
    QLabel::paintEvent(event);
    QPainter P(this);
    QRect rect = this->geometry();
    P.setRenderHint(QPainter::Antialiasing);
    P.setPen(QPen(Qt::green, 2));
    P.setFont(ft);
    P.drawText(QPoint(text_x, rect.height() - 20), label_name);
}
