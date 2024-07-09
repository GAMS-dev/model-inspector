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
#ifndef FILTERTREEMODEL_H
#define FILTERTREEMODEL_H

#include <QAbstractItemModel>
#include <QSortFilterProxyModel>

namespace gams {
namespace studio {
namespace mii {

class FilterTreeItem;

class AggregationTreeItemFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    AggregationTreeItemFilterProxyModel(QObject *parent = nullptr);

protected:
    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;
};

class FilterTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit FilterTreeModel(FilterTreeItem *rootItem, QObject *parent = nullptr);
    ~FilterTreeModel();

    FilterTreeItem* filterItem() const;

    QVariant data(const QModelIndex &index, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value,
                 int role = Qt::EditRole) override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    void updateParents(const QModelIndex &currentIndex,
                       const QVector<int> &roles = QVector<int>());
    void updateChilds(const QModelIndex &currentIndex,
                      const QVector<int> &roles = QVector<int>());

private:
    FilterTreeItem *mRootItem = nullptr;
};

}
}
}

#endif // FILTERTREEMODEL_H
