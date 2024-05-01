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
#ifndef SYMBOLVIEWFRAME_H
#define SYMBOLVIEWFRAME_H

#include "abstracttableviewframe.h"

namespace gams {
namespace studio{
namespace mii {

class SymbolFilterModel;
class SymbolHierarchicalHeaderView;
class SymbolModelInstanceTableModel;

class SymbolViewFrame final : public AbstractTableViewFrame
{
    Q_OBJECT

public:
    SymbolViewFrame(int view,
                    const QSharedPointer<AbstractModelInstance>& modelInstance,
                    QWidget *parent = nullptr,
                    Qt::WindowFlags f = Qt::WindowFlags());

    SymbolViewFrame(const QSharedPointer<AbstractModelInstance> &modelInstance,
                    const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                    QWidget *parent = nullptr,
                    Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int viewId) override;

    void setupView(const QSharedPointer<AbstractModelInstance> &modelInstance) override;

    ViewHelper::ViewDataType type() const override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    bool hasData() const override;

public slots:
    void evaluateFilters() override;

private slots:
    void customMenuRequested(const QPoint &pos);

    void resetHeaderFilter();

private:
    void setupView();

private:
    QMenu *mSelectionMenu;
    QAction *mResetAction = new QAction("Reset filter", this);
    QSharedPointer<SymbolModelInstanceTableModel> mBaseModel;
    SymbolHierarchicalHeaderView* mHorizontalHeader = nullptr;
    SymbolHierarchicalHeaderView* mVerticalHeader = nullptr;
    SymbolFilterModel* mHeaderFilterModel = nullptr;
};

}
}
}

#endif // SYMBOLVIEWFRAME_H
