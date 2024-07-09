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
#ifndef DATAMATRIX_H
#define DATAMATRIX_H

#include <QVariant>

namespace gams {
namespace studio {
namespace mii {

class DataRow
{
public:
    DataRow();

    DataRow(int entries);

    DataRow(const DataRow& other);

    DataRow(DataRow&& other) noexcept;

    ~DataRow();

    int entries() const;

    void setEntries(int entries);

    int entriesNl() const;

    void setEntriesNl(int entries);

    int* colIdx() const;

    void setColIdx(int* colIdx);

    ///
    /// \brief Pointer to input data, always available.
    /// \return Pointer to input data.
    ///
    double* inputData() const;

    void setInputData(double* inputData);

    ///
    /// \brief Pointer to output data, if available.
    /// \return Pointer to ouput data.
    ///
    double* outputData() const;

    void setOutputData(double* outputData);

    int* nlFlags() const;

    void setNlFlags(int *nlFlags);

    QVariant inputValue(int index, int lastSymIndex);

    QVariant outputValue(int index, int lastSymIndex);

    DataRow& operator=(const DataRow& other);

    DataRow& operator=(DataRow&& other) noexcept;

private:
    int mEntries;
    int mEntriesNl;
    int* mColIdx;
    double *mInputData;
    double *mOutputData;
    int *mNlFlags;
};

class DataMatrix
{
public:
    DataMatrix();

    DataMatrix(int rows, int columns, int modelType);

    DataMatrix(const DataMatrix& other);

    DataMatrix(DataMatrix&& other) noexcept;

    ~DataMatrix();

    int rowCount() const;

    int columnCount() const;

    double* evalPoint();

    DataRow* row(int row);

    bool isLinear() const;

    DataMatrix& operator=(const DataMatrix& other);

    DataMatrix& operator=(DataMatrix&& other) noexcept;

private:
    int mRowCount;
    int mColumnCount;
    DataRow *mRows;
    double *mEvalPoint;
    int mModelType;
};

}
}
}

#endif // DATAMATRIX_H
