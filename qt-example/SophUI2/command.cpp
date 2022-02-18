#include "mainwindow.h"
#include "./ui_mainwindow.h"

void MainWindow::get_preview_info()
{
    QEventLoop eventloop;
    QString cmd = face_api_url + "previewinfo";
    qDebug() << cmd;
    request.setRawHeader("Content-Type","application/json");
    request.setUrl(cmd);

    QByteArray stData;

    reply = manager->get(request);
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventloop, SLOT(quit()));
    eventloop.exec();

    QByteArray data = reply->readAll();

    QString recv_str = data.data();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_str.toUtf8(), &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        QJsonObject preview_info_json = rootobj.value("data").toObject();
        QJsonObject preview_port_list_json = preview_info_json.value("preview_port_list").toObject();
        if(!preview_info_json.isEmpty())
        {
            preview_port = preview_port_list_json.keys().at(0).toInt();
            qDebug() << "preview_port " << preview_port;
            preview_suffic = preview_info_json.value("preview_string").toString();
            qDebug() << "preview_suffic " << preview_suffic;
        }
    }
}

void MainWindow::get_preview_video_list()
{
    QEventLoop eventloop;
    QJsonObject request_json_obj;
    QByteArray postData;
    postData.append(QJsonDocument(request_json_obj).toJson());
    QString cmd = device_api_url + "getpreview";
    request.setRawHeader("Content-Type","application/json");
    request.setUrl(cmd);
    qDebug() << cmd;
    reply = manager->post(request,postData);
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventloop, SLOT(quit()));
    eventloop.exec();

    total_preview_list.clear();
    QByteArray data = reply->readAll();
    qDebug() << data;

    QString recv_str = data.data();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_str.toUtf8(), &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        QJsonArray preview_list_json = rootobj.value("selected_device_list").toArray();
        //qDebug() << "preview_list_json size: " << preview_list_json.size();
        for(int i = 0; i < preview_list_json.size(); ++i)
        {
            total_preview_list.append(preview_list_json.at(i).toString());
        }
        qDebug() << "total_preview_list: " << total_preview_list;
        QJsonArray current_preview_list_json = rootobj.value("preview_device_list").toArray();
        //qDebug() << "current_preview_list_json size: " << current_preview_list_json.size();
        current_preview_list.clear();
        for(int i = 0; i < current_preview_list_json.size(); ++i)
        {
            current_preview_list.append(current_preview_list_json.at(i).toString());
        }
        qDebug() << "current_preview_list: " << current_preview_list;
    }
}

void MainWindow::get_device_list()
{
    QEventLoop eventloop;
    QJsonObject request_json_obj;
    QByteArray postData;
    postData.append(QJsonDocument(request_json_obj).toJson());
    QString cmd = device_api_url + "getmediadevices";
    request.setRawHeader("Content-Type","application/json");
    request.setUrl(cmd);
    qDebug() << cmd;
    reply = manager->post(request,postData);
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventloop, SLOT(quit()));
    eventloop.exec();

    QByteArray data = reply->readAll();
    //qDebug() << data;
    _parse_device_list(data.data());

#if 1
    QStringList vids;
    for(int i = 0;i < vlist.size(); ++i)
    {
        VideoInfo * _vinfo = vlist.at(i);
        vids << _vinfo->name;
    }
    qDebug() << vids;
#endif
}

void MainWindow::_parse_device_list(QString recv_str)
{
    QJsonParseError json_error;

    QJsonDocument jsonDoc(QJsonDocument::fromJson(recv_str.toUtf8(), &json_error));

    if(json_error.error != QJsonParseError::NoError)
    {
        printf(" --- json error: %s\n",recv_str.toStdString().c_str());
        return;
    }

    if(jsonDoc.isObject())
    {
        QJsonObject rootobj = jsonDoc.object();
        QJsonArray device_list_json = rootobj.value("devices").toArray();
        //qDebug() << device_list_json;
        vlist.clear();
        for(int i = 0; i < device_list_json.size(); ++i)
        {
            QJsonObject _obj = device_list_json.at(i).toObject();
            VideoInfo * vinfo = new VideoInfo();
            vinfo->vid = _obj.value("media_device_id").toString();
            vinfo->name = _obj.value("media_device_name").toString();
            vinfo->url = _obj.value("media_device_key").toString();
            vinfo->type = _obj.value("media_device_type").toString();
            vlist.append(vinfo);
        }
    }
    qDebug() << vlist;
}

void MainWindow::set_preview_video_list()
{
    QEventLoop eventloop;
    QString cmd = device_api_url + "updatepreview";
    QJsonArray plist_array = QJsonArray::fromStringList(total_preview_list);
    QJsonArray cplist_array = QJsonArray::fromStringList(current_preview_list);

    QJsonObject preview_list_obj;
    preview_list_obj.insert("selected_device_list", plist_array);
    preview_list_obj.insert("preview_device_list", cplist_array);
    preview_list_obj.insert("preview_mode", QString::number(current_matrix_type));
    preview_list_obj.insert("last_set_name", "QT");

    QByteArray data = QJsonDocument(preview_list_obj).toJson();
    qDebug() << data;

    request.setRawHeader("Content-Type","application/json");
    request.setUrl(cmd);
    reply = manager->post(request, data);
    connect(manager, SIGNAL(finished(QNetworkReply*)), &eventloop, SLOT(quit()));
    eventloop.exec();
}
