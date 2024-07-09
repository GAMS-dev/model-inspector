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
#ifndef TABLEVIEWFRAME_H
#define TABLEVIEWFRAME_H

#include <QFrame>
#include <QSharedPointer>

#include "common.h"
#include "aggregation.h"

class QAbstractItemModel;
class QMenu;

namespace gams {
namespace studio{
namespace modelinspector {

namespace Ui {
class TableViewFrame;
}

class EquationAttributeTableModel;
class VariableAttributeTableModel;
class ColumnRowFilterModel;
class IdentifierFilterModel;
class IdentifierLabelFilterModel;
class ValueFormatProxyModel;
class LabelFilterModel;
class AggregationProxyModel;
class HierarchicalHeaderView;
class JaccobianTableModel;
class ModelInstanceTableModel;
class MinMaxModelInstanceTableModel;
class AbstractModelInstance;

class TableViewFrame : public QFrame
{
    Q_OBJECT

public:
    TableViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    virtual TableViewFrame* clone(int view) = 0;

    const IdentifierFilter& identifierFilter() const;

    const IdentifierFilter& defaultIdentifierFilter() const;

    virtual void setIdentifierFilter(const IdentifierFilter &filter);

    const ValueFilter& valueFilter() const;

    const ValueFilter& defaultValueFilter() const;

    virtual void setValueFilter(const ValueFilter &filter);

    const LabelFilter& labelFilter() const;

    const LabelFilter& defaultLabelFilter() const;

    virtual void setLabelFilter(const LabelFilter &filter);

    const Aggregation& aggregation() const;

    virtual const Aggregation& appliedAggregation() const;

    const Aggregation& defaultAggregation() const;

    virtual void setAggregation(const Aggregation &aggregation, int view);

    virtual PredefinedViewEnum type() const;

    virtual void setShowAbsoluteValues(bool absoluteValues);

    virtual QAbstractItemModel* model() const = 0;

    virtual void setupView(QSharedPointer<AbstractModelInstance> modelInstance, int view) = 0;

    virtual void setSearchSelection(const gams::studio::modelinspector::SearchResult &result);

    virtual void setupFiltersAggregation(QAbstractItemModel *model, const LabelFilter &filter);

    virtual void reset(int view);

    QList<SearchResult> search(const QString &term, bool isRegEx);

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void updateView();

signals:
    void newModelView(gams::studio::modelinspector::PredefinedViewEnum type);

private slots:
    virtual void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                          Qt::Orientation orientation) = 0;

signals:
    void filtersChanged();

protected:
    IdentifierStates defaultSymbolFilter(QAbstractItemModel *model,
                                         Qt::Orientation orientation) const;

    void setIdentifierFilterCheckState(int symbolIndex, Qt::CheckState state,
                                       Qt::Orientation orientation);

    virtual Aggregation getDefaultAggregation() const;

    virtual Aggregation appliedAggregation(const Aggregation &aggregation, int view) const;

    void cloneFilterAndAggregation(TableViewFrame *clone, int newView);

    virtual void updateValueFilter();

protected:
    Ui::TableViewFrame* ui;
    QSharedPointer<AbstractModelInstance> mModelInstance;

    LabelFilter mCurrentLabelFilter;
    LabelFilter mDefaultLabelFilter;

    IdentifierFilter mCurrentIdentifierFilter;
    IdentifierFilter mDefaultIdentifierFilter;

    ValueFilter mCurrentValueFilter;
    ValueFilter mDefaultValueFilter;

    Aggregation mCurrentAggregation;
    Aggregation mDefaultAggregation;

    HierarchicalHeaderView* mHorizontalHeader = nullptr;
    HierarchicalHeaderView* mVerticalHeader = nullptr;
    ValueFormatProxyModel* mValueFormatModel = nullptr;
    IdentifierFilterModel* mIdentifierFilterModel = nullptr;
    IdentifierLabelFilterModel* mIdentifierLabelFilterModel = nullptr;
    LabelFilterModel* mLabelFilterModel = nullptr;
    AggregationProxyModel* mAggregationModel = nullptr; // TODO not used in all views... additinal abstract class?
    ColumnRowFilterModel* mColumnRowFilterModel = nullptr;
};

class EqnTableViewFrame : public TableViewFrame
{
    Q_OBJECT

public:
    EqnTableViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    TableViewFrame* clone(int view) override;

    QAbstractItemModel* model() const override;

    PredefinedViewEnum type() const override
    {
        return PredefinedViewEnum::EqnAttributes;
    }

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance, int view) override;

    void setLabelFilter(const LabelFilter &filter) override;

    void setValueFilter(const ValueFilter &filter) override;

protected:
    Aggregation getDefaultAggregation() const override;

private slots:
    void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                  Qt::Orientation orientation) override;

private:
    QSharedPointer<EquationAttributeTableModel> mBaseModel;
};

class VarTableViewFrame : public TableViewFrame
{
    Q_OBJECT

public:
    VarTableViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    TableViewFrame* clone(int view) override;

    QAbstractItemModel* model() const override;

    PredefinedViewEnum type() const override
    {
        return PredefinedViewEnum::VarAttributes;
    }

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance, int view) override;

    void setLabelFilter(const LabelFilter &filter) override;

    void setValueFilter(const ValueFilter &filter) override;

    void setupFiltersAggregation(QAbstractItemModel *model, const LabelFilter &filter) override;

protected:
    Aggregation getDefaultAggregation() const override;

private slots:
    void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                  Qt::Orientation orientation) override;

private:
    QSharedPointer<VariableAttributeTableModel> mBaseModel;
};

class JaccTableViewFrame : public TableViewFrame
{
    Q_OBJECT

public:
    JaccTableViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    TableViewFrame* clone(int view) override;

    QAbstractItemModel* model() const override;

    PredefinedViewEnum type() const override
    {
        return PredefinedViewEnum::Jaccobian;
    }

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance, int view) override;

    void setLabelFilter(const LabelFilter &filter) override;

    void setValueFilter(const ValueFilter &filter) override;

public slots:
    void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                  Qt::Orientation orientation) override;

private:
    QSharedPointer<JaccobianTableModel> mBaseModel;
};

class FullTableViewFrame : public TableViewFrame
{
    Q_OBJECT

public:
    FullTableViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    TableViewFrame* clone(int view) override;

    QAbstractItemModel* model() const override;

    PredefinedViewEnum type() const override
    {
        return PredefinedViewEnum::Full;
    }

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance, int view) override;

    void setLabelFilter(const LabelFilter &filter) override;

    void setValueFilter(const ValueFilter &filter) override;

public slots:
    void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                  Qt::Orientation orientation) override;

private:
    QSharedPointer<ModelInstanceTableModel> mModelInstanceModel;
};

class MinMaxTableViewFrame : public TableViewFrame
{
    Q_OBJECT

public:
    MinMaxTableViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    TableViewFrame* clone(int view) override;

    QAbstractItemModel* model() const override;

    PredefinedViewEnum type() const override
    {
        return PredefinedViewEnum::MinMax;
    }

    const Aggregation& appliedAggregation() const override;

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance, int view) override;

    void setLabelFilter(const LabelFilter &filter) override;

    void setValueFilter(const ValueFilter &filter) override;

    void setAggregation(const Aggregation &aggregation, int view) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    void setupFiltersAggregation(QAbstractItemModel *model, const LabelFilter &filter) override;

    QList<Symbol> selectedEquations() const;
    QList<Symbol> selectedVariables() const;

    void reset(int view) override;

public slots:
    void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                  Qt::Orientation orientation) override;

private slots:
    void customMenuRequested(const QPoint &pos);

protected:
    Aggregation getDefaultAggregation() const override;

    Aggregation appliedAggregation(const Aggregation &aggregation, int view) const override;

private:
    void setupSelectionMenu();
    void handleRowColumnSelection(PredefinedViewEnum type);

private:
    QSharedPointer<MinMaxModelInstanceTableModel> mModelInstanceModel;
    QMenu *mSelectionMenu;

    QList<Symbol> mSelectedEquations;
    QList<Symbol> mSelectedVariables;
};

}
}
}

#endif // TABLEVIEWFRAME_H
