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
#include "symbolhierarchicalheaderview.h"
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

class SymbolHierarchicalHeaderView::HierarchicalHeaderView_private
{
public:
    HierarchicalHeaderView_private(const SymbolHierarchicalHeaderView *headerView)
        : mHeaderView(headerView)
        , mTextFlags(Qt::TextSingleLine | Qt::TextDontClip | Qt::TextIncludeTrailingSpaces)
    {
        mSymbolCellSize = symbolTextSize();
        mLabelCellSize = labelTextSize();
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

        paintHorizontalCell(painter, rect, styleOption, currentTop, label(sectionIndex, -1),
                            logicalIndex, sectionIndex, -1, true);
        for (int d=0; d<horizontalSectionDimension(sectionIndex); ++d) {
            paintHorizontalCell(painter, rect, styleOption, currentTop, label(sectionIndex, d),
                                logicalIndex, sectionIndex, d, false);
        }
        paintHorizontalCellSpacing(painter, rect, styleOption, currentTop, sectionIndex);

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
        painter->save();
        QSize size = labelCellSize(styleOption);
        styleOption.rect = QRect(rect.x(), currentTop, rect.width(), size.height());
        auto currentSym = symbol(sectionIndex);
        styleOption.text = horizontalCellText(logicalIndex, sectionIndex, dimension, isSymbol, currentSym, text);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        currentTop += size.height();
        painter->restore();
    }

    void paintHorizontalCellSpacing(QPainter *painter,
                                    const QRect &rect,
                                    const QStyleOptionHeader &option,
                                    int &currentTop,
                                    int sectionIndex)
    {
        QStyleOptionHeader styleOption(option);
        painter->save();
        if (horizontalSectionDimension(sectionIndex) < maximumSymbolDimension()) {
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

        paintVerticalCell(painter, rect, styleOption, currentLeft, label(sectionIndex, -1),
                          logicalIndex, sectionIndex, -1, true);
        for (int d=0; d<verticalSectionDimension(sectionIndex); ++d) {
            paintVerticalCell(painter, rect, styleOption, currentLeft, label(sectionIndex, d),
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
        painter->save();
        QSize size = isSymbol ? mSymbolCellSize : mLabelCellSize;
        styleOption.rect = QRect(currentLeft, rect.y(), size.width(), rect.height());
        auto currentSym = symbol(sectionIndex);
        styleOption.text = verticalCellText(logicalIndex, sectionIndex, dimension, isSymbol, currentSym, text);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        currentLeft += size.width();
        painter->restore();
    }

    void paintVerticalCellSpacing(QPainter *painter,
                                  const QRect &rect,
                                  const QStyleOptionHeader &option,
                                  int &currentLeft,
                                  int sectionIndex)
    {
        QStyleOptionHeader styleOption(option);
        painter->save();
        if (verticalSectionDimension(sectionIndex) < maximumSymbolDimension()) {
            styleOption.rect = QRect(currentLeft, rect.y(), rect.width(), rect.height());
            mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        }
        painter->restore();
    }

    QString horizontalCellText(int logicalIndex, int sectionIndex, int dimension,
                               bool isSymbol, const Symbol *symbol,
                               const QString &text)
    {
        if (isSymbol && logicalIndex > 0) {
            if (mHeaderView->sectionViewportPosition(logicalIndex) <= 0)
                return text;
            int prevSectionIdx = this->sectionIndex(logicalIndex-1);
            if (symbol && symbol->contains(prevSectionIdx))
                return QString();
        } else if (logicalIndex > 0) {
            if (mHeaderView->sectionViewportPosition(logicalIndex) <= 0)
                return text;
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
    {
        if (isSymbol && logicalIndex > 0) {
            if (mHeaderView->sectionViewportPosition(logicalIndex) <= 0)
                return text;
            int prevSectionIdx = this->sectionIndex(logicalIndex-1);
            if (symbol && symbol->contains(prevSectionIdx))
                return QString();
        } else if (logicalIndex > 0) {
            if (mHeaderView->sectionViewportPosition(logicalIndex) <= 0)
                return text;
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

    QSize labelCellSize(const QStyleOptionHeader& styleOption) const
    {
        Q_UNUSED(styleOption);
        return QSize(mLabelCellSize);
    }

    QSize symbolCellSize(const QStyleOptionHeader& styleOption) const
    {
        Q_UNUSED(styleOption);
        return QSize(mSymbolCellSize);
    }

    int maximumSymbolDimension()
    {
        return mHeaderView->model()->headerData(0, mHeaderView->orientation(), ViewHelper::DimensionRole).toInt();
    }

    Symbol* symbol(int sectionIndex) const
    {
        if (mHeaderView->orientation() == Qt::Vertical)
            return mHeaderView->modelInstance()->equation(sectionIndex);
        return mHeaderView->modelInstance()->variable(sectionIndex);
    }

    QString label(int sectionIndex, int dimension)
    {
        if (mHeaderView->orientation() == Qt::Horizontal) {
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
    QSize symbolTextSize() const
    {
        QSize size;
        QFontMetrics fm(mHeaderView->font());
        if (mHeaderView->orientation() == Qt::Horizontal) {
            for (const auto& var : mHeaderView->mModelInstance->variables()) {
                auto ts = fm.size(mTextFlags, var->name()+" ");
                if (ts.width() > size.width())
                    size = ts;
            }
            return size;
        }
        for (const auto& eqn : mHeaderView->mModelInstance->equations()) {
            auto ts = fm.size(mTextFlags, eqn->name()+" ");
            if (ts.width() > size.width())
                size = ts;
        }
        return size;
    }

    QSize labelTextSize() const
    {
        QFontMetrics fm(mHeaderView->font());
        // for now use a fixed label size length
        //return fm.size(mTextFlags, mHeaderView->mModelInstance->longestLabelText()+" ");
        return fm.size(mTextFlags, "aaaaaaaaaa");
    }

private:
    const SymbolHierarchicalHeaderView *mHeaderView;

    int mTextFlags;

    QSize mSymbolCellSize;
    QSize mLabelCellSize;
};

SymbolHierarchicalHeaderView::SymbolHierarchicalHeaderView(Qt::Orientation orientation,
                                                           const QSharedPointer<AbstractModelInstance> &modelInstance,
                                                           QWidget *parent)
    : QHeaderView(orientation, parent)
    , mModelInstance(modelInstance)
    , mPrivate(new HierarchicalHeaderView_private(this))
{
    setHighlightSections(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignTop);
    setResizeContentsPrecision(0);
}

SymbolHierarchicalHeaderView::~SymbolHierarchicalHeaderView()
{
    delete mPrivate;
}

QSharedPointer<AbstractModelInstance> SymbolHierarchicalHeaderView::modelInstance() const
{
    return mModelInstance;
}

void SymbolHierarchicalHeaderView::paintSection(QPainter *painter,
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

QSize SymbolHierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
    QSize size(mPrivate->symbolCellSize(styleOption));
    auto labelSize = mPrivate->labelCellSize(styleOption);
    if (orientation() == Qt::Horizontal) {
        if (size.width() < labelSize.width())
            size.setWidth(labelSize.width());
        for (int d=0; d<mPrivate->maximumSymbolDimension(); ++d) {
            size.rheight() += labelSize.height();
        }
    } else {
        for (int d=0; d<mPrivate->maximumSymbolDimension(); ++d) {
            size.rwidth() += labelSize.width();
        }
    }
    return size;
}

QStyleOptionHeader SymbolHierarchicalHeaderView::styleOptionForCell(int logicalIndex) const
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
