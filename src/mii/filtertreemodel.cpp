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
#include "filtertreemodel.h"
#include "filtertreeitem.h"

namespace gams {
namespace studio {
namespace mii {

AggregationTreeItemFilterProxyModel::AggregationTreeItemFilterProxyModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{

}

bool AggregationTreeItemFilterProxyModel::filterAcceptsRow(int sourceRow,
                                                           const QModelIndex &sourceParent) const
{
    auto sourceIndex = sourceModel()->index(sourceRow, 0, sourceParent);
    if (!sourceIndex.isValid()) return false;
    auto item = static_cast<FilterTreeItem*>(sourceIndex.internalPointer());
    if (!item->isVisible())
        return false;
    if (item->isCheckable() && QSortFilterProxyModel::filterAcceptsRow(sourceParent.row(), sourceParent.parent()))
        return true;
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

FilterTreeModel::FilterTreeModel(FilterTreeItem *rootItem, QObject *parent)
    : QAbstractItemModel(parent)
    , mRootItem(rootItem)
{

}

FilterTreeModel::~FilterTreeModel()
{
    delete mRootItem;
}

FilterTreeItem* FilterTreeModel::filterItem() const
{
    return mRootItem;
}

QVariant FilterTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    auto item = static_cast<FilterTreeItem*>(index.internalPointer());
    if (role == Qt::DisplayRole) {
        return item->text();
    }
    if (role == Qt::CheckStateRole && index.column() == 0 && item->isCheckable()) {
        return item->checked();
    }
    return QVariant();
}

bool FilterTreeModel::setData(const QModelIndex &index, const QVariant &value,
                              int role)
{
    if (role == Qt::CheckStateRole && index.column() == 0) {
        auto item = static_cast<FilterTreeItem*>(index.internalPointer());
        if (item->isEnabled() && item->isCheckable()) {
            item->setChecked(value.toBool() ? Qt::Checked : Qt::Unchecked);
            item->setSubTreeState(value.toBool() ? Qt::Checked : Qt::Unchecked);
            updateParents(index, { Qt::CheckStateRole });
            updateChilds(index, { Qt::CheckStateRole });
        }
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags FilterTreeModel::flags(const QModelIndex &index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    auto item = static_cast<FilterTreeItem*>(index.internalPointer());
    if (!item->isEnabled())
        return Qt::NoItemFlags;
    return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
}

QModelIndex FilterTreeModel::index(int row, int column,
                                   const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    FilterTreeItem *item;
    if (!parent.isValid())
        item = mRootItem;
    else
        item = static_cast<FilterTreeItem*>(parent.internalPointer());
    auto child = item->child(row);
    if (child)
        return createIndex(row, column, child);
    return QModelIndex();
}

QModelIndex FilterTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    auto child = static_cast<FilterTreeItem*>(index.internalPointer());
    auto item = child->parent();
    if (item == mRootItem)
        return QModelIndex();
    return createIndex(item->row(), 0, item);
}

int FilterTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    FilterTreeItem *item;
    if (!parent.isValid())
        item = mRootItem;
    else
        item = static_cast<FilterTreeItem*>(parent.internalPointer());
    return item->rowCount();
}

int FilterTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<FilterTreeItem*>(parent.internalPointer())->columnCount();
    return mRootItem->columnCount();
}

void FilterTreeModel::updateParents(const QModelIndex &currentIndex,
                                    const QVector<int> &roles)
{
    auto parent = currentIndex;
    while (parent.isValid()) {
        emit dataChanged(parent, parent, roles);
        parent = parent.parent();
    }
}

void FilterTreeModel::updateChilds(const QModelIndex &currentIndex,
                                   const QVector<int> &roles)
{
    if (!hasChildren(currentIndex))
        return;
    QModelIndexList indexes { currentIndex };
    while (!indexes.isEmpty()) {
        auto idx = indexes.takeFirst();
        emit dataChanged(idx, idx, roles);
        if (!hasChildren(idx))
            continue;
        for (int r=0; r<rowCount(idx); ++r) {
            indexes.append(this->index(r, 0, idx));
        }
    }
}

}
}
}
