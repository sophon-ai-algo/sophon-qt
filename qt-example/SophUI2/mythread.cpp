#include "mainwindow.h"
#include "./ui_mainwindow.h"

EventWidget::EventWidget()
{
    hbox = new QHBoxLayout(this);
    d_label = new QLabel("null");

    r_label = new QLabel("null");

    t_label = new QLabel("111.111");
    hbox->addWidget(d_label);
    hbox->setStretchFactor(t_label, 1);
    hbox->addWidget(r_label);
    hbox->setStretchFactor(r_label, 1);
    hbox->addWidget(t_label);
    hbox->setStretchFactor(t_label, 6);

    hbox->setContentsMargins(2, 2, 2, 2);
    this->resize(480, 72);
}
EventWidget::~EventWidget()
{
}

MyThread::~MyThread()
{
#ifdef ARM64
    inMat.release();
#endif
}
MyThread::MyThread(MainWindow * win, int c)
{
    w = win;
    isStop = false;
    num = c;
#ifdef ARM64
    inMat = Mat(1080, 1920, CV_8UC3);
#endif
}

void MyThread::closeThread()
{
    isStop = true;
}


void MyThread::run()
{
    QString jstr;
    QJsonObject obj;
    QJsonParseError json_error;
    double wscale = 1;//640.0 / 1920.0;
    double hscale = 1;//360.0 / 1080.0;
    unsigned int count = 0;

    gettimeofday(&t_start, nullptr);

    while (!isStop)
    {

  //      printf(" thead queue %d\n",json_queue.size());
        if(json_queue.isEmpty())
        {
            usleep(1000 * 40);
            continue;
        }

        int queue_size = 0;
        mutex.lock();
        queue_size = json_queue.size();
        mutex.unlock();
        while(queue_size > 5)
        {
            qDebug() << "th" << num << "json_queu.size(): " << json_queue.size() << " -- dequeue 1";
            mutex.lock();
            json_queue.dequeue();
            queue_size = json_queue.size();
            mutex.unlock();
            msleep(10);
        }
        mutex.lock();
        jstr = json_queue.head();
        json_queue.dequeue();
        mutex.unlock();

        //qDebug() << jstr;
        //qDebug() << " ======================== ";
        QJsonDocument jsonDoc(QJsonDocument::fromJson(jstr.toUtf8(), &json_error));

        if(json_error.error != QJsonParseError::NoError)
        {
            printf(" --- json error: %s\n",jstr.toStdString().c_str());
            continue;
        }

        if(jsonDoc.isObject())
        {
            QJsonObject rootobj = jsonDoc.object();
            QJsonValue frame_data = rootobj.value("frame_img");
            QJsonValue frame_base64 = frame_data.toObject().value("data");
            int width = frame_data.toObject().value("width").toInt();
//            int height = frame_data.toObject().value("height").toInt();
            QString _tmp_str = frame_base64.toString();
            //qDebug() << _tmp_str;
            QByteArray tmp_data = QByteArray::fromBase64(_tmp_str.toLatin1());

            if(tmp_data.size() == 0)
                return;
#ifdef ARM64
            if(w->current_preview_list.size() == 1)
            {
                _InputArray arrSrc(tmp_data.data(), tmp_data.size());
                //Mat inMat = imdecode(arrSrc, IMREAD_COLOR);
                imdecode(arrSrc, IMREAD_COLOR, &inMat);
                QImage _image( inMat.data,
                              inMat.cols, inMat.rows,
                              static_cast<int>(inMat.step),
                              QImage::Format_BGR888 );
                image_pixmap = QPixmap::fromImage(_image);
            }
            else {
                image_pixmap.loadFromData(tmp_data);
            }
#else
            image_pixmap.loadFromData(tmp_data);
#endif
            QFont ft;
            ft.setPixelSize(width / 32);
            QPainter P(&image_pixmap);
            P.setRenderHint(QPainter::Antialiasing);
			//只预览一路的时候，如果分辨率过大，图片压缩后，可能看不到框，所以加粗框线条
            if(w->current_preview_list.size() == 1)
            {
                P.setPen(QPen(Qt::green, 4));
            }else {
                P.setPen(QPen(Qt::green, 2));
            }
            QJsonArray faces = rootobj.value("preview_objs").toArray();
            if(faces.size() > 0)
            {
                for(int i = 0;i < faces.size(); ++i)
                {
                    QJsonObject _obj = faces[i].toObject();
                    QJsonArray value = _obj.value("detect_box").toArray();
                    QRect rect;
//                    qDebug() << value;
                    QString name = _obj.value("identity_name").toString();
                    QString age = _obj.value("age").toString();
                    QString gender = _obj.value("gender").toString();
                    QString glass = _obj.value("glasses").toString();
                    //qDebug() << "name: " << name << "age: " << age << "gender: " << gender << "glasses: " << glass;

                    if(name == "")
                        name = "未知" + QString::number(_obj.value("track_id").toInt());


                    double x1 = value[0].toDouble() * wscale;
                    double y1 = value[1].toDouble() * hscale;
                    double x2 = value[2].toDouble() * wscale;
                    double y2 = value[3].toDouble() * hscale;
                    rect.setRect(int(x1), int(y1), int(x2 - x1), int(y2 - y1));
 //                   qDebug() << rect;

                    P.drawRect(rect);
                    double text_x = x1;
                    double text_y = y1 > 10 ? y1 - 10 : y1;

                    P.setFont(ft);
                    P.drawText(text_x, text_y, name);
                    if(!age.isEmpty())
                    {
                        QString face_attr;
                        text_y = y2 + ft.pixelSize();
						if(age == "child")
							age = "小孩";
						else if(age == "child")
							age = "小孩";
						else if(age == "juvenile")
							age = "少年";
						else if(age == "youth")
							age = "青年";
                        else if(age == "middle-aged")
                            age = "中年";
                        else
                            age = "老年";
                        face_attr = "年龄: " + age;
                        P.drawText(text_x, text_y, face_attr);
                        text_y += ft.pixelSize();
						if(gender == "male")
							gender = "男";
						else
							gender = "女";
                        face_attr = "性别: " + gender;
                        P.drawText(text_x, text_y, face_attr);
                        text_y += ft.pixelSize();
						if(glass == "no")
							glass = "否";
						else
							glass = "是";
                        face_attr = "眼镜: " + glass;
                        P.drawText(text_x, text_y, face_attr);
                    }
                }
            }
            QString vid = rootobj.value("video_id").toString();
            QString vname = "";
            foreach(VideoInfo * info, w->vlist)
            {
                if(info->vid == vid)
                    vname = info->name;
            }

            //qDebug() << vname << "vid: " << vid;

            count++;
            notify(num, image_pixmap, vname, "rtsp");
#if 1
            gettimeofday(&t_end, nullptr);
            if(t_end.tv_sec - t_start.tv_sec >= 1)
            {
                //printf("T%d: %d fps \n", num, count);
                gettimeofday(&t_start, nullptr);
                count = 0;
            }
#endif
        }
    }
}
