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
#ifndef LOGVIEW_H
#define LOGVIEW_H

#include <QRegularExpression>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>

class HighlightingRule
{
public:
    HighlightingRule()
    {

    }

    QRegularExpression pattern() const
    {
        return mRegEx;
    }

    void setBold(int weight)
    {
        mFormat.setFontWeight(weight);
    }

    void setColor(const QBrush& brush)
    {
        mFormat.setForeground(brush);
    }

    void setPattern(const QRegularExpression &regex)
    {
        mRegEx = regex;
    }

    QTextCharFormat format() const
    {
        return mFormat;
    }

protected:
    QRegularExpression mRegEx;
    QTextCharFormat mFormat;
    static const QString timestampRegex;
};

class LogHighlighter
    : public QSyntaxHighlighter
{
    Q_OBJECT

public:
    LogHighlighter(QObject *parent = nullptr);

protected:
    void highlightBlock(const QString &text) override;

private:
    QVector<HighlightingRule> mHighlightingRules;
};

class LogView : public QPlainTextEdit
{
    Q_OBJECT

public:
    LogView(QWidget *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void resetZoom();

private:
    QFont mBaseFont;
    LogHighlighter mHighlighter;
};

#endif // LOGVIEW_H
