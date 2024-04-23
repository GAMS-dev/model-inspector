#include "symbolfiltermodel.h"
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"

namespace gams {
namespace studio {
namespace mii {

SymbolFilterModel::SymbolFilterModel(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                     const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                     QObject *parent)
    : QSortFilterProxyModel(parent)
    , mModelInstance(modelInstance)
    , mViewConfig(viewConfig)
    , mColumns(mModelInstance->columnCount(mViewConfig->viewId()))
    , mRows(mModelInstance->rowCount(mViewConfig->viewId()))
    , mColumnStates(new int[mColumns])
    , mRowStates(new int[mRows])
    , mColumnEntries(new int[mColumns])
    , mRowEntries(new int[mRows])
{
    std::fill(mColumnStates, mColumnStates+mColumns, 1);
    std::fill(mRowStates, mRowStates+mRows, 1);
    for (int c=0; c<mColumns; ++c) {
        mColumnEntries[c] = mModelInstance->columnEntryCount(c, mViewConfig->viewId());
    }
    for (int r=0; r<mRows; ++r) {
        mRowEntries[r] = mModelInstance->rowEntryCount(r, mViewConfig->viewId());
    }
}

SymbolFilterModel::~SymbolFilterModel()
{
    delete [] mColumnStates;
    delete [] mRowStates;
    delete [] mColumnEntries;
    delete [] mRowEntries;
}

void SymbolFilterModel::evaluateFilters()
{
    auto anyvar = evaluateColumnFilters();
    if (!anyvar) {
        std::fill(mRowStates, mRowStates+mRows, 0);
        invalidate();
        return;
    }
    auto anyeqn = evaluateRowFilters();
    if (!anyeqn) {
        std::fill(mColumnStates, mColumnStates+mColumns, 0);
        invalidate();
        return;
    }
    if (anyvar && anyeqn) {
        updateEntryCounts();
    }
    invalidate();
}

bool SymbolFilterModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    return mColumnStates[sourceColumn] && mColumnEntries[sourceColumn];
}

bool SymbolFilterModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    return mRowStates[sourceRow] && mRowEntries[sourceRow];
}

bool SymbolFilterModel::evaluateColumnFilters()
{
    bool ok, anyVarActive = false;
    for (int i=0; i<mColumns; ++i) {
        auto sectionIndex = sourceModel()->headerData(i, Qt::Horizontal).toInt(&ok);
        if (!ok)
            continue;
        auto variable = mModelInstance->variable(sectionIndex);
        int firstSection = variable->firstSection();
        if (firstSection < 0)
            continue;
        auto entries = variable->isScalar() ? 1 : variable->entries();
        const auto& item = mViewConfig->currentIdentifierFilter()[Qt::Horizontal][firstSection];
        if (item.Checked == Qt::Checked) { // variable is checked
            anyVarActive = true;
            std::fill(mColumnStates, mColumnStates+mColumns, 1);
            // local and global variable label filters are applied
            for (int idx : item.CheckStates) {
                mColumnStates[idx-firstSection] = 0; // Qt::Unchecked
            }
            const auto& unchecked = mViewConfig->currentLabelFiler().UncheckedLabels[Qt::Horizontal];
            if (!unchecked.isEmpty()) {
                for (int e=firstSection, l=i; e<firstSection+entries; ++e, ++l) {
                    evaluateColumnLabelFilters(variable->isScalar(), variable->sectionLabels()[e], l);
                }
            } else if (unchecked.size() == mViewConfig->currentLabelFiler().LabelCheckStates[Qt::Horizontal].size()) {
                std::fill(mColumnStates, mColumnStates+entries, 0);
            }
        } else { // variable is unchecked
            std::fill(mColumnStates, mColumnStates+entries, 0);
        }
        i += entries-1;
    }
    return anyVarActive;
}

void SymbolFilterModel::evaluateColumnLabelFilters(bool isScalar, const QStringList& labels, int logicalSection)
{
    auto unchecked = mViewConfig->currentLabelFiler().UncheckedLabels[Qt::Horizontal];
    if (mViewConfig->currentLabelFiler().Any) {
        if (isScalar)
            return;
        for (auto& label : labels) {
            if (!unchecked.contains(label, Qt::CaseInsensitive)) {
                return;
            }
        }
        mColumnStates[logicalSection] = 0;
    } else {
        for (auto& uncheckedLabel : unchecked) {
            if (labels.contains(uncheckedLabel, Qt::CaseInsensitive)) {
                mColumnStates[logicalSection] = 0;
                return;
            }
        }
    }
}

bool SymbolFilterModel::evaluateRowFilters()
{
    bool ok, anyEqnActive = false;
    for (int i=0; i<mRows; ++i) {
        auto sectionIndex = sourceModel()->headerData(i, Qt::Vertical).toInt(&ok);
        if (!ok)
            continue;
        auto equation = mModelInstance->equation(sectionIndex);
        int firstSection = equation->firstSection();
        if (firstSection < 0)
            continue;
        auto entries = equation->isScalar() ? 1 : equation->entries();
        const auto& item = mViewConfig->currentIdentifierFilter()[Qt::Vertical][firstSection];
        if (item.Checked == Qt::Checked) { // equation is checked
            anyEqnActive = true;
            std::fill(mRowStates, mRowStates+mRows, 1);
            // local and global equation label filters are applied
            for (int idx : item.CheckStates) {
                mRowStates[idx-firstSection] = 0; // Qt::Unchecked
            }
            const auto& unchecked = mViewConfig->currentLabelFiler().UncheckedLabels[Qt::Vertical];
            if (!unchecked.isEmpty()) {
                for (int e=firstSection, l=i; e<firstSection+entries; ++e, ++l) {
                    evaluateRowLabelFilters(equation->isScalar(), equation->sectionLabels()[e], l);
                }
            } else if (unchecked.size() == mViewConfig->currentLabelFiler().LabelCheckStates[Qt::Vertical].size()) {
                std::fill(mRowStates, mRowStates+entries, 0);
            }
        } else { // equation is unchecked
            std::fill(mRowStates, mRowStates+entries, 0);
        }
        i += entries-1;
    }
    return anyEqnActive;
}

void SymbolFilterModel::evaluateRowLabelFilters(bool isScalar, const QStringList& labels, int logicalSection)
{
    auto unchecked = mViewConfig->currentLabelFiler().UncheckedLabels[Qt::Vertical];
    if (mViewConfig->currentLabelFiler().Any) {
        if (isScalar)
            return;
        for (auto& label : labels) {
            if (!unchecked.contains(label, Qt::CaseInsensitive)) {
                return;
            }
        }
        mRowStates[logicalSection] = 0;
    } else {
        for (auto& uncheckedLabel : unchecked) {
            if (labels.contains(uncheckedLabel, Qt::CaseInsensitive)) {
                mRowStates[logicalSection] = 0;
                return;
            }
        }
    }
}

void SymbolFilterModel::updateEntryCounts()
{
    std::fill(mColumnEntries, mColumnEntries+mColumns, 0);
    for (int r=0; r<mRows; ++r) {
        if (mRowStates[r]) {
            const auto& indices = mModelInstance->rowIndices(mViewConfig->viewId(), r);
            for (int idx : indices) {
                if (mColumnStates[idx])
                    mColumnEntries[idx]++;
            }
        }
    }
    std::fill(mRowEntries, mRowEntries+mRows, 0);
    for (int c=0; c<mColumns; ++c) {
        if (mColumnStates[c]) {
            const auto& indices = mModelInstance->columnIndices(mViewConfig->viewId(), c);
            for (int idx : indices) {
                if (mRowStates[idx]) {
                    mRowEntries[idx]++;
                }
            }
        }
    }
}

}
}
}
