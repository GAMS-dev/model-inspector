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
#ifndef POSTOPTTREEMODEL_H
#define POSTOPTTREEMODEL_H

#include <QAbstractItemModel>

namespace gams {
namespace studio {
namespace mii {

class AbstractModelInstance;
class PostoptTreeItem;

class PostoptTreeModel final : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit PostoptTreeModel(int view,
                              const QSharedPointer<AbstractModelInstance>& modelInstance,
                              QObject *parent = nullptr);

    ~PostoptTreeModel() override;

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    QModelIndex parent(const QModelIndex &index) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

private:
    QSharedPointer<AbstractModelInstance> mModelInstance;
    QSharedPointer<PostoptTreeItem> mRootItem;
    int mView;
};

}
}
}

#endif // POSTOPTTREEMODEL_H
