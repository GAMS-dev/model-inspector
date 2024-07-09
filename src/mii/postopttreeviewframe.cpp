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
#include "postopttreeviewframe.h"
#include "ui_postopttreeviewframe.h"
#include "postopttreemodel.h"
#include "viewconfigurationprovider.h"
#include "abstractmodelinstance.h"
#include "valueformatproxymodel.h"

namespace gams {
namespace studio{
namespace mii {

PostoptTreeViewFrame::PostoptTreeViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractViewFrame(parent, f)
    , ui(new Ui::PostoptTreeViewFrame)
{
    ui->setupUi(this);
    connect(ui->treeView, &PostoptTreeView::openFilterDialog,
            this, &PostoptTreeViewFrame::openFilterDialog);
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::defaultConfiguration());
}

PostoptTreeViewFrame::PostoptTreeViewFrame(const QSharedPointer<AbstractModelInstance>& modelInstance,
                                           const QSharedPointer<AbstractViewConfiguration>& viewConfig,
                                           QWidget *parent,
                                           Qt::WindowFlags f)
    : PostoptTreeViewFrame(parent, f)
{
    mModelInstance = modelInstance;
    mViewConfig = viewConfig;
}

PostoptTreeViewFrame::~PostoptTreeViewFrame()
{

}

AbstractViewFrame *PostoptTreeViewFrame::clone(int viewId)
{
    auto viewConfig = QSharedPointer<AbstractViewConfiguration>(mModelInstance->clone(mViewConfig->viewId(), viewId));
    if (!viewConfig)
        viewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(),
                                                                                                        mModelInstance));
    auto frame = new PostoptTreeViewFrame(mModelInstance, viewConfig, parentWidget(), windowFlags());
    frame->setupView();
    frame->evaluateFilters();
    return frame;
}

void PostoptTreeViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    Q_UNUSED(absoluteValues);
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

SearchResult &PostoptTreeViewFrame::search(const QString &term, bool isRegEx)
{
    Q_UNUSED(term);
    Q_UNUSED(isRegEx);
    return searchResult();
}

void PostoptTreeViewFrame::setSearchSelection(const SearchResult::SearchEntry &result)
{
    Q_UNUSED(result);
}

void PostoptTreeViewFrame::setupView(const QSharedPointer<AbstractModelInstance>& modelInstance)
{
    mModelInstance = modelInstance;
    mViewConfig = QSharedPointer<AbstractViewConfiguration>(ViewConfigurationProvider::configuration(type(), mModelInstance));
    mViewConfig->currentValueFilter().UseAbsoluteValues = modelInstance->globalAbsolute();
    mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal = modelInstance->globalAbsolute();
    mModelInstance->loadViewData(mViewConfig);
    setupView();
}

ViewHelper::ViewDataType PostoptTreeViewFrame::type() const
{
    return ViewHelper::ViewDataType::Postopt;
}

void PostoptTreeViewFrame::zoomIn()
{
    ui->treeView->zoomIn(ViewHelper::ZoomFactor);
}

void PostoptTreeViewFrame::zoomOut()
{
    ui->treeView->zoomOut(ViewHelper::ZoomFactor);
}

void PostoptTreeViewFrame::resetZoom()
{
    ui->treeView->resetZoom();
}

bool PostoptTreeViewFrame::hasData() const
{
    return mBaseModel && mBaseModel->rowCount();
}

void PostoptTreeViewFrame::evaluateFilters()
{
    mModelInstance->loadViewData(mViewConfig);
    setupView();
    if (!mValueFormatModel)
        return;
    mValueFormatModel->setValueFilter(mViewConfig->currentValueFilter());
    ui->treeView->expandAll();
}

void PostoptTreeViewFrame::setupView()
{
    mBaseModel = new PostoptTreeModel(mViewConfig->viewId(),
                                      mModelInstance,
                                      ui->treeView);
    mValueFormatModel = new PostoptValueFormatProxyModel(ui->treeView);
    mValueFormatModel->setSourceModel(mBaseModel);

    auto oldSelectionModel = ui->treeView->selectionModel();
    ui->treeView->setModel(mValueFormatModel);
    delete oldSelectionModel;
    ui->treeView->expandAll();
    ui->treeView->resizeColumnToContents(0);
}

}
}
}
