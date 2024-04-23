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
