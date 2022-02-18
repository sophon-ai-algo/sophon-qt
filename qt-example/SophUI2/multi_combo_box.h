#ifndef MULTI_COMBO_BOX_H
#define MULTI_COMBO_BOX_H

#include "mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MultiComboBox; }
QT_END_NAMESPACE


class MultiComboBox : public QComboBox
{
    Q_OBJECT

public:
    MultiComboBox(QWidget *parent = Q_NULLPTR);
    ~MultiComboBox();

    // 添加item
    void AddItem(const QString& str, bool bChecked = false);
    void AddItems(const QList<ItemInfo>& lstItemInfo);
    void AddItems(const QMap<QString, bool>& mapStrChk);
    void AddItems(const QList<QString>& lstStr);
    // 删除item
    void RemoveItem(int idx);
    // 清空item
    void Clear();
    // 获取选中的数据字符串列表
    QStringList GetSelItemsText();
    // 获取选中item的信息
    QList<ItemInfo> GetSelItemsInfo();
    // 获取item文本
    QString GetItemText(int idx);
    // 获取item信息
    ItemInfo GetItemInfo(int idx);

signals:
    // popup显示信号
    void showingPopup();
    // popup隐藏信号
    void hidingPopup();

protected:
    void showPopup();
    // 重写QComboBox的hidePopup函数
    // 目的选择过程中，不隐藏listview
    void hidePopup();
    virtual void mousePressEvent(QMouseEvent * event);
    virtual void mouseReleaseEvent(QMouseEvent * event);
    virtual void mouseMoveEvent(QMouseEvent * event);

private:
    void UpdateText();

private slots:
    void sltActivated(int idx);

private:
    QLineEdit* pLineEdit;
    QListView* pListView;
    QStandardItemModel m_model;

};

#endif // MULTI_COMBO_BOX_H
