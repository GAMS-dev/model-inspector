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
#ifndef SEARCH_H
#define SEARCH_H

#include <functional>
#include <QSharedPointer>
#include <QString>

class QAbstractItemModel;

namespace gams {
namespace studio {
namespace mii {

class AbstractModelInstance;
class AbstractViewConfiguration;
struct SearchResult;

class Search
{
public:
    Search(const QSharedPointer<AbstractViewConfiguration> &viewConfig,
           QAbstractItemModel *dataModel,
           const QString &term,
           bool isRegEx);

    void run();

private:
    void searchStaticHeader(Qt::Orientation orientation);
    void searchHeaderHierarchy(Qt::Orientation orientation);
    void searchHeaderHierarchy(int logicalIndex, int sectionIndex,
                               Qt::Orientation orientation);

private:
    QSharedPointer<AbstractViewConfiguration> mViewConfig;
    QAbstractItemModel *mDataModel;
    std::function<bool(const QString&)> compare;
};

}
}
}

#endif // SEARCH_H
