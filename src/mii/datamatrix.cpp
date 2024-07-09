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
#include "datamatrix.h"

#include <algorithm>

namespace gams {
namespace studio {
namespace mii {

DataRow::DataRow()
    : mEntries(0)
    , mEntriesNl(0)
    , mColIdx(nullptr)
    , mInputData(nullptr)
    , mOutputData(nullptr)
    , mNlFlags(nullptr)
{

}

DataRow::DataRow(int entries)
    : mEntries(entries)
    , mEntriesNl(0)
    , mColIdx(new int[mEntries])
    , mInputData(new double[mEntries])
    , mOutputData(new double[mEntries])
    , mNlFlags(new int[mEntries])
{

}

DataRow::DataRow(const DataRow &other)
    : mEntries(other.entries())
    , mEntriesNl(other.mEntriesNl)
    , mColIdx(new int[mEntries])
    , mInputData(new double[mEntries])
    , mOutputData(new double[mEntries])
    , mNlFlags(new int[mEntries])
{
    std::copy(other.mColIdx, other.mColIdx+other.mEntries, mColIdx);
    std::copy(other.mInputData, other.mInputData+other.mEntries, mInputData);
    std::copy(other.mOutputData, other.mOutputData+other.mEntries, mOutputData);
    std::copy(other.mNlFlags, other.mNlFlags+other.mEntries, mNlFlags);
}

DataRow::DataRow(DataRow &&other) noexcept
    : mEntries(other.mEntries)
    , mEntriesNl(other.mEntriesNl)
    , mColIdx(other.mColIdx)
    , mInputData(other.mInputData)
    , mOutputData(other.mOutputData)
    , mNlFlags(other.mNlFlags)
{
    other.mEntries = 0;
    other.mEntriesNl = 0;
    other.mColIdx = nullptr;
    other.mInputData = nullptr;
    other.mOutputData = nullptr;
    other.mNlFlags = nullptr;
}

DataRow::~DataRow()
{
    if (mColIdx) delete [] mInputData;
    if (mOutputData) delete [] mOutputData;
    if (mInputData) delete [] mColIdx;
    if (mNlFlags) delete [] mNlFlags;
}

int DataRow::entries() const
{
    return mEntries;
}

void DataRow::setEntries(int entries)
{
    mEntries = entries;
}

int DataRow::entriesNl() const
{
    return mEntriesNl;
}

void DataRow::setEntriesNl(int entries)
{
    mEntriesNl = entries;
}

int *DataRow::colIdx() const
{
    return mColIdx;
}

void DataRow::setColIdx(int *colIdx)
{
    mColIdx = colIdx;
}

double *DataRow::inputData() const
{
    return mInputData;
}

void DataRow::setInputData(double *inputData)
{
    mInputData = inputData;
}

double *DataRow::outputData() const
{
    return mOutputData ? mOutputData : mInputData;
}

void DataRow::setOutputData(double *outputData)
{
    mOutputData = outputData;
}

int *DataRow::nlFlags() const
{
    return mNlFlags;
}

void DataRow::setNlFlags(int *nlFlags)
{
    mNlFlags = nlFlags;
}

QVariant DataRow::inputValue(int index, int lastSymIndex)
{
    int entries = lastSymIndex < mEntries ? lastSymIndex+1 : mEntries;
    for (int i=0; i<entries; ++i) {
        if (mColIdx[i] == index) {
            return mInputData[i];
        }
    }
    return QVariant();
}

QVariant DataRow::outputValue(int index, int lastSymIndex)
{
    int entries = lastSymIndex < mEntries ? lastSymIndex+1 : mEntries;
    for (int i=0; i<entries; ++i) {
        if (mColIdx[i] == index) {
            return outputData()[i];
        }
    }
    return QVariant();
}

DataRow& DataRow::operator=(const DataRow &other)
{
    delete [] mColIdx;
    delete [] mInputData;
    delete [] mOutputData;
    delete [] mNlFlags;
    mEntries = other.mEntries;
    mEntriesNl = other.mEntriesNl;
    mColIdx = new int[mEntries];
    mInputData = new double[mEntries];
    mOutputData = new double[mEntries];
    mNlFlags = new int[mEntries];
    std::copy(other.mColIdx, other.mColIdx+other.mEntries, mColIdx);
    std::copy(other.mInputData, other.mInputData+other.mEntries, mInputData);
    std::copy(other.mOutputData, other.mOutputData+other.mEntries, mOutputData);
    std::copy(other.mNlFlags, other.mNlFlags+other.mEntries, mNlFlags);
    return *this;
}

DataRow& DataRow::operator=(DataRow &&other) noexcept
{
    mEntries = other.mEntries;
    mEntriesNl = other.mEntriesNl;
    mColIdx = other.mColIdx;
    mInputData = other.mInputData;
    mOutputData = other.mOutputData;
    mNlFlags = other.mNlFlags;
    other.mEntries = 0;
    other.mEntriesNl = 0;
    other.mColIdx = nullptr;
    other.mInputData = nullptr;
    other.mOutputData = nullptr;
    other.mNlFlags = nullptr;
    return *this;
}

DataMatrix::DataMatrix()
    : mRowCount(0)
    , mColumnCount(0)
    , mRows(nullptr)
    , mEvalPoint(nullptr)
    , mModelType(0)
{

}

DataMatrix::DataMatrix(int rows, int columns, int modelType)
    : mRowCount(rows)
    , mColumnCount(columns)
    , mRows(new DataRow[mRowCount])
    , mEvalPoint(new double[columns])
    , mModelType(modelType)
{

}

DataMatrix::DataMatrix(const DataMatrix &other)
    : mRowCount(other.mRowCount)
    , mColumnCount(other.mColumnCount)
    , mRows(new DataRow[mRowCount])
    , mEvalPoint(new double[mColumnCount])
    , mModelType(other.mModelType)
{
    std::copy(other.mRows, other.mRows+other.mRowCount, mRows);
    std::copy(other.mEvalPoint, other.mEvalPoint+other.mColumnCount, mEvalPoint);
}

DataMatrix::DataMatrix(DataMatrix &&other) noexcept
    : mRowCount(other.mRowCount)
    , mColumnCount(other.mColumnCount)
    , mRows(other.mRows)
    , mEvalPoint(other.mEvalPoint)
    , mModelType(other.mModelType)
{
    other.mRowCount = 0;
    other.mColumnCount = 0;
    other.mRows = nullptr;
    other.mEvalPoint = nullptr;
}

DataMatrix::~DataMatrix()
{
    if (mRows) delete[] mRows;
    if (mEvalPoint) delete[] mEvalPoint;
}

int DataMatrix::rowCount() const
{
    return mRowCount;
}

int DataMatrix::columnCount() const
{
    return mColumnCount;
}

double *DataMatrix::evalPoint()
{
    return mEvalPoint;
}

DataRow *DataMatrix::row(int row)
{
    return (row < 0 || row > mRowCount) ? nullptr : mRows+row;
}

bool DataMatrix::isLinear() const
{
    return !mModelType;
}

DataMatrix& DataMatrix::operator=(const DataMatrix &other)
{
    delete [] mRows;
    mRowCount = other.mRowCount;
    mColumnCount = other.mColumnCount;
    mRows = new DataRow[mRowCount];
    std::copy(other.mRows, other.mRows+other.mRowCount, mRows);
    mEvalPoint = new double[mColumnCount];
    std::copy(other.mEvalPoint, other.mEvalPoint+other.mColumnCount, mEvalPoint);
    mModelType = other.mModelType;
    return *this;
}

DataMatrix& DataMatrix::operator=(DataMatrix &&other) noexcept
{
    mRowCount = other.mRowCount;
    mColumnCount = other.mColumnCount;
    mRows = other.mRows;
    mEvalPoint = other.mEvalPoint;
    other.mRowCount = 0;
    other.mColumnCount = 0;
    other.mRows = nullptr;
    other.mEvalPoint = nullptr;
    mModelType = other.mModelType;
    return *this;
}

}
}
}
