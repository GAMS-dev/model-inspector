/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023-2024 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023-2024 GAMS Development Corp. <support@gams.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "postopttreeview.h"
#include "postopttreeitem.h"
#include "common.h"

#include <QEvent>
#include <QMenu>
#include <QWheelEvent>

namespace gams {
namespace studio{
namespace mii {

PostoptTreeView::PostoptTreeView(QWidget *parent)
    : QTreeView(parent)
    , mMenu(new QMenu(this))
    , mCollapsAllAction(new QAction("Collapse All", this))
    , mExpandAllAction(new QAction("Expand All", this))
{
    mMenu->addAction(mCollapsAllAction);
    mMenu->addAction(mExpandAllAction);

    connect(this, &PostoptTreeView::customContextMenuRequested,
            this, &PostoptTreeView::showCustomContextMenu);
    connect(mCollapsAllAction, &QAction::triggered,
            this, &QTreeView::collapseAll);
    connect(mExpandAllAction, &QAction::triggered,
            this, &QTreeView::expandAll);
}

bool PostoptTreeView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheel = static_cast<QWheelEvent*>(event);
        if (wheel->modifiers() == Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0)
                zoomIn(ViewHelper::ZoomFactor);
            else
                zoomOut(ViewHelper::ZoomFactor);
            return true;
        }
    }
    return QTreeView::eventFilter(watched, event);
}

void PostoptTreeView::zoomIn(int range)
{
    zoom(range);
    resizeColumns();
}

void PostoptTreeView::zoomOut(int range)
{
    zoom(-range);
    resizeColumns();
}

void PostoptTreeView::resetZoom()
{
    setFont(mBaseFont);
    resizeColumns();
}

void PostoptTreeView::showCustomContextMenu(const QPoint &pos)
{
    auto index = indexAt(pos);
    if (!index.isValid()) return;
    mMenu->popup(viewport()->mapToGlobal(pos));
}

void PostoptTreeView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() != Qt::LeftButton) {
        QTreeView::mousePressEvent(event);
        return;
    }
    auto idx = indexAt(event->pos());
    if (idx.isValid()) {
        auto item = static_cast<PostoptTreeItem*>(idx.internalPointer());
        if (item->type() == PostoptTreeItem::ClickItem) {
            emit openFilterDialog();
        }
    }
    QTreeView::mousePressEvent(event);
}

void PostoptTreeView::zoom(int range)
{
    if (range == 0)
        return;
    QFont f = font();
    auto newSize = f.pointSizeF() + range;
    if (newSize <= 0)
        return;
    f.setPointSizeF(newSize);
    setFont(f);
    resizeColumns();
}

void PostoptTreeView::resizeColumns()
{
    for (int i=0; i<model()->columnCount(); ++i) {
        resizeColumnToContents(i);
    }
}

}
}
}
