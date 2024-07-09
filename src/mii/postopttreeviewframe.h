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
#ifndef POSTOPTTREEVIEWFRAME_H
#define POSTOPTTREEVIEWFRAME_H

#include "abstractviewframe.h"

namespace gams {
namespace studio{
namespace mii {

namespace Ui {
class PostoptTreeViewFrame;
}
class PostoptTreeModel;
class ValueFormatProxyModel;


class PostoptTreeViewFrame final : public AbstractViewFrame
{
    Q_OBJECT

public:
    PostoptTreeViewFrame(QWidget *parent = nullptr,
                         Qt::WindowFlags f = Qt::WindowFlags());

    PostoptTreeViewFrame(const QSharedPointer<AbstractModelInstance>& modelInstance,
                         const QSharedPointer<AbstractViewConfiguration>& viewConfig,
                         QWidget *parent = nullptr,
                         Qt::WindowFlags f = Qt::WindowFlags());

    ~PostoptTreeViewFrame() override;

    AbstractViewFrame* clone(int viewId) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    SearchResult& search(const QString &term, bool isRegEx) override;

    void setSearchSelection(const SearchResult::SearchEntry &result) override;

    void setupView(const QSharedPointer<AbstractModelInstance>& modelInstance) override;

    ViewHelper::ViewDataType type() const override;

    void zoomIn() override;

    void zoomOut() override;

    void resetZoom() override;

    bool hasData() const override;

signals:
    void openFilterDialog();

public slots:
    void evaluateFilters() override;

private:
    void setupView();

protected:
    Ui::PostoptTreeViewFrame* ui;
    PostoptTreeModel *mBaseModel;
    ValueFormatProxyModel* mValueFormatModel = nullptr;
};

}
}
}

#endif // POSTOPTTREEVIEWFRAME_H
