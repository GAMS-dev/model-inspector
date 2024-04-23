/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023 GAMS Development Corp. <support@gams.com>
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
 *
 */
#include "datahandler.h"
#include "abstractmodelinstance.h"
#include "datamatrix.h"
#include "postopttreeitem.h"
#include "viewconfigurationprovider.h"

#include <algorithm>
#include <functional>

#include <QSet>

#include <QDebug>

using namespace std;

namespace gams {
namespace studio {
namespace mii {

class DataHandler::AbstractDataProvider
{
protected:
    AbstractDataProvider(DataHandler *dataHandler,
                         AbstractModelInstance& modelInstance,
                         const QSharedPointer<AbstractViewConfiguration> &viewConfig)
        : mDataHandler(dataHandler)
        , mModelInstance(modelInstance)
        , mViewConfig(viewConfig)
        , mIsAbsoluteData(viewConfig->currentValueFilter().isAbsolute())
    {

    }

    AbstractDataProvider(const AbstractDataProvider& other)
        : mRowCount(other.mRowCount)
        , mColumnCount(other.mColumnCount)
        , mSymbolRowCount(other.mSymbolRowCount)
        , mSymbolColumnCount(other.mSymbolColumnCount)
        , mDataHandler(other.mDataHandler)
        , mModelInstance(other.mModelInstance)
        , mLogicalSectionMapping(other.mLogicalSectionMapping)
        , mViewConfig(other.mViewConfig->clone())
        , mIsAbsoluteData(other.mIsAbsoluteData)
    {

    }

    AbstractDataProvider(AbstractDataProvider&& other) noexcept
        : mRowCount(other.mRowCount)
        , mColumnCount(other.mColumnCount)
        , mSymbolRowCount(other.mSymbolRowCount)
        , mSymbolColumnCount(other.mSymbolColumnCount)
        , mDataHandler(other.mDataHandler)
        , mModelInstance(other.mModelInstance)
        , mLogicalSectionMapping(std::move(other.mLogicalSectionMapping))
        , mViewConfig(std::move(other.mViewConfig))
        , mIsAbsoluteData(other.mIsAbsoluteData)
    {
        other.mViewConfig = nullptr;
        other.mRowCount = 0;
        other.mColumnCount = 0;
        other.mSymbolRowCount = 0;
        other.mSymbolColumnCount = 0;
    }

public:
    virtual ~AbstractDataProvider()
    {
    }

    DataRow* dataRow(int row)
    {
        return mDataHandler->mDataMatrix->row(row);
    }

    virtual void loadData() = 0;

    virtual double data(int row, int column) const = 0;

    virtual int nlFlag(int row, int column) const
    {
        Q_UNUSED(row);
        Q_UNUSED(column);
        return 0;
    }

    virtual QVariant plainHeaderData(Qt::Orientation orientation,
                                     int logicalIndex,
                                     int dimension) const
    {
        const auto& labels = mViewConfig->sectionLabels(orientation);
        if (logicalIndex < labels.size()) {
            return labels[logicalIndex][dimension];
        }
        return QVariant();
    }

    virtual int headerData(Qt::Orientation orientation, int logicalIndex) const
    {
        if (logicalIndex >= 0 && logicalIndex < mLogicalSectionMapping[orientation].size()) {
            return mLogicalSectionMapping[orientation].at(logicalIndex);
        }
        return -1;
    }

    QVariant sectionLabels(Qt::Orientation orientation, int logicalIndex) const
    {
        const auto& labels = mViewConfig->sectionLabels(orientation);
        if (logicalIndex < labels.size()) {
            return labels[logicalIndex];
        }
        return QStringList();
    }

    int columnCount() const
    {
        return mColumnCount;
    }

    int rowCount() const
    {
        return mRowCount;
    }

    int symbolRowCount() const
    {
        return mSymbolRowCount;
    }

    int symbolColumnCount() const
    {
        return mSymbolColumnCount;
    }

    virtual int columnEntryCount(int column) const
    {
        Q_UNUSED(column);
        return 0;
    }

    virtual int rowEntryCount(int row) const
    {
        Q_UNUSED(row);
        return 0;
    }

    ///
    /// \brief Returns all non-zero indices.
    /// \param row Row the indices relate too.
    /// \return All non-zero indcies for the specified row.
    ///
    virtual const QList<int>& rowIndices(int row) const
    {
        Q_UNUSED(row);
        return mRowIndices;
    }

    ///
    /// \brief Returns all non-zero indices.
    /// \param row Column the indices relate too.
    /// \return All non-zero indcies for the specified column.
    ///
    virtual const QList<int>& columnIndices(int column) const
    {
        Q_UNUSED(column);
        return mColumnIndices;
    }

    virtual int maxSymbolDimension(Qt::Orientation orientation) const
    {
        Q_UNUSED(orientation);
        return 0;
    }

    QSharedPointer<AbstractViewConfiguration> viewConfig() const
    {
        return mViewConfig;
    }

    bool isAbsoluteData() const
    {
        return mIsAbsoluteData;
    }

    auto& operator=(const AbstractDataProvider& other)
    {
        mRowCount = other.mRowCount;
        mColumnCount = other.mColumnCount;
        mSymbolRowCount = other.mSymbolRowCount;
        mSymbolColumnCount = other.mSymbolColumnCount;
        mDataHandler = other.mDataHandler;
        mModelInstance = other.mModelInstance;
        mLogicalSectionMapping = other.mLogicalSectionMapping;
        mViewConfig = QSharedPointer<AbstractViewConfiguration>(other.mViewConfig->clone());
        mIsAbsoluteData = other.mIsAbsoluteData;
        return *this;
    }

    auto& operator=(AbstractDataProvider&& other) noexcept
    {
        mRowCount = other.mRowCount;
        mSymbolRowCount = other.mSymbolRowCount;
        other.mRowCount = 0;
        other.mSymbolRowCount = 0;
        mColumnCount = other.mColumnCount;
        mSymbolColumnCount = other.mSymbolColumnCount;
        other.mColumnCount = 0;
        other.mSymbolColumnCount = 0;
        mDataHandler = other.mDataHandler;
        mModelInstance = other.mModelInstance;
        mLogicalSectionMapping = std::move(other.mLogicalSectionMapping);
        mViewConfig = std::move(other.mViewConfig);
        mIsAbsoluteData = other.mIsAbsoluteData;
        return *this;
    }

protected:
    int mRowCount = 0;
    int mColumnCount = 0;
    int mSymbolRowCount = 0;
    int mSymbolColumnCount = 0;
    DataHandler *mDataHandler;
    AbstractModelInstance& mModelInstance;
    SectionMapping mLogicalSectionMapping;
    QSharedPointer<AbstractViewConfiguration> mViewConfig;
    double mDataMinimum = std::numeric_limits<double>::lowest();
    double mDataMaximum = std::numeric_limits<double>::max();
    QList<int> mRowIndices;
    QList<int> mColumnIndices;
    bool mIsAbsoluteData;
};

class IdentityDataProvider : public DataHandler::AbstractDataProvider
{
public:
    IdentityDataProvider(DataHandler *dataHandler,
                         AbstractModelInstance& modelInstance,
                         const QSharedPointer<AbstractViewConfiguration> &viewConfig)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
    {
        mRowCount = mModelInstance.equationRowCount();
        mSymbolRowCount = mRowCount;
        mColumnCount = mModelInstance.variableRowCount();
        mSymbolColumnCount = mColumnCount;
    }

    void loadData() override
    {

    }

    double data(int row, int column) const override
    {
        Q_UNUSED(row);
        Q_UNUSED(column);
        return 0.0;
    }
};

class BPScalingProvider final : public DataHandler::AbstractDataProvider
{
public:
    BPScalingProvider(DataHandler *dataHandler,
                      AbstractModelInstance& modelInstance,
                      const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                      const QSharedPointer<DataHandler::CoefficientInfo> &coeffInfo)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
        , mCoeffInfo(coeffInfo)
    {
        mSymbolRowCount = mModelInstance.equationCount() * 2;
        mRowCount = mSymbolRowCount + 2; // one row for max and min
        mSymbolColumnCount = mModelInstance.variableCount();
        mColumnCount = mSymbolColumnCount + 2;
        mDataMatrix = new double*[mRowCount];
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new double[mColumnCount];
            mNlFlags[r] = new int[mColumnCount];
            std::fill(mNlFlags[r], mNlFlags[r]+mColumnCount, 0);
            if (r % 2) {
                std::fill(mDataMatrix[r], mDataMatrix[r]+mColumnCount,
                          std::numeric_limits<double>::max());
            } else {
                std::fill(mDataMatrix[r], mDataMatrix[r]+mColumnCount,
                          std::numeric_limits<double>::lowest());
            }
        }
        mDataMinimum = std::numeric_limits<double>::max();
        mDataMaximum = std::numeric_limits<double>::lowest();
    }

    BPScalingProvider(const BPScalingProvider& other)
        : DataHandler::AbstractDataProvider(other)
        , mDataMatrix(new double*[mRowCount])
        , mCoeffInfo(other.mCoeffInfo)
        , mNlFlags(new int*[mRowCount])
    {
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new double[mColumnCount];
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mDataMatrix[r],
                      other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
    }

    BPScalingProvider(BPScalingProvider&& other) noexcept
        : DataHandler::AbstractDataProvider(std::move(other))
        , mDataMatrix(other.mDataMatrix)
        , mCoeffInfo(std::move(other.mCoeffInfo))
        , mNlFlags(other.mNlFlags)
    {
        other.mDataMatrix = nullptr;
        other.mNlFlags = nullptr;
    }

    ~BPScalingProvider() override
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
            delete [] mNlFlags[r];
        }
        delete [] mDataMatrix;
        delete [] mNlFlags;
    }

    void loadData() override
    {
        for (const auto& equation : mModelInstance.equations()) {
            mLogicalSectionMapping[Qt::Vertical].append(equation->firstSection());
            mLogicalSectionMapping[Qt::Vertical].append(equation->firstSection());
        }
        for (const auto& variable : mModelInstance.variables()) {
            mLogicalSectionMapping[Qt::Horizontal].append(variable->firstSection());
        }
        mIsAbsoluteData = mViewConfig->currentValueFilter().isAbsolute();
        mViewConfig->currentValueFilter().isAbsolute() ? aggregateAbs() : aggregateId();
    }

    double data(int row, int column) const override
    {
        return mDataMatrix[row][column];
    }

    int nlFlag(int row, int column) const override
    {
        return mNlFlags[row][column];
    }

    auto& operator=(const BPScalingProvider& other)
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
            delete [] mNlFlags[r];
        }
        if (mDataMatrix) delete [] mDataMatrix;
        if (mNlFlags) delete [] mNlFlags;
        mDataMatrix = new double*[mRowCount];
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new double[mColumnCount];
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
        mCoeffInfo = other.mCoeffInfo;
        return *this;
    }

    auto& operator=(BPScalingProvider&& other) noexcept
    {
        mDataMatrix = other.mDataMatrix;
        other.mDataMatrix = nullptr;
        mCoeffInfo = std::move(other.mCoeffInfo);
        mNlFlags = other.mNlFlags;
        other.mNlFlags = nullptr;
        return *this;
    }

private:
    void aggregateId()
    {
        int minRow = 1, maxRow = 0;
        for (const auto& equation : mModelInstance.equations()) {
            double rhsMin = std::numeric_limits<double>::max();
            double rhsMax = std::numeric_limits<double>::lowest();
            double eqnMin = std::numeric_limits<double>::max();
            double eqnMax = std::numeric_limits<double>::lowest();
            mCoeffInfo->count()[maxRow][mColumnCount-2] = mModelInstance.equationType(equation->firstSection());
            for (int r=equation->firstSection(); r<=equation->lastSection(); ++r) {
                auto sparseRow = dataRow(r);
                auto data = mModelInstance.useOutput() ? sparseRow->outputData() : sparseRow->inputData();
                auto rhs = mModelInstance.rhs(r);
                if (rhs != 0.0) {
                    rhsMin = std::min(rhsMin, rhs);
                    rhsMax = std::max(rhsMax, rhs);
                    if (rhs < 0) ++mCoeffInfo->count()[minRow][mColumnCount-1];
                    else if (rhs > 0) ++mCoeffInfo->count()[maxRow][mColumnCount-1];
                }
                for (int i=0; i<sparseRow->entries(); ++i) {
                    auto value = data[i];
                    auto column = mModelInstance.variable(sparseRow->colIdx()[i])->logicalIndex();
                    if (sparseRow->nlFlags()[i]) {
                        ++mCoeffInfo->nlFlags()[minRow][column];
                        ++mCoeffInfo->nlFlags()[maxRow][column];
                        ++mNlFlags[minRow][column];
                        ++mNlFlags[maxRow][column];
                    }
                    mDataMatrix[minRow][column] = std::min(value, mDataMatrix[minRow][column]);
                    mDataMatrix[maxRow][column] = std::max(value, mDataMatrix[maxRow][column]);
                    if (value < 0) {
                        ++mCoeffInfo->count()[minRow][column];
                    } else if (value > 0) {
                        ++mCoeffInfo->count()[maxRow][column];
                    }
                }
            }
            mDataMatrix[minRow][mColumnCount-2] = rhsMin;
            mDataMatrix[maxRow][mColumnCount-2] = rhsMax;
            mDataMatrix[mRowCount-1][mColumnCount-2] = std::min(mDataMatrix[mRowCount-1][mColumnCount-2], rhsMin);
            mDataMatrix[mRowCount-2][mColumnCount-2] = std::max(mDataMatrix[mRowCount-2][mColumnCount-2], rhsMax);
            for (int c=0; c<mColumnCount-2; ++c) {
                mDataMinimum = std::min(mDataMinimum, mDataMatrix[minRow][c]);
                mDataMaximum = std::max(mDataMaximum, mDataMatrix[maxRow][c]);
                mDataMatrix[mRowCount-1][c] = std::min(mDataMatrix[mRowCount-1][c], mDataMatrix[minRow][c]);
                mDataMatrix[mRowCount-2][c] = std::max(mDataMatrix[mRowCount-2][c], mDataMatrix[maxRow][c]);
                mNlFlags[mRowCount-1][c] += mNlFlags[minRow][c];
                mNlFlags[mRowCount-2][c] += mNlFlags[maxRow][c];
                eqnMin = std::min(eqnMin, mDataMatrix[minRow][c]);
                eqnMax = std::max(eqnMax, mDataMatrix[maxRow][c]);
                mNlFlags[minRow][mColumnCount-1] += mNlFlags[minRow][c];
                mNlFlags[maxRow][mColumnCount-1] += mNlFlags[maxRow][c];
                setEmtpyCell(minRow, c);
                setEmtpyCell(maxRow, c);
            }
            mDataMatrix[minRow][mColumnCount-1] = eqnMin;
            mDataMatrix[maxRow][mColumnCount-1] = eqnMax;
            for (int c=mColumnCount-2; c<mColumnCount; ++c) {
                mDataMinimum = std::min(mDataMinimum, mDataMatrix[minRow][c]);
                mDataMaximum = std::max(mDataMaximum, mDataMatrix[maxRow][c]);
                setEmtpyCell(minRow, c);
                setEmtpyCell(maxRow, c);
            }
            minRow += 2;
            maxRow += 2;
        }
        setEmtpyCell(mRowCount-2, mColumnCount-2);
        setEmtpyCell(mRowCount-1, mColumnCount-2);
        mDataMatrix[mRowCount-2][mColumnCount-1] = 0.0;
        mDataMatrix[mRowCount-1][mColumnCount-1] = 0.0;
        mViewConfig->defaultValueFilter().MinValue = mDataMinimum;
        mViewConfig->defaultValueFilter().MaxValue = mDataMaximum;
        if (mViewConfig->filterDialogState() != AbstractViewConfiguration::Apply) {
            mViewConfig->currentValueFilter().MinValue = mDataMinimum;
            mViewConfig->currentValueFilter().MaxValue = mDataMaximum;
        }
        mDataHandler->setModelMinimum(mDataMinimum);
        mDataHandler->setModelMaximum(mDataMaximum);
    }

    void aggregateAbs()
    {
        int minRow = 1, maxRow = 0;
        for (const auto& equation : mModelInstance.equations()) {
            double rhsMin = std::numeric_limits<double>::max();
            double rhsMax = std::numeric_limits<double>::lowest();
            double eqnMin = std::numeric_limits<double>::max();
            double eqnMax = std::numeric_limits<double>::lowest();
            mCoeffInfo->count()[maxRow][mColumnCount-2] = mModelInstance.equationType(equation->firstSection());
            for (int r=equation->firstSection(); r<=equation->lastSection(); ++r) {
                auto sparseRow = dataRow(r);
                auto data = mModelInstance.useOutput() ? sparseRow->outputData() : sparseRow->inputData();
                auto rhs = mModelInstance.rhs(r);
                if (rhs != 0.0) {
                    rhsMin = std::min(rhsMin, std::abs(rhs));
                    rhsMax = std::max(rhsMax, std::abs(rhs));
                    if (rhs < 0) ++mCoeffInfo->count()[minRow][mColumnCount-1];
                    else if (rhs > 0) ++mCoeffInfo->count()[maxRow][mColumnCount-1];
                }
                for (int i=0; i<sparseRow->entries(); ++i) {
                    auto value = data[i];
                    auto column = mModelInstance.variable(sparseRow->colIdx()[i])->logicalIndex();
                    if (sparseRow->nlFlags()[i]) {
                        ++mCoeffInfo->nlFlags()[minRow][column];
                        ++mCoeffInfo->nlFlags()[maxRow][column];
                        ++mNlFlags[minRow][column];
                        ++mNlFlags[maxRow][column];
                    }
                    mDataMatrix[minRow][column] = std::min(std::abs(data[i]), mDataMatrix[minRow][column]);
                    mDataMatrix[maxRow][column] = std::max(std::abs(data[i]), mDataMatrix[maxRow][column]);
                    if (value < 0) {
                        ++mCoeffInfo->count()[minRow][column];
                    } else if (value > 0) {
                        ++mCoeffInfo->count()[maxRow][column];
                    }
                }
            }
            mDataMatrix[minRow][mColumnCount-2] = rhsMin;
            mDataMatrix[maxRow][mColumnCount-2] = rhsMax;
            mDataMatrix[mRowCount-1][mColumnCount-2] = std::min(mDataMatrix[mRowCount-1][mColumnCount-2], rhsMin);
            mDataMatrix[mRowCount-2][mColumnCount-2] = std::max(mDataMatrix[mRowCount-2][mColumnCount-2], rhsMax);
            for (int c=0; c<mColumnCount-2; ++c) {
                mDataMinimum = std::min(mDataMinimum, mDataMatrix[minRow][c]);
                mDataMaximum = std::max(mDataMaximum, mDataMatrix[maxRow][c]);
                mDataMatrix[mRowCount-1][c] = std::min(mDataMatrix[mRowCount-1][c], mDataMatrix[minRow][c]);
                mDataMatrix[mRowCount-2][c] = std::max(mDataMatrix[mRowCount-2][c], mDataMatrix[maxRow][c]);
                mNlFlags[mRowCount-1][c] += mNlFlags[minRow][c];
                mNlFlags[mRowCount-2][c] += mNlFlags[maxRow][c];
                eqnMin = std::min(eqnMin, mDataMatrix[minRow][c]);
                eqnMax = std::max(eqnMax, mDataMatrix[maxRow][c]);
                mNlFlags[minRow][mColumnCount-1] += mNlFlags[minRow][c];
                mNlFlags[maxRow][mColumnCount-1] += mNlFlags[maxRow][c];
                setEmtpyCell(minRow, c);
                setEmtpyCell(maxRow, c);
            }
            mDataMatrix[minRow][mColumnCount-1] = eqnMin;
            mDataMatrix[maxRow][mColumnCount-1] = eqnMax;
            for (int c=mColumnCount-2; c<mColumnCount; ++c) {
                mDataMinimum = std::min(mDataMinimum, mDataMatrix[minRow][c]);
                mDataMaximum = std::max(mDataMaximum, mDataMatrix[maxRow][c]);
                setEmtpyCell(minRow, c);
                setEmtpyCell(maxRow, c);
            }
            minRow += 2;
            maxRow += 2;
        }
        setEmtpyCell(mRowCount-2, mColumnCount-2);
        setEmtpyCell(mRowCount-1, mColumnCount-2);
        mDataMatrix[mRowCount-2][mColumnCount-1] = 0.0;
        mDataMatrix[mRowCount-1][mColumnCount-1] = 0.0;
        mViewConfig->defaultValueFilter().MinValue = mDataMinimum;
        mViewConfig->defaultValueFilter().MaxValue = mDataMaximum;
        if (mViewConfig->filterDialogState() != AbstractViewConfiguration::Apply) {
            mViewConfig->currentValueFilter().MinValue = mDataMinimum;
            mViewConfig->currentValueFilter().MaxValue = mDataMaximum;
        }
        mDataHandler->setModelMinimum(mDataMinimum);
        mDataHandler->setModelMaximum(mDataMaximum);
    }

    void setEmtpyCell(int row, int column)
    {
        if (mDataMatrix[row][column] == std::numeric_limits<double>::lowest() ||
                mDataMatrix[row][column] == std::numeric_limits<double>::max()) {
            mDataMatrix[row][column] = 0.0;
        }
    }

private:
    double** mDataMatrix;
    QSharedPointer<DataHandler::CoefficientInfo> mCoeffInfo;
    int** mNlFlags;
};

class SymbolsDataProvider final : public DataHandler::AbstractDataProvider
{
private:
    class SymbolColumn
    {
    public:
        inline int entries() const
        {
            return mIndices.size();
        }

        inline QList<int>& indices()
        {
            return mIndices;
        }

    private:
        QList<int> mIndices;
    };

    class SymbolRow
    {
    public:
        SymbolRow()
        {

        }

        SymbolRow(const SymbolRow& other)
            : mEntries(other.mEntries)
            , mFirstIdx(other.mFirstIdx)
            , mData(new double[other.mEntries])
            , mNlFlags(new int[other.mEntries])
        {
            std::copy(other.mData, other.mData+other.mEntries, mData);
            std::copy(other.mNlFlags, other.mNlFlags+other.mEntries, mNlFlags);
        }

        SymbolRow(SymbolRow&& other) noexcept
            : mEntries(other.mEntries)
            , mFirstIdx(other.mFirstIdx)
            , mData(other.mData)
            , mNlFlags(other.mNlFlags)
            , mIndices(std::move(other.mIndices))
        {
            other.mEntries = 0;
            other.mFirstIdx = 0;
            other.mData = nullptr;
            other.mNlFlags = nullptr;
        }

        ~SymbolRow()
        {
            if (mData) delete [] mData;
            if (mNlFlags) delete [] mNlFlags;
        }

        inline int entries() const
        {
            return mEntries;
        }

        inline void setEntries(int entries)
        {
            mEntries = entries;
        }

        inline int firstIdx() const
        {
            return mFirstIdx;
        }

        inline void setFirstIdx(int firstIdx)
        {
            mFirstIdx = firstIdx;
        }

        inline int lastIdx() const
        {
            return mFirstIdx + mEntries - 1;
        }

        inline double* data()
        {
            return mData;
        }

        inline void setData(double* data)
        {
            mData = data;
        }

        inline int* nlFlags()
        {
            return mNlFlags;
        }

        inline void setNlFlags(int* nlFlags)
        {
            mNlFlags = nlFlags;
        }

        inline QList<int>& indices()
        {
            return mIndices;
        }

        auto& operator=(const SymbolRow& other)
        {
            if (mData) delete [] mData;
            if (mNlFlags) delete [] mNlFlags;
            mEntries = other.mEntries;
            mFirstIdx = other.mFirstIdx;
            mData = new double[other.mEntries];
            mNlFlags = new int[other.mEntries];
            mIndices = other.mIndices;
            std::copy(other.mData, other.mData+other.mEntries, mData);
            std::copy(other.mNlFlags, other.mNlFlags+other.mEntries, mNlFlags);
            return *this;
        }

        auto& operator=(SymbolRow&& other) noexcept
        {
            mEntries = other.mEntries;
            other.mEntries = 0;
            mFirstIdx = other.mFirstIdx;
            other.mFirstIdx = 0;
            mData = other.mData;
            other.mData = nullptr;
            mNlFlags = other.mNlFlags;
            other.mNlFlags = nullptr;
            mIndices = std::move(other.mIndices);
            return *this;
        }

    private:
        int mEntries = 0;
        int mFirstIdx = 0;
        double* mData = nullptr;
        int *mNlFlags = nullptr;
        QList<int> mIndices;
    };

public:
    SymbolsDataProvider(DataHandler *dataHandler,
                        AbstractModelInstance& modelInstance,
                        const QSharedPointer<AbstractViewConfiguration> &viewConfig)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
    {
        mDataMinimum = std::numeric_limits<double>::max();
        mDataMaximum = std::numeric_limits<double>::lowest();
    }

    SymbolsDataProvider(const SymbolsDataProvider& other)
        : DataHandler::AbstractDataProvider(other)
        , mRows(new SymbolRow[mRowCount])
        , mColumns(new SymbolColumn[mColumnCount])
        , mEqnDimension(other.mEqnDimension)
        , mVarDimension(other.mVarDimension)
    {
        std::copy(other.mRows, other.mRows+other.mRowCount, mRows);
        std::copy(other.mColumns, other.mColumns+other.mColumnCount, mColumns);
    }

    SymbolsDataProvider(SymbolsDataProvider&& other) noexcept
        : DataHandler::AbstractDataProvider(std::move(other))
        , mRows(other.mRows)
        , mColumns(other.mColumns)
        , mEqnDimension(other.mEqnDimension)
        , mVarDimension(other.mVarDimension)
    {
        other.mRowCount = 0;
        other.mColumnCount = 0;
        other.mRows = nullptr;
        other.mColumns = nullptr;
    }

    ~SymbolsDataProvider()
    {
        if (mColumns) delete [] mColumns;
        if (mRows) delete [] mRows;
    }

    void loadData() override
    {
        QList<Symbol*> equations;
        for (const auto &filter : std::as_const(mViewConfig->currentIdentifierFilter()[Qt::Vertical])) {
            if (filter.Checked == Qt::Unchecked)
                continue;
            auto equation = mModelInstance.equation(filter.SymbolIndex);
            if (!equation)
                continue;
            equations.append(equation);
            mEqnDimension = std::max(mEqnDimension, equation->dimension());
            for (int s=equation->firstSection(); s<=equation->lastSection(); ++s) {
                mLogicalSectionMapping[Qt::Vertical].append(s);
            }
        }
        mRowCount = mLogicalSectionMapping[Qt::Vertical].size();
        mRows = new SymbolRow[mRowCount];
        QList<Symbol*> variables;
        for (const auto &filter : std::as_const(mViewConfig->currentIdentifierFilter()[Qt::Horizontal])) {
            if (filter.Checked == Qt::Unchecked)
                continue;
            auto variable = mModelInstance.variable(filter.SymbolIndex);
            if (!variable)
                continue;
            variables.append(variable);
            mVarDimension = std::max(mVarDimension, variable->dimension());
            for (int s=variable->firstSection(); s<=variable->lastSection(); ++s) {
                mLogicalSectionMapping[Qt::Horizontal].append(s);
            }
        }
        if (mViewConfig->currentValueFilter().isAbsolute()) {
            value = std::bind(&SymbolsDataProvider::abs, this, std::placeholders::_1);
        } else {
            value = std::bind(&SymbolsDataProvider::identity, this, std::placeholders::_1);
        }
        mColumnCount += mLogicalSectionMapping[Qt::Horizontal].size();
        mColumns = new SymbolColumn[mColumnCount];
        mIsAbsoluteData = mViewConfig->currentValueFilter().UseAbsoluteValues;
        aggregate(equations, variables);
    }

    double data(int row, int column) const override
    {
        if (!mRows || !mRows[row].entries()) {
            return 0.0;
        }
        if (column < mRows[row].firstIdx() || column > mRows[row].lastIdx()) {
            return 0.0;
        }
        return mRows[row].data()[column-mRows[row].firstIdx()];
    }

    int nlFlag(int row, int column) const override
    {
        if (!mRows || !mRows[row].entries()) {
            return 0;
        }
        if (column < mRows[row].firstIdx() || column > mRows[row].lastIdx()) {
            return 0;
        }
        return mRows[row].nlFlags()[column-mRows[row].firstIdx()];
    }

    int columnEntryCount(int column) const override
    {
        return column < mColumnCount ? mColumns[column].entries() : 0;
    }

    int rowEntryCount(int row) const override
    {
        return row < mRowCount ? mRows[row].entries() : 0;
    }

    virtual const QList<int>& rowIndices(int row) const override
    {
        return row < mRowCount ? mRows[row].indices() : mRowIndices;
    }

    virtual const QList<int>& columnIndices(int column) const override
    {
        return column < mColumnCount ? mColumns[column].indices() : mColumnIndices;
    }

    int maxSymbolDimension(Qt::Orientation orientation) const override
    {
        return orientation == Qt::Horizontal ? mVarDimension : mEqnDimension;
    }

    auto& operator=(const SymbolsDataProvider& other)
    {
        delete [] mRows;
        delete [] mColumns;
        mRows = new SymbolRow[mRowCount];
        std::copy(other.mRows, other.mRows+other.mRowCount, mRows);
        mColumns = new SymbolColumn[mColumnCount];
        std::copy(other.mColumns, other.mColumns+other.mColumnCount, mColumns);
        mVarDimension = other.mVarDimension;
        mEqnDimension = other.mEqnDimension;
        return *this;
    }

    auto& operator=(SymbolsDataProvider&& other) noexcept
    {
        mRows = other.mRows;
        other.mRows = nullptr;
        mColumns = other.mColumns;
        other.mColumns = nullptr;
        mVarDimension = other.mVarDimension;
        mEqnDimension = other.mEqnDimension;
        return *this;
    }

private:
    void aggregate(QList<Symbol*>& equations, QList<Symbol*>& variables)
    {
        int rr = 0;
        for (auto* equation : equations) {
            for (int r=equation->firstSection(); r<=equation->lastSection(); ++r, ++rr) {
                auto sparseRow = dataRow(r);
                auto data = mModelInstance.useOutput() ? sparseRow->outputData() : sparseRow->inputData();
                int sparseIdx = 0;
                int variableEntries = 0;
                QList<int> sparseIndicies, rIndices;
                for (auto variable : variables) {
                    variableEntries += variable->entries();
                    for (; sparseIdx<sparseRow->entries(); ++sparseIdx) {
                        if (sparseRow->colIdx()[sparseIdx] > variable->lastSection()) {
                            break;
                        }
                        if (sparseRow->colIdx()[sparseIdx] < variable->firstSection()) {
                            continue;
                        }
                        if (acceptValue(value(data[sparseIdx]))) {
                            sparseIndicies.append(sparseIdx);
                            rIndices.append(sparseRow->colIdx()[sparseIdx] - variable->firstSection());
                        }
                    }
                }
                if (sparseIndicies.isEmpty())
                    continue;
                auto firstIdx = sparseRow->colIdx()[sparseIndicies.first()];
                auto lastIdx = sparseRow->colIdx()[sparseIndicies.last()];
                auto firstSection = variables.first()->firstSection();
                SymbolRow* row = &mRows[rr];
                row->setEntries(lastIdx - firstIdx + 1);
                row->setData(new double[row->entries()]);
                row->setNlFlags(new int[row->entries()]);
                row->setFirstIdx(firstIdx - firstSection);
                row->indices() = std::move(rIndices);
                if (variableEntries == sparseIndicies.size()) {
                    for (auto idx : sparseIndicies) {
                        int column = sparseRow->colIdx()[idx] - firstSection;
                        row->data()[column] = value(data[idx]);
                        row->nlFlags()[column] = sparseRow->nlFlags()[idx];
                        mDataMinimum = std::min(mDataMinimum, row->data()[idx]);
                        mDataMaximum = std::max(mDataMaximum, row->data()[idx]);
                        mColumns[column].indices().append(rr);
                    }
                } else {
                    std::fill(row->data(), row->data()+row->entries(), 0.0);
                    std::fill(row->nlFlags(), row->nlFlags()+row->entries(), 0);
                    for (auto idx : sparseIndicies) {
                        int column = sparseRow->colIdx()[idx] - firstIdx;
                        row->data()[column] = value(data[idx]);
                        row->nlFlags()[column] = sparseRow->nlFlags()[idx];
                        mDataMinimum = std::min(mDataMinimum, row->data()[idx]);
                        mDataMaximum = std::max(mDataMaximum, row->data()[idx]);
                        mColumns[sparseRow->colIdx()[idx] - firstSection].indices().append(rr);
                    }
                }
            }
        }
        if (mViewConfig->defaultValueFilter().MinValue == std::numeric_limits<double>::lowest() ||
            mViewConfig->defaultValueFilter().MaxValue == std::numeric_limits<double>::max()) {
            mViewConfig->defaultValueFilter().MinValue = mDataMinimum;
            mViewConfig->defaultValueFilter().MaxValue = mDataMaximum;
        }
        if (mViewConfig->filterDialogState() != AbstractViewConfiguration::Apply) {
            mViewConfig->currentValueFilter().MinValue = mDataMinimum;
            mViewConfig->currentValueFilter().MaxValue = mDataMaximum;
        }
    }

    inline double abs(double value)
    {
        return std::abs(value);
    }

    inline double identity(double value)
    {
        return value;
    }

    bool acceptValue(double value)
    {
        if (mViewConfig->currentValueFilter().ExcludeRange) {
            return (value < mViewConfig->currentValueFilter().MinValue || value > mViewConfig->currentValueFilter().MaxValue);
        }
        return (value >= mViewConfig->currentValueFilter().MinValue && value <= mViewConfig->currentValueFilter().MaxValue);
    }

private:
    SymbolRow* mRows = nullptr;
    SymbolColumn* mColumns = nullptr;
    std::function<double(double)> value;
    int mEqnDimension = 0;
    int mVarDimension = 0;
};

class BPOverviewDataProvider final : public DataHandler::AbstractDataProvider
{
public:
    BPOverviewDataProvider(DataHandler *dataHandler,
                           AbstractModelInstance& modelInstance,
                           const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                           const QSharedPointer<DataHandler::CoefficientInfo> &negPosCount)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
        , mCoeffCount(negPosCount)
    {
        mSymbolRowCount = mModelInstance.equationCount();
        mRowCount = mSymbolRowCount + 1;
        mSymbolColumnCount = mModelInstance.variableCount();
        mColumnCount = mSymbolColumnCount + 2;
        mDataMatrix = new char*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new char[mColumnCount];
            std::fill(mDataMatrix[r], mDataMatrix[r]+mColumnCount, 0);
        }
        mNlFlags = new int*[mRowCount];
        for (int r=0, rr=0; r<mRowCount; ++r, rr+=2) {
            mNlFlags[r] = new int[mColumnCount];
            std::fill(mNlFlags[r], mNlFlags[r]+mColumnCount, 0);
            if (rr+1 >= mCoeffCount->rowCount())
                continue;
            for (int c=0; c<mSymbolColumnCount; ++c) {
                if (c < mCoeffCount->columnCount()) {
                    mNlFlags[r][c] += mCoeffCount->nlFlags()[rr][c] + mCoeffCount->nlFlags()[rr+1][c];
                }
            }
        }
    }

    BPOverviewDataProvider(const BPOverviewDataProvider& other)
        : DataHandler::AbstractDataProvider(other)
        , mCoeffCount(other.mCoeffCount)
    {
        mDataMatrix = new char*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new char[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
        }
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
    }

    BPOverviewDataProvider(BPOverviewDataProvider&& other) noexcept
        : DataHandler::AbstractDataProvider(std::move(other))
        , mDataMatrix(other.mDataMatrix)
        , mCoeffCount(std::move(other.mCoeffCount))
        , mNlFlags(other.mNlFlags)
    {
        other.mDataMatrix = nullptr;
        other.mNlFlags = nullptr;
    }

    ~BPOverviewDataProvider()
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
        }
        if (mDataMatrix) delete [] mDataMatrix;
        for (int r=0; r<mRowCount; ++r) {
            delete [] mNlFlags[r];
        }
        if (mNlFlags) delete [] mNlFlags;
    }

    void loadData() override
    {
        int negRow = 1, posRow = 0;
        for (const auto& equations : mModelInstance.equations()) {
            mLogicalSectionMapping[Qt::Vertical].append(equations->firstSection());
        }
        for (const auto& variable : mModelInstance.variables()) {
            mLogicalSectionMapping[Qt::Horizontal].append(variable->firstSection());
        }
        for (int r=0; r<mModelInstance.equationCount(); ++r, negRow += 2, posRow += 2) {
            for (int c=0; c<mColumnCount-2; ++c) {
                if (mCoeffCount->count()[negRow][c] == 0 && mCoeffCount->count()[posRow][c] == 0) {
                    mDataMatrix[r][c] = 0x0;
                } else if (mCoeffCount->count()[negRow][c] == 0 && mCoeffCount->count()[posRow][c] > 0) {
                    mDataMatrix[r][c] = ValueHelper::Plus;
                } else if (mCoeffCount->count()[negRow][c] < 0 && mCoeffCount->count()[posRow][c] == 0) {
                    mDataMatrix[r][c] = ValueHelper::Minus;
                } else {
                    mDataMatrix[r][c] = ValueHelper::Mixed;
                }
            }
            mDataMatrix[r][mColumnCount-2] = mCoeffCount->count()[posRow][mCoeffCount->columnCount()-2];
            if (mCoeffCount->count()[negRow][mColumnCount-1] == 0 &&
                mCoeffCount->count()[posRow][mColumnCount-1] == 0) {
                mDataMatrix[r][mColumnCount-1] = '0';
            } else if (mCoeffCount->count()[negRow][mColumnCount-1] == 0 &&
                       mCoeffCount->count()[posRow][mColumnCount-1] > 0) {
                mDataMatrix[r][mColumnCount-1] = ValueHelper::Plus;
            } else if (mCoeffCount->count()[negRow][mColumnCount-1] < 0 &&
                       mCoeffCount->count()[posRow][mColumnCount-1] == 0) {
                mDataMatrix[r][mColumnCount-1] = ValueHelper::Minus;
            } else {
                mDataMatrix[r][mColumnCount-1] = ValueHelper::Mixed;
            }
        }
        int varColumn = 0;
        auto columns = mModelInstance.variableRowCount();
        double* lowerBounds = new double[columns];
        double* upperBounds = new double[columns];
        mModelInstance.variableLowerBounds(lowerBounds);
        mModelInstance.variableUpperBounds(upperBounds);
        for (const auto& variable : mModelInstance.variables()) {
            auto lower = std::numeric_limits<double>::max();
            auto upper = std::numeric_limits<double>::lowest();
            for (int i=variable->firstSection(); i<variable->lastSection(); ++i) {
                lower = std::min(lower, lowerBounds[i]);
                upper = std::max(upper, upperBounds[i]);
            }
            if (mModelInstance.variableType(variable->firstSection()) == 'x') { // x = continuous
                if (lower >= 0 && upper >= 0) {
                    mDataMatrix[mRowCount-1][varColumn] = ValueHelper::Plus;
                } else if (lower <= 0 && upper <= 0) {
                    mDataMatrix[mRowCount-1][varColumn] = ValueHelper::Minus;
                } else {
                    mDataMatrix[mRowCount-1][varColumn] = 'u';
                }
            } else {
                mDataMatrix[mRowCount-1][varColumn] = mModelInstance.variableType(variable->firstSection());
            }
            ++varColumn;
        }
        delete [] lowerBounds;
        delete [] upperBounds;
    }

    double data(int row, int column) const override
    {
        return mDataMatrix[row][column];
    }

    int nlFlag(int row, int column) const override
    {
        return mNlFlags[row][column];
    }

    auto& operator=(const BPOverviewDataProvider& other)
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
        }
        delete [] mDataMatrix;
        mDataMatrix = new char*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new char[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
        }
        mCoeffCount = other.mCoeffCount;
        for (int r=0; r<mRowCount; ++r) {
            delete [] mNlFlags[r];
        }
        delete [] mNlFlags;
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
        return *this;
    }

    auto& operator=(BPOverviewDataProvider&& other) noexcept
    {
        mDataMatrix = other.mDataMatrix;
        other.mDataMatrix = nullptr;
        mCoeffCount = std::move(other.mCoeffCount);
        mNlFlags = other.mNlFlags;
        other.mNlFlags = nullptr;
        return *this;
    }

private:
    char** mDataMatrix;
    QSharedPointer<DataHandler::CoefficientInfo> mCoeffCount;
    int** mNlFlags = nullptr;
};

class BPCountDataProvider final : public DataHandler::AbstractDataProvider
{
public:
    BPCountDataProvider(DataHandler *dataHandler,
                        AbstractModelInstance& modelInstance,
                        const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                        const QSharedPointer<DataHandler::CoefficientInfo> &coeffInfo)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
        , mCoeffInfo(coeffInfo)
    {
        mDataMinimum = std::numeric_limits<double>::max();
        mDataMaximum = std::numeric_limits<double>::lowest();
        mSymbolRowCount = mModelInstance.equationCount() * 2;
        mRowCount = mSymbolRowCount + 4;
        mSymbolColumnCount = mModelInstance.variableCount();
        mColumnCount = mSymbolColumnCount + 4;
        mDataMatrix = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new int[mColumnCount];

            for (int c=0; r<mCoeffInfo->rowCount() && c<mCoeffInfo->columnCount(); ++c) {
                mDataMatrix[r][c] = mCoeffInfo->count()[r][c];
                if (c != mColumnCount-4) {
                    mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[r][c]));
                    mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[r][c]));
                }
            }
            if (r < mCoeffInfo->rowCount())
                std::fill(mDataMatrix[r]+mCoeffInfo->columnCount(), mDataMatrix[r]+mColumnCount, 0);
            else
                std::fill(mDataMatrix[r], mDataMatrix[r]+mColumnCount, 0);
        }
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::fill(mNlFlags[r], mNlFlags[r]+mColumnCount, 0);
            if (r >= mCoeffInfo->rowCount())
                continue;
            std::copy(mCoeffInfo->nlFlags()[r], mCoeffInfo->nlFlags()[r]+mCoeffInfo->columnCount(), mNlFlags[r]);
        }
    }

    BPCountDataProvider(const BPCountDataProvider& other)
        : DataHandler::AbstractDataProvider(other)
        , mCoeffInfo(other.mCoeffInfo)
    {
        mDataMatrix = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new int[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
        }
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
    }

    BPCountDataProvider(BPCountDataProvider&& other) noexcept
        : DataHandler::AbstractDataProvider(std::move(other))
        , mDataMatrix(other.mDataMatrix)
        , mCoeffInfo(std::move(other.mCoeffInfo))
        , mNlFlags(other.mNlFlags)
    {
        other.mDataMatrix = nullptr;
        other.mNlFlags = nullptr;
    }

    ~BPCountDataProvider()
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
        }
        delete [] mDataMatrix;
        for (int r=0; r<mRowCount; ++r) {
            delete [] mNlFlags[r];
        }
        delete [] mNlFlags;
    }

    void loadData() override
    {
        for (const auto& equation : mModelInstance.equations()) {
            mLogicalSectionMapping[Qt::Vertical].append(equation->firstSection());
            mLogicalSectionMapping[Qt::Vertical].append(equation->firstSection());
        }
        for (const auto& variable : mModelInstance.variables()) {
            mLogicalSectionMapping[Qt::Horizontal].append(variable->firstSection());
        }
        for (int r=0, negRow = 1, posRow = 0; r<mModelInstance.equationCount(); ++r, negRow += 2, posRow += 2) {
            for (int v=0; v<mModelInstance.variableCount(); ++v) {
                mDataMatrix[negRow][mColumnCount-2] += mDataMatrix[negRow][v];
                mDataMatrix[posRow][mColumnCount-2] += mDataMatrix[posRow][v];
                mDataMatrix[mRowCount-3][v] += mDataMatrix[negRow][v];
                mDataMatrix[mRowCount-4][v] += mDataMatrix[posRow][v];
                mNlFlags[posRow][mColumnCount-2] += mNlFlags[posRow][v];
                mNlFlags[negRow][mColumnCount-2] += mNlFlags[negRow][v];
                mNlFlags[mRowCount-4][v] += mNlFlags[posRow][v];
                mNlFlags[mRowCount-3][v] += mNlFlags[negRow][v];
                mNlFlags[mRowCount-4][mColumnCount-2] += mNlFlags[posRow][v];
                mNlFlags[mRowCount-3][mColumnCount-2] += mNlFlags[negRow][v];
            }
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[negRow][mColumnCount-2]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[negRow][mColumnCount-2]));
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[posRow][mColumnCount-2]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[posRow][mColumnCount-2]));
            mDataMatrix[mRowCount-3][mColumnCount-3] += mDataMatrix[negRow][mColumnCount-3];
            mDataMatrix[mRowCount-4][mColumnCount-3] += mDataMatrix[posRow][mColumnCount-3];
            mDataMatrix[mRowCount-3][mColumnCount-2] += mDataMatrix[negRow][mColumnCount-2];
            mDataMatrix[mRowCount-4][mColumnCount-2] += mDataMatrix[posRow][mColumnCount-2];
        }
        for (int r=mRowCount-4; r<mRowCount-1; ++r) {
            for (int c=0; c<mColumnCount; ++c) {
                mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[r][c]));
                mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[r][c]));
            }
        }
        int index = 0;
        for (const auto& equation : mModelInstance.equations()) {
            mDataMatrix[index][mColumnCount-1] = equation->entries();
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[index][mColumnCount-1]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[index][mColumnCount-1]));
            index += 2;
        }
        index = 0;
        for (const auto& variable : mModelInstance.variables()) {
            mDataMatrix[mRowCount-2][index] = variable->entries();
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[mRowCount-2][index]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[mRowCount-2][index]));
            ++index;
        }
        int varColumn = 0;
        auto columns = mModelInstance.variableRowCount();
        double* lowerBounds = new double[columns];
        double* upperBounds = new double[columns];
        mModelInstance.variableLowerBounds(lowerBounds);
        mModelInstance.variableUpperBounds(upperBounds);
        for (const auto& variable : mModelInstance.variables()) {
            auto lower = std::numeric_limits<double>::max();
            auto upper = std::numeric_limits<double>::lowest();
            for (int i=variable->firstSection(); i<variable->lastSection(); ++i) {
                lower = std::min(lower, lowerBounds[i]);
                upper = std::max(upper, upperBounds[i]);
            }
            if (mModelInstance.variableType(variable->firstSection()) == 'x') { // x = continuous
                if (lower >= 0 && upper >= 0) {
                    mDataMatrix[mRowCount-1][varColumn] = ValueHelper::Plus;
                } else if (lower <= 0 && upper <= 0) {
                    mDataMatrix[mRowCount-1][varColumn] = ValueHelper::Minus;
                } else {
                    mDataMatrix[mRowCount-1][varColumn] = 'u';
                }
            } else {
                mDataMatrix[mRowCount-1][varColumn] = mModelInstance.variableType(variable->firstSection());
            }
            ++varColumn;
        }
        mViewConfig->defaultValueFilter().MinValue = mDataMinimum;
        mViewConfig->defaultValueFilter().MaxValue = mDataMaximum;
        if (mViewConfig->filterDialogState() != AbstractViewConfiguration::Apply) {
            mViewConfig->currentValueFilter().MinValue = mDataMinimum;
            mViewConfig->currentValueFilter().MaxValue = mDataMaximum;
        }
        delete [] lowerBounds;
        delete [] upperBounds;
    }

    double data(int row, int column) const override
    {
        return std::abs(mDataMatrix[row][column]);
    }

    int nlFlag(int row, int column) const override
    {
        return mNlFlags[row][column];
    }

    auto& operator=(const BPCountDataProvider& other)
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
        }
        delete [] mDataMatrix;
        mDataMatrix = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new int[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
        }
        mCoeffInfo = other.mCoeffInfo;
        for (int r=0; r<mRowCount; ++r) {
            delete [] mNlFlags[r];
        }
        if (mNlFlags) delete [] mNlFlags;
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
        return *this;
    }

    auto& operator=(BPCountDataProvider&& other) noexcept
    {
        mDataMatrix = other.mDataMatrix;
        other.mDataMatrix = nullptr;
        mCoeffInfo = std::move(other.mCoeffInfo);
        mNlFlags = other.mNlFlags;
        other.mNlFlags = nullptr;
        return *this;
    }

private:
    int** mDataMatrix;
    QSharedPointer<DataHandler::CoefficientInfo> mCoeffInfo;
    int** mNlFlags = nullptr;
};

class BPAverageDataProvider final : public DataHandler::AbstractDataProvider
{
public:
    BPAverageDataProvider(DataHandler *dataHandler,
                          AbstractModelInstance& modelInstance,
                          const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                          const QSharedPointer<DataHandler::CoefficientInfo> &coeffInfo)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
        , mCoeffInfo(coeffInfo)
    {
        mDataMinimum = std::numeric_limits<double>::max();
        mDataMaximum = std::numeric_limits<double>::lowest();
        mSymbolRowCount = mModelInstance.equationCount() * 2;
        mRowCount = mSymbolRowCount + 4;
        mSymbolColumnCount = mModelInstance.variableCount();
        mColumnCount = mSymbolColumnCount + 4;
        mDataMatrix = new double*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new double[mColumnCount];
            std::fill(mDataMatrix[r], mDataMatrix[r]+mColumnCount, 0);
        }
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::fill(mNlFlags[r], mNlFlags[r]+mColumnCount, 0);
            if (r >= mCoeffInfo->rowCount())
                continue;
            std::copy(mCoeffInfo->nlFlags()[r], mCoeffInfo->nlFlags()[r]+mCoeffInfo->columnCount(), mNlFlags[r]);
        }
    }

    BPAverageDataProvider(const BPAverageDataProvider& other)
        : DataHandler::AbstractDataProvider(other)
        , mCoeffInfo(other.mCoeffInfo)
    {
        mDataMatrix = new double*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new double[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
        }
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
    }

    BPAverageDataProvider(BPAverageDataProvider&& other) noexcept
        : DataHandler::AbstractDataProvider(std::move(other))
        , mDataMatrix(other.mDataMatrix)
        , mCoeffInfo(std::move(other.mCoeffInfo))
        , mNlFlags(other.mNlFlags)
    {
        other.mDataMatrix = nullptr;
        other.mNlFlags = nullptr;
    }

    ~BPAverageDataProvider()
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
        }
        delete [] mDataMatrix;
        for (int r=0; r<mRowCount; ++r) {
            delete [] mNlFlags[r];
        }
        delete [] mNlFlags;
    }

    void loadData() override
    {
        for (const auto& equation : mModelInstance.equations()) {
            mLogicalSectionMapping[Qt::Vertical].append(equation->firstSection());
            mLogicalSectionMapping[Qt::Vertical].append(equation->firstSection());
        }
        for (const auto& variable : mModelInstance.variables()) {
            mLogicalSectionMapping[Qt::Horizontal].append(variable->firstSection());
        }
        int index = 0;
        for (const auto& equation : mModelInstance.equations()) {
            mDataMatrix[index][mColumnCount-1] = equation->entries();
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[index][mColumnCount-1]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[index][mColumnCount-1]));
            index += 2;
        }
        index = 0;
        for (const auto& variable : mModelInstance.variables()) {
            mDataMatrix[mRowCount-2][index] = variable->entries();
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[mRowCount-2][index]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[mRowCount-2][index]));
            ++index;
        }
        for (int r=0, negRow = 1, posRow = 0; r<mModelInstance.equationCount(); ++r, negRow += 2, posRow += 2) {
            for (int c=0; c<mColumnCount-4; ++c) {
                mDataMatrix[negRow][c] = mCoeffInfo->count()[negRow][c] / mDataMatrix[mRowCount-2][c];
                mDataMatrix[negRow][mColumnCount-2] += mCoeffInfo->count()[negRow][c];
                mDataMatrix[mRowCount-3][c] += mCoeffInfo->count()[negRow][c];
                mDataMatrix[posRow][c] = mCoeffInfo->count()[posRow][c] / mDataMatrix[mRowCount-2][c];
                mDataMatrix[posRow][mColumnCount-2] += mCoeffInfo->count()[posRow][c];
                mDataMatrix[mRowCount-4][c] += mCoeffInfo->count()[posRow][c];
                mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[negRow][c]));
                mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[posRow][c]));
                mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[negRow][c]));
                mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[posRow][c]));
                mNlFlags[posRow][mColumnCount-2] += mNlFlags[posRow][c];
                mNlFlags[negRow][mColumnCount-2] += mNlFlags[negRow][c];
                mNlFlags[mRowCount-4][c] += mNlFlags[posRow][c];
                mNlFlags[mRowCount-3][c] += mNlFlags[negRow][c];
            }
            for (int c=mCoeffInfo->columnCount()-2;
                 posRow<mCoeffInfo->rowCount() && c<mCoeffInfo->columnCount(); ++c) {
                mDataMatrix[posRow][c] = mCoeffInfo->count()[posRow][c];
                if (c != mColumnCount-4) {
                    mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[posRow][c]));
                    mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[posRow][c]));
                }
            }
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[posRow][mColumnCount-1]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[posRow][mColumnCount-1]));
            mDataMatrix[negRow][mColumnCount-2] /= mDataMatrix[posRow][mColumnCount-1];
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[negRow][mColumnCount-2]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[negRow][mColumnCount-2]));
            mDataMatrix[posRow][mColumnCount-2] /= mDataMatrix[posRow][mColumnCount-1];
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[negRow][mColumnCount-2]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[negRow][mColumnCount-2]));
        }
        for (int c=0; c<mColumnCount-4; ++c) {
            mDataMatrix[mRowCount-3][c] /= mDataMatrix[mRowCount-2][c];
            mDataMatrix[mRowCount-4][c] /= mDataMatrix[mRowCount-2][c];
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[mRowCount-3][c]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[mRowCount-4][c]));
            mDataMinimum = std::min(mDataMinimum, double(mDataMatrix[mRowCount-3][c]));
            mDataMaximum = std::max(mDataMaximum, double(mDataMatrix[mRowCount-4][c]));
        }
        int varColumn = 0;
        auto columns = mModelInstance.variableRowCount();
        double* lowerBounds = new double[columns];
        double* upperBounds = new double[columns];
        mModelInstance.variableLowerBounds(lowerBounds);
        mModelInstance.variableUpperBounds(upperBounds);
        for (const auto& variable : mModelInstance.variables()) {
            auto lower = std::numeric_limits<double>::max();
            auto upper = std::numeric_limits<double>::lowest();
            for (int i=variable->firstSection(); i<variable->lastSection(); ++i) {
                lower = std::min(lower, lowerBounds[i]);
                upper = std::max(upper, upperBounds[i]);
            }
            if (mModelInstance.variableType(variable->firstSection()) == 'x') { // x = continuous
                if (lower >= 0 && upper >= 0) {
                    mDataMatrix[mRowCount-1][varColumn] = '+';
                } else if (lower <= 0 && upper <= 0) {
                    mDataMatrix[mRowCount-1][varColumn] = '-';
                } else {
                    mDataMatrix[mRowCount-1][varColumn] = 'u';
                }
            } else {
                mDataMatrix[mRowCount-1][varColumn] = mModelInstance.variableType(variable->firstSection());
            }
            ++varColumn;
        }
        mViewConfig->defaultValueFilter().MinValue = mDataMinimum;
        mViewConfig->defaultValueFilter().MaxValue = mDataMaximum;
        if (mViewConfig->filterDialogState() != AbstractViewConfiguration::Apply) {
            mViewConfig->currentValueFilter().MinValue = mDataMinimum;
            mViewConfig->currentValueFilter().MaxValue = mDataMaximum;
        }
        delete [] lowerBounds;
        delete [] upperBounds;
    }

    double data(int row, int column) const override
    {
        return std::abs(mDataMatrix[row][column]);
    }

    int nlFlag(int row, int column) const override
    {
        return mNlFlags[row][column];
    }

    auto& operator=(const BPAverageDataProvider& other)
    {
        for (int r=0; r<mRowCount; ++r) {
            delete [] mDataMatrix[r];
        }
        delete [] mDataMatrix;
        mDataMatrix = new double*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mDataMatrix[r] = new double[mColumnCount];
            std::copy(other.mDataMatrix[r], other.mDataMatrix[r]+other.mColumnCount, mDataMatrix[r]);
        }
        mCoeffInfo = other.mCoeffInfo;
        for (int r=0; r<mRowCount; ++r) {
            delete [] mNlFlags[r];
        }
        if (mNlFlags) delete [] mNlFlags;
        mNlFlags = new int*[mRowCount];
        for (int r=0; r<mRowCount; ++r) {
            mNlFlags[r] = new int[mColumnCount];
            std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
        }
        return *this;
    }

    auto& operator=(BPAverageDataProvider&& other) noexcept
    {
        mDataMatrix = other.mDataMatrix;
        other.mDataMatrix = nullptr;
        mCoeffInfo = std::move(other.mCoeffInfo);
        mNlFlags = other.mNlFlags;
        other.mNlFlags = nullptr;
        return *this;
    }

private:
    double** mDataMatrix;
    QSharedPointer<DataHandler::CoefficientInfo> mCoeffInfo;
    int** mNlFlags = nullptr;
};

class PostoptDataProvider final : public DataHandler::AbstractDataProvider
{
public:
    PostoptDataProvider(DataHandler *dataHandler,
                        AbstractModelInstance& modelInstance,
                        const QSharedPointer<AbstractViewConfiguration> &viewConfig)
        : DataHandler::AbstractDataProvider(dataHandler, modelInstance, viewConfig)
    {
        mColumnCount = 5;
        mRootItem = QSharedPointer<PostoptTreeItem>(new LinePostoptTreeItem());
        value = std::bind(&PostoptDataProvider::identity, this, std::placeholders::_1);
    }

    PostoptDataProvider(const PostoptDataProvider& other)
        : DataHandler::AbstractDataProvider(other)
        , mRootItem(other.mRootItem)
    {

    }

    PostoptDataProvider(PostoptDataProvider&& other) noexcept
        : DataHandler::AbstractDataProvider(std::move(other))
        , mRootItem(std::move(other.mRootItem))
    {
        other.mRootItem = nullptr;
    }

    ~PostoptDataProvider()
    {

    }

    void loadData() override
    {
        if (mViewConfig->currentValueFilter().isAbsolute()) {
            value = std::bind(&PostoptDataProvider::abs, this, std::placeholders::_1);
        } else {
            value = std::bind(&PostoptDataProvider::identity, this, std::placeholders::_1);
        }
        mRootItem = QSharedPointer<PostoptTreeItem>(new LinePostoptTreeItem());

        auto equations = new GroupPostoptTreeItem(ViewHelper::EquationHeaderText);
        auto eqnFilter = mViewConfig->currentIdentifierFilter()[Qt::Vertical];
        for (auto equation : mModelInstance.equations()) {
            if (!eqnFilter[equation->firstSection()].Checked) {
                continue;
            }
            auto eqnGroup = new GroupPostoptTreeItem(equation->name(), equations);
            for (int e=0; e<equation->entries(); ++e) {
                if (skipEntry(equation, e, Qt::Vertical))
                    continue;
                auto eqnLine = new GroupPostoptTreeItem(symbolName(equation, e));
                loadAttributes(equation, e, eqnLine);
                loadVariables(equation, e, eqnLine);
                if (eqnLine->rowCount()) {
                    eqnLine->setParent(eqnGroup);
                    eqnGroup->append(eqnLine);
                } else {
                    delete eqnLine;
                }
            }
            if (eqnGroup->rowCount()) {
                eqnGroup->setParent(equations);
                equations->append(eqnGroup);
            } else {
                delete eqnGroup;
            }
        }
        if (equations->rowCount()) {
            equations->setParent(mRootItem.get());
            mRootItem->append(equations);
        } else {
            delete equations;
        }

        auto variables = new GroupPostoptTreeItem(ViewHelper::VariableHeaderText);
        auto varFilter = mViewConfig->currentIdentifierFilter()[Qt::Horizontal];
        for (auto variable : mModelInstance.variables()) {
            if (!varFilter[variable->firstSection()].Checked) {
                continue;
            }
            auto varGroup = new GroupPostoptTreeItem(variable->name());
            for (int e=0; e<variable->entries(); ++e) {
                if (skipEntry(variable, e, Qt::Horizontal))
                    continue;
                auto varLine = new GroupPostoptTreeItem(symbolName(variable, e));
                loadAttributes(variable, e, varLine);
                loadEquations(variable, e, varLine);
                if (varLine->rowCount()) {
                    varLine->setParent(varGroup);
                    varGroup->append(varLine);
                } else {
                    delete varLine;
                }
            }
            if (varGroup->rowCount()) {
                varGroup->setParent(variables);
                variables->append(varGroup);
            } else {
                delete varGroup;
            }
        }
        if (variables->rowCount()) {
            variables->setParent(mRootItem.get());
            mRootItem->append(variables);
        } else {
            delete variables;
        }
        if (!mRootItem->rowCount()) {
            auto line = new ClickPostoptTreeItem("Please click here to configure the views content.", mRootItem.get());
            mRootItem->append(line);
        }
    }

    double data(int row, int column) const override
    {
        Q_UNUSED(row);
        Q_UNUSED(column);
        return 0.0;
    }

    QSharedPointer<PostoptTreeItem> dataTree() const
    {
        return mRootItem;
    }

    auto& operator=(const PostoptDataProvider& other)
    {
        mRootItem = other.mRootItem;
        return *this;
    }

    auto& operator=(PostoptDataProvider&& other) noexcept
    {
        mRootItem = other.mRootItem;
        other.mRootItem = nullptr;
        return *this;
    }

private:
    void loadAttributes(Symbol *symbol, int entry, PostoptTreeItem *parent)
    {
        bool abs = mViewConfig->currentValueFilter().isAbsolute();
        auto attributes = new GroupPostoptTreeItem(ViewHelper::AttributeHeaderText);
        for (const auto& label : AttributeHelper::attributeTextList()) {
            if (mViewConfig->currentAttributeFilter().value(label) == Qt::Unchecked) {
                continue;
            }
            QVariant value;
            if (symbol->isEquation()) {
                value = mModelInstance.equationAttribute(label, symbol->firstSection(), entry, abs);
            } else if (symbol->isVariable()) {
                value = mModelInstance.variableAttribute(label, symbol->firstSection(), entry, abs);
            }
            attributes->append(new LinePostoptTreeItem({label, value}, attributes));
        }
        if (attributes->rowCount()) {
            attributes->setParent(parent);
            parent->append(attributes);
        } else {
            delete attributes;
        }
    }

    void loadEquations(Symbol *variable, int entry, PostoptTreeItem *parent)
    {
        bool abs = mViewConfig->currentValueFilter().isAbsolute();
        auto equations = new LinePostoptTreeItem(PostoptTreeItem::EquationLineHeader);
        for (auto equation : mModelInstance.equations()) {
            auto eqnGroup = new GroupPostoptTreeItem(equation->name());
            for (int e=0; e<equation->entries(); ++e) {
                if (skipEntry(equation, e, Qt::Vertical))
                    continue;
                auto row = dataRow(equation->firstSection()+e);
                QVariant jacval;
                if (row) {
                    jacval = row->outputValue(variable->firstSection()+entry, variable->lastSection());
                }
                if (jacval.isValid()) {
                    auto name = symbolName(equation, e);
                    double jac = value(jacval.toDouble());
                    double xi = value(mModelInstance.equationAttribute(AttributeHelper::MarginalNumText, equation->firstSection(), e, abs).toDouble());
                    double jacxi = value(jacval.toDouble() * xi);
                    eqnGroup->append(new LinePostoptTreeItem({name, jac, xi, jacxi}, eqnGroup));
                }
            }
            if (eqnGroup->rowCount()) {
                eqnGroup->setParent(equations);
                equations->append(eqnGroup);
            } else {
                delete eqnGroup;
            }
        }
        if (equations->rowCount()) {
            equations->setParent(parent);
            parent->append(equations);
        } else {
            delete equations;
        }
    }

    void loadVariables(Symbol *equation, int entry, PostoptTreeItem *parent)
    {
        bool abs = mViewConfig->currentValueFilter().isAbsolute();
        auto variables = new LinePostoptTreeItem(PostoptTreeItem::VariableLineHeader);
        for (auto variable : mModelInstance.variables()) {
            auto varGroup = new GroupPostoptTreeItem(variable->name());
            for (int e=0; e<variable->entries(); ++e) {
                if (skipEntry(variable, e, Qt::Horizontal))
                    continue;
                auto row = dataRow(equation->firstSection()+entry);
                QVariant jacval;
                if (row) {
                    jacval = row->outputValue(variable->firstSection()+e, variable->lastSection());
                }
                if (jacval.isValid()) {
                    auto name = symbolName(variable, e);
                    double jac = value(jacval.toDouble());
                    double ui = value(mModelInstance.variableAttribute(AttributeHelper::LevelText, variable->firstSection(), e, abs).toDouble());
                    double jacui = value(jac * ui);
                    varGroup->append(new LinePostoptTreeItem({name, jac, ui, jacui}, varGroup));
                }
            }
            if (varGroup->rowCount()) {
                varGroup->setParent(variables);
                variables->append(varGroup);
            } else {
                delete varGroup;
            }
        }
        if (variables->rowCount()) {
            variables->setParent(parent);
            parent->append(variables);
        } else {
            delete variables;
        }
    }

    QString symbolName(Symbol *symbol, int entry)
    {
        if (symbol->isScalar())
            return symbol->name();
        int index = symbol->firstSection()+entry;
        if (!symbol->sectionLabels().contains(index))
            return QString("(..)");
        return QString("%1(%2)").arg(symbol->name(), symbol->sectionLabels()[index].join(", "));
    }

    bool skipEntry(Symbol *symbol, int entry, Qt::Orientation orientation)
    {
        if (symbol->isScalar())
            return false;
        int index = symbol->firstSection()+entry;
        auto labels = symbol->sectionLabels().value(index);
        auto states = mViewConfig->currentLabelFiler().LabelCheckStates.value(orientation);
        if (mViewConfig->currentLabelFiler().Any) {
            for (const auto& label : labels) {
                if (states.value(label) == Qt::Checked)
                    return false;
            }
        } else {
            for (const auto& label : labels) {
                if (states.value(label) == Qt::Unchecked)
                    return true;
            }
            return false;
        }
        return true;
    }

    double abs(double value)
    {
        return std::abs(value);
    }

    double identity(double value)
    {
        return value;
    }

private:
    QSharedPointer<PostoptTreeItem> mRootItem;
    std::function<double(double)> value;
};

DataHandler::DataHandler(AbstractModelInstance& modelInstance)
    : mModelInstance(modelInstance)
    , mDataMatrix(new DataMatrix)
{

}

DataHandler::~DataHandler()
{

}

void DataHandler::loadData(const QSharedPointer<AbstractViewConfiguration> &viewConfig)
{
    if (!viewConfig)
        return;
    if (viewConfig->viewType() == ViewHelper::ViewDataType::BP_Scaling &&
        mDataCache.contains(viewConfig->viewId()) &&
        mDataCache[viewConfig->viewId()]->isAbsoluteData() == viewConfig->currentValueFilter().isAbsolute()) {
        return;
    }
    auto provider = newProvider(viewConfig);
    mDataCache.remove(viewConfig->viewId());
    provider->loadData();
    mDataCache[viewConfig->viewId()] = provider;
}

QVariant DataHandler::data(int row, int column, int viewId) const
{
    if (mDataCache.contains(viewId) && mDataCache[viewId]->data(row, column) != 0.0) {
        return mDataCache[viewId]->data(row, column);
    }
    return QVariant();
}

int DataHandler::nlFlag(int row, int column, int viewId)
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->nlFlag(row, column) : 0;
}

QSharedPointer<PostoptTreeItem> DataHandler::dataTree(int viewId) const
{
    if (mDataCache.contains(viewId)) {
        auto provider = static_cast<PostoptDataProvider*>(mDataCache[viewId].get());
        return provider->dataTree();
    }
    return nullptr;
}

void DataHandler::removeViewData(int viewId)
{
    if (mDataCache.contains(viewId))
        mDataCache.remove(viewId);
}

void DataHandler::removeViewData()
{
    mDataCache.clear();
}

int DataHandler::headerData(int logicalIndex,
                            Qt::Orientation orientation,
                            int viewId) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->headerData(orientation, logicalIndex)
                                       : -1;
}

QVariant DataHandler::plainHeaderData(Qt::Orientation orientation,
                                      int viewId, int logicalIndex,
                                      int dimension) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->plainHeaderData(orientation, logicalIndex, dimension)
                                       : QVariant();
}

QVariant DataHandler::sectionLabels(Qt::Orientation orientation, int viewId, int logicalIndex) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->sectionLabels(orientation, logicalIndex)
                                       : QStringList();
}

int DataHandler::rowCount(int viewId) const
{
    if (mDataCache.contains(viewId)) {
        return mDataCache[viewId]->rowCount();
    }
    return 0;
}

int DataHandler::rowEntryCount(int row, int viewId) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->rowEntryCount(row) : 0;
}

int DataHandler::columnCount(int viewId) const
{
    if (mDataCache.contains(viewId)) {
        return mDataCache[viewId]->columnCount();
    }
    return 0;
}

int DataHandler::columnEntryCount(int column, int viewId) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->columnEntryCount(column) : 0;
}

const QList<int> &DataHandler::rowIndices(int viewId, int row) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->rowIndices(row) : mDummyIndices;
}

const QList<int> &DataHandler::columnIndices(int viewId, int column) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->columnIndices(column) : mDummyIndices;
}

int DataHandler::symbolRowCount(int viewId) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->symbolRowCount() : 0;
}

int DataHandler::symbolColumnCount(int viewId) const
{
    return mDataCache.contains(viewId) ? mDataCache[viewId]->symbolColumnCount() : 0;
}

double DataHandler::modelMinimum() const
{
    return mModelMinimum;
}

void DataHandler::setModelMinimum(double minimum)
{
    mModelMinimum = minimum;
}

double DataHandler::modelMaximum() const
{
    return mModelMaximum;
}

void DataHandler::setModelMaximum(double maximum)
{
    mModelMaximum = maximum;
}

int DataHandler::maxSymbolDimension(int viewId, Qt::Orientation orientation)
{
    if (!mDataCache.contains(viewId))
        return 0;
    return mDataCache[viewId]->maxSymbolDimension(orientation);
}

QSharedPointer<AbstractViewConfiguration> DataHandler::clone(int viewId, int newView)
{
    if (!mDataCache.contains(viewId))
        return nullptr;
    mDataCache[newView] = QSharedPointer<AbstractDataProvider>(cloneProvider(viewId));
    mDataCache[newView]->viewConfig()->setViewId(newView);
    return mDataCache[newView]->viewConfig();
}

void DataHandler::loadJacobian()
{
    mDataMatrix.reset(mModelInstance.jacobianData());
}

DataHandler::AbstractDataProvider* DataHandler::cloneProvider(int viewId)
{
    switch (mDataCache[viewId]->viewConfig()->viewType()) {
    case ViewHelper::ViewDataType::BP_Scaling:
    {
        auto provider = static_cast<BPScalingProvider*>(mDataCache[viewId].get());
        return new BPScalingProvider(*provider);
    }
    case ViewHelper::ViewDataType::Symbols:
    {
        auto provider = static_cast<SymbolsDataProvider*>(mDataCache[viewId].get());
        return new SymbolsDataProvider(*provider);
    }
    case ViewHelper::ViewDataType::BP_Overview:
    {
        auto provider = static_cast<BPOverviewDataProvider*>(mDataCache[viewId].get());
        return new BPOverviewDataProvider(*provider);
    }
    case ViewHelper::ViewDataType::BP_Count:
    {
        auto provider = static_cast<BPCountDataProvider*>(mDataCache[viewId].get());
        return new BPCountDataProvider(*provider);
    }
    case ViewHelper::ViewDataType::BP_Average:
    {
        auto provider = static_cast<BPAverageDataProvider*>(mDataCache[viewId].get());
        return new BPAverageDataProvider(*provider);
    }
    case ViewHelper::ViewDataType::Postopt:
    {
        auto provider = static_cast<PostoptDataProvider*>(mDataCache[viewId].get());
        return new PostoptDataProvider(*provider);
    }
    default:
    {
        auto provider = static_cast<IdentityDataProvider*>(mDataCache[viewId].get());
        return new IdentityDataProvider(*provider);
    }
    }
}

QSharedPointer<DataHandler::AbstractDataProvider> DataHandler::newProvider(const QSharedPointer<AbstractViewConfiguration> &viewConfig)
{
    if (!mCoeffCount || viewConfig->viewId() == (int)ViewHelper::ViewDataType::BP_Scaling) {
        mCoeffCount.reset(new CoefficientInfo(mModelInstance.variableCount()+2,
                                              mModelInstance.equationCount()*2));
    }
    switch (viewConfig->viewType()) {
    case ViewHelper::ViewDataType::BP_Scaling:
        return QSharedPointer<AbstractDataProvider>(new BPScalingProvider(this,
                                                                          mModelInstance,
                                                                          viewConfig,
                                                                          mCoeffCount));
    case ViewHelper::ViewDataType::Symbols:
        return QSharedPointer<AbstractDataProvider>(new SymbolsDataProvider(this,
                                                                            mModelInstance,
                                                                            viewConfig));
    case ViewHelper::ViewDataType::BP_Overview:
        return QSharedPointer<AbstractDataProvider>(new BPOverviewDataProvider(this,
                                                                               mModelInstance,
                                                                               viewConfig,
                                                                               mCoeffCount));
    case ViewHelper::ViewDataType::BP_Count:
        return QSharedPointer<AbstractDataProvider>(new BPCountDataProvider(this,
                                                                            mModelInstance,
                                                                            viewConfig,
                                                                            mCoeffCount));
    case ViewHelper::ViewDataType::BP_Average:
        return QSharedPointer<AbstractDataProvider>(new BPAverageDataProvider(this,
                                                                              mModelInstance,
                                                                              viewConfig,
                                                                              mCoeffCount));
    case ViewHelper::ViewDataType::Postopt:
        return QSharedPointer<AbstractDataProvider>(new PostoptDataProvider(this,
                                                                            mModelInstance,
                                                                            viewConfig));
    default:
        return QSharedPointer<AbstractDataProvider>(new IdentityDataProvider(this,
                                                                             mModelInstance,
                                                                             viewConfig));
    }
}

}
}
}
