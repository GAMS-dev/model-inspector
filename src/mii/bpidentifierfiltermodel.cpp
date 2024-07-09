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
#include "bpidentifierfiltermodel.h"
#include "abstractmodelinstance.h"

namespace gams {
namespace studio{
namespace mii {

BPIdentifierFilterModel::BPIdentifierFilterModel(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                                 QObject *parent)
    : QSortFilterProxyModel(parent)
    , mModelInstance(modelInstance)
{

}

void BPIdentifierFilterModel::setIdentifierFilter(const IdentifierFilter &filter)
{
    mIdentifierFilter = filter;
    invalidateFilter();
}

bool BPIdentifierFilterModel::filterAcceptsColumn(int sourceColumn,
                                                  const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    bool ok = false;
    auto sectionIndex = sourceModel()->headerData(sourceColumn, Qt::Horizontal,
                                                  ViewHelper::IndexDataRole).toInt(&ok);
    if (!ok || sectionIndex < 0)
        return true;
    for (auto iter=mIdentifierFilter[Qt::Horizontal].constBegin();
         iter!=mIdentifierFilter[Qt::Horizontal].constEnd(); ++iter) {
        if (iter->Checked == Qt::Checked) {
            continue;
        }
        auto variable = mModelInstance->variable(sectionIndex);
        if (iter->Text == variable->name()) {
            return false;
        }
    }
    return true;
}

bool BPIdentifierFilterModel::filterAcceptsRow(int sourceRow,
                                               const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);

    bool ok = false;
    auto sectionIndex = sourceModel()->headerData(sourceRow, Qt::Vertical,
                                                  ViewHelper::IndexDataRole).toInt(&ok);
    if (!ok || sectionIndex < 0)
        return true;
    for (auto iter=mIdentifierFilter[Qt::Vertical].constBegin();
         iter!=mIdentifierFilter[Qt::Vertical].constEnd(); ++iter) {
        if (iter->Checked == Qt::Checked) {
            continue;
        }
        auto equation = mModelInstance->equation(sectionIndex);
        if (iter->Text == equation->name()) {
            return false;
        }
    }
    return true;
}

}
}
}
