#ifndef DTOAFORMATPROXYMODEL_H
#define DTOAFORMATPROXYMODEL_H

#include <QIdentityProxyModel>

namespace gams {
namespace studio {
namespace mii {

class DtoaFormatProxyModel : public QIdentityProxyModel
{
    Q_OBJECT

public:
    DtoaFormatProxyModel(QObject *parent = nullptr);

    virtual QVariant data(const QModelIndex &index, int role) const override;
};

class DtoaBpAverageFormatProxyModel : public DtoaFormatProxyModel
{
    Q_OBJECT

public:
    DtoaBpAverageFormatProxyModel(QObject *parent = nullptr);

    virtual QVariant data(const QModelIndex &index, int role) const override;
};

}
}
}

#endif // DTOAFORMATPROXYMODEL_H
