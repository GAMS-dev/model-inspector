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
#include "symbol.h"
#include "labeltreeitem.h"

namespace gams {
namespace studio {
namespace mii {

int Symbol::offset() const
{
    return mOffset;
}

void Symbol::setOffset(int offset)
{
    mOffset = offset;
}

int Symbol::entries() const
{
    return mEntries;
}

void Symbol::setEntries(int entries)
{
    mEntries = entries;
}

QString Symbol::name() const
{
    return mName;
}

void Symbol::setName(const QString &name)
{
    mName = name;
}

int Symbol::dimension() const
{
    return mDimension;
}

void Symbol::setDimension(int dimension)
{
    mDimension = dimension;
}

Symbol::Type Symbol::type() const
{
    return mType;
}

void Symbol::setType(Type type)
{
    mType = type;
}

const DomainLabels &Symbol::domainLabels() const
{
    return mDomainLabels;
}

void Symbol::setDomainLabels(const DomainLabels &labels)
{
    mDomainLabels = labels;
}

void Symbol::appendDomainLabel(const QString &label)
{
    mDomainLabels.push_back(label);
}

SectionLabels &Symbol::sectionLabels()
{
    return mSectionLabels;
}

QString Symbol::label(int sectionIndex, int dimension) const
{
    auto data = mSectionLabels[sectionIndex];
    if (dimension < data.size())
        return data[dimension];
    return QString();
}

bool Symbol::contains(int sectionIndex) const
{
    return sectionIndex >= firstSection() && sectionIndex <= lastSection();
}

bool Symbol::isScalar() const
{
    return !mDimension;
}

int Symbol::lastOffset() const
{
    return mOffset+mEntries;
}

int Symbol::firstSection() const {
    return mFirstSection;
}

void Symbol::setFirstSection(int sectionIndex)
{
    mFirstSection = sectionIndex;
}

int Symbol::lastSection() const
{
    return mFirstSection+mEntries-1;
}

int Symbol::logicalIndex() const
{
    return mLogicalIndex;
}

void Symbol::setLogicalIndex(int index)
{
    mLogicalIndex = index;
}

const QSharedPointer<LabelTreeItem> &Symbol::labelTree() const
{
    return mLabelTree;
}

void Symbol::setLabelTree(const QSharedPointer<LabelTreeItem> &tree)
{
    mLabelTree = tree;
}

bool Symbol::isEquation() const
{
    return mType == Equation;
}

bool Symbol::isVariable() const
{
    return mType == Variable;
}

bool Symbol::operator==(const Symbol &other) const
{
    return mFirstSection == other.mFirstSection     &&
           mOffset == other.mOffset                 &&
           mEntries == other.mEntries               &&
           mName == other.mName                     &&
           mDimension == other.mDimension           &&
           mType == other.mType                     &&
           mDomainLabels == other.mDomainLabels     &&
           mSectionLabels == other.mSectionLabels   &&
           mLabelTree.get() == other.mLabelTree.get();
}

bool Symbol::operator!=(const Symbol &other) const
{
    return !(*this==other);
}

}
}
}
