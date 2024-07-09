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
#include "comprehensivetablemodel.h"
#include "abstractmodelinstance.h"

#include <QFont>

namespace gams {
namespace studio{
namespace mii {

ComprehensiveTableModel::ComprehensiveTableModel(QObject *parent)
    : QAbstractTableModel(parent)
    , mModelInstance(new EmptyModelInstance)
{

}

ComprehensiveTableModel::ComprehensiveTableModel(int view,
                                                 const QSharedPointer<AbstractModelInstance> &modelInstance,
                                                 QObject *parent)
    : QAbstractTableModel(parent)
    , mModelInstance(modelInstance)
    , mView(view)
{

}

ComprehensiveTableModel::~ComprehensiveTableModel()
{

}

void ComprehensiveTableModel::setModelInstance(const QSharedPointer<AbstractModelInstance> &modelInstance)
{
    beginResetModel();
    mModelInstance = modelInstance;
    endResetModel();
}

QVariant ComprehensiveTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::FontRole) {
        if (mModelInstance->nlFlag(index.row(), index.column(), mView)) {
            QFont font;
            font.setBold(true);
            font.setItalic(true);
            return font;
        }
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        return mModelInstance->data(index.row(), index.column(), mView);
    }
    return QVariant();
}

Qt::ItemFlags ComprehensiveTableModel::flags(const QModelIndex &index) const
{
    if (index.isValid())
        return QAbstractTableModel::flags(index) |
               Qt::ItemIsEnabled |
               Qt::ItemIsSelectable;
    return Qt::NoItemFlags;
}

QVariant ComprehensiveTableModel::headerData(int section,
                                             Qt::Orientation orientation,
                                             int role) const
{
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Vertical) {
            return section;
        } else {
            return mModelInstance->headerData(section, orientation,
                                              mView, ViewHelper::LabelDataRole);
        }
    }
    if (role == ViewHelper::IndexDataRole) {
        auto realIndex = mModelInstance->headerData(section, orientation,
                                                    mView, role).toInt();
        return realIndex < 0 ? QVariant() : realIndex;
    }
    if (role == ViewHelper::LabelDataRole) {
        return orientation == Qt::Vertical ? QVariant()
                                           : mModelInstance->headerData(section,
                                                                        orientation,
                                                                        mView,
                                                                        role);
    }
    if (role == ViewHelper::SectionLabelRole) {
        return mModelInstance->headerData(section, orientation,
                                          mView, role);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

QModelIndex ComprehensiveTableModel::index(int row,
                                           int column,
                                           const QModelIndex &parent) const
{
    if (hasIndex(row, column, parent))
        return createIndex(row, column);
    return QModelIndex();
}

int ComprehensiveTableModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mModelInstance->rowCount(mView);
}

int ComprehensiveTableModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);
    return mModelInstance->columnCount(mView);
}

QHash<int, QByteArray> ComprehensiveTableModel::roleNames() const
{
    return ViewHelper::roleNames();
}

int ComprehensiveTableModel::view() const
{
    return mView;
}

void ComprehensiveTableModel::setView(int view)
{
    mView = view;
}

BPOverviewTableModel::BPOverviewTableModel(QObject *parent)
    : ComprehensiveTableModel(parent)
{

}

BPOverviewTableModel::BPOverviewTableModel(int view,
                                           const QSharedPointer<AbstractModelInstance> &modelInstance,
                                           QObject *parent)
    : ComprehensiveTableModel(view, modelInstance, parent)
{

}

QVariant BPOverviewTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        auto value = mModelInstance->data(index.row(), index.column(), mView).toInt();
        if (!value)
            return QVariant();
        if (value == ValueHelper::Mixed)
            return ValueHelper::PlusMinus;
        return QChar(value);
    }
    return ComprehensiveTableModel::data(index, role);
}

QVariant BPOverviewTableModel::headerData(int section,
                                          Qt::Orientation orientation,
                                          int role) const
{
    if (role == Qt::DisplayRole || role == ViewHelper::LabelDataRole) {
        return mModelInstance->headerData(section, orientation,
                                          mView, ViewHelper::LabelDataRole);
    }
    if (role == ViewHelper::IndexDataRole) {
        return mModelInstance->headerData(section, orientation, mView, role);
    }
    if (role == ViewHelper::SectionLabelRole) {
        return mModelInstance->headerData(section, orientation, mView, role);
    }
    return ComprehensiveTableModel::headerData(section, orientation, role);
}

BPCountTableModel::BPCountTableModel(QObject *parent)
    : ComprehensiveTableModel(parent)
{

}

BPCountTableModel::BPCountTableModel(int view,
                                     const QSharedPointer<AbstractModelInstance> &modelInstance,
                                     QObject *parent)
    : ComprehensiveTableModel(view, modelInstance, parent)
{

}

QVariant BPCountTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        if (index.column() == mModelInstance->columnCount(mView)-4 ||
            index.row() == mModelInstance->rowCount(mView)-1) {
            auto value = mModelInstance->data(index.row(), index.column(), mView).toInt();
            return value ? QChar(value) : QVariant();
        }
        return mModelInstance->data(index.row(), index.column(), mView);
    }
    return ComprehensiveTableModel::data(index, role);
}

BPAverageTableModel::BPAverageTableModel(QObject *parent)
    : ComprehensiveTableModel(parent)
{

}

BPAverageTableModel::BPAverageTableModel(int view,
                                         const QSharedPointer<AbstractModelInstance> &modelInstance,
                                         QObject *parent)
    : ComprehensiveTableModel(view, modelInstance, parent)
{

}

QVariant BPAverageTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        if (index.column() == mModelInstance->columnCount(mView)-4 ||
            index.row() == mModelInstance->rowCount(mView)-1) {
            auto value = mModelInstance->data(index.row(), index.column(), mView).toInt();
            return value ? QChar(value) : QVariant();
        }
        return mModelInstance->data(index.row(), index.column(), mView);
    }
    return ComprehensiveTableModel::data(index, role);
}

}
}
}
