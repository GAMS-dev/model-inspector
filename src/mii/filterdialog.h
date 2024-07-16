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
#ifndef FILTERDIALOG_H
#define FILTERDIALOG_H

#include <QDialog>

#include "common.h"

class QSortFilterProxyModel;
class QLineEdit;
class QTreeView;

namespace gams {
namespace studio {
namespace mii {

namespace Ui {
class FilterDialog;
}

class AbstractViewConfiguration;
class FilterTreeItem;
class Symbol;

class FilterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FilterDialog(QWidget *parent = nullptr);
    ~FilterDialog() override;

    const QSharedPointer<AbstractViewConfiguration> &viewConfig() const;

    void setViewConfig(const QSharedPointer<AbstractViewConfiguration> &config);

signals:
    void viewConfigUpdated();

private slots:
    void applyButtonClicked();

    void resetButtonClicked();

    void cancelButtonClicked();

    void selectEqnEntries();

    void deselectEqnEntries();

    void selectVarEntries();

    void deselectVarEntries();

    void selectLabelEntries();

    void deselectLabelEntries();

    void labelBoxCurrentIndexChanged(int index);

    void selectAttrEntries();

    void deselectAttrEntries();

    void selectDimEntries();

    void deselectDimEntries();

private:
    void setupConnections();

    void setupEquationFilter(const IdentifierStates &filter, const IdentifierStates &dFilter);
    void setupVariableFilter(const IdentifierStates &filter, const IdentifierStates &dFilter);
    void setupAttributeFilter();
    void setupLabelFilter();
    FilterTreeItem* setupSymTreeItems(const QString &text,
                                      const IdentifierStates &filter,
                                      const IdentifierStates &dFilter);
    void setupLabelTreeItems(const QString &text, Qt::Orientation orientation, FilterTreeItem *root);
    void setupDimensionFilter();
    void setupSymbolDimensions(const QVector<LabelCheckStates>& labels, FilterTreeItem *root);

    void applyCheckState(QTreeView* view,
                         QSortFilterProxyModel *model,
                         Qt::CheckState state);
    IdentifierStates applyHeaderFilter(Qt::Orientation orientation, QSortFilterProxyModel *model);
    void applyValueFilter();
    void applyLabelFilter(Qt::Orientation orientation, QSortFilterProxyModel *model);
    void applySymbolDimensions();
    void resetSymbolDimensions();

    LabelCheckStates applyAttributeFilter(QSortFilterProxyModel *model);

    void updateRangeEdit(QLineEdit *edit, const QString &text);

    Qt::Orientation equationOrientation() const {
        return Qt::Vertical;
    }

    Qt::Orientation variableOrientation() const {
        return Qt::Horizontal;
    }

private:
    Ui::FilterDialog *ui;

    QSharedPointer<AbstractViewConfiguration> mViewConfig;

    QSortFilterProxyModel *mEqnFilterModel;
    QSortFilterProxyModel *mVarFilterModel;
    QSortFilterProxyModel *mLabelFilterModel;
    QSortFilterProxyModel *mAttrFilterModel;
    QSortFilterProxyModel *mDimFilterModel;
};

}
}
}

#endif // FILTERDIALOG_H
