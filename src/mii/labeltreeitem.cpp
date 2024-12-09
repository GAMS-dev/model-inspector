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
#include "labeltreeitem.h"

#include <QDebug>

namespace gams {
namespace studio {
namespace mii {

LabelTreeItem::LabelTreeItem(LabelTreeItem *parent)
    : mParent(parent)
{

}

LabelTreeItem::LabelTreeItem(const QString &text, LabelTreeItem *parent)
    : mParent(parent)
    , mText(text)
{

}

LabelTreeItem::LabelTreeItem(const char *text, LabelTreeItem *parent)
    : mParent(parent)
    , mText(text)
{

}

LabelTreeItem::~LabelTreeItem()
{
    qDeleteAll(mChilds);
}

void LabelTreeItem::append(LabelTreeItem *child)
{
    mChilds.append(child);
}

LabelTreeItem* LabelTreeItem::child(int index)
{
    if (index < 0 || index >= mChilds.size())
        return nullptr;
    return mChilds.at(index);
}

const QList<LabelTreeItem*> &LabelTreeItem::childs() const
{
    return mChilds;
}

QList<LabelTreeItem*> LabelTreeItem::visibleChilds() const
{
    QList<LabelTreeItem*> visible;
    Q_FOREACH(auto c, mChilds) {
        if (c->isVisible())
            visible.append(c);
    }
    return visible;
}

void LabelTreeItem::setChilds(const QList<LabelTreeItem *> &childs)
{
    mChilds = childs;
}

LabelTreeItem* LabelTreeItem::clone(LabelTreeItem* newParent) const
{
    auto root = new LabelTreeItem(newParent);
    root->setText(mText);
    root->setSectionIndex(mSectionIndex);
    root->setSections(mSections);
    Q_FOREACH(auto child, mChilds) {
        auto newChild = child->clone(root);
        root->append(newChild);
    }
    return root;
}

bool LabelTreeItem::hasChildren() const
{
    return !mChilds.isEmpty();
}

bool LabelTreeItem::isRoot() const
{
    return mParent == nullptr;
}

void LabelTreeItem::remove(LabelTreeItem *child)
{
    if (!child) return;
    mChilds.removeAll(child);
    child->setParent(nullptr);
}

int LabelTreeItem::firstSectionIndex() const
{
    if (sections().isEmpty()) {
        return -1;
    }
    auto secs = sections().values();
    std::sort(secs.begin(), secs.end());
    return secs.first();
}

int LabelTreeItem::sectionIndex() const
{
    return mSectionIndex;
}

void LabelTreeItem::setSectionIndex(int index)
{
    mSectionIndex = index;
}

LabelTreeItem *LabelTreeItem::parent() const
{
    return mParent;
}

void LabelTreeItem::setParent(LabelTreeItem *parent)
{
    mParent = parent;
}

QList<LabelTreeItem *> LabelTreeItem::siblings() const
{
    return mParent ? mParent->childs() : QList<LabelTreeItem*>();
}

int LabelTreeItem::size() const
{
    return mChilds.size();
}

QSet<int> LabelTreeItem::sections() const
{
    if (!hasChildren())
        return mSections;
    QSet<int> allSections;
    Q_FOREACH(auto child, mChilds) {
        allSections.unite(child->sections());
    }
    return allSections;
}

UnitedSections LabelTreeItem::unitedSections() const
{
    UnitedSections united;
    if (!hasChildren() && !mSections.isEmpty())
        united.append(mSections);
    QList<LabelTreeItem*> items { mChilds };
    while (!items.isEmpty()) {
        auto item = items.takeFirst();
        if (item->hasChildren()) {
            items.append(item->childs());
        } else if (item->isVisible()) {
            united.append(item->sections());
        }
    }
    return united;
}

void LabelTreeItem::setSections(const QSet<int> &sections)
{
    mSections = sections;
}

QList<int> LabelTreeItem::visibleSections() const
{
    if (!hasChildren() && isVisible())
        return mSections.values();
    QList<int> allSections;
    Q_FOREACH(auto child, mChilds) {
        if (child->isVisible())
            allSections.append(child->visibleSections());
    }
    return allSections;
}

QList<int> LabelTreeItem::visibleSectionsSorted() const
{
    auto list = visibleSections();
    std::sort(list.begin(), list.end());
    return list;
}

SectionLabels LabelTreeItem::sectionLabels(int startSection, int dimension) const
{// TODO header drawing rules... when to generate empty label cells
    SectionLabels sectionLabels;
    int extent = sectionExtent();
    QList<LabelTreeItem*> levelItems { childs() };
    if (levelItems.isEmpty())
        return sectionLabels;
    QList<LabelTreeItem*> newItems;
    for (int d=1, s=startSection; d<=dimension; ++d, s=startSection) {
        if (levelItems.isEmpty()) continue;
        int duplicate = extent / levelItems.size();
        while (!levelItems.isEmpty()) {
            auto item = levelItems.takeFirst();
            if (!item->isVisible())
                continue;
            newItems.append(item->visibleChilds());
            for (int repeat=0; repeat<duplicate; ++repeat) {
                auto data = sectionLabels[s];
                data << item->text();
                sectionLabels[s] = data;
                ++s;
            }
        }
        levelItems = newItems;
        newItems.clear();
    }
    return sectionLabels;
}

QString LabelTreeItem::text() const
{
    return mText;
}

void LabelTreeItem::setText(const QString &text)
{
    mText = text;
}

bool LabelTreeItem::isVisible() const
{
    if (!hasChildren())
        return mIsVisible;
    Q_FOREACH(auto child, mChilds) {
        if (child->isVisible()) {
            return true;
        }
    }
    return false;
}

void LabelTreeItem::setVisible(bool visible)
{
    mIsVisible = visible;
    Q_FOREACH(auto child, mChilds) {
        child->setVisible(visible);
    }
}

int LabelTreeItem::sectionExtent() const
{
    QList<LabelTreeItem*> leafs;
    if (!hasChildren() && !mSections.isEmpty())
        return 1;
    QList<LabelTreeItem*> items { mChilds };
    while (!items.isEmpty()) {
        auto item = items.takeFirst();
        if (!item->isVisible())
            continue;
        items.append(item->childs());
        if (!item->hasChildren())
            leafs.append(item);
    }
    return leafs.size();
}

LabelTreeItem* LabelTreeItem::visibleBranch(QList<LabelTreeItem*> &currentLevel,
                                            const QString &typeText, int dimension)
{
    LabelTreeItem* newItem = nullptr;
    while (!currentLevel.isEmpty()) {
        newItem = currentLevel.takeFirst();
        if (newItem->isVisible()) {
            newItem->setText(typeText + " - " + QString::number(dimension));
            break;
        } else {
            newItem->parent()->remove(newItem);
            delete newItem;
            newItem = nullptr;
        }
    }
    return newItem;
}

}
}
}
