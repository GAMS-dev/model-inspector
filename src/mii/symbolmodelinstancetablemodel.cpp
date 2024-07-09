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
#include "symbolmodelinstancetablemodel.h"
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"

#include <QFont>

namespace gams {
namespace studio{
namespace mii {

SymbolModelInstanceTableModel::SymbolModelInstanceTableModel(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                                             const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                                             QObject *parent)
    : QAbstractTableModel(parent)
    , mModelInstance(modelInstance)
    , mViewConfig(viewConfig)
{

}

void SymbolModelInstanceTableModel::setModelInstance(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    beginResetModel();
    mModelInstance = modelInstance;
    endResetModel();
}

QVariant SymbolModelInstanceTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::FontRole) {
        if (mModelInstance->nlFlag(index.row(), index.column(), mViewConfig->viewId())) {
            QFont font;
            font.setBold(true);
            font.setItalic(true);
            return font;
        }
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        return mModelInstance->data(index.row(), index.column(), mViewConfig->viewId());
    }
    if (role == ViewHelper::ColumnEntryRole) {
        return mModelInstance->columnEntryCount(index.column(), mViewConfig->viewId());
    }
    if (role == ViewHelper::RowEntryRole) {
        return mModelInstance->rowEntryCount(index.row(), mViewConfig->viewId());
    }
    return QVariant();
}

Qt::ItemFlags SymbolModelInstanceTableModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return QAbstractTableModel::flags(index) |
                Qt::ItemIsEnabled |
                Qt::ItemIsSelectable;
    return Qt::NoItemFlags;
}

QVariant SymbolModelInstanceTableModel::headerData(int section,
                                                   Qt::Orientation orientation,
                                                   int role) const
{
    if (role == Qt::DisplayRole) {
        auto realIndex = mModelInstance->headerData(section, orientation,
                                                    mViewConfig->viewId(),
                                                    ViewHelper::IndexDataRole).toInt();
        return realIndex < 0 ? QVariant() : realIndex;
    }
    if (role == ViewHelper::DimensionRole) {
        return mModelInstance->maxSymbolDimension(mViewConfig->viewId(), orientation);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex SymbolModelInstanceTableModel::index(int row,
                                                 int column,
                                                 const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return createIndex(row, column);
    return QModelIndex();
}

int SymbolModelInstanceTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mModelInstance->rowCount(mViewConfig->viewId());
}

int SymbolModelInstanceTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mModelInstance->columnCount(mViewConfig->viewId());
}

QHash<int, QByteArray> SymbolModelInstanceTableModel::roleNames() const
{
    return ViewHelper::roleNames();
}

}
}
}
