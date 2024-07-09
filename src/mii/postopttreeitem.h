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
#ifndef POSTOPTTREEITEM_H
#define POSTOPTTREEITEM_H

#include <QVector>
#include <QVariant>

namespace gams {
namespace studio {
namespace mii {

class PostoptTreeItem
{
public:
    enum Type
    {
        LineItem,
        GroupItem,
        ClickItem
    };

    explicit PostoptTreeItem(PostoptTreeItem* parent = nullptr)
        : mParent(parent)
    {

    }

    virtual ~PostoptTreeItem()
    {
        qDeleteAll(mChilds);
    }

    virtual QVariant data(int index) const = 0;

    void append(PostoptTreeItem *child)
    {
        if (child)
            mChilds.append(child);
    }

    PostoptTreeItem* child(int index) const
    {
        if (index < 0 || index >= mChilds.size())
            return nullptr;
        return mChilds.at(index);
    }

    int rowCount() const
    {
        return mChilds.size();
    }

    virtual int columnCount() const = 0;

    int row() const
    {
        if (mParent)
            return mParent->mChilds.indexOf(const_cast<PostoptTreeItem*>(this));
        return 0;
    }

    PostoptTreeItem* parent() const
    {
        return mParent;
    }

    void setParent(PostoptTreeItem* parent)
    {
        mParent = parent;
    }

    virtual Type type() const = 0;

    static const QVector<QVariant> EquationLineHeader;
    static const QVector<QVariant> VariableLineHeader;

private:
    PostoptTreeItem *mParent;

    QVector<PostoptTreeItem*> mChilds;
};

class GroupPostoptTreeItem : public PostoptTreeItem
{
public:
    explicit GroupPostoptTreeItem(const QString &value = QString(), PostoptTreeItem* parent = nullptr)
        : PostoptTreeItem(parent)
        , mValue(value)
    {

    }

    QVariant data(int index) const override
    {
        return index == 0 ? mValue : QVariant();
    }

    int columnCount() const override
    {
        return 1;
    }

    Type type() const override
    {
        return GroupItem;
    }

private:
    QString mValue;
};

class LinePostoptTreeItem : public PostoptTreeItem
{
public:
    explicit LinePostoptTreeItem(const QVector<QVariant>& data = QVector<QVariant>(),
                                 PostoptTreeItem* parent = nullptr)
        : PostoptTreeItem(parent)
        , mData(data)
    {

    }

    QVariant data(int index) const override
    {
        if (index < 0 || index >= mData.size())
            return QVariant();
        return mData[index];
    }

    int columnCount() const override
    {
        return mData.size();
    }

    Type type() const override
    {
        return LineItem;
    }

private:
    QVector<QVariant> mData;
};

class ClickPostoptTreeItem : public PostoptTreeItem
{
public:
    explicit ClickPostoptTreeItem(const QString &text = QString(),
                                  PostoptTreeItem* parent = nullptr)
        : PostoptTreeItem(parent)
        , mText(text)
    {

    }

    QVariant data(int index) const override
    {
        return index == 0 ? mText : QVariant();
    }

    int columnCount() const override
    {
        return 1;
    }

    Type type() const override
    {
        return ClickItem;
    }

private:
    QString mText;
};

}
}
}

#endif // POSTOPTTREEITEM_H
