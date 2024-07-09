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
#include "sectiontreemodel.h"
#include "sectiontreeitem.h"
#include "abstractviewframe.h"

#include <QDir>
#include <QStackedWidget>

namespace gams {
namespace studio {
namespace mii {

SectionTreeModel::SectionTreeModel(QObject *parent)
    : QAbstractItemModel(parent)
    , mRoot(new SectionGroupTreeItem({ViewHelper::ModelInstance}))
{
    mRoot->setType(ViewHelper::ViewDataType::Unknown);
}

SectionTreeModel::~SectionTreeModel()
{
    delete mRoot;
}

QString SectionTreeModel::scratchDir() const
{
    return mScratchDir;
}

void SectionTreeModel::setScratchDir(const QString &scratchDir)
{
    mScratchDir = scratchDir;
}

void SectionTreeModel::appendCustomView(const QString &text,
                                        AbstractViewFrame* widget,
                                        AbstractSectionTreeItem *customRoot)
{
    if (widget->type() == ViewHelper::ViewDataType::Unknown || !customRoot) {
        return;
    }
    bool active = customRoot->modelInstanceGroup()->isActive();
    beginResetModel();
    if (widget->type() == ViewHelper::ViewDataType::Postopt) {
        auto customPostopt = customRoot->find(ViewHelper::ViewDataType::PostoptGroup);
        if (!customPostopt) {
            customPostopt = new SectionGroupTreeItem(ViewHelper::Postopt, customRoot);
            customPostopt->setType(ViewHelper::ViewDataType::PostoptGroup);
            customPostopt->setCustom(true);
            customRoot->append(customPostopt);
        }
        auto item = new SectionTreeItem(text, widget, customPostopt);
        item->setType(widget->type());
        item->setCustom(true);
        customPostopt->append(item);
        customPostopt->setActive(active);
    } else if (widget->type() == ViewHelper::ViewDataType::Symbols) {
        auto customSymbolView = customRoot->find(ViewHelper::ViewDataType::SymbolsGroup);
        if (!customSymbolView) {
            customSymbolView = new SectionGroupTreeItem(ViewHelper::SymbolView, customRoot);
            customSymbolView->setType(ViewHelper::ViewDataType::SymbolsGroup);
            customSymbolView->setCustom(true);
            customRoot->append(customSymbolView);
        }
        auto item = new SectionTreeItem(text, widget, customSymbolView);
        item->setType(widget->type());
        item->setCustom(true);
        customSymbolView->append(item);
        customSymbolView->setActive(active);
    } else {
        auto customBlockpic = customRoot->find(ViewHelper::ViewDataType::BlockpicGroup);
        if (!customBlockpic) {
            customBlockpic = new SectionGroupTreeItem(ViewHelper::Blockpic, customRoot);
            customBlockpic->setType(ViewHelper::ViewDataType::BlockpicGroup);
            customBlockpic->setCustom(true);
            customRoot->append(customBlockpic);
        }
        auto item = new SectionTreeItem(text, widget, customBlockpic);
        item->setType(widget->type());
        item->setCustom(true);
        customBlockpic->append(item);
        customBlockpic->setActive(active);
    }
    endResetModel();
}

AbstractSectionTreeItem *SectionTreeModel::rootItem() const
{
    return mRoot;
}

QVariant SectionTreeModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    auto *item = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
    if (role == ItemDataTypeRole) {
        return (int)item->type();
    }
    if (role == Qt::FontRole && item->type() == ViewHelper::ViewDataType::ModelInstanceGroup) {
        QFont font;
        font.setBold(item->isActive());
        return font;
    }
    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        if (index.column() == 0)
            return item->text();
    }
    return QVariant();
}

bool SectionTreeModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    auto text = value.toString().trimmed();
    if (index.isValid() && role == Qt::EditRole && !text.isEmpty()) {
        auto item = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
        item->setText(text);
        emit dataChanged(index, index);
        return true;
    }
    return QAbstractItemModel::setData(index, value, role);
}

Qt::ItemFlags SectionTreeModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return QAbstractItemModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEditable;
    return Qt::NoItemFlags;
}

QVariant SectionTreeModel::headerData(int section, Qt::Orientation orientation,
                                      int role) const
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole && section == 0)
        return mRoot->text();
    return QVariant();
}

bool SectionTreeModel::setHeaderData(int section, Qt::Orientation orientation,
                                     const QVariant &value, int role)
{
    if (orientation == Qt::Horizontal && role == Qt::DisplayRole &&
            section == 0) {
        mRoot->setText(value.toString());
        return true;
    }
    return false;
}

QModelIndex SectionTreeModel::index(int row, int column,
                                    const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    auto parentItem = parent.isValid() ? static_cast<AbstractSectionTreeItem*>(parent.internalPointer()) : mRoot;

    AbstractSectionTreeItem *childItem = parentItem->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex SectionTreeModel::parent(const QModelIndex &index) const
{
    if (!index.isValid())
        return QModelIndex();
    auto *childItem = static_cast<AbstractSectionTreeItem*>(index.internalPointer());
    auto *parentItem = childItem->parent();
    if (parentItem == mRoot)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

int SectionTreeModel::rowCount(const QModelIndex &parent) const
{
    if (parent.column() > 0)
        return 0;
    auto parentItem = parent.isValid() ? static_cast<AbstractSectionTreeItem*>(parent.internalPointer()) : mRoot;
    return parentItem->childCount();
}

int SectionTreeModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return static_cast<AbstractSectionTreeItem*>(parent.internalPointer())->columnCount();
    return mRoot->columnCount();
}

QHash<int, QByteArray> SectionTreeModel::roleNames() const
{
    static QHash<int, QByteArray> mapping { { ItemDataTypeRole, "itemdata" } };
    return mapping;
}

bool SectionTreeModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (!parent.isValid())
        return false;
    auto parentItem = static_cast<AbstractSectionTreeItem*>(parent.internalPointer());
    auto customRoot = parentItem->customGroup();
    if (!parentItem->childCount())
        return false;
    beginResetModel();
    parentItem->remove(row, count);
    if (parentItem != customRoot && !parentItem->childCount() &&
        (parentItem->type() == ViewHelper::ViewDataType::BlockpicGroup ||
         parentItem->type() == ViewHelper::ViewDataType::PostoptGroup  ||
         parentItem->type() == ViewHelper::ViewDataType::SymbolsGroup)) {
        customRoot->remove(parentItem);
    }
    endResetModel();
    return true;
}

QList<AbstractViewFrame*> SectionTreeModel::removeItem(AbstractSectionTreeItem *item)
{
    if (!item)
        return QList<AbstractViewFrame*>();
    QList<AbstractViewFrame*> wgts;
    beginResetModel();
    auto p = item->parent();
    if (item->isGroup() && item->type() != ViewHelper::ViewDataType::CustomGroup) {
        auto customRoot = item->customGroup();
        wgts = item->removeChilds();
        customRoot->remove(item);
    } else if (!item->isGroup()) {
        wgts.append(item->widgets());
        p->remove(item);
    }
    endResetModel();
    return wgts;
}

QList<AbstractViewFrame*> SectionTreeModel::removeCustomRows(AbstractSectionTreeItem *cutomRoot)
{
    if (!cutomRoot)
        return QList<AbstractViewFrame*>();
    beginResetModel();
    auto wgts = cutomRoot->removeChilds();
    endResetModel();
    return wgts;
}

void SectionTreeModel::clearModelData()
{
    beginResetModel();
    mRoot->removeAllChilds();
    endResetModel();
}

void SectionTreeModel::loadModelData(QStackedWidget *stackedWidget,
                                     ViewHelper::MiiModeType mode,
                                     const QString &modelFileName)
{
    switch (mode) {
    case ViewHelper::MiiModeType::Single:
        loadSingleModeModelData(stackedWidget, modelFileName);
        break;
    case ViewHelper::MiiModeType::Multi:
        loadMultiModeModelData(stackedWidget);
    default:
        break;
    }
}

void SectionTreeModel::loadSingleModeModelData(QStackedWidget* stackedWidget,
                                               const QString &modelFilePath)
{
    QFileInfo fi(modelFilePath);
    auto group = new SectionGroupTreeItem(fi.fileName(), mRoot);
    group->setType(ViewHelper::ViewDataType::ModelInstanceGroup);
    mRoot->append(group);
    loadViewTreeData(group, stackedWidget);
    group->setActive(true);
}

void SectionTreeModel::loadMultiModeModelData(QStackedWidget* stackedWidget)
{
    bool setActive = true;
    QDir scratchDir(mScratchDir);
    for (const auto& entry : scratchDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        auto group = new SectionGroupTreeItem(entry, mRoot);
        group->setType(ViewHelper::ViewDataType::ModelInstanceGroup);
        QDir groupScratchDir(mScratchDir + QDir::separator() + entry);
        group->setScratchDir(groupScratchDir.absolutePath());
        mRoot->append(group);
        loadViewTreeData(group, stackedWidget);
        if (setActive) {
            group->setActive(setActive);
            setActive = false;
        } else {
            group->setActive(setActive);
        }
    }
}

void SectionTreeModel::loadViewTreeData(AbstractSectionTreeItem *root, QStackedWidget *stackedWidget)
{
    auto predefinedRoot = new SectionGroupTreeItem(ViewHelper::PredefinedViews, root);
    predefinedRoot->setType(ViewHelper::ViewDataType::PredefinedGroup);
    root->append(predefinedRoot);
    auto blockpicItem = new SectionGroupTreeItem(ViewHelper::Blockpic, predefinedRoot);
    blockpicItem->setType(ViewHelper::ViewDataType::BlockpicGroup);
    predefinedRoot->append(blockpicItem);
    for (int i=0; i<ViewHelper::PredefinedViewTexts.size(); ++i) {
        if (ViewHelper::PredefinedViewTexts.at(i) == ViewHelper::BPScaling) {
            auto widget = stackedWidget->widget((int)ViewHelper::ViewDataType::BP_Scaling);
            auto item = new SectionTreeItem(ViewHelper::PredefinedViewTexts.at(i),
                                            static_cast<AbstractViewFrame*>(widget->children().last()),
                                            blockpicItem);
            item->setType(ViewHelper::PredefinedViewTexts.at(i));
            blockpicItem->append(item);
        } else if (ViewHelper::PredefinedViewTexts.at(i) == ViewHelper::BPOverview) {
            auto widget = stackedWidget->widget((int)ViewHelper::ViewDataType::BP_Overview);
            auto item = new SectionTreeItem(ViewHelper::PredefinedViewTexts.at(i),
                                            static_cast<AbstractViewFrame*>(widget->children().last()),
                                            blockpicItem);
            item->setType(ViewHelper::PredefinedViewTexts.at(i));
            blockpicItem->append(item);
        } else if (ViewHelper::PredefinedViewTexts.at(i) == ViewHelper::BPCount) {
            auto widget = stackedWidget->widget((int)ViewHelper::ViewDataType::BP_Count);
            auto item = new SectionTreeItem(ViewHelper::PredefinedViewTexts.at(i),
                                            static_cast<AbstractViewFrame*>(widget->children().last()),
                                            blockpicItem);
            item->setType(ViewHelper::PredefinedViewTexts.at(i));
            blockpicItem->append(item);
        } else if (ViewHelper::PredefinedViewTexts.at(i) == ViewHelper::BPAverage) {
            auto widget = stackedWidget->widget((int)ViewHelper::ViewDataType::BP_Average);
            auto item = new SectionTreeItem(ViewHelper::PredefinedViewTexts.at(i),
                                            static_cast<AbstractViewFrame*>(widget->children().last()),
                                            blockpicItem);
            item->setType(ViewHelper::PredefinedViewTexts.at(i));
            blockpicItem->append(item);
        } else if (ViewHelper::PredefinedViewTexts.at(i) == ViewHelper::Postopt) {
            auto widget = stackedWidget->widget((int)ViewHelper::ViewDataType::Postopt);
            auto item = new SectionTreeItem(ViewHelper::PredefinedViewTexts.at(i),
                                            static_cast<AbstractViewFrame*>(widget->children().last()),
                                            predefinedRoot);
            item->setType(ViewHelper::PredefinedViewTexts.at(i));
            predefinedRoot->append(item);
        }
    }
    auto customRoot = new SectionGroupTreeItem(ViewHelper::CustomViews, root);
    customRoot->setType(ViewHelper::ViewDataType::CustomGroup);
    customRoot->setCustom(true);
    root->append(customRoot);
}

}
}
}
