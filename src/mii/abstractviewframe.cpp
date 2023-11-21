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

void AbstractViewFrame::updateFilters(AbstractViewConfiguration::Options options)
{
    if (options | AbstractViewConfiguration::IdentifierConfig) {
        updateIdentifierFilter();
    }
    if (options | AbstractViewConfiguration::LabelConfig) {
        updateLabelFilter();
    }
    if (options | AbstractViewConfiguration::ValueConfig) {
        updateValueFilter();
    }
}

SearchResult &AbstractViewFrame::searchResult()
{
    return mViewConfig->searchResult();
}

QSharedPointer<AbstractViewConfiguration> AbstractViewFrame::viewConfig() const
{
    return mViewConfig;
}

void AbstractViewFrame::setViewConfig(QSharedPointer<AbstractViewConfiguration> viewConfig)
{
    mViewConfig = viewConfig;
}

void AbstractViewFrame::updateIdentifierFilter()
{

}

void AbstractViewFrame::updateLabelFilter()
{

}

void AbstractViewFrame::updateValueFilter()
{

}

}
}
}
