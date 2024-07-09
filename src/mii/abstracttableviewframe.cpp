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
#include "abstracttableviewframe.h"
#include "ui_standardtableviewframe.h"
#include "search.h"
#include "viewconfigurationprovider.h"

namespace gams {
namespace studio{
namespace mii {

AbstractTableViewFrame::AbstractTableViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractViewFrame(parent, f)
    , ui(new Ui::StandardTableViewFrame)
{
    ui->setupUi(this);
}

AbstractTableViewFrame::~AbstractTableViewFrame()
{
    delete ui;
}

void AbstractTableViewFrame::setSearchSelection(const SearchResult::SearchEntry &result)
{
    if (result.Index < 0) return;
    if (result.Orientation == Qt::Horizontal) {
        ui->tableView->selectColumn(result.Index);
    } else {
        ui->tableView->selectRow(result.Index);
    }
}

SearchResult& AbstractTableViewFrame::search(const QString &term, bool isRegEx)
{
    if (!mViewConfig->searchResult().Entries.isEmpty()) {
        mViewConfig->searchResult().Entries.clear();
    }
    if (!term.isEmpty()) {
        Search search(mViewConfig, ui->tableView->model(), term, isRegEx);
        search.run();
    }
    return mViewConfig->searchResult();
}

void AbstractTableViewFrame::zoomIn()
{
    ui->tableView->zoomIn(ViewHelper::ZoomFactor);
}

void AbstractTableViewFrame::zoomOut()
{
    ui->tableView->zoomOut(ViewHelper::ZoomFactor);
}

void AbstractTableViewFrame::resetZoom()
{
    ui->tableView->resetZoom();
}

}
}
}
