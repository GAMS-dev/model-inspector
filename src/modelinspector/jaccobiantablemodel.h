#ifndef JACCOBIANTABLEMODEL_H
#define JACCOBIANTABLEMODEL_H

#include <QAbstractTableModel>
#include <QSharedPointer>

#include "common.h"

namespace gams {
namespace studio{
namespace modelinspector {

class AbstractModelInstance;

class JaccobianTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit JaccobianTableModel(QObject *parent = nullptr);

    ~JaccobianTableModel();

    void setModelInstance(const QSharedPointer<AbstractModelInstance> &modelInstance);

    QVariant data(const QModelIndex &index, int role) const override;

    Qt::ItemFlags flags(const QModelIndex &index) const override;

    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;

    int view() const;

    void setView(int view);

private:
    int mView;
    ViewDataType mViewType;
    QSharedPointer<AbstractModelInstance> mModelInstance;
};

}
}
}

#endif // JACCOBIANTABLEMODEL_H
