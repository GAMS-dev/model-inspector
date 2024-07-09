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
#ifndef COMPREHENSIVETABLEMODEL_H
#define COMPREHENSIVETABLEMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

namespace gams {
namespace studio{
namespace mii {

class AbstractModelInstance;

class ComprehensiveTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ComprehensiveTableModel(QObject *parent = nullptr);

    explicit ComprehensiveTableModel(int view,
                                     const QSharedPointer<AbstractModelInstance> &modelInstance,
                                     QObject *parent = nullptr);

    ~ComprehensiveTableModel() override;

    void setModelInstance(const QSharedPointer<AbstractModelInstance> &modelInstance);

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    QHash<int, QByteArray> roleNames() const override;

    int view() const;

    void setView(int view);

protected:
    QSharedPointer<AbstractModelInstance> mModelInstance;
    int mView;
};

class BPOverviewTableModel final : public ComprehensiveTableModel
{
    Q_OBJECT

public:
    explicit BPOverviewTableModel(QObject *parent = nullptr);

    explicit BPOverviewTableModel(int view,
                                  const QSharedPointer<AbstractModelInstance> &modelInstance,
                                  QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
};

class BPCountTableModel final : public ComprehensiveTableModel
{
    Q_OBJECT

public:
    explicit BPCountTableModel(QObject *parent = nullptr);

    explicit BPCountTableModel(int view,
                               const QSharedPointer<AbstractModelInstance> &modelInstance,
                               QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
};

class BPAverageTableModel final : public ComprehensiveTableModel
{
    Q_OBJECT

public:
    explicit BPAverageTableModel(QObject *parent = nullptr);

    explicit BPAverageTableModel(int view,
                                 const QSharedPointer<AbstractModelInstance> &modelInstance,
                                 QObject *parent = nullptr);

    QVariant data(const QModelIndex &index, int role) const override;
};

}
}
}

#endif // COMPREHENSIVETABLEMODEL_H
