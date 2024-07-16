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
#include "filterdialog.h"
#include "ui_filterdialog.h"
#include "filtertreeitem.h"
#include "filtertreemodel.h"
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"

#include <QSortFilterProxyModel>

namespace gams {
namespace studio {
namespace mii {

FilterDialog::FilterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FilterDialog)
    , mViewConfig(ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::Unknown,
                                                           QSharedPointer<AbstractModelInstance>(new EmptyModelInstance)))
    , mEqnFilterModel(nullptr)
    , mVarFilterModel(nullptr)
    , mLabelFilterModel(nullptr)
    , mAttrFilterModel(nullptr)
{
    ui->setupUi(this);
    ui->labelView->sortByColumn(0, Qt::AscendingOrder);
    ui->minEdit->setValidator(new QDoubleValidator(ui->minEdit));
    ui->maxEdit->setValidator(new QDoubleValidator(ui->maxEdit));
    setupConnections();
}

FilterDialog::~FilterDialog()
{
    delete ui;
}

const QSharedPointer<AbstractViewConfiguration> &FilterDialog::viewConfig() const
{
    return mViewConfig;
}

void FilterDialog::setViewConfig(const QSharedPointer<AbstractViewConfiguration> &config)
{
    mViewConfig = config;
    setupEquationFilter(mViewConfig->currentIdentifierFilter().value(equationOrientation()),
                        mViewConfig->defaultIdentifierFilter().value(equationOrientation()));
    setupVariableFilter(mViewConfig->currentIdentifierFilter().value(variableOrientation()),
                        mViewConfig->defaultIdentifierFilter().value(variableOrientation()));
    setupAttributeFilter();
    setupLabelFilter();
    setupDimensionFilter();

    ui->minEdit->setText(QString::number(mViewConfig->currentValueFilter().MinValue));
    updateRangeEdit(ui->minEdit, ui->minEdit->text());
    ui->maxEdit->setText(QString::number(mViewConfig->currentValueFilter().MaxValue));
    updateRangeEdit(ui->maxEdit, ui->maxEdit->text());
    ui->excludeBox->setChecked(mViewConfig->currentValueFilter().ExcludeRange);
    ui->absoluteBox->setChecked(mViewConfig->currentValueFilter().UseAbsoluteValues);
    if (mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal)
        ui->absoluteBox->setEnabled(false);
    else
        ui->absoluteBox->setEnabled(true);
    ui->epsBox->setChecked(mViewConfig->currentValueFilter().ShowEps);
    ui->nInfBox->setChecked(mViewConfig->currentValueFilter().ShowNInf);
    ui->pInfBox->setChecked(mViewConfig->currentValueFilter().ShowPInf);
}

void FilterDialog::applyButtonClicked()
{
    mViewConfig->setFilterDialogState(AbstractViewConfiguration::Apply);
    mViewConfig->currentIdentifierFilter()[variableOrientation()] = applyHeaderFilter(variableOrientation(), mVarFilterModel);
    mViewConfig->currentIdentifierFilter()[equationOrientation()] = applyHeaderFilter(equationOrientation(), mEqnFilterModel);
    applyValueFilter();
    applyLabelFilter(variableOrientation(), mLabelFilterModel);
    applyLabelFilter(equationOrientation(), mLabelFilterModel);
    mViewConfig->currentLabelFiler().Any = ui->labelBox->currentIndex();
    mViewConfig->setCurrentAttributeFilter(applyAttributeFilter(mAttrFilterModel));
    applySymbolDimensions();
    emit viewConfigUpdated();
}

void FilterDialog::resetButtonClicked()
{
    ui->rowEdit->setText("");
    ui->columnEdit->setText("");
    ui->labelEdit->setText("");
    ui->labelBox->setCurrentIndex(0);
    mViewConfig->setFilterDialogState(AbstractViewConfiguration::Reset);
    IdentifierFilter identifierFilter = mViewConfig->defaultIdentifierFilter();
    for (const auto& entry : std::as_const(mViewConfig->currentIdentifierFilter()[Qt::Horizontal])) {
        identifierFilter[Qt::Horizontal][entry.SymbolIndex].CheckStates = entry.CheckStates;
    }
    for (const auto& entry : std::as_const(mViewConfig->currentIdentifierFilter()[Qt::Vertical])) {
        identifierFilter[Qt::Vertical][entry.SymbolIndex].CheckStates = entry.CheckStates;
    }
    mViewConfig->currentIdentifierFilter() = identifierFilter;
    mViewConfig->resetLabelFilter();
    auto filter = mViewConfig->defaultValueFilter();
    if (mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal) {
        filter.UseAbsoluteValues = mViewConfig->currentValueFilter().UseAbsoluteValues;
        filter.UseAbsoluteValuesGlobal = mViewConfig->currentValueFilter().UseAbsoluteValuesGlobal;
    }
    mViewConfig->setCurrentValueFilter(filter);
    resetSymbolDimensions();
    setViewConfig(mViewConfig);
    applyButtonClicked();
}

void FilterDialog::cancelButtonClicked()
{
    setViewConfig(mViewConfig);
    ui->rowEdit->setText("");
    ui->columnEdit->setText("");
    ui->labelEdit->setText("");
    ui->labelBox->setCurrentIndex(0);
    close();
}

void FilterDialog::selectEqnEntries()
{
    if (!mEqnFilterModel)
        return;
    applyCheckState(ui->rowView, mEqnFilterModel, Qt::Checked);
    ui->rowView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::deselectEqnEntries()
{
    if (!mEqnFilterModel)
        return;
    applyCheckState(ui->rowView, mEqnFilterModel, Qt::Unchecked);
    ui->rowView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::selectVarEntries()
{
    if (!mVarFilterModel)
        return;
    applyCheckState(ui->columnView, mVarFilterModel, Qt::Checked);
    ui->columnView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::deselectVarEntries()
{
    if (!mVarFilterModel)
        return;
    applyCheckState(ui->columnView, mVarFilterModel, Qt::Unchecked);
    ui->columnView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::selectLabelEntries()
{
    if (!mLabelFilterModel)
        return;
    applyCheckState(ui->labelView, mLabelFilterModel, Qt::Checked);
    ui->labelView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::deselectLabelEntries()
{
    if (!mLabelFilterModel)
        return;
    applyCheckState(ui->labelView, mLabelFilterModel, Qt::Unchecked);
    ui->labelView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::labelBoxCurrentIndexChanged(int index)
{
    switch (index) {
    case 1: // Any
        mViewConfig->currentLabelFiler().Any = true;
        break;
    default: // All
        mViewConfig->currentLabelFiler().Any = false;
        break;
    }
}

void FilterDialog::selectAttrEntries()
{
    applyCheckState(ui->attributeView, mAttrFilterModel, Qt::Checked);
    ui->attributeView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::deselectAttrEntries()
{
    applyCheckState(ui->attributeView, mAttrFilterModel, Qt::Unchecked);
    ui->attributeView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::selectDimEntries()
{
    applyCheckState(ui->dimView, mDimFilterModel, Qt::Checked);
    ui->dimView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::deselectDimEntries()
{
    applyCheckState(ui->dimView, mDimFilterModel, Qt::Unchecked);
    ui->dimView->dataChanged(QModelIndex(), QModelIndex());
}

void FilterDialog::setupConnections()
{
    connect(ui->applyButton, &QPushButton::clicked, this, &FilterDialog::applyButtonClicked);
    connect(ui->cancelButton, &QPushButton::clicked, this, &FilterDialog::cancelButtonClicked);
    connect(ui->resetButton, &QPushButton::clicked, this, &FilterDialog::resetButtonClicked);
    connect(this, &QDialog::rejected, this, &FilterDialog::cancelButtonClicked);
    connect(ui->selectEqnButton, &QPushButton::clicked, this, &FilterDialog::selectEqnEntries);
    connect(ui->deselectEqnButton, &QPushButton::clicked, this, &FilterDialog::deselectEqnEntries);
    connect(ui->selectVarButton, &QPushButton::clicked, this, &FilterDialog::selectVarEntries);
    connect(ui->deselectVarButton, &QPushButton::clicked, this, &FilterDialog::deselectVarEntries);
    connect(ui->selectLabelButton, &QPushButton::clicked, this, &FilterDialog::selectLabelEntries);
    connect(ui->deselectLabelButton, &QPushButton::clicked, this, &FilterDialog::deselectLabelEntries);
    connect(ui->labelBox, &QComboBox::currentIndexChanged, this, &FilterDialog::labelBoxCurrentIndexChanged);
    connect(ui->absoluteBox, &QCheckBox::stateChanged,
            this, [this](){
                updateRangeEdit(ui->minEdit, ui->minEdit->text());
                updateRangeEdit(ui->maxEdit, ui->maxEdit->text());
            });
    connect(ui->minEdit, &QLineEdit::textEdited,
            this, [this](const QString &text) {
                updateRangeEdit(ui->minEdit, text);
            });
    connect(ui->maxEdit, &QLineEdit::textEdited,
            this, [this](const QString &text) {
                updateRangeEdit(ui->maxEdit, text);
            });
    connect(ui->selectAttrButton, &QPushButton::clicked, this, &FilterDialog::selectAttrEntries);
    connect(ui->deselectAttrButton, &QPushButton::clicked, this, &FilterDialog::deselectAttrEntries);
    connect(ui->selectDimButton, &QPushButton::clicked, this, &FilterDialog::selectDimEntries);
    connect(ui->deselectDimButton, &QPushButton::clicked, this, &FilterDialog::deselectDimEntries);
}

void FilterDialog::setupEquationFilter(const IdentifierStates &filter, const IdentifierStates &dFilter)
{
    auto filterItem = setupSymTreeItems(ViewHelper::EquationHeaderText, filter, dFilter);
    auto oldEqnModel = ui->rowView->selectionModel();
    auto eqnTreeModel = new FilterTreeModel(filterItem, ui->rowView);
    mEqnFilterModel = new QSortFilterProxyModel(ui->rowView);
    mEqnFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mEqnFilterModel->setRecursiveFilteringEnabled(true);
    mEqnFilterModel->setSourceModel(eqnTreeModel);
    ui->rowView->setModel(mEqnFilterModel);
    ui->rowView->expandAll();
    if (oldEqnModel)
        oldEqnModel->deleteLater();
    connect(ui->rowEdit, &QLineEdit::textChanged,
            this, [this](const QString &text){
                            if (!mEqnFilterModel) return;
                            mEqnFilterModel->setFilterWildcard(text);
                            ui->rowView->expandAll();
                        });
}

void FilterDialog::setupVariableFilter(const IdentifierStates &filter, const IdentifierStates &dFilter)
{
    auto filterItem = setupSymTreeItems(ViewHelper::VariableHeaderText, filter, dFilter);
    auto oldVarModel = ui->columnView->selectionModel();
    auto varTreeModel = new FilterTreeModel(filterItem, ui->columnView);
    mVarFilterModel = new QSortFilterProxyModel(ui->columnEdit);
    mVarFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mVarFilterModel->setRecursiveFilteringEnabled(true);
    mVarFilterModel->setSourceModel(varTreeModel);
    ui->columnView->setModel(mVarFilterModel);
    ui->columnView->expandAll();
    if (oldVarModel)
        oldVarModel->deleteLater();
    connect(ui->columnEdit, &QLineEdit::textChanged,
            this, [this](const QString &text) {
                                                if (!mVarFilterModel) return;
                                                mVarFilterModel->setFilterWildcard(text);
                                                ui->columnView->expandAll();
    });
}

void FilterDialog::setupAttributeFilter()
{
    auto root = new FilterTreeItem("", Qt::Unchecked);
    auto attributeItem = new FilterTreeItem(ViewHelper::AttributeHeaderText, Qt::Unchecked, root);
    attributeItem->setCheckable(false);
    root->append(attributeItem);
    auto checkStates = mViewConfig->currentAttributeFilter();
    for (auto iter=checkStates.constKeyValueBegin(); iter!=checkStates.constKeyValueEnd(); ++iter) {
        auto attrItem = new FilterTreeItem(iter->first,
                                           iter->second ? Qt::Checked : Qt::Unchecked,
                                           attributeItem);
        attributeItem->append(attrItem);
    }
    auto oldAttrModel = ui->attributeView->selectionModel();
    auto attrTreeModel = new FilterTreeModel(root, ui->attributeView);
    mAttrFilterModel = new QSortFilterProxyModel(ui->attributeView);
    mAttrFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mAttrFilterModel->setRecursiveFilteringEnabled(true);
    mAttrFilterModel->setSourceModel(attrTreeModel);
    ui->attributeView->setModel(mAttrFilterModel);
    ui->attributeView->expandAll();
    if (oldAttrModel)
        oldAttrModel->deleteLater();
    connect(ui->attributeEdit, &QLineEdit::textChanged,
            this, [this](const QString &text){
                mAttrFilterModel->setFilterWildcard(text);
                ui->attributeView->expandAll();
            });
}

void FilterDialog::setupLabelFilter()
{
    ui->labelBox->setCurrentIndex(mViewConfig->currentLabelFiler().Any);
    auto labelFilterItem = new FilterTreeItem("", Qt::Unchecked);
    labelFilterItem->setCheckable(false);
    setupLabelTreeItems(ViewHelper::EquationHeaderText,
                        equationOrientation(),
                        labelFilterItem);
    setupLabelTreeItems(ViewHelper::VariableHeaderText,
                        variableOrientation(),
                        labelFilterItem);

    auto oldLabelModel = ui->labelView->selectionModel();
    auto labelTreeModel = new FilterTreeModel(labelFilterItem, ui->labelView);
    mLabelFilterModel = new QSortFilterProxyModel(ui->labelView);
    mLabelFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mLabelFilterModel->setRecursiveFilteringEnabled(true);
    mLabelFilterModel->setSourceModel(labelTreeModel);
    ui->labelView->setModel(mLabelFilterModel);
    ui->labelView->expandAll();
    if (oldLabelModel)
        oldLabelModel->deleteLater();
    connect(ui->labelEdit, &QLineEdit::textChanged,
            this, [this](const QString &text){
                                                mLabelFilterModel->setFilterWildcard(text);
                                                ui->labelView->expandAll();
                                              });
}

FilterTreeItem* FilterDialog::setupSymTreeItems(const QString &text,
                                                const IdentifierStates &filter,
                                                const IdentifierStates &dFilter)
{
    auto root = new FilterTreeItem(QString(), Qt::Unchecked);
    root->setCheckable(false);
    auto symbols = new FilterTreeItem(text, Qt::Unchecked, root);
    symbols->setCheckable(false);
    for (const auto& item : filter) {
        auto fItem = new FilterTreeItem(item.Text, item.Checked, symbols);
        auto defaultItem = dFilter.value(item.SymbolIndex);
        if (mViewConfig->viewType() == ViewHelper::ViewDataType::Symbols    &&
                item.Checked == Qt::Unchecked                               &&
                defaultItem.Checked == Qt::Unchecked) {
            fItem->setEnabled(false);
        }
        fItem->setSymbolIndex(item.SymbolIndex);
        symbols->append(fItem);
    }
    root->append(symbols);
    return root;
}

void FilterDialog::setupLabelTreeItems(const QString &text,
                                       Qt::Orientation orientation,
                                       FilterTreeItem *root)
{
    auto typeItem = new FilterTreeItem(text, Qt::Unchecked, root);
    typeItem->setCheckable(false);
    root->append(typeItem);

    auto checkStates = mViewConfig->currentLabelFiler().LabelCheckStates[orientation];
    for (auto iter=checkStates.constKeyValueBegin();
         iter!=checkStates.constKeyValueEnd(); ++iter) {
        auto treeItem = new FilterTreeItem(iter->first,
                                           iter->second ? Qt::Checked : Qt::Unchecked,
                                           typeItem);
        typeItem->append(treeItem);
    }
}

void FilterDialog::setupDimensionFilter()
{
    auto root = new FilterTreeItem(QString(), Qt::Unchecked);
    auto eqns = new FilterTreeItem(ViewHelper::EquationHeaderText, Qt::Unchecked, root);
    eqns->setCheckable(false);
    root->append(eqns);
    setupSymbolDimensions(mViewConfig->equationLabels(), eqns);
    auto vars = new FilterTreeItem(ViewHelper::VariableHeaderText, Qt::Unchecked, root);
    vars->setCheckable(false);
    root->append(vars);
    setupSymbolDimensions(mViewConfig->variableLabels(), vars);

    auto oldDimModel = ui->dimView->selectionModel();
    auto dimTreeModel = new FilterTreeModel(root, ui->dimView);
    mDimFilterModel = new QSortFilterProxyModel(ui->dimView);
    mDimFilterModel->setFilterCaseSensitivity(Qt::CaseInsensitive);
    mDimFilterModel->setRecursiveFilteringEnabled(true);
    mDimFilterModel->setSourceModel(dimTreeModel);
    ui->dimView->setModel(mDimFilterModel);
    ui->dimView->expandAll();
    if (oldDimModel)
        oldDimModel->deleteLater();
    connect(ui->dimEdit, &QLineEdit::textChanged,
            this, [this](const QString &text){
                mDimFilterModel->setFilterWildcard(text);
                ui->dimView->expandAll();
            });
}

void FilterDialog::setupSymbolDimensions(const QVector<LabelCheckStates>& labels, FilterTreeItem *root)
{
    for (int d=0; d<labels.size(); ++d) {
        auto dimItem = new FilterTreeItem(QString("Dimension %1").arg(d+1), Qt::Checked, root);
        root->append(dimItem);
        for (auto iter=labels[d].constKeyValueBegin(); iter!=labels[d].constKeyValueEnd(); ++iter) {
            auto labelItem = new FilterTreeItem(iter->first, iter->second, dimItem);
            dimItem->append(labelItem);
        }
    }
}

void FilterDialog::applyCheckState(QTreeView *view,
                                   QSortFilterProxyModel *model,
                                   Qt::CheckState state)
{
    QModelIndexList indexes;
    for(int row=0; row<model->rowCount(); ++row) {
        auto index = model->index(row, 0);
        if (view->isExpanded(index) && !view->isRowHidden(row, QModelIndex()))
            indexes.append(index);
    }
    while (!indexes.isEmpty()) {
        auto index = indexes.takeFirst();
        if (model->flags(index) == Qt::NoItemFlags)
            continue;
        model->setData(index, state, Qt::CheckStateRole);
        if (model->hasChildren(index)) {
            for(int row=0; row<model->rowCount(index); ++row)
                indexes.append(model->index(row, 0, index));
        }
    }
}

IdentifierStates FilterDialog::applyHeaderFilter(Qt::Orientation orientation, QSortFilterProxyModel *model)
{
    QList<FilterTreeItem*> items {
        static_cast<FilterTreeModel*>(model->sourceModel())->filterItem()
    };
    IdentifierStates filter;
    while (!items.isEmpty()) {
        auto item = items.takeFirst();
        items.append(item->childs());
        if (!item->isCheckable())
            continue;
        IdentifierState identifierState;
        identifierState.Enabled = item->isEnabled();
        identifierState.SymbolIndex = item->symbolIndex();
        identifierState.Text =  item->text();
        identifierState.Checked = item->checked();
        identifierState.CheckStates = mViewConfig->currentIdentifierFilter()[orientation][item->symbolIndex()].CheckStates;
        filter[item->symbolIndex()] = identifierState;
    }
    return filter;
}

void FilterDialog::applyValueFilter()
{
    mViewConfig->currentValueFilter().MinValue = ui->minEdit->text().toDouble();
    mViewConfig->currentValueFilter().MaxValue = ui->maxEdit->text().toDouble();
    mViewConfig->currentValueFilter().ExcludeRange = ui->excludeBox->isChecked();
    mViewConfig->currentValueFilter().UseAbsoluteValues = ui->absoluteBox->isChecked();
    mViewConfig->currentValueFilter().ShowPInf = ui->pInfBox->isChecked();
    mViewConfig->currentValueFilter().ShowNInf = ui->nInfBox->isChecked();
    mViewConfig->currentValueFilter().ShowEps = ui->epsBox->isChecked();
}

void FilterDialog::applyLabelFilter(Qt::Orientation orientation, QSortFilterProxyModel *model)
{
    FilterTreeItem* root = nullptr;
    auto childs = static_cast<FilterTreeModel*>(model->sourceModel())->filterItem()->childs();
    for (auto child : childs) {
        if (orientation == variableOrientation() && child->text() == ViewHelper::VariableHeaderText) {
            root = child;
            break;
        } else if (orientation == equationOrientation() && child->text() == ViewHelper::EquationHeaderText) {
            root = child;
            break;
        }
    }

    QStringList unchecked;
    LabelCheckStates filter;
    if (root) {
        QList<FilterTreeItem*> items { root };
        while (!items.isEmpty()) {
            auto item = items.takeFirst();
            items.append(item->childs());
            if (!item->isCheckable())
                continue;
            filter[item->text()] = item->checked();
            if (item->checked() == Qt::Unchecked)
                unchecked << item->text();
        }
    }
    mViewConfig->currentLabelFiler().LabelCheckStates[orientation] = std::move(filter);
    mViewConfig->currentLabelFiler().UncheckedLabels[orientation] = std::move(unchecked);
}

void FilterDialog::applySymbolDimensions()
{
    auto types = static_cast<FilterTreeModel*>(mDimFilterModel->sourceModel())->filterItem()->childs();
    for (auto type : types) {
        if (type->text() == ViewHelper::EquationHeaderText) {
            for (auto dim : type->childs()) {
                int d = dim->text().remove("Dimension ").toInt() - 1;
                for (auto label : dim->childs()) {
                    mViewConfig->equationLabels()[d][label->text()] = label->checked();
                }
            }
        } else if (type->text() == ViewHelper::VariableHeaderText) {
            for (auto dim : type->childs()) {
                int d = dim->text().remove("Dimension ").toInt() - 1;
                for (auto label : dim->childs()) {
                    mViewConfig->variableLabels()[d][label->text()] = label->checked();
                }
            }
        }
    }
}

void FilterDialog::resetSymbolDimensions()
{
    auto& eqnLabels = mViewConfig->equationLabels();
    for (auto& hash : eqnLabels) {
        for (auto iter=hash.keyValueBegin(); iter!=hash.keyValueEnd(); ++iter) {
            iter->second = Qt::Checked;
        }
    }
    auto& varLabels = mViewConfig->variableLabels();
    for (auto& hash : varLabels) {
        for (auto iter=hash.keyValueBegin(); iter!=hash.keyValueEnd(); ++iter) {
            iter->second = Qt::Checked;
        }
    }
}

LabelCheckStates FilterDialog::applyAttributeFilter(QSortFilterProxyModel *model)
{
    auto root = static_cast<FilterTreeModel*>(model->sourceModel())->filterItem()->child(0);
    LabelCheckStates filter;
    for (auto item : root->childs()) {
        filter[item->text()] = item->checked();
    }
    return filter;
}

void FilterDialog::updateRangeEdit(QLineEdit *edit, const QString &text)
{
    QPalette palette;
    if (ui->absoluteBox->isChecked() && text.startsWith("-")) {
        palette.setColor(QPalette::Text, Qt::red);
    } else {
        palette.setColor(QPalette::Text, QApplication::palette().text().color());
    }
    edit->setPalette(palette);
}

}
}
}
