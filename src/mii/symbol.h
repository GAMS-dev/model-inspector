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
#ifndef SYMBOL_H
#define SYMBOL_H

#include "common.h"

#include <QString>
#include <QSharedPointer>

namespace gams {
namespace studio {
namespace mii {

class LabelTreeItem;

class Symbol
{
public:
    enum Type
    {
        Equation,
        Variable,
        Unknown
    };

    int offset() const;

    void setOffset(int offset);

    int entries() const;

    void setEntries(int entries);

    QString name() const;

    void setName(const QString &name);

    int dimension() const;

    void setDimension(int dimension);

    Type type() const;

    void setType(Type type);

    const DomainLabels &domainLabels() const;

    void setDomainLabels(const DomainLabels &labels);

    void appendDomainLabel(const QString &label);

    SectionLabels& sectionLabels();

    QString label(int sectionIndex, int dimension) const;

    bool contains(int sectionIndex) const;

    bool isScalar() const;

    int lastOffset() const;

    int firstSection() const;

    void setFirstSection(int sectionIndex);

    int lastSection() const;

    int logicalIndex() const;

    void setLogicalIndex(int index);

    const QSharedPointer<LabelTreeItem> &labelTree() const;

    void setLabelTree(const QSharedPointer<LabelTreeItem> &tree);

    bool isEquation() const;

    bool isVariable() const;

    bool operator==(const Symbol &other) const;

    bool operator!=(const Symbol &other) const;

private:
    int mFirstSection = -1;
    int mOffset = -1;
    int mEntries = -1;
    QString mName;
    int mDimension = -1;
    Type mType = Unknown;
    int mLogicalIndex = -1;

    SectionLabels mSectionLabels;

    DomainLabels mDomainLabels;

    QSharedPointer<LabelTreeItem> mLabelTree = nullptr;
};

}
}
}

#endif // SYMBOL_H
