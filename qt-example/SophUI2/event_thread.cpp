#include "mainwindow.h"
#include "./ui_mainwindow.h"

EventThread::EventThread(MainWindow * win)
{
    w = win;
    isStop = false;
    _icon = QImage(QString(":/resource/moshengren-icon.png"));
}

void EventThread::closeThread()
{
    isStop = true;
}
void EventThread::run()
{
    QString jstr;

    gettimeofday(&t_start, nullptr);

    while (1)
    {
        if(isStop)
            return;

  //      printf(" thead queue %d\n",ev_queue.size());
        mutex.lock();
        bool is_empty = ev_queue.isEmpty();
        mutex.unlock();
        if(is_empty)
        {
            usleep(1000 * 1);
            continue;
        }

        mutex.lock();
        jstr = ev_queue.head();
        ev_queue.dequeue();
        mutex.unlock();
        parse_event_data(jstr);
    }
}

void EventThread::show_event(QJsonObject rootobj)
{
    //QJsonValue identification_json = rootobj.value("identity_info");
    QString base64_header = "data:image/jpeg;base64,";


    QString device_id = rootobj.value("media_device_id").toString();
    QString identity_name = rootobj.value("identity_name").toString();
    QString user_type = rootobj.value("user_type").toString();
    QString recognize_face_image = rootobj.value("recognize_face_image").toString();
    QString register_image = rootobj.value("register_image").toString();
    DetectEvent * detect_ev = new DetectEvent();
    detect_ev->event_id = rootobj.value("event_id").toString();
    detect_ev->timestamp = rootobj.value("timestamp").toDouble();

    printf("%s\n",user_type.toStdString().data());

    //qDebug() << "\n\n" << identification_json << "--" << "\n\n";
    int bStranger = QString::compare("stranger",user_type);
    if(!bStranger)
        detect_ev->name = "陌生人";
    else
        detect_ev->name = identity_name;

    //qDebug() << user_reco.toObject().value("register_image").toString().size();
    if(register_image.size() == 0)
        detect_ev->d_pixmap = QPixmap::fromImage(_icon);
    else {
        QByteArray tmp_data = QByteArray::fromBase64(register_image.mid(base64_header.size()).toLatin1());
        detect_ev->d_pixmap.loadFromData(tmp_data);
    }

    QByteArray tmp_data = QByteArray::fromBase64(recognize_face_image.mid(base64_header.size()).toLatin1());

    //qDebug() << "face_img" << tmp_data;
    detect_ev->r_pixmap.loadFromData(tmp_data);
    //qDebug() << "event_id: " << detect_ev->event_id << " timestamp: " << detect_ev->timestamp;
    int i = 0;
    for(i = 0; i < w->current_preview_list.length(); ++i)
    {
        if(device_id == w->current_preview_list.at(i))
        {
            foreach(VideoInfo * _vinfo, w->vlist)
            {
                if(device_id == _vinfo->vid)
                {
                    if(_vinfo->type == "photo")
                    {
                        sig_notify(i, detect_ev->r_pixmap, _vinfo->name, _vinfo->type);
                    }
                    break;
                }
            }
        }
    }

    sig_update_event(detect_ev);
}
void EventThread::parse_event_data(QString recv_str)
{
    QString jstr;
    QJsonObject obj;
    QJsonParseError json_error;

    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_str.toUtf8(), &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" --- json error: %s\n",jstr.toStdString().c_str());
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        show_event(rootobj);
    }
}

void MainWindow::_update_event(DetectEvent * detect_ev)
{
    EventWidget * ew = new EventWidget();

    if((flayout->rowCount()) == 11)
    {
        int row_count = flayout->rowCount();
        QLayoutItem * li = flayout->itemAt(row_count);
        if(li)
        {
            EventWidget * _ew = qobject_cast<EventWidget *>(li->widget());
     //       DetectEvent * _ev = _ew->ev;
      //      delete _ev;
            delete _ew;
        }
        flayout->removeRow(row_count - 1);
    }
    //qDebug() << QDateTime::fromTime_t(detect_ev->timestamp).toString("hh:mm:ss");
    ew->t_label->setText(tr("<font style = 'font-size:25px; color:#FFFFFF;'> %1 </font><br/>").arg(detect_ev->name)
              + tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font><br/>").arg(QDateTime::fromTime_t(detect_ev->timestamp/1000).toString("hh:mm:ss"))
              + tr("<font style = 'font-size:20px; color:#FFFFFF;'>%1</font>").arg(QDateTime::fromTime_t(detect_ev->timestamp/1000).toString("yyyy-MM-dd")));
    ew->t_label->setAlignment(Qt::AlignTop);

    ew->r_pixmap = detect_ev->r_pixmap;
    ew->d_pixmap = detect_ev->d_pixmap;
    ew->r_label->setAlignment(Qt::AlignCenter);
    ew->d_label->setAlignment(Qt::AlignCenter);
    ew->r_label->setPixmap(ew->r_pixmap.scaled(84,84));
    ew->d_label->setPixmap(ew->d_pixmap.scaled(84,84));
    flayout->insertRow(0, ew);

    delete detect_ev;
    //qDebug() << "flayout row count " << flayout->rowCount();
}
