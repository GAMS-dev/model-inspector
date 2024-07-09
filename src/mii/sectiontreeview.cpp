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
#include "sectiontreeview.h"
#include "sectiontreeitem.h"

#include <QDebug>
#include <QMenu>

namespace gams {
namespace studio {
namespace mii {

SectionTreeView::SectionTreeView(QWidget *parent)
    : QTreeView(parent)
    , mMenu(new QMenu(this))
    , mLoadModelInstance(new QAction("Load instance", this))
    , mSaveViewAction(new QAction("Save", this))
    , mRemoveViewAction(new QAction("Remove", this))
    , mRenameViewAction(new QAction("Rename", this))
    , mCollapsAllAction(new QAction("Collapse All", this))
    , mExpandAllAction(new QAction("Expand All", this))
{
    mMenu->addAction(mLoadModelInstance);
    mMenu->addSeparator();
    mMenu->addAction(mSaveViewAction);
    mMenu->addAction(mRemoveViewAction);
    mMenu->addSeparator();
    mMenu->addAction(mRenameViewAction);
    mMenu->addSeparator();
    mMenu->addAction(mCollapsAllAction);
    mMenu->addAction(mExpandAllAction);

    connect(this, &SectionTreeView::customContextMenuRequested,
            this, &SectionTreeView::showCustomContextMenu);
    connect(mLoadModelInstance, &QAction::triggered,
            this, &SectionTreeView::loadModelInstance);
    connect(mSaveViewAction, &QAction::triggered,
            this, &SectionTreeView::saveViewTriggered);
    connect(mRemoveViewAction, &QAction::triggered,
            this, &SectionTreeView::removeViewTriggered);
    connect(mRenameViewAction, &QAction::triggered,
            this, &SectionTreeView::renameViewTriggered);
    connect(mCollapsAllAction, &QAction::triggered,
            this, &QTreeView::collapseAll);
    connect(mExpandAllAction, &QAction::triggered,
            this, &QTreeView::expandAll);
}

ViewActionStates SectionTreeView::viewActionStates() const
{
    auto index = currentIndex();
    return viewActionStates(index);
}

void SectionTreeView::showCustomContextMenu(const QPoint &pos)
{
    auto index = indexAt(pos);
    if (!index.isValid()) return;
    auto states = viewActionStates(index);
    mLoadModelInstance->setEnabled(states.LoadInstance);
    mSaveViewAction->setEnabled(states.SaveEnabled);
    mRemoveViewAction->setEnabled(states.RemoveEnabled);
    mRenameViewAction->setEnabled(states.RenameEnabled);
    mMenu->popup(viewport()->mapToGlobal(pos));
}

void SectionTreeView::renameViewTriggered()
{
    auto cIdx = currentIndex();
    if (cIdx.isValid())
        edit(cIdx);
}

void SectionTreeView::currentChanged(const QModelIndex &current,
                                     const QModelIndex &previous)
{
    if (!current.isValid() || !previous.isValid())
        return;
    auto item = static_cast<AbstractSectionTreeItem*>(current.internalPointer());
    auto inst = item->modelInstanceGroup();
    if (!inst->isActive()) {
        emit logMessage("Warning: Inactive view clicked. Please load the related model instance via the context menu.");
        return;
    }
    emit currentItemChanged();
    QTreeView::currentChanged(current, previous);
}

ViewActionStates SectionTreeView::viewActionStates(const QModelIndex &index) const
{
    ViewActionStates states;
    if (index.isValid()) {
        auto item = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
        states.LoadInstance = !item->isActive();
        if (item->isActive()) {
            if (item->isCustom()) {
                if (item->type() == ViewHelper::ViewDataType::CustomGroup) {
                    states.SaveEnabled = false;
                    states.RemoveEnabled = false;
                } else if (item->isGroup()) {
                    states.SaveEnabled = false;
                    states.RemoveEnabled = true;
                } else {
                    states.SaveEnabled = true;
                    states.RemoveEnabled = true;
                }
            } else {
                if (item->isGroup()) {
                    states.SaveEnabled = false;
                    states.RemoveEnabled = false;
                } else {
                    states.SaveEnabled = true;
                    states.RemoveEnabled = false;
                }
            }
        } else {
            states.SaveEnabled = false;
            states.RemoveEnabled = false;
            states.RenameEnabled = false;
        }
    }
    return states;
}

}
}
}
