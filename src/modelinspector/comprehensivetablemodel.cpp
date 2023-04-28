#include "comprehensivetablemodel.h"
#include "abstractmodelinstance.h"

namespace gams {
namespace studio{
namespace modelinspector {

ComprehensiveTableModel::ComprehensiveTableModel(QObject *parent)
    : QAbstractTableModel(parent)
{

}

ComprehensiveTableModel::ComprehensiveTableModel(int view,
                                                 QSharedPointer<AbstractModelInstance> modelInstance,
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
            auto realIndex = mModelInstance->headerData(section, orientation,
                                                        mView, Mi::IndexDataRole).toInt();
            return realIndex < 0 ? QVariant() : realIndex;
        } else {
            return mModelInstance->headerData(section, orientation,
                                              mView, Mi::LabelDataRole);
        }
    }
    if (role == Mi::IndexDataRole) {
        auto realIndex = mModelInstance->headerData(section, orientation,
                                                    mView, Mi::IndexDataRole).toInt();
        return realIndex < 0 ? QVariant() : realIndex;
    }
    if (role == Mi::LabelDataRole) {
        return orientation == Qt::Vertical ? QVariant()
                                           : mModelInstance->headerData(section,
                                                                        orientation,
                                                                        mView,
                                                                        Mi::LabelDataRole);
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
    return Mi::roleNames();
}

int ComprehensiveTableModel::view() const
{
    return mView;
}

void ComprehensiveTableModel::setView(int view)
{
    mView = view;
}

BlockpicOverviewTableModel::BlockpicOverviewTableModel(QObject *parent)
    : ComprehensiveTableModel(parent)
{

}

BlockpicOverviewTableModel::BlockpicOverviewTableModel(int view,
                                                       QSharedPointer<AbstractModelInstance> modelInstance,
                                                       QObject *parent)
    : ComprehensiveTableModel(view, modelInstance, parent)
{

}

QVariant BlockpicOverviewTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        return QChar(mModelInstance->data(index.row(), index.column(), mView).toInt());
    }
    return QVariant();
}

QVariant BlockpicOverviewTableModel::headerData(int section,
                                                Qt::Orientation orientation,
                                                int role) const
{
    if (role == Qt::DisplayRole || role == Mi::LabelDataRole) {
        return mModelInstance->headerData(section, orientation,
                                          mView, Mi::LabelDataRole);
    }
    return QAbstractItemModel::headerData(section, orientation, role);
}

BlockpicCountTableModel::BlockpicCountTableModel(QObject *parent)
    : ComprehensiveTableModel(parent)
{

}

BlockpicCountTableModel::BlockpicCountTableModel(int view,
                                                 QSharedPointer<AbstractModelInstance> modelInstance,
                                                 QObject *parent)
    : ComprehensiveTableModel(view, modelInstance, parent)
{

}

QVariant BlockpicCountTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        if (index.column() == mModelInstance->columnCount(mView)-4 ||
            index.row() == mModelInstance->rowCount(mView)-1) {
            return QChar(mModelInstance->data(index.row(), index.column(), mView).toInt());
        }
        return mModelInstance->data(index.row(), index.column(), mView);
    }
    return QVariant();
}

BlockpicAverageTableModel::BlockpicAverageTableModel(QObject *parent)
    : ComprehensiveTableModel(parent)
{

}

BlockpicAverageTableModel::BlockpicAverageTableModel(int view,
                                                     QSharedPointer<AbstractModelInstance> modelInstance,
                                                     QObject *parent)
    : ComprehensiveTableModel(view, modelInstance, parent)
{

}

QVariant BlockpicAverageTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::TextAlignmentRole) {
        return Qt::AlignRight;
    }
    if (role == Qt::DisplayRole && index.isValid()) {
        if (index.column() == mModelInstance->columnCount(mView)-4 ||
            index.row() == mModelInstance->rowCount(mView)-1) {
            return QChar(mModelInstance->data(index.row(), index.column(), mView).toInt());
        }
        return mModelInstance->data(index.row(), index.column(), mView);
    }
    return QVariant();
}

}
}
}
