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
#include "abstractmodelinstance.h"
#include "viewconfigurationprovider.h"

#include <QPainter>

namespace gams {
namespace studio{
namespace mii {

class HierarchicalHeaderView::HierarchicalHeaderView_private
{
public:
    HierarchicalHeaderView_private(const HierarchicalHeaderView *headerView)
        : mHeaderView(headerView)
        , mTextFlags(Qt::TextSingleLine | Qt::TextDontClip)
    {
        QFontMetrics fm(mHeaderView->font());
        mSymbolCellSize = symbolTextSize();
        mLabelCellSize = fm.size(mTextFlags, "Max ");
    }

    int sectionIndex(int logicalIndex) const
    {
        bool ok = false;
        auto index = mHeaderView->model()->headerData(logicalIndex,
                                                      mHeaderView->orientation()).toInt(&ok);
        return !ok ? -1 : index;
    }

    void paintHorizontalSection(QPainter *painter,
                                const QRect &rect,
                                const QStyleOptionHeader &option,
                                int sectionIndex)
    {
        QPointF oldBrushOrigin(painter->brushOrigin());

        QStyleOptionHeader styleOption(option);
        if (styleOption.state.testFlag(QStyle::State_Sunken) ||
                styleOption.state.testFlag(QStyle::State_On)) {
            QStyle::State state(QStyle::State_Sunken | QStyle::State_On);
            styleOption.state &= (~state);
        }

        auto var = mHeaderView->modelInstance()->variable(sectionIndex);
        auto text = var ? var->name() : QString();
        QSize size = symbolCellSize(styleOption);

        styleOption.rect = QRect(rect.x(), rect.y(), rect.width(), size.height());
        styleOption.text = text;

        painter->save();
        mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        painter->restore();

        painter->setBrushOrigin(oldBrushOrigin);
    }

    void paintVerticalSection(QPainter *painter,
                              const QRect &rect,
                              const QStyleOptionHeader &option,
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

        QSize symSize = symbolCellSize(styleOption);
        styleOption.rect = QRect(currentLeft, rect.y(), symSize.width(), rect.height());
        styleOption.text = mHeaderView->modelInstance()->plainHeaderData(mHeaderView->orientation(),
                                                                         mHeaderView->viewConfig()->viewId(),
                                                                         sectionIndex,
                                                                         0).toString();
        painter->save();
        mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        painter->restore();
        currentLeft += symSize.width();

        styleOption.rect = QRect(currentLeft, rect.y(), mLabelCellSize.width(), rect.height());
        styleOption.text = mHeaderView->modelInstance()->plainHeaderData(mHeaderView->orientation(),
                                                                         mHeaderView->viewConfig()->viewId(),
                                                                         sectionIndex,
                                                                         1).toString();
        painter->save();
        mHeaderView->style()->drawControl(QStyle::CE_HeaderSection, &styleOption, painter, mHeaderView);
        mHeaderView->style()->drawControl(QStyle::CE_HeaderLabel, &styleOption, painter, mHeaderView);
        painter->restore();
        painter->setBrushOrigin(oldBrushOrigin);
    }

    QSize symbolCellSize(const QStyleOptionHeader& styleOption) const
    {
        QSize decorationsSize(mHeaderView->style()->sizeFromContents(QStyle::CT_HeaderSection,
                                                                     &styleOption,
                                                                     mSymbolCellSize,
                                                                     mHeaderView));
        QSize size(mSymbolCellSize);
        size.rwidth() += decorationsSize.width();
        return size;
    }

    QSize labelCellSize(const QStyleOptionHeader& styleOption) const
    {
        QSize decorationsSize(mHeaderView->style()->sizeFromContents(QStyle::CT_HeaderSection,
                                                                     &styleOption,
                                                                     mLabelCellSize,
                                                                     mHeaderView));
        QSize size(mLabelCellSize);
        size.rwidth() += decorationsSize.width();
        return size;
    }

    QSize sectionSize(const QStyleOptionHeader& styleOption) const
    {
        QSize size(symbolCellSize(styleOption));
        size.rwidth() += labelCellSize(styleOption).width();
        return size;
    }

private:
    QSize symbolTextSize() const
    {
        QFontMetrics fm(mHeaderView->font());
        if (mHeaderView->orientation() == Qt::Horizontal) {
            auto text = mHeaderView->mModelInstance->longestVariableText();
            for (const auto& label : mHeaderView->mViewConfig->additionalHorizontalSymbolLabels()) {
                if (text.size() < label.size())
                    text = label;
            }
            text.append(" ");
            return fm.size(mTextFlags, text);
        }
        auto text = mHeaderView->mModelInstance->longestEquationText();
        for (const auto& label : mHeaderView->mViewConfig->additionalVerticalSymbolLabels()) {
            if (text.size() < label.size())
                text = label;
        }
        text.append(" ");
        return fm.size(mTextFlags, text);
    }

private:
    const HierarchicalHeaderView *mHeaderView;

    int mTextFlags;
    QSize mSymbolCellSize;
    QSize mLabelCellSize;
};

HierarchicalHeaderView::HierarchicalHeaderView(Qt::Orientation orientation,
                                               const QSharedPointer<AbstractModelInstance> &modelInstance,
                                               const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                                               QWidget *parent)
    : QHeaderView(orientation, parent)
    , mModelInstance(modelInstance)
    , mViewConfig(viewConfig)
    , mPrivate(new HierarchicalHeaderView_private(this))
{
    setHighlightSections(true);
    setSectionsClickable(true);
    setDefaultAlignment(Qt::AlignLeft | Qt::AlignTop);
    setResizeContentsPrecision(0);
}

HierarchicalHeaderView::~HierarchicalHeaderView()
{
    delete mPrivate;
}

QSharedPointer<AbstractModelInstance> HierarchicalHeaderView::modelInstance() const
{
    return mModelInstance;
}

const QSharedPointer<AbstractViewConfiguration> &HierarchicalHeaderView::viewConfig() const
{
    return mViewConfig;
}

void HierarchicalHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    if (rect.isValid()) {
        auto option = styleOptionForCell(logicalIndex);
        int sectionIdx = mPrivate->sectionIndex(logicalIndex);
        if (sectionIdx < 0 )
            return;
        if (orientation() == Qt::Horizontal) {
            mPrivate->paintHorizontalSection(painter, rect, option, sectionIdx);
        } else {
            mPrivate->paintVerticalSection(painter, rect, option, sectionIdx);
        }
        return;
    }
    QHeaderView::paintSection(painter, rect, logicalIndex);
}

QSize HierarchicalHeaderView::sectionSizeFromContents(int logicalIndex) const
{
    QStyleOptionHeader styleOption(styleOptionForCell(logicalIndex));
    return mPrivate->sectionSize(styleOption);
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
