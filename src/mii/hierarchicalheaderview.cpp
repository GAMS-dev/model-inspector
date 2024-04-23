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
#include "hierarchicalheaderview.h"
#include "filtertreeitem.h"
#include "labelfilterwidget.h"
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"

#include <QPainter>
#include <QPointer>
#include <QMenu>
#include <QMouseEvent>
#include <QWidgetAction>

#include <QDebug>

namespace gams {
namespace studio{
namespace mii {

typedef QMap<Qt::Orientation, QMap<int, QVector<int>>> IdentifierLabelSections;

class HierarchicalHeaderView::HierarchicalHeaderView_private
{
public:
    HierarchicalHeaderView_private(const HierarchicalHeaderView *headerView)
        : mHeaderView(headerView)
    {

    }

    void init()
    {
        QFontMetrics fm(mHeaderView->font());
        auto flags = Qt::TextSingleLine | Qt::TextDontClip;
        mMinCellSize = symbolTextSize();
        mEmptyTextSize = fm.size(flags, "");
        mFilterIconSize = QSize(mEmptyTextSize.height(), mEmptyTextSize.height());
        mIconWidth = QSize(mFilterIconSize.width(), 0.0);

        QIcon iconOn(FilterIconOn);
        mPixmapFilterOn = iconOn.pixmap(mFilterIconSize);
        QIcon iconOff(FilterIconOff);
        mPixmapFilterOff = iconOff.pixmap(mFilterIconSize);
    }

    bool isDimensionFilter(const QPoint &position)
    {
        int logicalIndex = mHeaderView->logicalIndexAt(position);
        if (logicalIndex < 0)
            return false;
        QPoint pos;
        if (mHeaderView->orientation() == Qt::Horizontal) {
            pos = QPoint(mHeaderView->sectionViewportPosition(logicalIndex), 0);
        } else {
            pos = QPoint(0, mHeaderView->sectionViewportPosition(logicalIndex));
        }
        return QRect(pos, mFilterIconSize).contains(position);
    }

    int sectionIndex(int logicalIndex) const
    {
        bool ok = false;
        auto index = mHeaderView->model()->headerData(logicalIndex,
                                                      mHeaderView->orientation()).toInt(&ok);
        return !ok ? -1 : index;
    }

    int horizontalSectionDimension(int sectionIndex)
    {
        return symbol(sectionIndex)->dimension();
    }

    int verticalSectionDimension(int sectionIndex)
    {
        if (!ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType()))
            return 1;
        return symbol(sectionIndex)->dimension();
    }

    void paintHorizontalSection(QPainter *painter,
                                const QRect &rect,
                                const QStyleOptionHeader &option,
                                int logicalIndex,
                                int sectionIndex)
    {
        int currentTop = rect.y();
        QPointF oldBrushOrigin(painter->brushOrigin());

        QStyleOptionHeader styleOption(option);
        if (styleOption.state.testFlag(QStyle::State_Sunken) ||
                styleOption.state.testFlag(QStyle::State_On)) {
            QStyle::State state(QStyle::State_Sunken | QStyle::State_On);
            styleOption.state &= (~state);
        }

        paintHorizontalCell(painter, rect, styleOption, currentTop,
                            label(sectionIndex, -1, mHeaderView->orientation()),
                            logicalIndex, sectionIndex, -1, true);
        if (ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType())) {
            for (int d=0; d<horizontalSectionDimension(sectionIndex); ++d) {
                paintHorizontalCell(painter, rect, styleOption, currentTop,
                                    label(sectionIndex, d, mHeaderView->orientation()),
                                    logicalIndex, sectionIndex, d, false);
            }
            paintHorizontalCellSpacing(painter, rect, styleOption, currentTop, sectionIndex);
        }

        painter->setBrushOrigin(oldBrushOrigin);
    }

    void paintHorizontalCell(QPainter *painter,
                             const QRect &rect,
                             const QStyleOptionHeader &option,
                             int &currentTop,
                             const QString &text,
                             int logicalIndex,
                             int sectionIndex,
                             int dimension,
                             bool isSymbol)
    {
        QStyleOptionHeader styleOption(option);
        QSize size = cellSize(styleOption);
        styleOption.rect = QRect(rect.x(), currentTop, rect.width(), size.height());
        auto currentSym = symbol(sectionIndex);
        styleOption.text = horizontalCellText(logicalIndex, sectionIndex, dimension, isSymbol, currentSym, text);

        painter->save();
        if (isSymbol && ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType())) {
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
            styleOption.rect = QRect(rect.x()+mFilterIconSize.width(), currentTop, rect.width(), size.height());
            mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
            if (!styleOption.text.isEmpty() && symbol(sectionIndex)->isScalar()) {
                painter->drawPixmap(rect.x(), rect.y(), mFilterIconSize.width(),
                                    mFilterIconSize.height(), mPixmapFilterOff);
            } else if (!styleOption.text.isEmpty()) {
                painter->drawPixmap(rect.x(), rect.y(), mFilterIconSize.width(),
                                    mFilterIconSize.height(),
                                    ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType()) ? mPixmapFilterOn : mPixmapFilterOff);
            }
        } else {
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
            mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        }
        painter->restore();

        currentTop += size.height();
    }

    void paintHorizontalCellSpacing(QPainter *painter,
                                    const QRect &rect,
                                    const QStyleOptionHeader &option,
                                    int &currentTop,
                                    int sectionIndex)
    {
        QStyleOptionHeader styleOption(option);
        painter->save();
        if (horizontalSectionDimension(sectionIndex) < symbolDimension()) {
            styleOption.rect = QRect(rect.x(), currentTop, rect.width(), rect.height());
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        }
        painter->restore();
    }

    void paintVerticalSection(QPainter *painter,
                              const QRect &rect,
                              const QStyleOptionHeader &option,
                              int logicalIndex,
                              int sectionIndex)
    {
        int currentLeft = rect.x();
        QPointF oldBrushOrigin(painter->brushOrigin());

        QStyleOptionHeader styleOption(option);
        if (styleOption.state.testFlag(QStyle::State_Sunken) ||
                styleOption.state.testFlag(QStyle::State_On)) {
            QStyle::State state(QStyle::State_Sunken | QStyle::State_On);
            styleOption.state &= (~state);
        }

        paintVerticalCell(painter, rect, styleOption, currentLeft,
                          label(sectionIndex, -1, mHeaderView->orientation()),
                          logicalIndex, sectionIndex, -1, true);
        for (int d=0; d<verticalSectionDimension(sectionIndex); ++d) {
            paintVerticalCell(painter, rect, styleOption, currentLeft,
                              label(sectionIndex, d, mHeaderView->orientation()),
                              logicalIndex, sectionIndex, d, false);
        }
        paintVerticalCellSpacing(painter, rect, styleOption, currentLeft, sectionIndex);

        painter->setBrushOrigin(oldBrushOrigin);
    }

    void paintVerticalCell(QPainter *painter,
                           const QRect &rect,
                           const QStyleOptionHeader &option,
                           int &currentLeft,
                           const QString &text,
                           int logicalIndex,
                           int sectionIndex,
                           int dimension,
                           bool isSymbol)
    {
        QStyleOptionHeader styleOption(option);
        QSize size = cellSize(styleOption);
        styleOption.rect = QRect(currentLeft, rect.y(), size.width(), rect.height());
        auto currentSym = symbol(sectionIndex);
        styleOption.text = verticalCellText(logicalIndex, sectionIndex, dimension, isSymbol, currentSym, text);

        painter->save();
        if (isSymbol && ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType())) {
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
            styleOption.rect = QRect(currentLeft+mFilterIconSize.width(), rect.y(), size.width(), rect.height());
            mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
            if (!styleOption.text.isEmpty() && symbol(sectionIndex)->isScalar()) {
                painter->drawPixmap(currentLeft, rect.y(), mFilterIconSize.width(),
                                    mFilterIconSize.height(), mPixmapFilterOff);
            } else if (!styleOption.text.isEmpty()) {
                painter->drawPixmap(rect.x(), rect.y(), mFilterIconSize.width(),
                                    mFilterIconSize.height(),
                                    ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType()) ? mPixmapFilterOn : mPixmapFilterOff);
            }
        } else {
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
            mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        }
        painter->restore();

        currentLeft += size.width();
    }

    void paintVerticalCellSpacing(QPainter *painter,
                                  const QRect &rect,
                                  const QStyleOptionHeader &option,
                                  int &currentLeft,
                                  int sectionIndex)
    {
        QStyleOptionHeader styleOption(option);
        painter->save();
        if (verticalSectionDimension(sectionIndex) < symbolDimension()) {
            styleOption.rect = QRect(currentLeft, rect.y(), rect.width(), rect.height());
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        }
        painter->restore();
    }

    QString horizontalCellText(int logicalIndex, int sectionIndex, int dimension,
                               bool isSymbol, const Symbol *symbol,
                               const QString &text)
    {// TODO header drawing rules... simplify and no on the fly stuff
        if (isSymbol && logicalIndex > 0) {
            int prevSectionIdx = this->sectionIndex(logicalIndex-1);
            if (symbol && symbol->contains(prevSectionIdx))
                return QString();
        } else if (logicalIndex > 0) {
            int prevSectionIdx = this->sectionIndex(logicalIndex-1);
            if (symbol && symbol->contains(prevSectionIdx) &&
                    symbol->label(sectionIndex, dimension) == symbol->label(prevSectionIdx, dimension) &&
                        (dimension == 0 ||
                            symbol->label(sectionIndex, dimension-1) == symbol->label(prevSectionIdx, dimension-1))) {
                    return QString();
            }
        }
        return text;
    }

    QString verticalCellText(int logicalIndex, int sectionIndex, int dimension,
                             bool isSymbol, const Symbol *symbol,
                             const QString &text)
    {// TODO header drawing rules... simplify and no on the fly stuff
        if (!ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType())) {
            return mHeaderView->modelInstance()->plainHeaderData(mHeaderView->orientation(),
                                                                 mHeaderView->viewConfig()->viewId(),
                                                                 sectionIndex,
                                                                 dimension < 0 ? 0 : dimension+1).toString();
        }
        if (isSymbol && logicalIndex > 0) {
            int prevSectionIdx = this->sectionIndex(logicalIndex-1);
            if (symbol && symbol->contains(prevSectionIdx))
                return QString();
        } else if (logicalIndex > 0) {
            int prevSectionIdx = this->sectionIndex(logicalIndex-1);
            if (symbol && symbol->contains(prevSectionIdx) &&
                    symbol->label(sectionIndex, dimension) == symbol->label(prevSectionIdx, dimension) &&
                        (dimension == 0 ||
                            symbol->label(sectionIndex, dimension-1) == symbol->label(prevSectionIdx, dimension-1))) {
                    return QString();
            }
        }
        return text;
    }

    QSize cellSize(QStyleOptionHeader styleOption) const
    {
        QSize decorationsSize(mHeaderView->style()->sizeFromContents(QStyle::CT_HeaderSection,
                                                                     &styleOption,
                                                                     QSize(),
                                                                     mHeaderView));
        return mMinCellSize + decorationsSize + mIconWidth - mEmptyTextSize;
    }

    FilterTreeItem* filterTree(int logicalIndex, int sectionIdx, Symbol *symbol)
    {
        auto root = new FilterTreeItem;
        root->setCheckable(false);
        root->setSymbolIndex(symbol->firstSection());
        root->setSectionIndex(symbol->firstSection());
        root->setText(symbol->name());

        QVector<int> visibleSections = visibleLabelSections(logicalIndex, sectionIdx, symbol);

        QString label;
        QMap<int, QStringList> subData;
        auto filter = mHeaderView->viewConfig()->currentIdentifierFilter()[mHeaderView->orientation()][symbol->firstSection()];
        for (int section=symbol->firstSection(); section<=symbol->lastSection(); ++section) {
            if (symbol->sectionLabels().isEmpty()) {
                continue;
            }
            auto subLabels = symbol->sectionLabels()[section];
            if (symbol->sectionLabels()[section].isEmpty()) {
                continue;
            }
            if (label.isEmpty()) {
                label = subLabels.takeFirst();
                subData[section] = subLabels;
            } else if (label == symbol->sectionLabels()[section].first()) {
                subLabels.removeFirst();
                subData[section] = subLabels;
            } else {
                labelFilterTree(label, subData, filter, visibleSections, symbol->firstSection(), root);
                label = subLabels.takeFirst();
                subData.clear();
                subData[section] = subLabels;
            }
        }
        labelFilterTree(label, subData, filter, visibleSections, symbol->firstSection(), root);
        return root;
    }

    int maxSymbolDimension(Qt::Orientation orientation)
    {
        if (!ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType())) {
            // show 1 dim for eqns and non for vars
            return orientation == Qt::Vertical ? 1 : 0;
        }
        if (mHeaderView->viewConfig()->viewType() == ViewHelper::ViewDataType::Symbols) {
            return mHeaderView->model()->headerData(0, orientation, ViewHelper::DimensionRole).toInt();
        }
        if (orientation == Qt::Horizontal) {
            if (mHeaderView->model()->columnCount() != mHeaderView->modelInstance()->variableRowCount())
                return maximumSymbolDimension(); // TODO filter case
        }
        if (orientation == Qt::Vertical) {
            if (mHeaderView->model()->rowCount() != mHeaderView->modelInstance()->equationRowCount())
                return maximumSymbolDimension(); // TODO filter case
        }
        return symbolDimension();
    }

    Symbol* symbol(int sectionIndex) const
    {
        if (mHeaderView->orientation() == Qt::Vertical)
            return mHeaderView->modelInstance()->equation(sectionIndex);
        return mHeaderView->modelInstance()->variable(sectionIndex);
    }

    int maximumSymbolDimension() const
    {
        if (mHeaderView->orientation() == Qt::Vertical)
            return mHeaderView->modelInstance()->maximumEquationDimension();
        return mHeaderView->modelInstance()->maximumVariableDimension();
    }

    QString label(int sectionIndex, int dimension, Qt::Orientation orientation)
    {
        if (orientation == Qt::Horizontal) {
            auto var = mHeaderView->modelInstance()->variable(sectionIndex);
            if (!var)
                return QString();
            if (dimension < 0)
                return var->name();
            return var->label(sectionIndex, dimension);
        }
        auto eqn = mHeaderView->modelInstance()->equation(sectionIndex);
        if (!eqn)
            return QString();
        if (dimension < 0)
            return eqn->name();
        return eqn->label(sectionIndex, dimension);
    }

private:
    int symbolDimension() const
    {
        if (!ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType()) && mHeaderView->orientation() == Qt::Vertical) {
            return 1; // non standard views have always diminsion 1
        }
        return mHeaderView->orientation() == Qt::Horizontal ? mHeaderView->modelInstance()->maximumVariableDimension() :
                                                              mHeaderView->modelInstance()->maximumEquationDimension();
    }

    QSize symbolTextSize() const
    {
        QFontMetrics fm(mHeaderView->font());
        auto flags = Qt::TextSingleLine | Qt::TextDontClip;
        if (ViewHelper::isStandardView(mHeaderView->viewConfig()->viewType())) {
            if (mHeaderView->orientation() == Qt::Horizontal)
                return fm.size(flags, mHeaderView->mModelInstance->longestVariableText());
            else
                return fm.size(flags, mHeaderView->mModelInstance->longestEquationText());
        }
        return fm.size(flags, "aaaaaaaa"); // TODO !!! fix header space issue
    }

    QVector<int> visibleLabelSections(int logicalIndex, int sectionIdx, const Symbol *symbol)
    {
        if (mVisibleLabelSections[mHeaderView->orientation()].contains(symbol->firstSection())) {
            return mVisibleLabelSections[mHeaderView->orientation()][symbol->firstSection()];
        }
        QVector<int> visibleSections;
        int logicalEnd = logicalIndex + symbol->lastSection() - sectionIdx;
        int logicalStart = logicalEnd - symbol->entries() + 1;
        for (int i=logicalStart; i<=logicalEnd; ++i) {
            int sIndex = sectionIndex(i);
            if (symbol->contains(sIndex))
                visibleSections.push_back(sIndex);
        }
        mVisibleLabelSections[mHeaderView->orientation()][symbol->firstSection()] = visibleSections;
        return mVisibleLabelSections[mHeaderView->orientation()][symbol->firstSection()];
    }

    void labelFilterTree(const QString &label,
                         const QMap<int, QStringList> &data,
                         const IdentifierState &state,
                         const QVector<int> &visibleSections,
                         int symbolIndex,
                         FilterTreeItem *parent)
    {
        if (data.isEmpty() || label.isEmpty()) return;
        FilterTreeItem *labelItem = nullptr;
        bool enabled = parent->parent() ? visibleSections.contains(data.firstKey()) : true;
        if (state.isValid() && state.CheckStates.contains(data.firstKey())) {
            Qt::CheckState checkState;
            if (enabled) {
                checkState = state.CheckStates.contains(data.firstKey()) ? Qt::Unchecked : Qt::Checked;
            } else {
                checkState = Qt::Unchecked;
            }
            labelItem = new FilterTreeItem(label, checkState, parent);
            labelItem->setSectionIndex(data.firstKey());
            labelItem->setEnabled(enabled);
        } else {
            labelItem = new FilterTreeItem(label, enabled ? Qt::Checked : Qt::Unchecked, parent);
            labelItem->setSectionIndex(data.firstKey());
            labelItem->setEnabled(enabled);
        }
        labelItem->setSymbolIndex(symbolIndex);
        parent->append(labelItem);

        QString subLabel;
        QMap<int, QStringList> subData;
        for (auto iter=data.keyValueBegin(); iter!=data.keyValueEnd(); ++iter) {
            auto subLabels = iter->second;
            if (subLabels.isEmpty()) {
                continue;
            }
            if (subLabel.isEmpty()) {
                subLabel = subLabels.takeFirst();
                subData[iter->first] = subLabels;
            } else if (subLabel == subLabels.first()) {
                subLabels.removeFirst();
                subData[iter->first] = subLabels;
            } else {
                labelFilterTree(subLabel, subData, state, visibleSections, symbolIndex, labelItem);
                subLabel = subLabels.takeFirst();
                subData.clear();
                subData[iter->first] = subLabels;
            }
        }
        labelFilterTree(subLabel, subData, state, visibleSections, symbolIndex, labelItem);
    }

private:
    const QString FilterIconOn = ":/img/filter";
    const QString FilterIconOff = ":/img/filter-off";

    const HierarchicalHeaderView *mHeaderView;

    QSize mFilterIconSize;
    QSize mMinCellSize;
    QSize mEmptyTextSize;
    QSize mIconWidth;

    QPixmap mPixmapFilterOn;
    QPixmap mPixmapFilterOff;

    IdentifierLabelSections mVisibleLabelSections;
};

HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation,
                                               const QSharedPointer<AbstractModelInstance> &modelInstance,
                                               const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                               QWidget *parent)
    : QHeaderView(orientation, parent)
    , mPrivate(new HierarchicalHeaderView_private(this))
    , mModelInstance(modelInstance)
    , mViewConfig(viewConfig)
    , mFilterMenu(new QMenu(this))
    , mFilterWidget(new LabelFilterWidget(orientation, this))
{
    setHighlightSections(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignTop);
    setResizeContentsPrecision(50);

    auto filterAction = new QWidgetAction(mFilterMenu);
    filterAction->setDefaultWidget(mFilterWidget);
    mFilterMenu->addAction(filterAction);

    connect(this, &HierarchicalHeaderView::customContextMenuRequested,
            this, &HierarchicalHeaderView::customMenuRequested);
    connect(mFilterWidget, &LabelFilterWidget::filterChanged,
            this, &HierarchicalHeaderView::on_filterChanged);
}

HierarchicalHeaderView::~HierarchicalHeaderView()
{
    delete mPrivate;
}

QSharedPointer<AbstractModelInstance> HierarchicalHeaderView::modelInstance() const
{
    return mModelInstance;
}

void HierarchicalHeaderView::setModel(QAbstractItemModel *model)
{
    mPrivate->init();
    QHeaderView::setModel(model);
}

const QSharedPointer<AbstractViewConfiguration> &HierarchicalHeaderView::viewConfig() const
{
    return mViewConfig;
}

void HierarchicalHeaderView::customMenuRequested(const QPoint &position)
{
    if (mViewConfig->viewType() != ViewHelper::ViewDataType::Symbols)
        return;
    bool ok = false;
    int logicalIndex = logicalIndexAt(position);
    int sectionIndex = model()->headerData(logicalIndex, orientation()).toInt(&ok);
    if (!ok)
        return;
    Symbol* symbol = mPrivate->symbol(sectionIndex);
    if (symbol->isScalar())
        return;
    auto filterTree = mPrivate->filterTree(logicalIndex, sectionIndex, symbol);
    mFilterWidget->setData(filterTree);
    mFilterMenu->popup(viewport()->mapToGlobal(position));
}

void HierarchicalHeaderView::on_filterChanged(const IdentifierState& state,
                                              Qt::Orientation orientation)
{
    mViewConfig->currentIdentifierFilter()[orientation][state.SymbolIndex] = state;
    emit filterChanged();
}

void HierarchicalHeaderView::paintSection(QPainter *painter,
                                          const QRect &rect,
                                          int logicalIndex) const
{
    if (rect.isValid()) {
        auto option = styleOptionForCell(logicalIndex);
        int sectionIdx = mPrivate->sectionIndex(logicalIndex);
        if (sectionIdx < 0 ) return;
        if (orientation() == Qt::Horizontal) {
            mPrivate->paintHorizontalSection(painter, rect, option, logicalIndex, sectionIdx);
        } else {
            mPrivate->paintVerticalSection(painter, rect, option, logicalIndex, sectionIdx);
        }
        return;
    }
    QHeaderView::paintSection(painter, rect, logicalIndex);
}

QSize HierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
    QSize size(mPrivate->cellSize(styleOption));

    int dimension = mPrivate->maxSymbolDimension(orientation());
    for (int d=0; d<dimension; ++d) {
        if (orientation() == Qt::Horizontal) {
            size.rheight() += mPrivate->cellSize(styleOption).height();
        } else {
            size.rwidth() += mPrivate->cellSize(styleOption).width();
        }
    }
    return size;
}

void HierarchicalHeaderView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && mPrivate->isDimensionFilter(event->pos())) {
        emit customContextMenuRequested(event->pos());
        event->accept();
        return;
    }
    QHeaderView::mousePressEvent(event);
}

QStyleOptionHeader HierarchicalHeaderView::styleOptionForCell(int logicalIndex) const
{
    QStyleOptionHeader option;
    initStyleOption(&option);

    if (window()->isActiveWindow())
        option.state |= QStyle::State_Active;

    option.textAlignment = Qt::AlignLeft | Qt::AlignTop;
    option.iconAlignment = Qt::AlignVCenter;
    option.section = logicalIndex;

    int visual = visualIndex(logicalIndex);
    if (count() == 1) {
        option.position = QStyleOptionHeader::OnlyOneSection;
    } else {
        if (visual == 0)
            option.position = QStyleOptionHeader::Beginning;
        else
            option.position = (visual == count()-1 ? QStyleOptionHeader::End :
                                                     QStyleOptionHeader::Middle);
    }

    if(sectionsClickable()) {
        if(highlightSections() && selectionModel()) {
            if(orientation() == Qt::Horizontal) {
                if(selectionModel()->columnIntersectsSelection(logicalIndex, rootIndex()))
                    option.state |= QStyle::State_On;

                if(selectionModel()->isColumnSelected(logicalIndex, rootIndex()))
                    option.state |= QStyle::State_Sunken;
            } else {
                if(selectionModel()->rowIntersectsSelection(logicalIndex, rootIndex()))
                    option.state |= QStyle::State_On;

                if(selectionModel()->isRowSelected(logicalIndex, rootIndex()))
                    option.state |= QStyle::State_Sunken;
            }
        }
    }

    if(selectionModel()) {
        bool previousSelected = false;

        if(orientation() == Qt::Horizontal)
            previousSelected = selectionModel()->isColumnSelected(this->logicalIndex(visual - 1),
                                                                  rootIndex());
        else
            previousSelected = selectionModel()->isRowSelected(this->logicalIndex(visual - 1),
                                                               rootIndex());

        bool nextSelected = false;

        if(orientation() == Qt::Horizontal)
            nextSelected = selectionModel()->isColumnSelected(this->logicalIndex(visual + 1),
                                                              rootIndex());
        else
            nextSelected = selectionModel()->isRowSelected(this->logicalIndex(visual + 1),
                                                           rootIndex());

        if (previousSelected && nextSelected) {
            option.selectedPosition = QStyleOptionHeader::NextAndPreviousAreSelected;
        } else if (previousSelected) {
            option.selectedPosition = QStyleOptionHeader::PreviousIsSelected;
        } else if (nextSelected) {
            option.selectedPosition = QStyleOptionHeader::NextIsSelected;
        } else {
            option.selectedPosition = QStyleOptionHeader::NotAdjacent;
        }
    }

    return option;
}

}
}
}
