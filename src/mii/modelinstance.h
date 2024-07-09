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
#ifndef MODELINSTANCE_H
#define MODELINSTANCE_H

#include "abstractmodelinstance.h"

#include "gevmcc.h"
#include "gmomcc.h"
#include "dctmcc.h"

#include <QVariant>

namespace gams {
namespace studio {
namespace mii {

class DataHandler;
class DataMatrix;

class ModelInstance final : public AbstractModelInstance
{
public:
    ModelInstance(bool useOutput = false,
                  const QString &workspace = ".",
                  const QString &systemDir = QString(),
                  const QString &scratchDir = QString());

    ~ModelInstance() override;

    QString modelName() const override;

    int equationCount() const override;

    int equationCount(ValueHelper::EquationType type) const override;

    unsigned char equationType(int row) const override;

    int equationRowCount() const override;

    Symbol* equation(int sectionIndex) const override;

    const QVector<Symbol*>& equations() const override;

    int variableCount() const override;

    int variableCount(ValueHelper::VariableType type) const override;

    char variableType(int column) const override;

    int variableRowCount() const override;

    Symbol* variable(int sectionIndex) const override;

    const QVector<Symbol*>& variables() const override;

    void variableLowerBounds(double *bounds) override;

    void variableUpperBounds(double *bounds) override;

    double rhs(int row) const override;

    QString longestEquationText() const override;

    QString longestVariableText() const override;

    QString longestLabelText() const override;

    int maximumEquationDimension() const override;

    int maximumVariableDimension() const override;

    double modelMinimum() const override;

    double modelMaximum() const override;

    const QVector<Symbol*>& symbols(Symbol::Type type) const override;

    void loadBaseData() override;

    void loadViewData(const QSharedPointer<AbstractViewConfiguration> &viewConfig) override;

    int rowCount(int viewId) const override;

    int rowEntryCount(int row, int viewId) const override;

    int columnCount(int viewId) const override;

    int columnEntryCount(int column, int viewId) const override;

    const QList<int>& rowIndices(int viewId, int row) const override;

    const QList<int>& columnIndices(int viewId, int column) const override;

    int symbolRowCount(int viewId) const override;

    int symbolColumnCount(int viewId) const override;

    QSharedPointer<AbstractViewConfiguration> clone(int viewId, int newViewId) override;

    QVariant data(int row, int column, int viewId) const override;

    int nlFlag(int row, int column, int viewId) override;

    QSharedPointer<PostoptTreeItem> dataTree(int viewId) const override;

    QVariant headerData(int logicalIndex,
                        Qt::Orientation orientation,
                        int viewId,
                        int role) const override;

    QVariant plainHeaderData(Qt::Orientation orientation,
                             int viewId,
                             int logicalIndex,
                             int dimension) const override;
    
    DataMatrix* jacobianData() override;

    QVariant equationAttribute(const QString &header,
                               int index, int entry, bool abs) const override;

    QVariant variableAttribute(const QString &header,
                               int index, int entry, bool abs) const override;

    int maxSymbolDimension(int viewId, Qt::Orientation orientation) const override;

    void removeViewData(int viewId) override;

    void removeViewData() override;

private:
    void initialize();

    int symbolCount() const;

    void loadScratchData();

    void loadEvaluationPoint(double *evalPoint, int size);

    void loadSymbols();
    Symbol* loadSymbol(int index);
    void loadEquationDimensions(Symbol *symbol);
    void loadVariableDimensions(Symbol *symbol);
    void loadLabels();

    QPair<double, double> equationBounds(int row) const;

    bool isInf(double value) const;
    double specialValue(double value) const;
    bool isSpecialValue(double value) const;
    QVariant specialValuePostopt(double value, bool abs) const;
    QVariant specialMarginalEquValueBasis(double value, int rIndex, bool abs);
    QVariant specialMarginalVarValueBasis(double value, int cIndex, bool abs);

    static int errorCallback(int count, const char *message);

private:
    DataHandler *mDataHandler;

    gevHandle_t mGEV = nullptr;
    gmoHandle_t mGMO = nullptr;
    dctHandle_t mDCT = nullptr;
    std::function<QVariant(double, int, bool)> specialMarginalEquValuePtr;
    std::function<QVariant(double, int, bool)> specialMarginalVarValuePtr;

    int mMaxEquationDimension = 0;
    int mMaxVariableDimension = 0;

    QList<Symbol*> hSectionIndexToSymbol;
    QList<Symbol*> vSectionIndexToSymbol;

    QVector<Symbol*> mEquations;
    QVector<Symbol*> mVariables;

    QString mLongestLabel;
    QString mLongestEqnText;
    QString mLongestVarText;
};

}
}
}

#endif // MODELINSTANCE_H
