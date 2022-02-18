#include "multi_combo_box.h"


MultiComboBox::MultiComboBox(QWidget *parent)
    : QComboBox(parent)
{
    pLineEdit = new QLineEdit(this);
    pLineEdit->setReadOnly(true);
    this->setLineEdit(pLineEdit);
    this->lineEdit()->disconnect();

    pListView = new QListView(this);
    this->setView(pListView);

    this->setModel(&m_model);

    connect(this, SIGNAL(activated(int)), this, SLOT(sltActivated(int)));
}

MultiComboBox::~MultiComboBox()
{
}

void MultiComboBox::AddItem(const QString& str, bool bChecked /*= false*/)
{
    QStandardItem* item = new QStandardItem(str);
    QFont ft;
    ft.setPointSize(80);
    item->setFont(ft);
    item->setCheckable(true);
    item->setCheckState(bChecked ? Qt::Checked : Qt::Unchecked);
    m_model.appendRow(item);

    UpdateText();
}

void MultiComboBox::AddItems(const QList<ItemInfo>& lstItemInfo)
{
    for (auto a : lstItemInfo)
    {
        AddItem(a.str, a.bChecked);
    }
}

void MultiComboBox::AddItems(const QMap<QString, bool>& mapStrChk)
{
    for (auto it = mapStrChk.begin(); it != mapStrChk.end(); ++it)
    {
        AddItem(it.key(), it.value());
    }
}

void MultiComboBox::AddItems(const QList<QString>& lstStr)
{
    for (auto a : lstStr)
    {
        AddItem(a, false);
    }
}

void MultiComboBox::RemoveItem(int idx)
{
    m_model.removeRow(idx);
    UpdateText();
}

void MultiComboBox::Clear()
{
    m_model.clear();
    UpdateText();
}

QStringList MultiComboBox::GetSelItemsText()
{
    QStringList lst;
    QString str = pLineEdit->text();
    if (str.isEmpty())
    {
        return lst;
    }
    else
    {
        return pLineEdit->text().split(",");
    }
}

QList<ItemInfo> MultiComboBox::GetSelItemsInfo()
{
    QList<ItemInfo> lstInfo;
    for (int i = 0; i < m_model.rowCount(); i++)
    {
        QStandardItem* item = m_model.item(i);
        if (item->checkState() == Qt::Unchecked) continue;

        ItemInfo info;
        info.idx = i;
        info.str = item->text();
        info.bChecked = true;

        lstInfo << info;
    }

    return lstInfo;
}

QString MultiComboBox::GetItemText(int idx)
{
    if (idx < 0 || idx >= m_model.rowCount())
    {
        return QString("");
    }

    return m_model.item(idx)->text();
}

ItemInfo MultiComboBox::GetItemInfo(int idx)
{
    ItemInfo info;
    if (idx < 0 || idx >= m_model.rowCount())
    {
        return info;
    }

    QStandardItem* item = m_model.item(idx);
    info.idx = idx;
    info.str = item->text();
    info.bChecked = (item->checkState() == Qt::Checked);

    return info;
}

void MultiComboBox::showPopup()
{
    emit showingPopup();
    QComboBox::showPopup();
}

void MultiComboBox::hidePopup()
{
    int width = this->view()->width();
    int height = this->view()->height();
    int x = QCursor::pos().x() - mapToGlobal(geometry().topLeft()).x() + geometry().x();
    int y = QCursor::pos().y() - mapToGlobal(geometry().topLeft()).y() + geometry().y();

    QRect rectView(0, this->height(), width, height);
    if (!rectView.contains(x, y))
    {
        emit hidingPopup();
        QComboBox::hidePopup();
    }
}

void MultiComboBox::mousePressEvent(QMouseEvent * event)
{
    QComboBox::mousePressEvent(event);
    event->accept();
}

void MultiComboBox::mouseReleaseEvent(QMouseEvent * event)
{
    QComboBox::mouseReleaseEvent(event);
    event->accept();
}

void MultiComboBox::mouseMoveEvent(QMouseEvent * event)
{
    QComboBox::mouseMoveEvent(event);
    event->accept();
}

void MultiComboBox::UpdateText()
{
    QStringList lstTxt;
    for (int i = 0; i < m_model.rowCount(); ++i)
    {
        QStandardItem* item = m_model.item(i);
        if (item->checkState() == Qt::Unchecked) continue;

        lstTxt << item->text();
    }

    pLineEdit->setText(lstTxt.join(","));
    pLineEdit->setToolTip(lstTxt.join("\n"));
}

void MultiComboBox::sltActivated(int idx)
{
    QStandardItem* item = m_model.item(idx);
    if (nullptr == item) return;

    Qt::CheckState state = (item->checkState() == Qt::Checked) ? Qt::Unchecked : Qt::Checked;
    item->setCheckState(state);

    UpdateText();
}
