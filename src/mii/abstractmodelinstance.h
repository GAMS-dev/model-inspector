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
#ifndef ABSTRACTMODELINSTANCE_H
#define ABSTRACTMODELINSTANCE_H

#include "symbol.h"

#include <QString>
#include <QStringList>
#include <QSharedPointer>

namespace gams {
namespace studio {
namespace mii {

class AbstractViewConfiguration;
class DataMatrix;
class PostoptTreeItem;

class AbstractModelInstance
{
protected:
    AbstractModelInstance(const QString &workspace,
                          const QString &systemDir,
                          const QString &scratchDir);

public:
    enum State
    {
        Valid,
        Warning,
        Error
    };

    virtual ~AbstractModelInstance();

    QString workspace() const;

    void setWorkspace(const QString &workspace);

    QString systemDirectory() const;

    void setSystemDirectory(const QString &systemDir);

    QString scratchDirectory() const;

    void setScratchDirectory(const QString &scratchDir);

    bool globalAbsolute() const;

    void setGlobalAbsolute(bool absolute);

    bool useOutput() const;

    void setUseOutput(bool useOutput);

    virtual double modelMinimum() const = 0;
    virtual double modelMaximum() const = 0;

    QString logMessages();

    virtual QString modelName() const;

    /**
     * @brief Total number of equations.
     * @return Equation count.
     */
    virtual int equationCount() const;

    /**
     * @brief Number of equations defined by <c>type</c>.
     * @param GMO equation type.
     */
    virtual int equationCount(ValueHelper::EquationType type) const;

    virtual unsigned char equationType(int row) const = 0;

    /**
     * @brief Total number of equation rows.
     * @return Equation row count.
     */
    virtual int equationRowCount() const;

    /**
     * @brief Specific equation by <c>sectionIndex</c>
     * @param Section index of equation in raw data matrix.
     * @return Equation.
     */
    virtual Symbol* equation(int sectionIndex) const;

    /**
     * @brief All equations known by the model instance.
     * @return All known equations.
     */
    virtual const QVector<Symbol*>& equations() const = 0;

    /**
     * @brief Total number of variables.
     */
    virtual int variableCount() const;

    virtual char variableType(int column) const = 0;

    /**
     * @brief Number of variables defined by <c>type</c>.
     * @param Equation type.
     */
    virtual int variableCount(ValueHelper::VariableType type) const;

    /**
     * @brief Total number of variable rows.
     * @return Variable row count.
     */
    virtual int variableRowCount() const;

    /**
     * @brief Specific variable by <c>sectionIndex</c>
     * @param Section index of variable in raw data matrix.
     * @return Variable.
     */
    virtual Symbol* variable(int sectionIndex) const;

    /**
     * @brief All variables known by the model instance.
     * @return All known variables.
     */
    virtual const QVector<Symbol*>& variables() const = 0;

    virtual void variableLowerBounds(double *bounds) = 0;

    virtual void variableUpperBounds(double *bounds) = 0;

    virtual double rhs(int row) const = 0;

    const QStringList& labels() const;

    virtual QString longestEquationText() const = 0;

    virtual QString longestVariableText() const = 0;

    virtual QString longestLabelText() const = 0;

    virtual int maximumEquationDimension() const = 0;

    virtual int maximumVariableDimension() const = 0;

    virtual const QVector<Symbol*>& symbols(Symbol::Type type) const = 0;

    virtual void loadBaseData() = 0;

    virtual int rowCount(int view) const = 0;

    virtual int rowEntryCount(int row, int view) const = 0;

    virtual int columnCount(int view) const = 0;

    virtual const QList<int>& rowIndices(int viewId, int row) const = 0;

    virtual const QList<int>& columnIndices(int viewId, int column) const = 0;

    virtual int columnEntryCount(int column, int view) const = 0;

    virtual int symbolRowCount(int view) const = 0;

    virtual int symbolColumnCount(int view) const = 0;

    virtual QSharedPointer<AbstractViewConfiguration> clone(int view, int newView) = 0;

    virtual void loadViewData(const QSharedPointer<AbstractViewConfiguration>& viewConfig) = 0;

    virtual QVariant data(int row, int column, int view) const = 0;

    virtual int nlFlag(int row, int column, int viewId);

    virtual QSharedPointer<PostoptTreeItem> dataTree(int view) const = 0;

    virtual QVariant plainHeaderData(Qt::Orientation orientation,
                                     int view, int logicalIndex,
                                     int dimension = 0) const = 0;

    virtual QVariant headerData(int logicalIndex,
                                Qt::Orientation orientation,
                                int view, int role) const = 0;

    virtual DataMatrix* jacobianData() = 0;

    virtual QVariant equationAttribute(const QString &header, int index, int entry, bool abs) const;

    virtual QVariant variableAttribute(const QString &header, int index, int entry, bool abs) const;

    virtual int maxSymbolDimension(int viewId, Qt::Orientation orientation) const = 0;

    virtual void removeViewData(int viewId) = 0;

    virtual void removeViewData() = 0;

    State state() const;

protected:
    State mState = Valid;

    QString mScratchDir;
    QString mWorkspace;
    QString mSystemDir;

    bool mGlobalAbsolute = false;

    bool mUseOutput = false;

    QStringList mLogMessages;

    QStringList mLabels;
};

class EmptyModelInstance final : public AbstractModelInstance
{
public:
    EmptyModelInstance(const QString &workspace = ".",
                       const QString &systemDir = QString(),
                       const QString &scratchDir = QString());

    ~EmptyModelInstance() override;

    const QVector<Symbol*>& equations() const override;

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

    unsigned char equationType(int row) const override;

    char variableType(int column) const override;

    int rowCount(int view) const override;

    int rowEntryCount(int row, int view) const override;

    int columnCount(int view) const override;

    int columnEntryCount(int column, int view) const override;

    const QList<int>& rowIndices(int viewId, int row) const override;

    const QList<int>& columnIndices(int viewId, int column) const override;

    int symbolRowCount(int view) const override;

    int symbolColumnCount(int view) const override;

    QSharedPointer<AbstractViewConfiguration> clone(int view, int newView) override;

    void loadViewData(const QSharedPointer<AbstractViewConfiguration>& viewConfig) override;

    QVariant data(int row, int column, int view) const override;

    QSharedPointer<PostoptTreeItem> dataTree(int view) const override;

    QVariant headerData(int logicalIndex,
                        Qt::Orientation orientation,
                        int view, int role) const override;

    QVariant plainHeaderData(Qt::Orientation orientation,
                             int view, int logicalIndex,
                             int dimension) const override;

    DataMatrix* jacobianData() override;

    int maxSymbolDimension(int viewId, Qt::Orientation orientation) const override;

    void removeViewData(int viewId) override;

    void removeViewData() override;

private:
    QVector<Symbol*> mSymbols;
    QSharedPointer<PostoptTreeItem> mRootItem;
    QList<int> mIndices;
};

}
}
}

#endif // ABSTRACTMODELINSTANCE_H
