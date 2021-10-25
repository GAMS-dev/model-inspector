#include "filtertreeitem.h"

namespace gams {
namespace studio {
namespace modelinspector {

const QString FilterTreeItem::EquationText = "Equations";
const QString FilterTreeItem::VariableText = "Variables";

FilterTreeItem::FilterTreeItem(const QString &text,
                               bool checkable,
                               FilterTreeItem *parent)
    : FilterTreeItem(text, Qt::Unchecked, -1, parent)
{
    setCheckable(checkable);
}

FilterTreeItem::FilterTreeItem(const QString &text,
                                 Qt::CheckState checkState,
                                 int index,
                                 FilterTreeItem *parent)
    : mParent(parent)
    , mText(text)
    , mChecked(checkState)
    , mIndex(index)
{

}

FilterTreeItem::~FilterTreeItem()
{
    qDeleteAll(mChilds);
}

void FilterTreeItem::append(FilterTreeItem *child)
{
    mChilds.append(child);
}

FilterTreeItem* FilterTreeItem::child(int row)
{
    if (row < 0 || row >= mChilds.size())
        return nullptr;
    return mChilds.at(row);
}

QList<FilterTreeItem*> FilterTreeItem::childs() const
{
    return mChilds;
}

int FilterTreeItem::columnCount() const
{
    return 1;
}

int FilterTreeItem::rowCount() const
{
    return mChilds.size();
}

int FilterTreeItem::row() const
{
    if (mParent)
        return mParent->mChilds.indexOf(const_cast<FilterTreeItem*>(this));
    return 0;
}

FilterTreeItem* FilterTreeItem::parent()
{
    return mParent;
}

QString FilterTreeItem::text() const
{
    return mText;
}

bool FilterTreeItem::isCheckable() const
{
    return mCheckable;
}

void FilterTreeItem::setCheckable(bool checkable)
{
    mCheckable = checkable;
}

bool FilterTreeItem::isEnabled() const
{
    return mEnabled;
}

void FilterTreeItem::setEnabled(bool enabled)
{
    mEnabled = enabled;
}

Qt::CheckState FilterTreeItem::checked()
{
    if (mChilds.isEmpty())
        return mChecked;
    int unchecked = 0, checked = 0;
    for (int i=0; i<mChilds.size(); ++i) {
        switch (mChilds[i]->checked()) {
            case Qt::Unchecked:
                ++unchecked;
                break;
            case Qt::Checked:
                ++checked;
                break;
            default:
                return Qt::PartiallyChecked;
        }
    }
    if (unchecked == mChilds.size())
        return Qt::Unchecked;
    if (checked == mChilds.size())
        return Qt::Checked;
    return Qt::PartiallyChecked;
}

void FilterTreeItem::setChecked(Qt::CheckState state)
{
    mChecked = state;
}

void FilterTreeItem::setSubTreeState(Qt::CheckState checked)
{
    QList<FilterTreeItem*> subTree(mChilds);
    while (!subTree.isEmpty()) {
        auto item = subTree.takeFirst();
        item->setChecked(checked);
        subTree.append(item->childs());
    }
}

int FilterTreeItem::index() const
{
    return mIndex;
}

void FilterTreeItem::setIndex(int index)
{
    mIndex = index;
}

}
}
}
