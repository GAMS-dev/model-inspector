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
#include "logview.h"
#include "mii/common.h"

#include <QEvent>
#include <QWheelEvent>

LogHighlighter::LogHighlighter(QObject *parent)
    : QSyntaxHighlighter(parent)
{
    HighlightingRule errorRule;
    errorRule.setBold(QFont::Bold);
    errorRule.setColor(QBrush(Qt::red));
    errorRule.setPattern(QRegularExpression(".*error.*", QRegularExpression::CaseInsensitiveOption));
    mHighlightingRules.push_back(errorRule);
    HighlightingRule warningRule;
    warningRule.setBold(QFont::Bold);
    warningRule.setColor(QBrush(QColorConstants::Svg::orange));
    warningRule.setPattern(QRegularExpression(".*warning.*", QRegularExpression::CaseInsensitiveOption));
    mHighlightingRules.push_back(warningRule);
}

void LogHighlighter::highlightBlock(const QString &text)
{
    for (const HighlightingRule &rule : std::as_const(mHighlightingRules)) {
        QRegularExpressionMatchIterator matchIterator = rule.pattern().globalMatch(text);
        while (matchIterator.hasNext()) {
            QRegularExpressionMatch match = matchIterator.next();
            setFormat(match.capturedStart(), (int)match.capturedLength(), rule.format());
        }
    }
}

LogView::LogView(QWidget *parent)
    : QPlainTextEdit(parent)
    , mBaseFont(font())
{
    mHighlighter.setDocument(document());
}

bool LogView::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::Wheel) {
        QWheelEvent *wheel = dynamic_cast<QWheelEvent*>(event);
        if (wheel && wheel->modifiers() == Qt::ControlModifier) {
            if (wheel->angleDelta().y() > 0)
                zoomIn(gams::studio::mii::ViewHelper::ZoomFactor);
            else
                zoomOut(gams::studio::mii::ViewHelper::ZoomFactor);
            return true;
        }
    }
    return QPlainTextEdit::eventFilter(watched, event);
}

void LogView::resetZoom()
{
    setFont(mBaseFont);
}
