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
#ifndef LABELTREEITEM_H
#define LABELTREEITEM_H

#include "common.h"

#include <QSet>

namespace gams {
namespace studio {
namespace mii {

class LabelTreeItem
{
public:
    explicit LabelTreeItem(LabelTreeItem *parent);

    explicit LabelTreeItem(const QString &text = QString(),
                           LabelTreeItem *parent = nullptr);

    explicit LabelTreeItem(const char *text, LabelTreeItem *parent = nullptr);

    ~LabelTreeItem();

    void append(LabelTreeItem *child);

    LabelTreeItem* child(int index);

    const QList<LabelTreeItem*>& childs() const;

    QList<LabelTreeItem*> visibleChilds() const;

    void setChilds(const QList<LabelTreeItem*> &childs);

    LabelTreeItem* clone(LabelTreeItem* newParent = nullptr) const;

    bool hasChildren() const;

    bool isRoot() const;

    void remove(LabelTreeItem *child);

    int firstSectionIndex() const;

    int sectionIndex() const;

    void setSectionIndex(int index);

    LabelTreeItem* parent() const;

    void setParent(LabelTreeItem *parent);

    ///
    /// \brief All siblings of one level including the
    ///        one this function was called on.
    /// \return All siblings of one level
    ///
    QList<LabelTreeItem*> siblings() const;

    int size() const;

    QSet<int> sections() const;

    UnitedSections unitedSections() const;

    void setSections(const QSet<int> &sections);

    ///
    /// \brief Visible sections from the view of the source model,
    ///        i.e. childs which are invisible are not considered.
    /// \return List of visible sections, which might be unsorted.
    ///
    QList<int> visibleSections() const;

    QList<int> visibleSectionsSorted() const;

    SectionLabels sectionLabels(int startSection, int dimension) const;

    QString text() const;

    void setText(const QString &text);

    bool isVisible() const;

    void setVisible(bool visible);

    int sectionExtent() const;

    static LabelTreeItem* visibleBranch(QList<LabelTreeItem*> &currentLevel,
                                        const QString &typeText, int dimension);

private:
    LabelTreeItem *mParent;
    QString mText;
    int mSectionIndex = -1;
    QSet<int> mSections;
    QList<LabelTreeItem*> mChilds;
    bool mIsVisible = true;
};

}
}
}

#endif // LABELTREEITEM_H
