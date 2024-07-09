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
#include "bpviewframe.h"
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"
#include "comprehensivetablemodel.h"
#include "ui_standardtableviewframe.h"
#include "hierarchicalheaderview.h"
#include "bpidentifierfiltermodel.h"
#include "valueformatproxymodel.h"
#include "dtoaformatproxymodel.h"
#include "symbol.h"

#include <QAction>
#include <QMenu>
#include <QSharedPointer>

namespace gams {
namespace studio{
namespace mii {

AbstractBPViewFrame::AbstractBPViewFrame(ComprehensiveTableModel *model,
                                         QWidget *parent,
                                         Qt::WindowFlags f)
    : AbstractTableViewFrame(parent, f)
    , mBaseModel(model)
    , mSelectionMenu(new QMenu(this))
{
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    mSelectionMenu->addAction(mSymbolAction);
    connect(mSymbolAction, &QAction::triggered, this, [this]{handleRowColumnSelection();});
    connect(ui->tableView, &QWidget::customContextMenuRequested,
            this, &AbstractBPViewFrame::customMenuRequested);
}

AbstractBPViewFrame::~AbstractBPViewFrame()
{

}

const QList<Symbol*>& AbstractBPViewFrame::selectedEquations() const
{
    return mSelectedEquations;
}

const QList<Symbol*>& AbstractBPViewFrame::selectedVariables() const
{
    return mSelectedVariables;
}

bool AbstractBPViewFrame::hasData() const
{
    return mBaseModel && mBaseModel->rowCount() && mBaseModel->columnCount();
}

void AbstractBPViewFrame::customMenuRequested(const QPoint &pos)
{
    if (ui->tableView->selectionModel()->selectedIndexes().empty())
        return;
    auto view = mViewConfig->viewId();
    if (ui->tableView->selectionModel()->selectedIndexes().first().row() < mModelInstance->symbolRowCount(view) &&
        ui->tableView->selectionModel()->selectedIndexes().first().column() < mModelInstance->symbolColumnCount(view)) {
        mSymbolAction->setEnabled(true);
    } else {
        mSymbolAction->setEnabled(false);
    }
    mSelectionMenu->popup(ui->tableView->viewport()->mapToGlobal(pos));
}

void AbstractBPViewFrame::handleRowColumnSelection()
{
    QMap<int, Symbol*> rowSymbols, columnSymbols;
    auto indexes = ui->tableView->selectionModel()->selectedIndexes();
    for (const auto& index : indexes) {
        if (!index.isValid())
            continue;
        if (index.column() >= mModelInstance->variableCount())
            continue;
        if (index.row() >= mModelInstance->symbolRowCount(mViewConfig->viewId()))
            continue;
        int section = ui->tableView->model()->headerData(index.row(), Qt::Vertical,
                                                     ViewHelper::IndexDataRole).toInt();
        auto equation = mModelInstance->equation(section);
        rowSymbols[section] = equation;
        section = ui->tableView->model()->headerData(index.column(), Qt::Horizontal,
                                                     ViewHelper::IndexDataRole).toInt();
        auto variable = mModelInstance->variable(section);
        columnSymbols[section] = variable;
    }
    mSelectedEquations = rowSymbols.values();
    mSelectedVariables = columnSymbols.values();
    emit newSymbolViewRequested();
}

void AbstractBPViewFrame::setIdentifierFilterCheckState(int symbolIndex,
                                                        Qt::CheckState state,
                                                        Qt::Orientation orientation)
{
    auto symbols = mViewConfig->currentIdentifierFilter().value(orientation);
    for (auto iter=symbols.begin(); iter!=symbols.end(); ++iter) {
        if (iter->SymbolIndex == symbolIndex) {
            iter->Checked = state;
            break;
        }
    }
    mViewConfig->currentIdentifierFilter()[orientation] = symbols;
}

BPOverviewViewFrame::BPOverviewViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new BPOverviewTableModel, parent, f)
{
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::defaultConfiguration());
}

BPOverviewViewFrame::BPOverviewViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                         const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                         QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new BPOverviewTableModel, parent, f)
{
    mModelInstance = modelInstance;
    mViewConfig = viewConfig;
}

AbstractTableViewFrame *BPOverviewViewFrame::clone(int viewId)
{
    auto viewConfig = QSharedPointer<AbstractViewConfiguration>(mModelInstance->clone(mViewConfig->viewId(), viewId));
    if (!viewConfig)
        viewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(),
                                                                                                        mModelInstance));
    auto frame = new BPOverviewViewFrame(mModelInstance, viewConfig, parentWidget(), windowFlags());
    frame->setupView();
    frame->evaluateFilters();
    return frame;
}

void BPOverviewViewFrame::setupView(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    mModelInstance = modelInstance;
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(), mModelInstance));
    mViewConfig->currentValueFilter().UseAbsoluteValues = modelInstance->globalAbsolute();
    mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = modelInstance->globalAbsolute();
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

void BPOverviewViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    Q_UNUSED(absoluteValues);
}

void BPOverviewViewFrame::evaluateFilters()
{
    if (mIdentifierFilterModel)
        mIdentifierFilterModel->setIdentifierFilter(mViewConfig->currentIdentifierFilter());
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void BPOverviewViewFrame::setupView()
{
    auto baseModel = new BPOverviewTableModel(mViewConfig->viewId(),
                                              mModelInstance,
                                              ui->tableView);
    mIdentifierFilterModel = new BPIdentifierFilterModel(mModelInstance, ui->tableView);
    mIdentifierFilterModel->setSourceModel(baseModel);

    auto oldSelectionModel = ui->tableView->selectionModel();
    ui->tableView->setModel(mIdentifierFilterModel);
    delete oldSelectionModel;

    ui->tableView->verticalHeader()->setVisible(true);
    ui->tableView->horizontalHeader()->setVisible(true);
    mBaseModel = QSharedPointer<BPOverviewTableModel>(baseModel);

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

BPCountViewFrame::BPCountViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new ComprehensiveTableModel, parent, f)
{
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::defaultConfiguration());
}

BPCountViewFrame::BPCountViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                   const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                   QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new ComprehensiveTableModel, parent, f)
{
    mModelInstance = modelInstance;
    mViewConfig = viewConfig;
}

AbstractTableViewFrame *BPCountViewFrame::clone(int viewId)
{
    auto viewConfig = QSharedPointer<AbstractViewConfiguration>(mModelInstance->clone(mViewConfig->viewId(), viewId));
    if (!viewConfig)
        viewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(),
                                                                                                        mModelInstance));
    auto frame = new BPCountViewFrame(mModelInstance, viewConfig, parentWidget(), windowFlags());
    frame->setupView();
    frame->evaluateFilters();
    return frame;
}

void BPCountViewFrame::setupView(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    mModelInstance = modelInstance;
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(), mModelInstance));
    mViewConfig->currentValueFilter().UseAbsoluteValues = modelInstance->globalAbsolute();
    mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = modelInstance->globalAbsolute();
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

void BPCountViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    Q_UNUSED(absoluteValues);
}

void BPCountViewFrame::evaluateFilters()
{
    if (mIdentifierFilterModel)
        mIdentifierFilterModel->setIdentifierFilter(mViewConfig->currentIdentifierFilter());
    if (mValueFormatModel)
        mValueFormatModel->setValueFilter(mViewConfig->currentValueFilter());
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void BPCountViewFrame::setupView()
{
    mVerticalHeader = new HierarchicalHeaderView(Qt::Vertical,
                                                 mModelInstance,
                                                 mViewConfig,
                                                 ui->tableView);
    auto baseModel = new BPCountTableModel(mViewConfig->viewId(),
                                           mModelInstance,
                                           ui->tableView);
    mValueFormatModel = new BPValueFormatTypeProxyModel(ui->tableView);
    mValueFormatModel->setSourceModel(baseModel);
    mIdentifierFilterModel = new BPIdentifierFilterModel(mModelInstance, ui->tableView);
    mIdentifierFilterModel->setSourceModel(mValueFormatModel);

    ui->tableView->setVerticalHeader(mVerticalHeader);
    auto oldSelectionModel = ui->tableView->selectionModel();
    ui->tableView->setModel(mIdentifierFilterModel);
    delete oldSelectionModel;
    ui->tableView->verticalHeader()->setVisible(true);
    ui->tableView->horizontalHeader()->setVisible(true);
    mBaseModel = QSharedPointer<ComprehensiveTableModel>(baseModel);

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

BPAverageViewFrame::BPAverageViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new BPAverageTableModel, parent, f)
{
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::defaultConfiguration());
}

BPAverageViewFrame::BPAverageViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                       const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                       QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new BPAverageTableModel, parent, f)
{
    mModelInstance = modelInstance;
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    mViewConfig = viewConfig;
}

AbstractTableViewFrame *BPAverageViewFrame::clone(int viewId)
{
    auto viewConfig = QSharedPointer<AbstractViewConfiguration>(mModelInstance->clone(mViewConfig->viewId(), viewId));
    if (!viewConfig)
        viewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(),
                                                                                                        mModelInstance));
    auto frame = new BPAverageViewFrame(mModelInstance, viewConfig, parentWidget(), windowFlags());
    frame->setupView();
    frame->evaluateFilters();
    return frame;
}

void BPAverageViewFrame::setupView(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    mModelInstance = modelInstance;
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(), mModelInstance));
    mViewConfig->currentValueFilter().UseAbsoluteValues = modelInstance->globalAbsolute();
    mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = modelInstance->globalAbsolute();
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

void BPAverageViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    Q_UNUSED(absoluteValues);
}

void BPAverageViewFrame::evaluateFilters()
{
    if (mIdentifierFilterModel)
        mIdentifierFilterModel->setIdentifierFilter(mViewConfig->currentIdentifierFilter());
    if (mValueFormatModel)
        mValueFormatModel->setValueFilter(mViewConfig->currentValueFilter());
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void BPAverageViewFrame::setupView()
{
    mVerticalHeader = new HierarchicalHeaderView(Qt::Vertical,
                                                 mModelInstance,
                                                 mViewConfig,
                                                 ui->tableView);
    auto baseModel = new BPAverageTableModel(mViewConfig->viewId(),
                                             mModelInstance,
                                             ui->tableView);
    mValueFormatModel = new BPValueFormatTypeProxyModel(ui->tableView);
    mValueFormatModel->setSourceModel(baseModel);
    mIdentifierFilterModel = new BPIdentifierFilterModel(mModelInstance, ui->tableView);
    mIdentifierFilterModel->setSourceModel(mValueFormatModel);
    auto dtoaModel = new DtoaBpAverageFormatProxyModel(ui->tableView);
    dtoaModel->setSourceModel(mIdentifierFilterModel);

    ui->tableView->setVerticalHeader(mVerticalHeader);
    auto oldSelectionModel = ui->tableView->selectionModel();
    ui->tableView->setModel(dtoaModel);
    delete oldSelectionModel;
    ui->tableView->verticalHeader()->setVisible(true);
    ui->tableView->horizontalHeader()->setVisible(true);
    mBaseModel = QSharedPointer<ComprehensiveTableModel>(baseModel);

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

BPScalingViewFrame::BPScalingViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new ComprehensiveTableModel, parent, f)
{
    ui->tableView->setContextMenuPolicy(Qt::CustomContextMenu);
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::defaultConfiguration());
}

BPScalingViewFrame::BPScalingViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                                       const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                       QWidget *parent, Qt::WindowFlags f)
    : AbstractBPViewFrame(new ComprehensiveTableModel, parent, f)
{
    mModelInstance = modelInstance;
    mViewConfig = viewConfig;
}

AbstractTableViewFrame* BPScalingViewFrame::clone(int viewId)
{
    auto viewConfig = QSharedPointer<AbstractViewConfiguration>(mModelInstance->clone(mViewConfig->viewId(), viewId));
    if (!viewConfig)
        viewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(),
                                                                                                        mModelInstance));
    auto frame = new BPScalingViewFrame(mModelInstance, viewConfig, parentWidget(), windowFlags());
    frame->setupView();
    frame->evaluateFilters();
    return frame;
}

void BPScalingViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    Q_UNUSED(absoluteValues);
    if (!mBaseModel || !mValueFormatModel)
        return;
    mModelInstance->loadViewData(mViewConfig);
    emit mBaseModel->dataChanged(QModelIndex(), QModelIndex(), {Qt::DisplayRole});
    mValueFormatModel->setValueFilter(mViewConfig->currentValueFilter());
}

void BPScalingViewFrame::setupView(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    mModelInstance = modelInstance;
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(), mModelInstance));
    mViewConfig->currentValueFilter().UseAbsoluteValues = modelInstance->globalAbsolute();
    mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = modelInstance->globalAbsolute();
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

void BPScalingViewFrame::evaluateFilters()
{
    if (!mBaseModel)
        return;
    if (mIdentifierFilterModel)
        mIdentifierFilterModel->setIdentifierFilter(mViewConfig->currentIdentifierFilter());
    mModelInstance->loadViewData(mViewConfig);
    if (mValueFormatModel)
        mValueFormatModel->setValueFilter(mViewConfig->currentValueFilter());
    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

void BPScalingViewFrame::setupView()
{
    mVerticalHeader = new HierarchicalHeaderView(Qt::Vertical,
                                                 mModelInstance,
                                                 mViewConfig,
                                                 ui->tableView);
    auto baseModel = new ComprehensiveTableModel(mViewConfig->viewId(),
                                                 mModelInstance,
                                                 ui->tableView);
    mValueFormatModel = new ValueFormatProxyModel(ui->tableView);
    mValueFormatModel->setSourceModel(baseModel);
    mIdentifierFilterModel = new BPIdentifierFilterModel(mModelInstance, ui->tableView);
    mIdentifierFilterModel->setSourceModel(mValueFormatModel);
    auto dtoaModel = new DtoaFormatProxyModel(ui->tableView);
    dtoaModel->setSourceModel(mIdentifierFilterModel);

    ui->tableView->setVerticalHeader(mVerticalHeader);
    auto oldSelectionModel = ui->tableView->selectionModel();
    ui->tableView->setModel(dtoaModel);
    delete oldSelectionModel;
    ui->tableView->horizontalHeader()->setVisible(true);
    ui->tableView->verticalHeader()->setVisible(true);
    mBaseModel = QSharedPointer<ComprehensiveTableModel>(baseModel);

    ui->tableView->resizeColumnsToContents();
    ui->tableView->resizeRowsToContents();
}

}
}
}
