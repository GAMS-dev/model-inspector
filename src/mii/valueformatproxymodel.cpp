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
#include "valueformatproxymodel.h"

namespace gams {
namespace studio {
namespace mii {

ValueFormatProxyModel::ValueFormatProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{

}

void ValueFormatProxyModel::setValueFilter(const ValueFilter &valueFilter)
{
    beginResetModel();
    mValueFilter = valueFilter;
    endResetModel();
}

QVariant ValueFormatProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole)
        return sourceModel()->data(index, role);
    bool ok = false;
    double value = QIdentityProxyModel::data(index, role).toDouble(&ok);
    if (ok) {
        if (!mValueFilter.ExcludeRange && value >= mValueFilter.MinValue && value <= mValueFilter.MaxValue)
            return value;
        else if (mValueFilter.ExcludeRange && (value < mValueFilter.MinValue || value > mValueFilter.MaxValue))
            return value;
    }
    return QVariant();
}

PostoptValueFormatProxyModel::PostoptValueFormatProxyModel(QObject *parent)
    : ValueFormatProxyModel(parent)
{

}

void PostoptValueFormatProxyModel::setValueFilter(const ValueFilter &valueFilter)
{
    beginResetModel();
    mValueFilter = valueFilter;
    endResetModel();
}

QVariant PostoptValueFormatProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole)
        return QIdentityProxyModel::data(index, role);
    return applyFilter(QIdentityProxyModel::data(index, role));
}

QVariant PostoptValueFormatProxyModel::applyFilter(const QVariant &data) const
{
    if (!data.isValid())
        return data;
    if (!data.toString().compare(ValueHelper::EPSText, Qt::CaseInsensitive)) {
        if  (mValueFilter.ShowEps)
            return data;
        return QVariant();
    }
    if (!data.toString().trimmed().compare(ValueHelper::PINFText, Qt::CaseInsensitive)) {
        if (mValueFilter.ShowPInf)
            return data;
        return QVariant();
    }
    if (!data.toString().trimmed().compare(ValueHelper::NINFText, Qt::CaseInsensitive)) {
        if (mValueFilter.ShowNInf)
            return data;
        return QVariant();
    }
    bool ok = false;
    double value = data.toDouble(&ok);
    if (ok) {
        if (!mValueFilter.ExcludeRange && value >= mValueFilter.MinValue && value <= mValueFilter.MaxValue)
            return value;
        else if (mValueFilter.ExcludeRange && (value < mValueFilter.MinValue || value > mValueFilter.MaxValue))
            return value;
        return QVariant();
    }
    return data;
}

BPValueFormatTypeProxyModel::BPValueFormatTypeProxyModel(QObject *parent)
    : ValueFormatProxyModel(parent)
{

}

QVariant BPValueFormatTypeProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    if (role != Qt::DisplayRole ||
        index.column() == columnCount()-4 || index.row() == rowCount()-1)
        return QIdentityProxyModel::data(index, role);
    bool ok = false;
    double value = QIdentityProxyModel::data(index, role).toDouble(&ok);
    if (ok) {
        double retval = value;
        if (mValueFilter.UseAbsoluteValues)
            value = std::abs(value);
        if (!mValueFilter.ExcludeRange && value >= mValueFilter.MinValue && value <= mValueFilter.MaxValue)
            return retval;
        else if (mValueFilter.ExcludeRange && (value < mValueFilter.MinValue || value > mValueFilter.MaxValue))
            return retval;
    }
    return QVariant();
}

}
}
}
