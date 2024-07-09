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
#include "searchresultmodel.h"

namespace gams {
namespace studio {
namespace mii {

SearchResultModel::SearchResultModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

SearchResultModel::~SearchResultModel()
{

}

QVariant SearchResultModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole)
        return QVariant();
    if (index.row() >= mData.Entries.size())
        return QVariant();
    if (index.column() == 0) {
        return mData.Entries[index.row()].Index;
    }
    if (index.column() == 1) {
        return mData.Entries[index.row()].Orientation == Qt::Horizontal? "Horizontal" : "Vertical";
    }
    return QVariant();
}

void SearchResultModel::updateData(const SearchResult &data)
{
    beginResetModel();
    mData = data;
    endResetModel();
}

SearchResult::SearchEntry SearchResultModel::entry(int index)
{
    if (index >= mData.Entries.size())
        return SearchResult::SearchEntry();
    return mData.Entries.at(index);
}

QVariant SearchResultModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        return mHeaderData.at(section);
    }
    return QVariant();
}

int SearchResultModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mHeaderData.size();
}

int SearchResultModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mData.Entries.size();
}

}
}
}
