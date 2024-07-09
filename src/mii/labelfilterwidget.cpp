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
#include "labelfilterwidget.h"
#include "ui_labelfilterwidget.h"
#include "filtertreemodel.h"
#include "filtertreeitem.h"

#include <QSortFilterProxyModel>
#include <QMenu>

namespace gams {
namespace studio {
namespace mii {

LabelFilterWidget::LabelFilterWidget(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LabelFilterWidget)
    , mOrientation(orientation)
    , mBaseModel(nullptr)
    , mFilterModel(nullptr)
{
    ui->setupUi(this);
    connect(ui->applyButton, &QPushButton::clicked, this, &LabelFilterWidget::applyClicked);
    connect(ui->selectButton, &QPushButton::clicked, this, &LabelFilterWidget::selectClicked);
    connect(ui->deselectButton, &QPushButton::clicked, this, &LabelFilterWidget::deselectClicked);
}

LabelFilterWidget::~LabelFilterWidget()
{
    delete ui;
}

FilterTreeItem *LabelFilterWidget::data() const
{
    if (mBaseModel) {
        return mBaseModel->filterItem();
    }
    return nullptr;
}

void LabelFilterWidget::setData(FilterTreeItem *rootItem)
{
    auto oldModel = ui->labelView->selectionModel();
    mBaseModel = new FilterTreeModel(rootItem, ui->labelView);
    mFilterModel = new QSortFilterProxyModel(ui->labelView);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setRecursiveFilteringEnabled(true);
    mFilterModel->setSourceModel(mBaseModel);
    ui->labelView->setModel(mFilterModel);
    ui->labelView->expandAll();
    if (oldModel)
        oldModel->deleteLater();
    connect(ui->labelEdit, &QLineEdit::textChanged,
            this, &LabelFilterWidget::applyFilter);
}

void LabelFilterWidget::showEvent(QShowEvent *event)
{
    if (!mFilterModel)
        return;
    applyFilter(ui->labelEdit->text());
    QWidget::showEvent(event);
}

void LabelFilterWidget::applyClicked()
{
    emit filterChanged(identifierState(), mOrientation);
    auto menu = qobject_cast<QMenu*>(this->parent());
    if (menu)
        menu->close();
    ui->labelEdit->clear();
}

void LabelFilterWidget::selectClicked()
{
    applyCheckState(true);
}

void LabelFilterWidget::deselectClicked()
{
    applyCheckState(false);
}

void LabelFilterWidget::applyFilter(const QString &text)
{
    if (!mFilterModel)
        return;
    mFilterModel->setFilterWildcard(text);
    ui->labelView->expandAll();
}

void LabelFilterWidget::applyCheckState(bool state)
{
    if (!mFilterModel)
        return;
    QModelIndexList indexes(mFilterModel->rowCount());
    for(int row=0; row<mFilterModel->rowCount(); ++row) {
        indexes[row] = mFilterModel->index(row, 0);
    }
    while (!indexes.isEmpty()) {
        auto index = indexes.takeFirst();
        if (mFilterModel->flags(index) == Qt::NoItemFlags)
            continue;
        if (!mFilterModel->hasChildren(index))
            mFilterModel->setData(index, state, Qt::CheckStateRole);
        if (mFilterModel->hasChildren(index)) {
            for(int row=0; row<mFilterModel->rowCount(index); ++row)
                indexes.append(mFilterModel->index(row, 0, index));
        }
    }
}

IdentifierState LabelFilterWidget::identifierState()
{
    if (!mFilterModel)
        return IdentifierState();
    auto rootItem = static_cast<FilterTreeModel*>(mFilterModel->sourceModel())->filterItem();
    QList<FilterTreeItem*> items { rootItem };
    IdentifierState state;
    state.Enabled = true;
    state.SectionIndex = items.first()->sectionIndex();
    state.SymbolIndex = items.first()->symbolIndex();
    state.Text = items.first()->text();
    while (!items.isEmpty()) {
        auto item = items.takeFirst();
        items.append(item->childs());
        if (!item->isCheckable())
            continue;
        if (item->checked() == Qt::Unchecked) {
            state.CheckStates.insert(item->sectionIndex());
        }
    }
    state.Checked = rootItem->checked() != Qt::Unchecked ? Qt::Checked : Qt::Unchecked;
    return state;
}

} // namespace mii
} // namespace studio
} // namespace gams
