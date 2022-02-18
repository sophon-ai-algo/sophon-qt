#include "mainwindow.h"
#include "./ui_mainwindow.h"

VL_Dialog::VL_Dialog(MainWindow * win)
{
    main_win = win;
    this->setWindowOpacity(0.9);
    this->setObjectName("channel");
    this->setStyleSheet("QDialog#channel{border-image:url(:/resource/channel_bg.png);}");
    this->setWindowFlag(Qt::FramelessWindowHint);
    this->setGeometry(0, 86, 318, 843);
#if 0
    cboxs_layout = new QGridLayout(this);
    cboxs_layout->setContentsMargins(10, 30, 10, 30);
    cboxs_layout->setHorizontalSpacing(1);
    cboxs_layout->setVerticalSpacing(1);
#endif
    ch_list = new QListWidget(this);
    ch_list->setObjectName("ch_list");
    ch_list->resize(318, 650);
    ch_list->move(10,53);
    ch_list->setStyleSheet("background-color:rgba(0,0,0,0);");
    connect(ch_list, SIGNAL(itemClicked(QListWidgetItem *)), this, SLOT(itemclick(QListWidgetItem *)));

    QPushButton * close_button;
    QPushButton * enter_button;

    enter_button = new QPushButton("", this);
    close_button = new QPushButton("", this);
    enter_button->setIconSize(QSize(72,28));
    close_button->setIconSize(QSize(72,28));
    enter_button->setIcon(QIcon(":/resource/button-xuanze.png"));
    close_button->setIcon(QIcon(":/resource/button-quxiao.png"));
    enter_button->setFocusPolicy(Qt::NoFocus);
    close_button->setFocusPolicy(Qt::NoFocus);
    enter_button->setStyleSheet("background-color:rgb(17,155,197);");
    close_button->setStyleSheet("background-color:rgb(17,155,197);");
    connect(enter_button, SIGNAL(clicked()), this, SLOT(enter_click_cb()));
    connect(close_button, SIGNAL(clicked()), this, SLOT(close_click_cb()));

    enter_button->move(44,800);
    close_button->move(204,800);
}

VL_Dialog::~VL_Dialog()
{

}

void VL_Dialog::itemclick(QListWidgetItem * item)
{
    QVariant check = item->data(Qt::UserRole);
    bool _check = check.toBool();
    if(_check)
    {
        item->setData(Qt::UserRole, false);
        item->setIcon(QIcon(":/resource/Checkbox-weixuanze.png"));
    }
    else
    {
        item->setData(Qt::UserRole, true);
        item->setIcon(QIcon(":/resource/Checkbox-xuanze.png"));
    }
}


void VL_Dialog::fill_channels()
{
    QFont ft;
    ft.setPixelSize(30);
#if 0
    for(int i = 0; i < 16; ++i)
    {
        QListWidgetItem * item = new QListWidgetItem;
        item->setSizeHint(QSize(300, 37));
        item->setData(Qt::DisplayRole, QString::number(i));
        item->setData(Qt::CheckStateRole, Qt::Checked);
        ch_list->addItem(item);
    }
#endif
#if 1
    foreach(VideoInfo * vinfo, main_win->vlist)
    {
        QListWidgetItem * item = new QListWidgetItem;
        item->setSizeHint(QSize(300, 37));
        item->setData(Qt::DisplayRole, vinfo->name);
        item->setIcon(QIcon(":/resource/Checkbox-weixuanze.png"));
        item->setData(Qt::UserRole, false);
        item->setFont(ft);

        for(int i = 0;i < main_win->total_preview_list.size(); ++i)
        {
            if(vinfo->vid == main_win->total_preview_list.at(i))
            {
                item->setIcon(QIcon(":/resource/Checkbox-xuanze.png"));
                item->setData(Qt::UserRole, true);
            }
        }
        ch_list->addItem(item);
        qDebug() << "add checkbox " << vinfo->name;
    }
#endif
}
void VL_Dialog::enter_click_cb()
{
    main_win->restart_flag = false;
    main_win->total_preview_list.clear();

    for(int i = 0; i < ch_list->count(); ++i)
    {
        QListWidgetItem * _item = ch_list->item(i);
        QVariant check = _item->data(Qt::UserRole);
        QString item_str = _item->data(Qt::DisplayRole).toString();
        if(check.toBool())
        {
            VideoInfo * vinfo = main_win->vlist.at(i);
            main_win->total_preview_list.append(vinfo->vid);
        }
    }
    main_win->preview_start_index = 0;
    main_win->set_current_preview_list(main_win->preview_start_index);

    qDebug() << "rebuild total_preview_list: " << main_win->total_preview_list;
    qDebug() << "rebuild current_preview_list: " << main_win->current_preview_list;

    ch_list->clear();

    main_win->stop_video_thread();
    main_win->clear_preview_labels();
    main_win->set_preview_video_list();
    main_win->set_current_preview_list(0);
    main_win->create_preview_labels();
    main_win->create_video_thread();
    main_win->restart_flag = true;
    main_win->send_event_config();
    this->hide();
}
void VL_Dialog::close_click_cb()
{
    ch_list->clear();
    this->hide();
}
