/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd
*
* Author:      maojj <maojunjie@uniontech.com>
* Maintainer:  maojj <maojunjie@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#include "mem_summary_view_widget.h"
#include "system/device_db.h"
#include "system/mem.h"
#include "common/common.h"
#include "base/base_detail_item_delegate.h"

#include <QHeaderView>
#include <DApplicationHelper>
#include <QAbstractTableModel>
#include <QStyledItemDelegate>
#include <QPainter>
#include <DApplication>
#include <DStyle>

using namespace core::system;
using namespace common::format;

class DeailTableModel : public QAbstractTableModel
{
public:
    explicit DeailTableModel(QObject *parent = nullptr);
    virtual ~DeailTableModel();

private:
    MemInfo *m_memInfo;

protected:
    int rowCount(const QModelIndex &) const
    {
        return 7;
    }

    int columnCount(const QModelIndex &) const
    {
        return 2;
    }

    QVariant data(const QModelIndex &index, int role) const
    {
        if (!index.isValid())
            return QVariant();

        int row = index.row();
        int column = index.column();

        if (role == Qt::DisplayRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Used");
                else if (column == 1)
                    return QApplication::translate("DeailTableModel", "Available");
                break;
            case 1:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Shared");
                else if (column == 1)
                    return QApplication::translate("DeailTableModel", "Cached");
                break;
            case 2:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Buffers");
                else if (column == 1)
                    return QApplication::translate("DeailTableModel", "Cached swap");
                break;
            case 3:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Active");
                else if (column == 1)
                    return QApplication::translate("DeailTableModel", "Inactive");
                break;
            case 4:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Dirty");
                else if (column == 1)
                    return QApplication::translate("DeailTableModel", "Mapped");
                break;
            case 5:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Total swap");
                else if (column == 1)
                    return QApplication::translate("DeailTableModel", "Free swap");
                break;
            case 6:
                if (column == 0)
                    return QApplication::translate("DeailTableModel", "Slab");
                break;
            default:
                break;
            }
        } else if (role == Qt::UserRole) {
            switch (row) {
            case 0:
                if (column == 0)
                    return formatUnit((m_memInfo->memTotal() - m_memInfo->memAvailable())  << 10, B, 1);
                else if (column == 1)
                    return formatUnit(m_memInfo->memAvailable() << 10, B, 1);
                break;
            case 1:
                if (column == 0)
                    return formatUnit(m_memInfo->shmem() << 10, B, 1);
                else if (column == 1)
                    return formatUnit(m_memInfo->cached() << 10, B, 1);
                break;
            case 2:
                if (column == 0)
                    return formatUnit(m_memInfo->buffers() << 10, B, 1);
                else if (column == 1)
                    return formatUnit(m_memInfo->swapCached() << 10, B, 1);
                break;
            case 3:
                if (column == 0)
                    return formatUnit(m_memInfo->active() << 10, B, 1);
                else if (column == 1)
                    return formatUnit(m_memInfo->inactive() << 10, B, 1);
                break;
            case 4:
                if (column == 0)
                    return formatUnit(m_memInfo->dirty() << 10, B, 1);
                else if (column == 1)
                    return formatUnit(m_memInfo->mapped() << 10, B, 1);
                break;
            case 5:
                if (column == 0)
                    return formatUnit(m_memInfo->swapTotal() << 10, B, 1);
                else if (column == 1)
                    return formatUnit(m_memInfo->swapFree() << 10, B, 1);
                break;
            case 6:
                if (column == 0)
                    return formatUnit(m_memInfo->slab() << 10, B, 1);
                break;
            default:
                break;
            }
        } else if (role == Qt::TextColorRole) {
            const auto &palette = DApplicationHelper::instance()->applicationPalette();
            return palette.color(DPalette::Text);
        }

        return QVariant();
    }

    Qt::ItemFlags flags(const QModelIndex &) const
    {
        return Qt::NoItemFlags;
    }
};

DeailTableModel::DeailTableModel(QObject *parent): QAbstractTableModel(parent)
{
    m_memInfo = DeviceDB::instance()->memInfo();
}

DeailTableModel::~DeailTableModel()
{

}

MemSummaryViewWidget::MemSummaryViewWidget(QWidget *parent)
    : DTableView(parent)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    this->horizontalHeader()->setVisible(false);
    this->verticalHeader()->setVisible(false);

    this->horizontalHeader()->setStretchLastSection(true);

    this->horizontalHeader()->setSectionResizeMode(DHeaderView::Stretch);
    this->verticalHeader()->setSectionResizeMode(DHeaderView::ResizeToContents);
    this->setGridStyle(Qt::NoPen);
    this->setFrameShape(QFrame::NoFrame);

    m_model = new DeailTableModel(this);
    this->setModel(m_model);
    this->setItemDelegate(new BaseDetailItemDelegate(this));

    setFixedHeight(260);
}

void MemSummaryViewWidget::onModelUpdate()
{
    this->viewport()->update();
}

void MemSummaryViewWidget::fontChanged(const QFont &font)
{
    m_font = font;
    this->setFont(m_font);
}

void MemSummaryViewWidget::paintEvent(QPaintEvent *event)
{
    DTableView::paintEvent(event);

    QPainter painter(this->viewport());
    painter.setRenderHint(QPainter::Antialiasing, true);
    const auto &palette = DApplicationHelper::instance()->applicationPalette();
    QColor frameColor = palette.color(DPalette::TextTips);
    frameColor.setAlphaF(0.3);

    painter.setPen(QPen(frameColor, 1, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
    painter.setBrush(Qt::NoBrush);
    painter.drawLine(this->horizontalHeader()->sectionSize(0) - 1, 2, this->horizontalHeader()->sectionSize(0) - 1, this->viewport()->height() - 2);
    painter.drawRoundedRect(this->rect().adjusted(1, 1, -1, -1), 6, 6);
}

