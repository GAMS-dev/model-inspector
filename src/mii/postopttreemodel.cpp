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
#include "postopttreemodel.h"
#include "postopttreeitem.h"
#include "abstractmodelinstance.h"

namespace gams {
namespace studio {
namespace mii {

PostoptTreeModel::PostoptTreeModel(int view,
                                   const QSharedPointer<AbstractModelInstance>& modelInstance,
                                   QObject *parent)
    : QAbstractItemModel(parent)
    , mModelInstance(modelInstance)
    , mRootItem(mModelInstance->dataTree(view))
    , mView(view)
{

}

PostoptTreeModel::~PostoptTreeModel()
{

}

QVariant PostoptTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role == Qt::DisplayRole) {
        auto *item = static_cast<PostoptTreeItem*>(index.internalPointer());
        return item->data(index.column());
    }
    if (role == Qt::TextAlignmentRole && index.column() > 0) {
        return Qt::AlignRight;
    }
    return QVariant();
}

Qt::ItemFlags PostoptTreeModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return Qt::NoItemFlags;
}

QVariant PostoptTreeModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return mRootItem->data(section);
    return QVariant();
}

QModelIndex PostoptTreeModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();
    PostoptTreeItem *parentItem = parent.isValid() ? static_cast<PostoptTreeItem*>(parent.internalPointer())
                                                   : mRootItem.get();
    PostoptTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex PostoptTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    auto *childItem = static_cast<PostoptTreeItem*>(index.internalPointer());
    auto *parentItem = childItem->parent();
    if (parentItem == mRootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int PostoptTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    PostoptTreeItem *parentItem;
    if (!parent.isValid())
        parentItem = mRootItem.get();
    else
        parentItem = static_cast<PostoptTreeItem*>(parent.internalPointer());
    return parentItem->rowCount();
}

int PostoptTreeModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mModelInstance->columnCount(mView);
}

}
}
}
