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
#ifndef SEARCHRESULTMODEL_H
#define SEARCHRESULTMODEL_H

#include <QAbstractTableModel>

#include "common.h"

namespace gams {
namespace studio {
namespace mii {

class SearchResultModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    SearchResultModel(QObject *parent = nullptr);
    ~SearchResultModel() override;

    QVariant data(const QModelIndex &index,
                  int role = Qt::DisplayRole) const override;

    void updateData(const SearchResult &data);

    SearchResult::SearchEntry entry(int index);

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    const QStringList mHeaderData { "Index", "Orientation"};
    SearchResult mData;
};

}
}
}

#endif // SEARCHRESULTMODEL_H
