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
#ifndef SYMBOLMODELINSTANCETABLEMODEL_H
#define SYMBOLMODELINSTANCETABLEMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

namespace gams {
namespace studio{
namespace mii {

class AbstractModelInstance;
class AbstractViewConfiguration;

class SymbolModelInstanceTableModel final : public QAbstractTableModel
{
    Q_OBJECT

public:
    SymbolModelInstanceTableModel(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                  const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                  QObject *parent = nullptr);

    void setModelInstance(const QSharedPointer<AbstractModelInstance> &modelInstance);

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

private:
    QSharedPointer<AbstractModelInstance> mModelInstance;
    QSharedPointer<AbstractViewConfiguration> mViewConfig;
};

}
}
}

#endif // SYMBOLMODELINSTANCETABLEMODEL_H
