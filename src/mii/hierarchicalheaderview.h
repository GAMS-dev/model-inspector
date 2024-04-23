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
#ifndef HIERARCHICALHEADERVIEW_H
#define HIERARCHICALHEADERVIEW_H

#include <QHeaderView>

#include "common.h"

class QMenu;

namespace gams {
namespace studio{
namespace mii {

class AbstractModelInstance;
class AbstractViewConfiguration;
class LabelFilterWidget;

class HierarchicalHeaderView final : public QHeaderView
{
    Q_OBJECT

public:
    HierarchicalHeaderView(Qt::Orientation orientation,
                           const QSharedPointer<AbstractModelInstance> &modelInstance,
                           const QSharedPointer<AbstractViewConfiguration> &viewConfig,
                           QWidget *parent = nullptr);

    ~HierarchicalHeaderView() override;

    QSharedPointer<AbstractModelInstance> modelInstance() const;

    void setModel(QAbstractItemModel *model) override;

    const QSharedPointer<AbstractViewConfiguration>& viewConfig() const;

public slots:
    void customMenuRequested(const QPoint &position);

signals:
    void filterChanged();

private slots:
    void on_filterChanged(const gams::studio::mii::IdentifierState& state,
                          Qt::Orientation orientation);

protected:
    void paintSection(QPainter *painter, const QRect &rect,
                      int logicalIndex) const override;

    QSize sectionSizeFromContents(int logicalIndex) const override;

    void mousePressEvent(QMouseEvent *event) override;

private:
    QStyleOptionHeader styleOptionForCell(int logicalIndex) const;

private:
    class HierarchicalHeaderView_private;
    HierarchicalHeaderView_private *mPrivate;
    QSharedPointer<AbstractModelInstance> mModelInstance;
    QSharedPointer<AbstractViewConfiguration> mViewConfig;
    QMenu *mFilterMenu;
    LabelFilterWidget *mFilterWidget;
};

}
}
}

#endif // HIERARCHICALHEADERVIEW_H
