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
#ifndef FILTERTREEITEM_H
#define FILTERTREEITEM_H

#include <QString>
#include <QList>

namespace gams {
namespace studio {
namespace mii {

class FilterTreeItem
{
public:
    explicit FilterTreeItem(const QString &text = QString(),
                            Qt::CheckState checkState = Qt::Checked,
                            FilterTreeItem *parent = nullptr);

    ~FilterTreeItem();

    FilterTreeItem* findChild(const QString &text);

    void append(FilterTreeItem *child);

    FilterTreeItem* child(int index);

    QList<FilterTreeItem*> childs() const;

    bool hasChildren() const;

    FilterTreeItem* contains(const QString& label);

    int columnCount() const;

    int rowCount() const;

    int row() const;

    FilterTreeItem* parent() const;
    void setParent(FilterTreeItem *parent);

    QString text() const;
    void setText(const QString &text);

    bool isCheckable() const;
    void setCheckable(bool checkable);

    bool isEnabled() const;
    void setEnabled(bool enabled);

    Qt::CheckState checked();
    void setChecked(Qt::CheckState state);

    ///
    /// \brief Set sub tree state, i.e. set the state for all child items.
    /// \param Check state for childs items.
    ///
    void setSubTreeState(Qt::CheckState checked);

    int sectionIndex() const;
    void setSectionIndex(int index);

    int symbolIndex() const;
    void setSymbolIndex(int index);

    bool isVisible() const;
    void setVisible(bool visible);

private:
    FilterTreeItem *mParent = nullptr;

    QList<FilterTreeItem*> mChilds;

    QString mText;

    Qt::CheckState mChecked;

    int mSectionIndex = -1;

    int mSymbolIndex = -1;

    bool mCheckable = true;
    bool mEnabled = true;
    bool mVisible = true;
};

}
}
}

#endif // FILTERTREEITEM_H
