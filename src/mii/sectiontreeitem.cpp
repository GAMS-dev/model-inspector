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
#include "sectiontreeitem.h"
#include "common.h"

namespace gams {
namespace studio {
namespace mii {

AbstractSectionTreeItem::AbstractSectionTreeItem(const QString &text,
                                                 AbstractSectionTreeItem *parent)
    : mText(text)
    , mParent(parent)
{

}

AbstractSectionTreeItem::AbstractSectionTreeItem(const QString &text,
                                                 AbstractViewFrame *widget,
                                                 AbstractSectionTreeItem *parent)
    : mText(text)
    , mParent(parent)
    , mWidget(widget)
{

}

AbstractSectionTreeItem::AbstractSectionTreeItem(const AbstractSectionTreeItem &other)
    : mText(other.mText)
    , mParent(other.mParent)
    , mType(other.mType)
    , mCustom(other.mCustom)
    , mWidget(other.mWidget)
{

}

AbstractSectionTreeItem::AbstractSectionTreeItem(AbstractSectionTreeItem &&other) noexcept
    : mText(std::move(other.mText))
    , mParent(other.mParent)
    , mType(other.mType)
    , mCustom(other.mCustom)
    , mWidget(other.mWidget)
{

}

AbstractSectionTreeItem::~AbstractSectionTreeItem()
{

}

void AbstractSectionTreeItem::append(AbstractSectionTreeItem *child)
{
    Q_UNUSED(child);
}

QList<AbstractViewFrame *> AbstractSectionTreeItem::removeChilds()
{
    return QList<AbstractViewFrame*>();
}

void AbstractSectionTreeItem::remove(AbstractSectionTreeItem *child)
{
    Q_UNUSED(child);
}

void AbstractSectionTreeItem::remove(int index, int count)
{
    Q_UNUSED(index);
    Q_UNUSED(count);
}

AbstractSectionTreeItem *AbstractSectionTreeItem::child(int row)
{
    Q_UNUSED(row);
    return nullptr;
}

int AbstractSectionTreeItem::childCount() const
{
    return 0;
}

int AbstractSectionTreeItem::columnCount() const
{
    return 1;
}

int AbstractSectionTreeItem::rowCount() const
{
    return 0;
}

QString AbstractSectionTreeItem::text() const
{
    return mText;
}

void AbstractSectionTreeItem::setText(const QString &text)
{
    mText = text;
}

AbstractViewFrame *AbstractSectionTreeItem::widget() const
{
    return mWidget;
}

void AbstractSectionTreeItem::setWidget(AbstractViewFrame* page)
{
    mWidget = page;
}

int AbstractSectionTreeItem::row() const
{
    return parent() ? parent()->childs().indexOf(this) : 0;
}

AbstractSectionTreeItem *AbstractSectionTreeItem::customGroup()
{
    if (mType == ViewHelper::ViewDataType::CustomGroup)
        return this;
    auto scrGroup = modelInstanceGroup();
    if (!scrGroup)
        return nullptr;
    for (auto item : scrGroup->childs()) {
        if (item->type() == ViewHelper::ViewDataType::CustomGroup)
            return item;
    }
    return nullptr;
}

AbstractSectionTreeItem *AbstractSectionTreeItem::predefinedGroup()
{
    if (mType == ViewHelper::ViewDataType::PredefinedGroup)
        return this;
    auto scrGroup = modelInstanceGroup();
    if (!scrGroup)
        return nullptr;
    for (auto item : scrGroup->childs()) {
        if (item->type() == ViewHelper::ViewDataType::PredefinedGroup)
            return item;
    }
    return nullptr;
}

AbstractSectionTreeItem *AbstractSectionTreeItem::symbolGroup()
{
    if (mType == ViewHelper::ViewDataType::SymbolsGroup)
        return this;
    for (auto item : childs()) {
        auto group = item->customGroup();
        if (group)
            return group;
    }
    return nullptr;
}

AbstractSectionTreeItem *AbstractSectionTreeItem::modelInstanceGroup()
{
    AbstractSectionTreeItem* scratchGroup = nullptr;
    auto p = this;
    while (p) {
        if (!p)
            break;
        if (p->type() == ViewHelper::ViewDataType::ModelInstanceGroup)
            scratchGroup = p;
        p = p->parent();
    }
    return scratchGroup;
}

AbstractSectionTreeItem *AbstractSectionTreeItem::find(ViewHelper::ViewDataType type)
{
    QList<AbstractSectionTreeItem*> items { this };
    while (!items.isEmpty()) {
        auto item = items.takeFirst();
        if (item->type() == type)
            return item;
        items.append(item->childs());
    }
    return nullptr;
}

AbstractSectionTreeItem& AbstractSectionTreeItem::operator=(const AbstractSectionTreeItem &other)
{
    mText = other.mText;
    mParent = other.mParent;
    mType = other.mType;
    mCustom = other.mCustom;
    mWidget = other.mWidget;
    return *this;
}

AbstractSectionTreeItem& AbstractSectionTreeItem::operator=(AbstractSectionTreeItem &&other) noexcept
{
    mText = other.mText;
    mParent = other.mParent;
    mType = other.mType;
    mCustom = other.mCustom;
    mWidget = other.mWidget;
    return *this;
}

QList<AbstractViewFrame *> AbstractSectionTreeItem::widgets() const
{
    return QList<AbstractViewFrame*>();
}

ViewHelper::ViewDataType AbstractSectionTreeItem::type() const
{
    return mType;
}

void AbstractSectionTreeItem::setType(ViewHelper::ViewDataType type)
{
    mType = type;
}

void AbstractSectionTreeItem::setType(const QString &text)
{
    if (text == ViewHelper::BPScaling)
        mType = ViewHelper::ViewDataType::BP_Scaling;
    else if (text == ViewHelper::BPOverview)
        mType = ViewHelper::ViewDataType::BP_Overview;
    else if (text == ViewHelper::BPCount)
        mType = ViewHelper::ViewDataType::BP_Count;
    else if (text == ViewHelper::BPAverage)
        mType = ViewHelper::ViewDataType::BP_Average;
    else if (text == ViewHelper::Postopt)
        mType = ViewHelper::ViewDataType::Postopt;
    else if (text == ViewHelper::SymbolView)
        mType = ViewHelper::ViewDataType::Symbols;
    else if (text == ViewHelper::Blockpic)
        mType = ViewHelper::ViewDataType::BlockpicGroup;
    else
        mType = ViewHelper::ViewDataType::Unknown;
}

bool AbstractSectionTreeItem::isCustom() const
{
    return mCustom;
}

void AbstractSectionTreeItem::setCustom(bool custom)
{
    mCustom = custom;
}

bool AbstractSectionTreeItem::isGroup() const
{
    switch (mType) {
    case ViewHelper::ViewDataType::BlockpicGroup:
    case ViewHelper::ViewDataType::SymbolsGroup:
    case ViewHelper::ViewDataType::CustomGroup:
    case ViewHelper::ViewDataType::PredefinedGroup:
    case ViewHelper::ViewDataType::ModelInstanceGroup:
        return true;
    default:
        return false;
    }
}

bool AbstractSectionTreeItem::isModelInstanceGroup() const
{
    return ViewHelper::ViewDataType::ModelInstanceGroup == mType;
}

QString AbstractSectionTreeItem::scratchDir() const
{
    return mScratchDir;
}

void AbstractSectionTreeItem::setScratchDir(const QString &scratchDir)
{
    mScratchDir = scratchDir;
}

AbstractSectionTreeItem *AbstractSectionTreeItem::parent() const
{
    return mParent;
}

void AbstractSectionTreeItem::setParent(AbstractSectionTreeItem *parent)
{
    mParent = parent;
}

SectionGroupTreeItem::SectionGroupTreeItem(const QString &text,
                                           AbstractSectionTreeItem *parent)
    : AbstractSectionTreeItem(text, parent)
{

}

SectionGroupTreeItem::SectionGroupTreeItem(const SectionGroupTreeItem &other)
    : AbstractSectionTreeItem(other)
    , mChilds(other.mChilds)
{

}

SectionGroupTreeItem::SectionGroupTreeItem(SectionGroupTreeItem &&other) noexcept
    : AbstractSectionTreeItem(std::move(other))
    , mChilds(std::move(other.mChilds))
{

}

SectionGroupTreeItem::~SectionGroupTreeItem()
{
    qDeleteAll(mChilds);
}

bool SectionGroupTreeItem::isActive() const
{
    for (auto child : mChilds) {
        return child->isActive();
    }
    return false;
}

void SectionGroupTreeItem::setActive(bool active)
{
    if (type() == ViewHelper::ViewDataType::ModelInstanceGroup) {
        if (active) {
            setText(text().split(" ").first().trimmed());
        } else {
            setText(text() + ActivePostfix);
        }
    }
    for (auto child : mChilds) {
        child->setActive(active);
    }
}

void SectionGroupTreeItem::append(AbstractSectionTreeItem *child)
{
    mChilds.append(child);
}

void SectionGroupTreeItem::removeAllChilds()
{
    qDeleteAll(mChilds);
    mChilds.clear();
}

QList<AbstractViewFrame*> SectionGroupTreeItem::removeChilds()
{
    QList<AbstractViewFrame*> wgts = widgets();
    qDeleteAll(mChilds);
    mChilds.clear();
    return wgts;
}

void SectionGroupTreeItem::remove(AbstractSectionTreeItem *child)
{
    if (!child)
        return;
    mChilds.removeOne(child);
    delete child;
}

void SectionGroupTreeItem::remove(int index, int count)
{
    if (index+count > mChilds.size())
        return;
    QVector<AbstractSectionTreeItem*> items(index+count);
    for (int i=index; i<index+count; ++i) {
        items[i] = mChilds.at(i);
    }
    mChilds.remove(index, count);
    qDeleteAll(items);
}

AbstractSectionTreeItem *SectionGroupTreeItem::child(int row)
{
    if (row < 0 || row >= mChilds.size())
        return nullptr;
    return mChilds.at(row);
}

const QList<AbstractSectionTreeItem *> &SectionGroupTreeItem::childs() const
{
    return mChilds;
}

int SectionGroupTreeItem::childCount() const
{
    return mChilds.count();
}

QList<AbstractViewFrame*> SectionGroupTreeItem::widgets() const
{
    QList<AbstractViewFrame*> wgts;
    for (auto child : mChilds) {
        wgts.append(child->widgets());
    }
    return wgts;
}

SectionGroupTreeItem &SectionGroupTreeItem::operator=(const SectionGroupTreeItem &other)
{
    mChilds = other.childs();
    return *this;
}

SectionGroupTreeItem &SectionGroupTreeItem::operator=(SectionGroupTreeItem &&other) noexcept
{
    mChilds = other.childs();
    return *this;
}

SectionTreeItem::SectionTreeItem(const QString &text,
                                 AbstractViewFrame *widget,
                                 AbstractSectionTreeItem *parent)
    : AbstractSectionTreeItem(text, widget, parent)
{

}

SectionTreeItem::SectionTreeItem(const SectionTreeItem &other)
    : AbstractSectionTreeItem(other)
    , mChilds(other.mChilds)
{

}

SectionTreeItem::SectionTreeItem(SectionTreeItem &&other) noexcept
    : AbstractSectionTreeItem(std::move(other))
    , mChilds(std::move(other.mChilds))
{

}

SectionTreeItem::~SectionTreeItem()
{

}

bool SectionTreeItem::isActive() const
{
    return mActive;
}

void SectionTreeItem::setActive(bool active)
{
    mActive = active;
}

void SectionTreeItem::append(AbstractSectionTreeItem *child)
{
    Q_UNUSED(child);
}

void SectionTreeItem::removeAllChilds()
{
    mChilds.clear();
}

QList<AbstractViewFrame *> SectionTreeItem::removeChilds()
{
    return QList<AbstractViewFrame*>();
}

void SectionTreeItem::remove(int index, int count)
{
    Q_UNUSED(index);
    Q_UNUSED(count);
}

AbstractSectionTreeItem *SectionTreeItem::child(int row)
{
    Q_UNUSED(row);
    return nullptr;
}

const QList<AbstractSectionTreeItem*> &SectionTreeItem::childs() const
{
    return mChilds;
}

int SectionTreeItem::childCount() const
{
    return 0;
}

QList<AbstractViewFrame*> SectionTreeItem::widgets() const
{
    QList<AbstractViewFrame*> wgts;
    if (widget())
        wgts.append(widget());
    return wgts;
}

SectionTreeItem &SectionTreeItem::operator=(const SectionTreeItem &other)
{
    mChilds = other.childs();
    return *this;
}

SectionTreeItem &SectionTreeItem::operator=(SectionTreeItem &&other) noexcept
{
    mChilds = other.childs();
    return *this;
}

}
}
}
