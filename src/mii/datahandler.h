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
#ifndef DATAHANDLER_H
#define DATAHANDLER_H

#include <QVariant>
#include <QSharedPointer>

namespace gams {
namespace studio {
namespace mii {

class Aggregation;
class AbstractModelInstance;
class AbstractViewConfiguration;
class DataMatrix;
class PostoptTreeItem;

typedef QMap<Qt::Orientation, QList<int>> SectionMapping;

class DataHandler
{
public:
    class AbstractDataProvider;

    struct CoefficientInfo
    {
        CoefficientInfo(const CoefficientInfo& other)
            : mRowCount(other.mRowCount)
            , mColumnCount(other.mColumnCount)
            , mCount(new int*[mRowCount])
            , mNlFlags(new int*[mRowCount])
        {
            for (int r=0; r<mRowCount; ++r) {
                mCount[r] = new int[mColumnCount];
                mNlFlags[r] = new int[mColumnCount];
                std::copy(other.mCount[r], other.mCount[r]+other.mColumnCount, mCount[r]);
                std::copy(other.mNlFlags[r], other.mNlFlags[r]+other.mColumnCount, mNlFlags[r]);
            }
        }

        CoefficientInfo(CoefficientInfo&& other) noexcept
            : mRowCount(other.mRowCount)
            , mColumnCount(other.mColumnCount)
            , mCount(other.mCount)
            , mNlFlags(other.mNlFlags)
        {
            other.mCount = nullptr;
            other.mNlFlags = nullptr;
            other.mColumnCount = 0;
            other.mRowCount = 0;
        }

        CoefficientInfo(int columnCount, int rowCount)
            : mRowCount(rowCount)
            , mColumnCount(columnCount)
            , mCount(new int*[mRowCount])
            , mNlFlags(new int*[mRowCount])
        {
            for (int r=0; r<mRowCount; ++r) {
                mCount[r] = new int[mColumnCount];
                mNlFlags[r] = new int[mColumnCount];
                std::fill(mCount[r], mCount[r]+mColumnCount, 0);
                std::fill(mNlFlags[r], mNlFlags[r]+mColumnCount, 0);
            }
        }

        ~CoefficientInfo()
        {
            for (int r=0; r<mRowCount; ++r) {
                delete [] mCount[r];
            }
            if (mCount) delete [] mCount;
            for (int r=0; r<mRowCount; ++r) {
                delete [] mNlFlags[r];
            }
            if (mNlFlags) delete [] mNlFlags;
        }

        int** count()
        {
            return mCount;
        }

        int** nlFlags()
        {
            return mNlFlags;
        }

        int columnCount() const
        {
            return mColumnCount;
        }

        int rowCount() const
        {
            return mRowCount;
        }

        auto& operator=(const CoefficientInfo& other)
        {
            mRowCount = other.mColumnCount;
            mColumnCount = other.mColumnCount;
            for (int r=0; r<mRowCount; ++r) {
                delete [] mCount[r];
            }
            if (mCount) delete [] mCount;
            mCount = new int*[mRowCount];
            for (int r=0; r<mRowCount; ++r) {
                mCount[r] = new int[mColumnCount];
                std::copy(other.mCount[r], other.mCount[r]+other.mColumnCount, mCount[r]);
            }
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

        auto& operator=(CoefficientInfo&& other) noexcept
        {
            mCount = other.mCount;
            other.mCount = nullptr;
            mNlFlags = other.mNlFlags;
            other.mNlFlags = nullptr;
            mColumnCount = other.mColumnCount;
            other.mColumnCount = 0;
            mRowCount = other.mRowCount;
            other.mRowCount = 0;
            return *this;
        }

    private:
        int mRowCount = 0;
        int mColumnCount = 0;
        int** mCount = nullptr;
        int** mNlFlags = nullptr;
    };

    DataHandler(AbstractModelInstance& modelInstance);

    ~DataHandler();

    void loadData(const QSharedPointer<AbstractViewConfiguration> &viewConfig);

    QVariant data(int row, int column, int viewId) const;

    int nlFlag(int row, int column, int viewId);

    QSharedPointer<PostoptTreeItem> dataTree(int viewId) const;

    void removeViewData(int viewId);

    void removeViewData();

    int headerData(int logicalIndex, Qt::Orientation orientation, int viewId) const;

    QVariant plainHeaderData(Qt::Orientation orientation,
                             int viewId, int logicalIndex,
                             int dimension) const;

    QVariant sectionLabels(Qt::Orientation orientation,
                           int viewId, int logicalIndex) const;

    int rowCount(int viewId) const;

    int rowEntryCount(int row, int viewId) const;

    int columnCount(int viewId) const;

    int columnEntryCount(int column, int viewId) const;

    const QList<int>& rowIndices(int viewId, int row) const;

    const QList<int>& columnIndices(int viewId, int column) const;

    int symbolRowCount(int viewId) const;

    int symbolColumnCount(int viewId) const;

    double modelMinimum() const;
    void setModelMinimum(double minimum);

    double modelMaximum() const;
    void setModelMaximum(double maximum);

    int maxSymbolDimension(int viewId, Qt::Orientation orientation);

    QSharedPointer<AbstractViewConfiguration> clone(int viewId, int newView);
    
    void loadJacobian();

private:
    AbstractDataProvider *cloneProvider(int viewId);
    QSharedPointer<AbstractDataProvider> newProvider(const QSharedPointer<AbstractViewConfiguration> &viewConfig);

private:
    AbstractModelInstance& mModelInstance;
    double mModelMinimum = std::numeric_limits<double>::max();
    double mModelMaximum = std::numeric_limits<double>::lowest();

    QScopedPointer<DataMatrix> mDataMatrix;
    QSharedPointer<CoefficientInfo> mCoeffCount;

    ///
    /// \brief Abstract data provider cache, where key is the view ID.
    ///
    QMap<int, QSharedPointer<AbstractDataProvider>> mDataCache;

    QList<int> mDummyIndices;
};

}
}
}

#endif // DATAHANDLER_H
