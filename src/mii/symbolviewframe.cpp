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
#include "symbolviewframe.h"
#include "viewconfigurationprovider.h"
#include "symbolhierarchicalheaderview.h"
#include "ui_standardtableviewframe.h"
#include "symbolmodelinstancetablemodel.h"
#include "abstractmodelinstance.h"
#include "symbolfiltermodel.h"
#include "dtoaformatproxymodel.h"

#include <QAction>
#include <QMenu>
#include <QScrollBar>

namespace gams {
namespace studio{
namespace mii {

SymbolViewFrame::SymbolViewFrame(int view,
                                 const QSharedPointer<AbstractModelInstance> &modelInstance,
                                 QWidget *parent,
                                 Qt::WindowFlags f)
    : AbstractTableViewFrame(parent, f)
    , mSelectionMenu(new QMenu(this))
{
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    mSelectionMenu->addAction(mResetAction);
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(),
                                                                                                     modelInstance));
    mViewConfig->setViewId(view);
    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &SymbolViewFrame::customMenuRequested);
    connect(mResetAction, &QAction::triggered, this, &SymbolViewFrame::resetHeaderFilter);
}

SymbolViewFrame::SymbolViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                 const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                 QWidget *parent, Qt::WindowFlags f)
    : AbstractTableViewFrame(parent, f)
    , mSelectionMenu(new QMenu(this))
{
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    mSelectionMenu->addAction(mResetAction);
    mModelInstance = modelInstance;
    mViewConfig = viewConfig;
    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &SymbolViewFrame::customMenuRequested);
    connect(mResetAction, &QAction::triggered, this, &SymbolViewFrame::resetHeaderFilter);
}

AbstractTableViewFrame *SymbolViewFrame::clone(int viewId)
{
    auto viewConfig = QSharedPointer<AbstractViewConfiguration>(mModelInstance->clone(mViewConfig->viewId(), viewId));
    auto frame = new SymbolViewFrame(mModelInstance, viewConfig, parentWidget(), windowFlags());
    frame->setupView();
    frame->evaluateFilters();
    return frame;
}

void SymbolViewFrame::setupView(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    mModelInstance = modelInstance;
    mViewConfig->setModelInstance(mModelInstance);
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

ViewHelper::ViewDataType SymbolViewFrame::type() const
{
    return ViewHelper::ViewDataType::Symbols;
}

bool SymbolViewFrame::hasData() const
{
    return mBaseModel && mBaseModel->rowCount() && mBaseModel->columnCount();
}

void SymbolViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    if (!mBaseModel)
        return;
    mViewConfig->currentValueFilter().UseAbsoluteValues = absoluteValues;
    mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = absoluteValues;
    mViewConfig->defaultValueFilter().MaxValue = std::numeric_limits<double>::max();
    mViewConfig->defaultValueFilter().MinValue = std::numeric_limits<double>::lowest();
    mViewConfig->currentValueFilter().MaxValue = std::numeric_limits<double>::max();
    mViewConfig->currentValueFilter().MinValue = std::numeric_limits<double>::lowest();
    evaluateFilters();
}

void SymbolViewFrame::evaluateFilters()
{
    if (!mHeaderFilterModel)
        return;
    mModelInstance->loadViewData(mViewConfig);
    mHeaderFilterModel->evaluateFilters();
    emit filtersChanged();
}

void SymbolViewFrame::customMenuRequested(const QPoint &pos)
{
    mSelectionMenu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}

void SymbolViewFrame::resetHeaderFilter()
{
    if (!mHorizontalHeader || !mVerticalHeader)
        return;
    auto localAbs = mViewConfig->currentValueFilter().UseAbsoluteValues;
    auto globalAbs = mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal;
    mViewConfig->resetValueFilter();
    if (globalAbs) {
        mViewConfig->currentValueFilter().UseAbsoluteValues = localAbs;
        mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = globalAbs;
    }
    mViewConfig->resetIdentifierFilter();
    mViewConfig->resetLabelFilter();
    evaluateFilters();
}

void SymbolViewFrame::setupView()
{
    mHorizontalHeader = new SymbolHierarchicalHeaderView(Qt::Horizontal,
                                                         mModelInstance,
                                                         ui->tableView);
    connect(mHorizontalHeader, &SymbolHierarchicalHeaderView::filterChanged,
            this, &SymbolViewFrame::evaluateFilters);
    mVerticalHeader = new SymbolHierarchicalHeaderView(Qt::Vertical,
                                                       mModelInstance,
                                                       ui->tableView);
    connect(mVerticalHeader, &SymbolHierarchicalHeaderView::filterChanged,
            this, &SymbolViewFrame::evaluateFilters);

    auto baseModel = new SymbolModelInstanceTableModel(mModelInstance, mViewConfig, ui->tableView);
    mHeaderFilterModel = new SymbolFilterModel(mModelInstance, mViewConfig, ui->tableView);
    mHeaderFilterModel->setSourceModel(baseModel);
    auto dtoaModel = new DtoaFormatProxyModel(ui->tableView);
    dtoaModel->setSourceModel(mHeaderFilterModel);

    ui->tableView->setHorizontalHeader(mHorizontalHeader);
    ui->tableView->setVerticalHeader(mVerticalHeader);
    auto oldSelectionModel = ui->tableView->selectionModel();
    ui->tableView->setModel(dtoaModel);
    delete oldSelectionModel;
    mHorizontalHeader->setVisible(true);
    mVerticalHeader->setVisible(true);

    mBaseModel = QSharedPointer<SymbolModelInstanceTableModel>(baseModel);

    connect(ui->tableView->verticalScrollBar(), &QScrollBar::valueChanged,
            ui->tableView->model(), [this]{
        emit ui->tableView->model()->headerDataChanged(Qt::Vertical, 0, 1);
    });
    connect(ui->tableView->horizontalScrollBar(), &QScrollBar::valueChanged,
            ui->tableView->model(), [this]{
        emit ui->tableView->model()->headerDataChanged(Qt::Horizontal, 0, 1);
    });
}

}
}
}
