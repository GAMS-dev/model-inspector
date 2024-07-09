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
#ifndef MODELINSPECTOR_H
#define MODELINSPECTOR_H

#include <QFuture>
#include <QSharedPointer>
#include <QWidget>

#include "common.h"

namespace gams {
namespace studio{
namespace mii {

namespace Ui {
class ModelInspector;
}

class AbstractViewFrame;
class AbstractModelInstance;
class AbstractSectionTreeItem;
class AbstractViewConfiguration;
class SectionTreeModel;
class SearchResultModel;

class ModelInspector final : public QWidget
{
    Q_OBJECT

public:
    explicit ModelInspector(QWidget *parent = nullptr);
    ~ModelInspector() override;

    QString scratchDir() const;
    void setScratchDir(const QString &baseDir);

    QString baseScratchDir() const;
    void setBaseScratchDir(const QString &baseDir);

    QString workspace() const;
    void setWorkspace(const QString &workspace);

    QString systemDirectory() const;
    void setSystemDirectory(const QString &systemDir);

    QString modelFilePath() const;
    void setModelFilePath(const QString &newModelFilePath);

    bool showOutput() const;
    void setShowOutput(bool showOutpu);

    ViewHelper::MiiModeType miiMode() const;
    void setMiiMode(ViewHelper::MiiModeType miiMode);
    
    void setShowAbsoluteValuesGlobal(bool absoluteValues);

    SearchResult& searchHeaders(const QString &term, bool isRegEx);
    SearchResult& searchResult();

    ViewActionStates viewActionStates() const;

    void loadModelInstance(bool loadModel);
    void reloadModelInstance();

    QSharedPointer<AbstractViewConfiguration> viewConfig();

    void cancelRun();

    void zoomIn();
    void zoomOut();
    void resetZoom();

    void updateFilters();

signals:
    void filtersChanged();

    void openFilterDialog();

    void viewChanged(int);

    void newLogMessage(const QString&);

    void dataLoaded();

public slots:
    void saveModelView();

    void createNewSymbolView();

    void removeModelView();

    void setCurrentView();

    void setSearchSelection(const gams::studio::mii::SearchResult::SearchEntry &result);

private slots:
    void selectScalingView();

    void switchModelInstance();

private:
    void setupConnections();

    void setupModelInstanceView(bool loadModel);

    void clearDefaultViewData();

    void loadModelInstance(const QString &scrdir);

    void setCurrentViewIndex(ViewHelper::ViewType viewType, ViewHelper::ViewDataType viewDataType);

    AbstractViewFrame* currentView() const;

    int currentViewIndex(AbstractViewFrame* view) const;

    QModelIndex customIndex(AbstractSectionTreeItem* instanceRoot);

    QModelIndex predefinedIndex(AbstractSectionTreeItem* instanceRoot);

private:
    Ui::ModelInspector* ui;
    QString mScratchDir;
    QString mBaseScratchDir;
    QString mWorkspace;
    QString mSystemDir;
    QString mModelFilePath;
    SearchResult mDefaultSearchResult;
    ViewHelper::MiiModeType mMiiMode = ViewHelper::MiiModeType::None;

    SectionTreeModel* mSectionModel = nullptr;
    QSharedPointer<AbstractModelInstance> mModelInstance;
    QFuture<void> mFutureData;
};

}
}
}

#endif // MODELINSPECTOR_H
