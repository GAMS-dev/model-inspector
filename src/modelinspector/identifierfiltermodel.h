#ifndef IDENTIFIERFILTERMODEL_H
#define IDENTIFIERFILTERMODEL_H

#include <QSharedPointer>
#include <QSortFilterProxyModel>

#include "common.h"

namespace gams {
namespace studio {
namespace modelinspector {

class ModelInstance;

class IdentifierFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    IdentifierFilterModel(QSharedPointer<ModelInstance> modelInstance,
                          QObject *parent = nullptr);

    IdentifierFilter& identifierFilter();
    IdentifierFilter& defaultIdentifierFilter();
    void setIdentifierFilter(const IdentifierFilter &filter);

    void setIdentifierCheckState(int symbolIndex, Qt::CheckState state,
                                 Qt::Orientation orientation);

protected:
    bool filterAcceptsColumn(int sourceColumn,
                             const QModelIndex &sourceParent) const override;

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

private:
    QSharedPointer<ModelInstance> mModelInstance;
    IdentifierFilter mIdentifierFilter;
    IdentifierFilter mDefaultIdentifierFilter;
};

class IdentifierLabelFilterModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    IdentifierLabelFilterModel(QSharedPointer<ModelInstance> modelInstance,
                               QObject *parent = nullptr);

    void clearIdentifierFilter();

    IdentifierFilter& identifierFilter();
    IdentifierState identifierState(int symbolIndex,
                                    Qt::Orientation orientation) const;
    void setIdentifierState(const IdentifierState &state,
                            Qt::Orientation orientation);

protected:
    bool filterAcceptsColumn(int sourceColumn,
                             const QModelIndex &sourceParent) const override;

    bool filterAcceptsRow(int sourceRow,
                          const QModelIndex &sourceParent) const override;

private:
    QSharedPointer<ModelInstance> mModelInstance;
    IdentifierFilter mIdentifierFilter;
};

}
}
}

#endif // IDENTIFIERFILTERMODEL_H
