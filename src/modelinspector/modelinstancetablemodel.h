#ifndef MODELINSTANCETABLEMODEL_H
#define MODELINSTANCETABLEMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "common.h"

namespace gams {
namespace studio{
namespace modelinspector {

class ModelInstance;

class ModelInstanceTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit ModelInstanceTableModel(QObject *parent = nullptr);

    ~ModelInstanceTableModel();

    void setModelInstance(const QSharedPointer<ModelInstance> &modelInstance);

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int view() const
    {
        return mView;
    }

    void setView(int view)
    {
        mView = view;
    }

private:
    PredefinedViewEnum mViewType;
    QSharedPointer<ModelInstance> mModelInstance;
    int mView;
};

}
}
}

#endif // MODELINSTANCETABLEMODEL_H
