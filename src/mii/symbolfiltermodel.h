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
#ifndef SYMBOLFILTERMODEL_H
#define SYMBOLFILTERMODEL_H

#include <QSharedPointer>
#include <QSortFilterProxyModel>

#include "common.h"

namespace gams {
namespace studio {
namespace mii {

class AbstractModelInstance;
class AbstractViewConfiguration;
class Symbol;

class SymbolFilterModel final : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    SymbolFilterModel(const QSharedPointer<AbstractModelInstance> &modelInstance,
                      const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                      QObject *parent = nullptr);

    ~SymbolFilterModel();

    void evaluateFilters();

protected:
    bool filterAcceptsColumn(int sourceColumn,
                             const QModelIndex &sourceParent) const override;

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

private:
    bool evaluateColumnFilters();

    void evaluateColumnLabelFilters(bool isScalar, const QStringList &labels, int logicalSection);

    bool evaluateRowFilters();

    void evaluateRowLabelFilters(bool isScalar, const QStringList &labels, int logicalSection);

    void updateEntryCounts();

private:
    QSharedPointer<AbstractModelInstance> mModelInstance;
    QSharedPointer<AbstractViewConfiguration> mViewConfig;
    int mColumns;
    int mRows;
    int* mColumnStates = nullptr;
    int* mRowStates = nullptr;
    int* mColumnEntries = nullptr;
    int* mRowEntries = nullptr;
};

}
}
}

#endif // SYMBOLFILTERMODEL_H
