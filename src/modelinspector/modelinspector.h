/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef MODELINSPECTOR_H
#define MODELINSPECTOR_H

#include <QSharedPointer>
#include <QWidget>

#include "common.h"
#include "aggregation.h"

namespace gams {
namespace studio{
namespace modelinspector {

namespace Ui {
class ModelInspector;
}

class AbstractTableViewFrame;
class AbstractModelInstance;
class SectionTreeModel;
class SearchResultModel;

class ModelInspector : public QWidget
{// TODO !!! use ViewConfig in MI interface?
    Q_OBJECT

public:
    explicit ModelInspector(QWidget *parent = nullptr);
    ~ModelInspector();

    QString scratchDir() const;
    void setScratchDir(const QString &scratchDir);

    QString workspace() const;
    void setWorkspace(const QString &workspace);

    QString systemDirectory() const;
    void setSystemDirectory(const QString &systemDir);

    bool showOutput() const;
    void setShowOutput(bool showOutpu);

    void setShowAbsoluteValues(bool absoluteValues);

    QList<SearchResult> searchHeaders(const QString &term, bool isRegEx);

    ViewActionStates viewActionStates() const;

    void loadModelInstance(bool loadModel = true);
    void reloadModelInstance();
    QSharedPointer<AbstractModelInstance> modelInstance() const;

    IdentifierFilter identifierFilter() const;
    IdentifierFilter defaultIdentifierFilter() const;
    void setIdentifierFilter(const IdentifierFilter &filter);

    ValueFilter valueFilter() const;
    ValueFilter defaultValueFilter() const;
    void setValueFilter(const ValueFilter &filter);

    LabelFilter labelFilter() const;
    LabelFilter defaultLabelFilter() const;
    void setLabelFilter(const LabelFilter &filter);

    Aggregation aggregation() const;
    Aggregation defaultAggregation() const;
    void setAggregation(const Aggregation &aggregation);

    ViewDataType viewType() const;

    void resetColumnRowFilter();

    void zoomIn();
    void zoomOut();
    void resetZoom();

signals:
    void filtersChanged();

    void viewChanged(int);

    void newLogMessage(const QString&); // TODO (AF) use message types when integrated into studio

public slots:
    void resetDefaultViews();

    void saveModelView();
    void createNewSymbolView();

    void removeModelView();

    void setCurrentView(int index);

    void setCurrentViewIndex(gams::studio::modelinspector::ViewType type);

    void setSearchSelection(const gams::studio::modelinspector::SearchResult &result);

private:
    void setupConnections();

    void setupModelInstanceView(bool loadModel);

    void clearCustomViews();

    AbstractTableViewFrame* currentView() const;

private:
    Ui::ModelInspector* ui;
    QString mScratchDir;
    QString mWorkspace;
    QString mSystemDir;

    SectionTreeModel* mSectionModel = nullptr;
    QSharedPointer<AbstractModelInstance> mModelInstance;
    QMap<int, AbstractTableViewFrame*> mCustomViews;
};

}
}
}

#endif // MODELINSPECTOR_H
