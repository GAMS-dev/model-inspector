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
#include "search.h"
#include "common.h"
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"

#include <QAbstractItemModel>
#include <QRegularExpression>

#include <QDebug>

namespace gams {
namespace studio {
namespace mii {

Search::Search(const QSharedPointer<AbstractViewConfiguration> &viewConfig,
               QAbstractItemModel *dataModel,
               const QString &term,
               bool isRegEx)
    : mViewConfig(viewConfig)
    , mDataModel(dataModel)
{
    mViewConfig->searchResult().Term = term;
    mViewConfig->searchResult().IsRegEx = isRegEx;
    if (isRegEx) {
        QRegularExpression regex(term);
        compare = [regex](const QString &text) {
            return regex.match(text).hasMatch();
        };
    } else {
        compare = [&term](const QString &text) {
            return text.contains(term, Qt::CaseInsensitive);
        };
    }
}

void Search::run()
{
    if (!mDataModel)
        return;
    switch (mViewConfig->viewType()) {
    case ViewHelper::ViewDataType::BP_Overview:
    case ViewHelper::ViewDataType::BP_Count:
    case ViewHelper::ViewDataType::BP_Average:
    case ViewHelper::ViewDataType::BP_Scaling:
        searchStaticHeader(Qt::Horizontal);
        searchStaticHeader(Qt::Vertical);
        break;
    default:
        searchHeaderHierarchy(Qt::Horizontal);
        searchHeaderHierarchy(Qt::Vertical);
        break;
    }
}

void Search::searchStaticHeader(Qt::Orientation orientation)
{
    int sections = orientation == Qt::Horizontal ? mDataModel->columnCount()
                                                 : mDataModel->rowCount();
    for (int section=0; section<sections; ++section) {
        auto labels = mDataModel->headerData(section, orientation, ViewHelper::SectionLabelRole).toStringList();
        for (const auto& label : labels) {
            if (compare(label)) {
                mViewConfig->searchResult().Entries.append(SearchResult::SearchEntry{section, orientation});
                break;
            }
        }
    }
}

void Search::searchHeaderHierarchy(Qt::Orientation orientation)
{
    bool ok = false;
    int sections = orientation == Qt::Horizontal ? mDataModel->columnCount() :
                                                   mDataModel->rowCount();
    for (int section=0; section<sections; ++section) {
        int realSection = mDataModel->headerData(section, orientation).toInt(&ok);
        if (!ok) continue;
        searchHeaderHierarchy(section, realSection, orientation);
    }
}

void Search::searchHeaderHierarchy(int logicalIndex, int sectionIndex, Qt::Orientation orientation)
{
    auto sym = orientation == Qt::Horizontal ? mViewConfig->modelInstance()->variable(sectionIndex)
                                             : mViewConfig->modelInstance()->equation(sectionIndex);
    if (compare(sym->name())) {
        mViewConfig->searchResult().Entries.append(SearchResult::SearchEntry{logicalIndex, orientation});
    } else {
        auto labels = sym->sectionLabels()[sectionIndex];
        for (const auto& label : labels) {
            if (compare(label)) {
                mViewConfig->searchResult().Entries.append(SearchResult::SearchEntry{logicalIndex, orientation});
                break;
            }
        }
    }
}

}
}
}
