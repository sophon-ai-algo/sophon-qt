#include "mainwindow.h"
#include "./ui_mainwindow.h"

struct timeval time1;
struct timeval time2;
static int count = 0;

void MainWindow::ws_init()
{
    m_event_url = "ws://" + DEFAULT_BOX_IP + ":5679";
    m_video_url = "ws://" + DEFAULT_BOX_IP + ":" + QString::number(preview_port) + preview_suffic;
    m_cmd_url = "ws://" + DEFAULT_BOX_IP + ":54547";
    qDebug() << m_video_url;

    event_ws_timer = new QTimer(this);
    connect(event_ws_timer, SIGNAL(timeout()), this, SLOT(_reconnect_event_ws()));
    event_ws_timer->start(1000);

    video_ws_timer = new QTimer(this);
    connect(video_ws_timer, SIGNAL(timeout()), this, SLOT(_reconnect_video_ws()));
    video_ws_timer->start(1000);

    cmd_ws_timer = new QTimer(this);
    connect(cmd_ws_timer, SIGNAL(timeout()), this, SLOT(_reconnect_cmd_ws()));
    cmd_ws_timer->start(1000);

    connect(&m_video_webSocket, &QWebSocket::connected, this, &MainWindow::_onConnected);
    connect(&m_video_webSocket, &QWebSocket::disconnected, this, &MainWindow::_disConnected);
    connect(&m_video_webSocket, &QWebSocket::binaryFrameReceived, this, &MainWindow::_onBinaryReceived);

    connect(&m_event_webSocket, &QWebSocket::connected, this, &MainWindow::_onConnected_event);
    connect(&m_event_webSocket, &QWebSocket::disconnected, this, &MainWindow::_disConnected_event);
    connect(&m_event_webSocket, &QWebSocket::textFrameReceived, this, &MainWindow::_onTextReceived_event);

    connect(&m_cmd_webSocket, &QWebSocket::connected, this, &MainWindow::_onConnected_cmd);
    connect(&m_cmd_webSocket, &QWebSocket::disconnected, this, &MainWindow::_disConnected_cmd);
    connect(&m_cmd_webSocket, &QWebSocket::textFrameReceived, this, &MainWindow::_onTextReceived_cmd);
}

void MainWindow::_reconnect_event_ws()
{
    m_event_webSocket.open(QUrl(m_event_url));
    qDebug() << "reconnect event ws";
}

void MainWindow::_reconnect_video_ws()
{
    m_video_webSocket.open(QUrl(m_video_url));
    qDebug() << "reconnect video ws ";
}

void MainWindow::_reconnect_cmd_ws()
{
    m_cmd_webSocket.open(QUrl(m_cmd_url));
    qDebug() << "reconnect cmd ws ";
}

void MainWindow::_disConnected()
{
    qDebug() << "video ws disconnect !!!!!!!!!!";
    if(!video_ws_timer->isActive())
    {
        qDebug() << "start video ws timer";
        video_ws_timer->start(1000);
        for(int n = 0; n < current_preview_list.size(); ++n)
        {
            MyThread * _thread = video_thread.at(n);
            _thread->video_id = nullptr;
            for(int i = 0; i < _thread->json_queue.size(); ++i)
            {
                _thread->mutex.lock();
                _thread->json_queue.dequeue();
                _thread->mutex.unlock();
            }
        }
    }
}

void MainWindow::_onConnected()
{
    qDebug() << "video ws connect !!!!!!!!!!";
    gettimeofday(&time1, nullptr);
    if(video_ws_timer->isActive())
        video_ws_timer->stop();
}


void MainWindow::parse_recv_data(QString recv_str)
{
    QJsonObject obj;
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_str.toUtf8(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        //printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        printf(" --- json error \n");
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        QJsonValue frame_data = rootobj.value("frame_img");
        if(frame_data.isUndefined())
            return;
        QJsonValue frame_base64 = frame_data.toObject().value("data");
        QString _tmp_str = frame_base64.toString();
        //qDebug() << _tmp_str;
        QByteArray tmp_data = QByteArray::fromBase64(_tmp_str.toLatin1());
        //qDebug() << tmp_data;

    }

}

void MainWindow::_onBinaryReceived(QByteArray  message)
{
    QJsonObject obj;
    QJsonParseError json_error;
    char * recv_data = message.data();
    QString vid;
    int preview_id = -1;

    if(restart_flag == false)
        return;

    gettimeofday(&time2, nullptr);
    count++;
    if(time2.tv_sec - time1.tv_sec >= 1) {
       //qDebug() << "fps: " << count / (time2.tv_sec - time1.tv_sec);
       count = 0;
       gettimeofday(&time1, nullptr);
    }

    //qDebug() << recv_data;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_data, &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        //printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        printf(" --- json error \n");
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
#if 0
        qDebug() << " -------------- ";
        qDebug() << rootobj.keys();
        qDebug() << " -------------- ";
#endif
        vid = rootobj.value("media_device_id").toString();
        //preview_id = rootobj.value("preview_id").toString().toInt();
        //qDebug() << "preview id: " << preview_id << " vid: " << vid;

        int i = 0;
        for(i = 0; i < current_preview_list.size(); ++i)
        {
            if(vid == current_preview_list.at(i))
            {
                preview_id = i;
                break;
            }
        }

        if(preview_id < 0)
        {
            qDebug() << "preview_id is error !!!! " << preview_id;
            return;
        }
        if(preview_id >= video_thread.size())
        {
            qDebug() << "preview_id:  "<< preview_id << " > current_matrix_type";
            return;
        }

        MyThread * _thread = video_thread.at(preview_id);
        if(!_thread)
            qDebug() << " error !!! thread is null!" << preview_id;
        if(_thread->video_id.isEmpty() || _thread->video_id != vid)
        {
           // qDebug() << " !!!!!!!  video thread error!!! ";
           // qDebug() << preview_id << ": " << _thread->video_id;
        }

        _thread->mutex.lock();
        _thread->json_queue.enqueue(recv_data);
        _thread->mutex.unlock();
    }

}

void MainWindow::send_event_config()
{
    QJsonObject config_json;
    QJsonArray event_list;
    QStringList vids;
    for(int n = 0; n < current_preview_list.size(); ++n)
    {
        MyThread * _thread = video_thread.at(n);
        if(!_thread->video_id.isEmpty())
            event_list.append(_thread->video_id);
    }

    config_json.insert("with_face_image", true);
    config_json.insert("with_register_image", true);
    config_json.insert("media_device_ids", event_list);
    qDebug() << " -----  ws event_config: " << config_json;

    if(m_event_webSocket.isValid())
    {
        int ret = m_event_webSocket.sendTextMessage(QString(QJsonDocument(config_json).toJson()));
        qDebug() << "send event ws config size " << ret;
    }
}


void MainWindow::_disConnected_event()
{
    qDebug() << "event ws disconnect !!!!!!!!!!";
    if(!event_ws_timer->isActive())
    {
        qDebug() << "start event ws timer";
        event_ws_timer->start(1000);
    }
}
void MainWindow::_onConnected_event()
{
    qDebug() << "event ws connect !!!!!!!!!!";
    if(event_ws_timer->isActive())
        event_ws_timer->stop();
    send_event_config();
}

void MainWindow::_onTextReceived_event(QString  message)
{
    QJsonObject obj;
    QJsonParseError json_error;
    QString recv_data = message;
    QString vid;

    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_data.toUtf8(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        //printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        printf(" --- json error \n");
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
#if 0
        qDebug() << " -------------- ";
        qDebug() << rootobj.keys();
        qDebug() << " -------------- ";
#endif
        vid = rootobj.value("device_id").toString();
        //qDebug() << "device id: " << vid << " event_id: " << rootobj.value("event_id").toString();
        event_thread->mutex.lock();
        event_thread->ev_queue.enqueue(recv_data);
        event_thread->mutex.unlock();
    }

}

void MainWindow::_disConnected_cmd()
{
    qDebug() << "cmd ws disconnect !!!!!!!!!!";
    if(!cmd_ws_timer->isActive())
    {
        qDebug() << "start cmd ws timer";
        cmd_ws_timer->start(1000);
    }
}
void MainWindow::_onConnected_cmd()
{
    qDebug() << "cmd ws connect !!!!!!!!!!";
    if(cmd_ws_timer->isActive())
        cmd_ws_timer->stop();
}

void MainWindow::_onTextReceived_cmd(QString  message)
{
    QJsonObject obj;
    QJsonParseError json_error;
    QString recv_data = message;
    QString vid;

    //qDebug() << "-----------------";
    //qDebug() << "-----------------";
    qDebug() << "ws recv cmd: " << recv_data;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_data.toUtf8(), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        //printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        printf(" --- json error \n");
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
#if 0
        qDebug() << " -------------- ";
        qDebug() << rootobj.keys();
        qDebug() << " -------------- ";
#endif
        QString ower = rootobj.value("last_set_name").toString();
        if(ower != "QT")
        {
            restart_flag = false;
            get_device_list();
            QJsonArray preview_list_json = rootobj.value("selected_device_list").toArray();
            total_preview_list.clear();
            for(int i = 0; i < preview_list_json.size(); ++i)
            {
                total_preview_list.append(preview_list_json.at(i).toString());
            }
            QJsonArray current_preview_list_json = rootobj.value("preview_device_list").toArray();
            current_preview_list.clear();
            for(int i = 0; i < current_preview_list_json.size(); ++i)
            {
                current_preview_list.append(current_preview_list_json.at(i).toString());
            }
            int pmode = rootobj.value("preview_mode").toInt();
            if(pmode != 1 && pmode != 4 && pmode != 16)
                qDebug() << " !!!!!!!!!!! pmode error !!!!!!!";
            current_matrix_type = static_cast<MATRIX_TYPE>(pmode);
            qDebug() << current_preview_list;

            stop_video_thread();
            clear_preview_labels();
            //set_current_preview_list(0);
            create_preview_labels();
            create_video_thread();
            restart_flag = true;
            send_event_config();

            QJsonObject mdevice = rootobj.value("modify_media_device").toObject();
        }
    }
}
