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
#ifndef POSTOPTTREEVIEW_H
#define POSTOPTTREEVIEW_H

#include <QTreeView>

namespace gams {
namespace studio{
namespace mii {

class PostoptTreeView : public QTreeView
{
    Q_OBJECT

public:
    PostoptTreeView(QWidget *parent = nullptr);

    bool eventFilter(QObject *watched, QEvent *event) override;

    void zoomIn(int range = 1);
    void zoomOut(int range = 1);
    void resetZoom();

signals:
    void openFilterDialog();

public slots:
    void showCustomContextMenu(const QPoint &pos);

protected:
    void mousePressEvent(QMouseEvent *event) override;

private:
    void zoom(int range);

    void resizeColumns();

private:
    QFont mBaseFont;
    QMenu* mMenu;
    QAction* mCollapsAllAction;
    QAction* mExpandAllAction;
};

}
}
}

#endif // POSTOPTTREEVIEW_H
