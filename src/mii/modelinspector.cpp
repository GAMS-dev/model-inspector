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
#include "modelinspector.h"
#include "ui_modelinspector.h"
#include "modelinstance.h"
#include "sectiontreemodel.h"
#include "sectiontreeitem.h"
#include "viewconfigurationprovider.h"
#include "symbolviewframe.h"

#include <QtConcurrent>
#include <QDir>

#include <QDebug>

namespace gams {
namespace studio{
namespace mii {

ModelInspector::ModelInspector(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ModelInspector)
    , mSectionModel(new SectionTreeModel(this))
    , mModelInstance(new EmptyModelInstance)
{
    ui->setupUi(this);
    ui->bpScalingFrame->viewConfig()->setViewId((int)ViewHelper::ViewDataType::BP_Scaling);
    ui->bpOverviewFrame->viewConfig()->setViewId((int)ViewHelper::ViewDataType::BP_Overview);
    ui->bpCountFrame->viewConfig()->setViewId((int)ViewHelper::ViewDataType::BP_Count);
    ui->bpAverageFrame->viewConfig()->setViewId((int)ViewHelper::ViewDataType::BP_Average);
    mSectionModel->loadModelData(ui->stackedWidget, ViewHelper::MiiModeType::None);
    ui->sectionView->setModel(mSectionModel);
    loadModelInstance(false);
    setupConnections();
    ui->sectionView->expandAll();
    ui->splitter->setStretchFactor(1,1);
}

ModelInspector::~ModelInspector()
{
    delete ui;
}

QString ModelInspector::scratchDir() const
{
    return mScratchDir;
}

void ModelInspector::setScratchDir(const QString &scratchDir)
{
    mScratchDir = scratchDir;
}

QString ModelInspector::baseScratchDir() const
{
    return mBaseScratchDir;
}

void ModelInspector::setBaseScratchDir(const QString &baseDir)
{
    mBaseScratchDir = baseDir;
}

QString ModelInspector::workspace() const
{
    return mWorkspace;
}

void ModelInspector::setWorkspace(const QString &workingDir)
{
    mWorkspace = workingDir;
}

QString ModelInspector::systemDirectory() const
{
    return mSystemDir;
}

void ModelInspector::setSystemDirectory(const QString &systemDir)
{
    mSystemDir = systemDir;
}

bool ModelInspector::showOutput() const
{
    return mModelInstance->useOutput();
}

void ModelInspector::setShowOutput(bool showOutput)
{
    mModelInstance->setUseOutput(showOutput);
}

ViewHelper::MiiModeType ModelInspector::miiMode() const
{
    return mMiiMode;
}

void ModelInspector::setMiiMode(ViewHelper::MiiModeType miiMode)
{
    mMiiMode = miiMode;
}

///
/// \brief Set show absolute values globaly.
/// \remark The global config has higher priority then
///         the local configuration (filter dialog).
///
void ModelInspector::setShowAbsoluteValuesGlobal(bool absoluteValues)
{
    if (mModelInstance->globalAbsolute() == absoluteValues)
        return;
    mModelInstance->setGlobalAbsolute(absoluteValues);
    for (auto widget : mSectionModel->rootItem()->widgets()) {
        widget->viewConfig()->currentValueFilter().UseAbsoluteValues = absoluteValues;
        widget->viewConfig()->currentValueFilter().UseAbsoluteValuesGlobal = absoluteValues;
        widget->setShowAbsoluteValues(absoluteValues);
    }
}

SearchResult& ModelInspector::searchHeaders(const QString &term, bool isRegEx)
{
    auto frame = currentView();
    return frame ? frame->search(term, isRegEx) : mDefaultSearchResult;
}

SearchResult& ModelInspector::searchResult()
{
    auto frame = currentView();
    return frame ? frame->searchResult() : mDefaultSearchResult;
}

ViewActionStates ModelInspector::viewActionStates() const
{
    return ui->sectionView->viewActionStates();
}

void ModelInspector::loadModelInstance(bool loadModel)
{
    clearDefaultViewData();
    mSectionModel->clearModelData();
    mSectionModel->setScratchDir(mBaseScratchDir);
    mSectionModel->loadModelData(ui->stackedWidget, mMiiMode, mModelFilePath);
    if (!mSectionModel->rootItem()->childs().isEmpty() && mMiiMode == ViewHelper::MiiModeType::Multi) {
        auto scrDir = mSectionModel->rootItem()->childs().first()->scratchDir();
        setScratchDir(scrDir);
    }
    ui->sectionView->expandAll();
    mModelInstance->removeViewData();
    setupModelInstanceView(loadModel);
    emit newLogMessage(mModelInstance->logMessages());
}

void ModelInspector::loadModelInstance(const QString &scrdir)
{
    clearDefaultViewData();
    setScratchDir(scrdir);
    ui->sectionView->expandAll();
    mModelInstance->removeViewData();
    setupModelInstanceView(true);
    emit newLogMessage(mModelInstance->logMessages());
}

void ModelInspector::reloadModelInstance()
{
    ui->bpOverviewFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
    ui->bpCountFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
    ui->bpAverageFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
    auto loadData = [this]{
        mModelInstance->loadViewData(ui->bpScalingFrame->viewConfig());
        auto customGroup = mSectionModel->rootItem()->customGroup();
        if (!customGroup)
            return;
        for (auto view : customGroup->widgets()) {
            if (view->type() == ViewHelper::ViewDataType::Postopt)
                continue;
            mModelInstance->loadViewData(view->viewConfig());
        }
        if (mModelInstance->state() == AbstractModelInstance::Error)
            emit newLogMessage(mModelInstance->logMessages());
        emit dataLoaded();
        emit filtersChanged();
    };
    mFutureData = QtConcurrent::run(loadData);
}

QSharedPointer<AbstractViewConfiguration> ModelInspector::viewConfig()
{
    auto frame = currentView();
    if (frame) {
        return frame->viewConfig();
    }
    return QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::Unknown,
                                                                                              QSharedPointer<AbstractModelInstance>(new EmptyModelInstance)));
}

void ModelInspector::cancelRun()
{
    if (mFutureData.isRunning()) {
        mFutureData.cancel();
        mFutureData.waitForFinished();
    }
}

void ModelInspector::zoomIn()
{
    for (auto widget : mSectionModel->rootItem()->widgets()) {
        widget->zoomIn();
    }
}

void ModelInspector::zoomOut()
{
    for (auto widget : mSectionModel->rootItem()->widgets()) {
        widget->zoomOut();
    }
}

void ModelInspector::resetZoom()
{
    for (auto widget : mSectionModel->rootItem()->widgets()) {
        widget->resetZoom();
    }
}

void ModelInspector::updateFilters()
{
    auto frame = currentView();
    if (frame) {
        frame->evaluateFilters();
        emit filtersChanged();
    }
}

void ModelInspector::saveModelView()
{
    if (!ui->sectionView->viewActionStates().SaveEnabled)
        return;
    auto view = currentView();
    if (!view) {
        emit newLogMessage("ERROR: ModelInspector::saveModelView() widget nullptr!");
        return;
    }
    auto index = ui->sectionView->currentIndex();
    auto text = index.data().toString();
    auto clone = view->clone(ViewConfigurationProvider::nextViewId());
    ui->stackedWidget->addWidget(clone);
    ViewHelper::ViewDataType dataType = ViewHelper::ViewDataType::Unknown;
    switch (clone->type()) {
    case ViewHelper::ViewDataType::BP_Overview:
    case ViewHelper::ViewDataType::BP_Count:
    case ViewHelper::ViewDataType::BP_Average:
    case ViewHelper::ViewDataType::BP_Scaling:
        dataType = ViewHelper::ViewDataType::BlockpicGroup;
        connect(static_cast<AbstractBPViewFrame*>(clone), &AbstractBPViewFrame::filtersChanged,
                this, &ModelInspector::filtersChanged);
        connect(static_cast<AbstractBPViewFrame*>(clone), &AbstractBPViewFrame::newSymbolViewRequested,
                this, &ModelInspector::createNewSymbolView);
        break;
    case ViewHelper::ViewDataType::Postopt:
        dataType = ViewHelper::ViewDataType::PostoptGroup;
        connect(static_cast<PostoptTreeViewFrame*>(clone), &PostoptTreeViewFrame::openFilterDialog,
                this, &ModelInspector::openFilterDialog);
        break;
    case ViewHelper::ViewDataType::Symbols:
        dataType = ViewHelper::ViewDataType::SymbolsGroup;
        break;
    default:
        dataType = clone->type();
        break;
    }
    auto item = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
    if (item) {
        auto customGroup = item->customGroup();
        if (!customGroup)
            return;
        mSectionModel->appendCustomView(text, clone, customGroup);
        ui->sectionView->expandAll();
        setCurrentViewIndex(ViewHelper::ViewType::Custom, dataType);
    }
}

void ModelInspector::createNewSymbolView()
{
    auto currentBPView = static_cast<BPScalingViewFrame*>(currentView());
    if (!currentBPView) {
        emit newLogMessage("ERROR: ModelInspector::createNewSymbolView() widget nullptr!");
        return;
    }
    auto view = new SymbolViewFrame(ViewConfigurationProvider::nextViewId(),
                                    mModelInstance,
                                    ui->stackedWidget,
                                    currentBPView->windowFlags());
    view->viewConfig()->currentValueFilter().UseAbsoluteValues =
            currentBPView->viewConfig()->currentValueFilter().UseAbsoluteValues;
    view->viewConfig()->currentValueFilter().UseAbsoluteValuesGlobal =
            currentBPView->viewConfig()->currentValueFilter().UseAbsoluteValuesGlobal;
    view->viewConfig()->updateIdentifierFilter(currentBPView->selectedEquations(),
                                               currentBPView->selectedVariables());
    view->setupView(mModelInstance);
    auto page = ui->stackedWidget->addWidget(view);
    QString pageName = ViewHelper::SymbolView;
    if (currentBPView->selectedEquations().size() == 1 && currentBPView->selectedVariables().size() == 1) {
        pageName = currentBPView->selectedEquations().constFirst()->name() + " + " +
                   currentBPView->selectedVariables().constFirst()->name();
    } else if (currentBPView->selectedEquations().size() > 1 && currentBPView->selectedVariables().size() > 1) {
        pageName = currentBPView->selectedEquations().constFirst()->name() + ".."  +
                   currentBPView->selectedEquations().constLast()->name() + " + " +
                   currentBPView->selectedVariables().constFirst()->name() + ".."  +
                   currentBPView->selectedVariables().constLast()->name();
    }
    view->viewConfig()->setEquationLabels(currentBPView->selectedEquations());
    view->viewConfig()->setVariableLabels(currentBPView->selectedVariables());
    view->viewConfig()->setSelectedEquations(currentBPView->selectedEquations());
    view->viewConfig()->setSelectedVariables(currentBPView->selectedVariables());
    ui->stackedWidget->setCurrentIndex(page);
    auto index = ui->sectionView->currentIndex();
    auto item = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
    if (item) {
        auto customGroup = item->customGroup();
        if (!customGroup)
            return;
        mSectionModel->appendCustomView(pageName, view, customGroup);
        ui->sectionView->expandAll();
        setCurrentViewIndex(ViewHelper::ViewType::Custom, ViewHelper::ViewDataType::SymbolsGroup);
        connect(view, &SymbolViewFrame::filtersChanged,
                this, &ModelInspector::filtersChanged);
        emit view->filtersChanged();
    }
}

void ModelInspector::removeModelView()
{
    if (!ui->sectionView->viewActionStates().RemoveEnabled)
        return;
    auto currentIndex = ui->sectionView->currentIndex();
    if (!currentIndex.isValid())
        return;
    auto item = static_cast<AbstractSectionTreeItem*>(currentIndex.internalPointer());
    auto parent = item->parent();
    auto customViewIndex = customIndex(item->modelInstanceGroup());
    auto predefinedViewIndex = predefinedIndex(item->modelInstanceGroup());
    for (auto widget : mSectionModel->removeItem(item)) {
        ui->stackedWidget->removeWidget(widget);
        mModelInstance->removeViewData(widget->viewConfig()->viewId());
        delete widget;
    }
    if (!parent->childCount()) {
        mSectionModel->removeItem(parent);
    }
    if (!ui->sectionView->model()->rowCount(customViewIndex)) {
        int pos = ui->sectionView->model()->rowCount(predefinedViewIndex) - 1;
        auto index = ui->sectionView->model()->index(pos, 0, predefinedViewIndex);
        ui->sectionView->setCurrentIndex(index);
    } else if (ui->sectionView->model()->rowCount(customViewIndex) > 0) {
        auto group = ui->sectionView->model()->index(ui->sectionView->model()->rowCount(customViewIndex)-1, 0, customViewIndex);
        auto lastIndex = ui->sectionView->model()->index(ui->sectionView->model()->rowCount(group)-1, 0, group);
        ui->sectionView->setCurrentIndex(lastIndex);
    }
    ui->sectionView->expandAll();
}

void ModelInspector::setCurrentView()
{
    auto view = currentView();
    if (!view) {
        return;
    }
    int index = currentViewIndex(view);
    ui->stackedWidget->setCurrentIndex(index);
    if (!view->hasData() && mFutureData.isFinished()) {
        view->setupView(mModelInstance);
    }
    emit viewChanged((int)view->type());
}

void ModelInspector::setCurrentViewIndex(ViewHelper::ViewType viewType,
                                         ViewHelper::ViewDataType viewDataType)
{
    if (viewType != ViewHelper::ViewType::Custom)
        return;
    auto dirIndex = ui->sectionView->model()->index(0, 0);
    auto customViewIndex = ui->sectionView->model()->index((int)viewType, 0, dirIndex);
    int pos = ui->sectionView->model()->rowCount(customViewIndex) - 1;
    if (pos < 0)
        return;
    QModelIndex index;
    auto groupParent = ui->sectionView->model()->index(pos, 0, customViewIndex).parent();
    for (int r=0; r<groupParent.model()->rowCount(groupParent); ++r) {
        auto groupIndex = ui->sectionView->model()->index(r, 0, groupParent);
        if (groupIndex.data(SectionTreeModel::ItemDataTypeRole).toInt() == (int)viewDataType) {
            int last = groupIndex.model()->rowCount(groupIndex)-1;
            index = ui->sectionView->model()->index(last, 0, groupIndex);
            break;
        }
    }
    if (index.isValid()) {
        ui->sectionView->setCurrentIndex(index);
    }
}

void ModelInspector::setSearchSelection(const gams::studio::mii::SearchResult::SearchEntry &result)
{
    auto frame = currentView();
    if (frame) frame->setSearchSelection(result);
}

void ModelInspector::setupConnections()
{
    connect(ui->sectionView, &SectionTreeView::currentItemChanged,
            this, &ModelInspector::setCurrentView);
    connect(ui->sectionView, &SectionTreeView::saveViewTriggered,
            this, &ModelInspector::saveModelView);
    connect(ui->sectionView, &SectionTreeView::removeViewTriggered,
            this, &ModelInspector::removeModelView);
    connect(ui->sectionView, &SectionTreeView::loadModelInstance,
            this, &ModelInspector::switchModelInstance);
    connect(ui->sectionView, &SectionTreeView::logMessage,
            this, &ModelInspector::newLogMessage);
    connect(ui->bpScalingFrame, &BPScalingViewFrame::filtersChanged,
            this, &ModelInspector::filtersChanged);
    connect(ui->bpOverviewFrame, &AbstractBPViewFrame::newSymbolViewRequested,
            this, &ModelInspector::createNewSymbolView);
    connect(ui->bpCountFrame, &AbstractBPViewFrame::newSymbolViewRequested,
            this, &ModelInspector::createNewSymbolView);
    connect(ui->bpAverageFrame, &AbstractBPViewFrame::newSymbolViewRequested,
            this, &ModelInspector::createNewSymbolView);
    connect(ui->bpScalingFrame, &AbstractBPViewFrame::newSymbolViewRequested,
            this, &ModelInspector::createNewSymbolView);
    connect(this, &ModelInspector::dataLoaded,
            this, &ModelInspector::selectScalingView);
    connect(ui->postoptFrame, &PostoptTreeViewFrame::openFilterDialog,
            this, &ModelInspector::openFilterDialog);
}

void ModelInspector::setupModelInstanceView(bool loadModel)
{
    auto loadData = [this, loadModel]{
        bool useOutput = mModelInstance->useOutput();
        bool globalAbs = mModelInstance->globalAbsolute();
        if (loadModel) {
            mModelInstance = QSharedPointer<AbstractModelInstance>(new ModelInstance(useOutput,
                                                                                     mWorkspace,
                                                                                     mSystemDir,
                                                                                     mScratchDir));
            mModelInstance->setGlobalAbsolute(globalAbs);
        }
        if (mModelInstance->state() == AbstractModelInstance::Error) {
            mModelInstance = QSharedPointer<AbstractModelInstance>(new EmptyModelInstance);
            mModelInstance->setUseOutput(useOutput);
        }
        mModelInstance->loadBaseData();
        if (mModelInstance->state() == AbstractModelInstance::Error)
            emit newLogMessage(mModelInstance->logMessages());
        emit dataLoaded();
    };
    mFutureData = QtConcurrent::run(loadData);
}

void ModelInspector::clearDefaultViewData()
{
    ui->bpOverviewFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
    ui->bpCountFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
    ui->bpAverageFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
    ui->postoptFrame->setupView(QSharedPointer<AbstractModelInstance>(new EmptyModelInstance));
}

void ModelInspector::selectScalingView()
{
    int row = 0;
    ui->bpScalingFrame->setupView(mModelInstance);
    for (auto item : mSectionModel->rootItem()->childs()) {
        if (item->isActive()) {
            row = item->row();
            break;
        }
    }
    auto grid1 = ui->sectionView->model()->index(row, 0);
    auto root = ui->sectionView->model()->index((int)ViewHelper::ViewType::Predefined, 0, grid1);
    auto group = ui->sectionView->model()->index(0, 0, root);
    auto index = ui->sectionView->model()->index((int)ViewHelper::ViewDataType::BP_Scaling, 0, group);
    ui->sectionView->setCurrentIndex(index);
    ui->stackedWidget->setCurrentIndex((int)ViewHelper::ViewDataType::BP_Scaling);
}

void ModelInspector::switchModelInstance()
{
    auto index = ui->sectionView->currentIndex();
    if (!index.isValid())
        return;
    auto item = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
    auto inst = item->modelInstanceGroup();
    for (auto sibling : inst->parent()->childs()) {
        if (!sibling->isActive())
            continue;
        auto wgts = mSectionModel->removeCustomRows(sibling->customGroup());
        for (auto wgt : wgts) {
            ui->stackedWidget->removeWidget(wgt);
            wgt->setParent(nullptr);
            delete wgt;
        }
        sibling->setActive(false);
        break;
    }
    if (!inst)
        return;
    inst->setActive(true);
    loadModelInstance(inst->scratchDir());
}

AbstractViewFrame* ModelInspector::currentView() const
{
    auto currentIndex = ui->sectionView->currentIndex();
    if (!currentIndex.isValid())
        return nullptr;
    auto item = static_cast<AbstractSectionTreeItem*>(currentIndex.internalPointer());
    return item ? item->widget() : nullptr;
}

int ModelInspector::currentViewIndex(AbstractViewFrame* view) const
{
    if (view->parentWidget() == ui->stackedWidget) {
        return ui->stackedWidget->indexOf(view);
    }
    return ui->stackedWidget->indexOf(view->parentWidget());
}

QModelIndex ModelInspector::customIndex(AbstractSectionTreeItem* instanceRoot)
{
    auto root = ui->sectionView->model()->index(instanceRoot->row(), 0);
    return ui->sectionView->model()->index((int)ViewHelper::ViewType::Custom, 0, root);
}

QModelIndex ModelInspector::predefinedIndex(AbstractSectionTreeItem *instanceRoot)
{
    auto root = ui->sectionView->model()->index(instanceRoot->row(), 0);
    return ui->sectionView->model()->index((int)ViewHelper::ViewType::Predefined, 0, root);
}

QString ModelInspector::modelFilePath() const
{
    return mModelFilePath;
}

void ModelInspector::setModelFilePath(const QString &newModelFilePath)
{
    mModelFilePath = newModelFilePath;
}

}
}
}
