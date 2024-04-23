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
#include "abstractviewframe.h"
#include "abstractmodelinstance.h"

namespace gams {
namespace studio {
namespace mii {

AbstractViewFrame::AbstractViewFrame(QWidget *parent, Qt::WindowFlags f)
    : QFrame(parent, f)
    , mModelInstance(new EmptyModelInstance)
{

}

AbstractViewFrame::~AbstractViewFrame()
{

}

SearchResult &AbstractViewFrame::searchResult()
{
    return mViewConfig->searchResult();
}

const QSharedPointer<AbstractViewConfiguration> &AbstractViewFrame::viewConfig() const
{
    return mViewConfig;
}

void AbstractViewFrame::setViewConfig(const QSharedPointer<AbstractViewConfiguration> &viewConfig)
{
    mViewConfig = viewConfig;
}

void AbstractViewFrame::evaluateFilters()
{

}

EmtpyViewFrame::EmtpyViewFrame(QWidget *parent, Qt::WindowFlags f)
    : AbstractViewFrame(parent, f)
{

}

EmtpyViewFrame::~EmtpyViewFrame()
{

}

AbstractViewFrame *EmtpyViewFrame::clone(int viewId)
{
    Q_UNUSED(viewId);
    return nullptr;
}

void EmtpyViewFrame::setShowAbsoluteValues(bool absoluteValues)
{
    Q_UNUSED(absoluteValues);
}

ViewHelper::ViewDataType EmtpyViewFrame::type() const
{
    return ViewHelper::ViewDataType::Unknown;
}

void EmtpyViewFrame::zoomIn()
{

}

void EmtpyViewFrame::zoomOut()
{

}

void EmtpyViewFrame::resetZoom()
{

}

SearchResult &EmtpyViewFrame::search(const QString &term, bool isRegEx)
{
    Q_UNUSED(term);
    Q_UNUSED(isRegEx);
    return mSearchResult;
}

void EmtpyViewFrame::setSearchSelection(const SearchResult::SearchEntry &result)
{
    Q_UNUSED(result);
}

void EmtpyViewFrame::setupView(const QSharedPointer<AbstractModelInstance>& modelInstance)
{
    Q_UNUSED(modelInstance);
}

bool EmtpyViewFrame::hasData() const
{
    return false;
}

}
}
}
