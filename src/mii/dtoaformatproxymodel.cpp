#include "dtoaformatproxymodel.h"
#include "numerics.h"

namespace gams {
namespace studio {
namespace mii {

DtoaFormatProxyModel::DtoaFormatProxyModel(QObject *parent)
    : QIdentityProxyModel(parent)
{

}

QVariant DtoaFormatProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    auto data = QIdentityProxyModel::data(index, role);
    if (role == Qt::DisplayRole && data.isValid()) {
        auto value = data.toDouble();
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    }
    return data;
}

DtoaBpAverageFormatProxyModel::DtoaBpAverageFormatProxyModel(QObject *parent)
    : DtoaFormatProxyModel(parent)
{

}

QVariant DtoaBpAverageFormatProxyModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    auto data = QIdentityProxyModel::data(index, role);
    if (role == Qt::DisplayRole && data.isValid() && index.column() < columnCount()-4 &&
        index.row() < rowCount()-4){
        auto value = data.toDouble();
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    }
    return data;
}

}
}
}
