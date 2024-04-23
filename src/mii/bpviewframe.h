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
#ifndef BPVIEWFRAME_H
#define BPVIEWFRAME_H

#include "abstracttableviewframe.h"

namespace gams {
namespace studio{
namespace mii {

class BPOverviewTableModel;
class BPIdentifierFilterModel;
class ComprehensiveTableModel;
class HierarchicalHeaderView;
class ComprehensiveTableModel;
class ValueFormatProxyModel;
class Symbol;

class AbstractBPViewFrame : public AbstractTableViewFrame
{
    Q_OBJECT

public:
    AbstractBPViewFrame(ComprehensiveTableModel *model,
                        QWidget *parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~AbstractBPViewFrame();

    const QList<Symbol*>& selectedEquations() const;
    const QList<Symbol*>& selectedVariables() const;

    bool hasData() const override;

signals:
    void newSymbolViewRequested();

private slots:
    void customMenuRequested(const QPoint &pos);

    void handleRowColumnSelection();

protected:
    void setIdentifierFilterCheckState(int symbolIndex,
                                       Qt::CheckState state,
                                       Qt::Orientation orientation);

protected:
    QSharedPointer<ComprehensiveTableModel> mBaseModel;
    BPIdentifierFilterModel* mIdentifierFilterModel = nullptr;

    QMenu *mSelectionMenu;
    QAction *mSymbolAction = new QAction("Show selected symbols", this);

    QList<Symbol*> mSelectedEquations;
    QList<Symbol*> mSelectedVariables;
};

class BPOverviewViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPOverviewViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPOverviewViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                        const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                        QWidget *parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int viewId) override;

    void setupView(const QSharedPointer<AbstractModelInstance> &modelInstance) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    inline ViewHelper::ViewDataType type() const override
    {
        return ViewHelper::ViewDataType::BP_Overview;
    }

public slots:
    void evaluateFilters() override;

private:
    void setupView();
};

class BPCountViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPCountViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPCountViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                     const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                     QWidget *parent = nullptr,
                     Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int viewId) override;

    void setupView(const QSharedPointer<AbstractModelInstance> &modelInstance) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    inline ViewHelper::ViewDataType type() const override
    {
        return ViewHelper::ViewDataType::BP_Count;
    }

public slots:
    void evaluateFilters() override;

private:
    void setupView();

private:
    ValueFormatProxyModel* mValueFormatModel = nullptr;
    HierarchicalHeaderView* mVerticalHeader = nullptr;
};

class BPAverageViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPAverageViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPAverageViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                       const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                       QWidget *parent = nullptr,
                       Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int viewId) override;

    void setupView(const QSharedPointer<AbstractModelInstance> &modelInstance) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    inline ViewHelper::ViewDataType type() const override
    {
        return ViewHelper::ViewDataType::BP_Average;
    }

public slots:
    void evaluateFilters() override;

private:
    void setupView();

private:
    ValueFormatProxyModel* mValueFormatModel = nullptr;
    HierarchicalHeaderView* mVerticalHeader = nullptr;
};

class BPScalingViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPScalingViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPScalingViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                       const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                       QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame* clone(int viewId) override;

    inline ViewHelper::ViewDataType type() const override
    {
        return ViewHelper::ViewDataType::BP_Scaling;
    }

    void setShowAbsoluteValues(bool absoluteValues) override;

    void setupView(const QSharedPointer<AbstractModelInstance> &modelInstance) override;

public slots:
    void evaluateFilters() override;

private:
    void setupView();

private:
    ValueFormatProxyModel* mValueFormatModel = nullptr;
    HierarchicalHeaderView* mVerticalHeader = nullptr;
};

}
}
}

#endif // BPVIEWFRAME_H
