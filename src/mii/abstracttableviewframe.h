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
#ifndef ABSTRACTTABLEVIEWFRAME_H
#define ABSTRACTTABLEVIEWFRAME_H

#include "common.h"
#include "abstractviewframe.h"

#include <QSharedPointer>

class QAbstractItemModel;

namespace gams {
namespace studio{
namespace mii {

namespace Ui {
class StandardTableViewFrame;
}

class AbstractTableViewFrame : public AbstractViewFrame
{
    Q_OBJECT

public:
    AbstractTableViewFrame(QWidget *parent = nullptr,
                           Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~AbstractTableViewFrame();

    void setSearchSelection(const SearchResult::SearchEntry &result) override;

    SearchResult& search(const QString &term, bool isRegEx) override;

    void zoomIn() override;

    void zoomOut() override;

    void resetZoom() override;

signals:
    void filtersChanged();

    void searchResultUpdated(const QList<SearchResult>&);

protected:
    Ui::StandardTableViewFrame* ui;
};

}
}
}

#endif // ABSTRACTTABLEVIEWFRAME_H
