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
#ifndef SECTIONTREEITEM_H
#define SECTIONTREEITEM_H

#include <QString>
#include <QVector>

#include "common.h"

namespace gams {
namespace studio {
namespace mii {

class AbstractViewFrame;

class AbstractSectionTreeItem
{
protected:
    explicit AbstractSectionTreeItem(const QString &text,
                                     AbstractSectionTreeItem *parent = nullptr);

    explicit AbstractSectionTreeItem(const QString &text,
                                     AbstractViewFrame* widget,
                                     AbstractSectionTreeItem *parent = nullptr);

    AbstractSectionTreeItem(const AbstractSectionTreeItem& other);

    AbstractSectionTreeItem(AbstractSectionTreeItem&& other) noexcept;

public:
    virtual ~AbstractSectionTreeItem();

    virtual void append(AbstractSectionTreeItem *child);

    virtual void removeAllChilds() = 0;

    virtual QList<AbstractViewFrame*> removeChilds();

    virtual void remove(AbstractSectionTreeItem *child);

    virtual void remove(int index, int count);

    virtual AbstractSectionTreeItem *child(int row);

    virtual const QList<AbstractSectionTreeItem*>& childs() const = 0;

    virtual int childCount() const;

    virtual QList<AbstractViewFrame*> widgets() const;

    virtual int columnCount() const;

    virtual int rowCount() const;

    ViewHelper::ViewDataType type() const;

    void setType(ViewHelper::ViewDataType type);

    void setType(const QString &text);

    bool isCustom() const;

    void setCustom(bool custom);

    bool isGroup() const;

    bool isModelInstanceGroup() const;

    virtual bool isActive() const = 0;

    virtual void setActive(bool active) = 0;

    QString scratchDir() const;

    void setScratchDir(const QString& scratchDir);

    AbstractSectionTreeItem *parent() const;

    void setParent(AbstractSectionTreeItem *parent);

    QString text() const;

    void setText(const QString &text);

    AbstractViewFrame* widget() const;

    void setWidget(AbstractViewFrame* page);

    int row() const;

    AbstractSectionTreeItem* customGroup();

    AbstractSectionTreeItem* predefinedGroup();

    AbstractSectionTreeItem* symbolGroup();

    AbstractSectionTreeItem* modelInstanceGroup();

    AbstractSectionTreeItem* find(ViewHelper::ViewDataType type);

    AbstractSectionTreeItem &operator=(const AbstractSectionTreeItem& other);

    AbstractSectionTreeItem &operator=(AbstractSectionTreeItem&& other) noexcept;

protected:
    const QString ActivePostfix = " (inactive)";

private:
    QString mText;
    QString mScratchDir;
    AbstractSectionTreeItem *mParent;
    ViewHelper::ViewDataType mType = ViewHelper::ViewDataType::Unknown;
    bool mCustom = false;

    ///
    /// \brief Target widget.
    ///
    /// \remark Do not delete. Owned by QStackedWidget.
    ///
    AbstractViewFrame* mWidget = nullptr;
};

class SectionGroupTreeItem final : public AbstractSectionTreeItem
{
public:
    explicit SectionGroupTreeItem(const QString &text,
                                  AbstractSectionTreeItem *parent = nullptr);

    SectionGroupTreeItem(const SectionGroupTreeItem& other);

    SectionGroupTreeItem(SectionGroupTreeItem&& other) noexcept;

    virtual ~SectionGroupTreeItem() override;

    bool isActive() const override;

    void setActive(bool active) override;

    void append(AbstractSectionTreeItem *child) override;

    void removeAllChilds() override;

    QList<AbstractViewFrame*> removeChilds() override;

    void remove(AbstractSectionTreeItem *child) override;

    void remove(int index, int count) override;

    AbstractSectionTreeItem *child(int row) override;

    const QList<AbstractSectionTreeItem*>& childs() const override;

    int childCount() const override;

    QList<AbstractViewFrame*> widgets() const override;

    SectionGroupTreeItem& operator=(const SectionGroupTreeItem& other);

    SectionGroupTreeItem& operator=(SectionGroupTreeItem&& other) noexcept;

private:
    QVector<AbstractSectionTreeItem*> mChilds;
};

class SectionTreeItem final : public AbstractSectionTreeItem
{
public:
    explicit SectionTreeItem(const QString &text,
                             AbstractViewFrame* widget,
                             AbstractSectionTreeItem *parent = nullptr);

    SectionTreeItem(const SectionTreeItem& other);

    SectionTreeItem(SectionTreeItem&& other) noexcept;

    virtual ~SectionTreeItem();

    bool isActive() const override;

    void setActive(bool active) override;

    void append(AbstractSectionTreeItem *child) override;

    void removeAllChilds() override;

    QList<AbstractViewFrame*> removeChilds() override;

    void remove(int index, int count) override;

    AbstractSectionTreeItem *child(int row) override;

    const QList<AbstractSectionTreeItem*>& childs() const override;

    int childCount() const override;

    QList<AbstractViewFrame*> widgets() const override;

    SectionTreeItem& operator=(const SectionTreeItem& other);

    SectionTreeItem& operator=(SectionTreeItem&& other) noexcept;

private:
    QVector<AbstractSectionTreeItem*> mChilds;
    bool mActive = false;
};

}
}
}

#endif // SECTIONTREEITEM_H
