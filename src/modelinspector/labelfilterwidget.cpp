#include "labelfilterwidget.h"
#include "ui_labelfilterwidget.h"
#include "filtertreemodel.h"
#include "filtertreeitem.h"

#include <QSortFilterProxyModel>
#include <QMenu>

namespace gams {
namespace studio {
namespace modelinspector {

LabelFilterWidget::LabelFilterWidget(Qt::Orientation orientation, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::LabelFilterWidget)
    , mOrientation(orientation)
{
    ui->setupUi(this);
}

LabelFilterWidget::~LabelFilterWidget()
{
    delete ui;
}

void LabelFilterWidget::setData(FilterTreeItem *rootItem)
{
    auto oldModel = ui->labelView->selectionModel();
    auto treeModel = new FilterTreeModel(rootItem, ui->labelView);
    mFilterModel = new QSortFilterProxyModel(ui->labelView);
    mFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mFilterModel->setRecursiveFilteringEnabled(true);
    mFilterModel->setSourceModel(treeModel);
    ui->labelView->setModel(mFilterModel);
    ui->labelView->expandAll();
    if (oldModel)
        oldModel->deleteLater();
    connect(ui->labelEdit, &QLineEdit::textChanged,
            this, &LabelFilterWidget::applyFilter);
}

void LabelFilterWidget::showEvent(QShowEvent *event)
{
    applyFilter(ui->labelEdit->text());
    QWidget::showEvent(event);
}

void LabelFilterWidget::on_applyButton_clicked()
{
    emit filterChanged(identifierState(), mOrientation);
    static_cast<QMenu*>(this->parent())->close();
    ui->labelEdit->clear();
}

void LabelFilterWidget::on_selectButton_clicked()
{
    applyCheckState(true);
}

void LabelFilterWidget::on_deselectButton_clicked()
{
    applyCheckState(false);
}

void LabelFilterWidget::applyFilter(const QString &text)
{
    mFilterModel->setFilterWildcard(text);
    ui->labelView->expandAll();
}

void LabelFilterWidget::applyCheckState(bool state)
{
    QModelIndexList indexes;
    for(int row=0; row<mFilterModel->rowCount(); ++row) {
        indexes.append(mFilterModel->index(row, 0));
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
    QList<FilterTreeItem*> items {
        static_cast<FilterTreeModel*>(mFilterModel->sourceModel())->filterItem()
    };
    IdentifierState state;
    state.SectionIndex = items.first()->index();
    state.SymbolIndex = items.first()->symbolIndex();
    while (!items.isEmpty()) {
        auto item = items.takeFirst();
        items.append(item->childs());
        if (!item->isCheckable())
            continue;
        state.LabelCheckStates[item->index()] = item->checked();
    }
    return state;
}

} // namespace modelinspector
} // namespace studio
} // namespace gams
