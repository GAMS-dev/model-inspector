#ifndef BPVIEWFRAME_H
#define BPVIEWFRAME_H

#include "abstracttableviewframe.h"

namespace gams {
namespace studio{
namespace modelinspector {

class BPOverviewTableModel;
class BPIdentifierFilterModel;
class ComprehensiveTableModel;
class HierarchicalHeaderView;
class ComprehensiveTableModel;
class ValueFormatProxyModel;
class AbsFormatProxyModel;
class Symbol;

class AbstractBPViewFrame : public AbstractTableViewFrame
{
    Q_OBJECT

public:
    AbstractBPViewFrame(ComprehensiveTableModel *model,
                        QWidget *parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    virtual ~AbstractBPViewFrame();

    const QList<Symbol*>& selectedEquations() const;
    const QList<Symbol*>& selectedVariables() const;

    void setAggregation(const Aggregation &aggregation) override;

    void setIdentifierFilter(const IdentifierFilter &filter) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    void setValueFilter(const ValueFilter &filter) override;

    void reset() override;

signals:
    void newSymbolViewRequested();

protected slots:
    void setIdentifierLabelFilter(const gams::studio::modelinspector::IdentifierState &state,
                                  Qt::Orientation orientation) override;

private slots:
    void customMenuRequested(const QPoint &pos);

    void handleRowColumnSelection();

protected:
    void setIdentifierFilterCheckState(int symbolIndex,
                                       Qt::CheckState state,
                                       Qt::Orientation orientation);

protected:
    QSharedPointer<ComprehensiveTableModel> mBaseModel;
    ValueFormatProxyModel* mValueFormatModel = nullptr; // TODO !!! not needed for overview view
    BPIdentifierFilterModel* mIdentifierFilterModel = nullptr;

    QMenu *mSelectionMenu;
    QAction *mSymbolAction = new QAction("Show selected symbols", this);

    QList<Symbol*> mSelectedEquations;
    QList<Symbol*> mSelectedVariables;
};

class BPOverviewViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPOverviewViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPOverviewViewFrame(QSharedPointer<AbstractModelInstance> modelInstance,
                        QSharedPointer<AbstractViewConfiguration> viewConfig,
                        QWidget *parent = nullptr,
                        Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int view) override;

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance) override;

    void setValueFilter(const ValueFilter &filter) override;

    inline ViewDataType type() const override
    {
        return ViewDataType::BP_Overview;
    }

    void updateView() override;

private:
    void setupView();
};

class BPCountViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPCountViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPCountViewFrame(QSharedPointer<AbstractModelInstance> modelInstance,
                     QSharedPointer<AbstractViewConfiguration> viewConfig,
                     QWidget *parent = nullptr,
                     Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int view) override;

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    inline ViewDataType type() const override
    {
        return ViewDataType::BP_Count;
    }

    void updateView() override;

private:
    void setupView();

private:
    AbsFormatProxyModel* mAbsFormatModel = nullptr;
    HierarchicalHeaderView* mVerticalHeader = nullptr;
};

class BPAverageViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPAverageViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPAverageViewFrame(QSharedPointer<AbstractModelInstance> modelInstance,
                       QSharedPointer<AbstractViewConfiguration> viewConfig,
                       QWidget *parent = nullptr,
                       Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame *clone(int view) override;

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance) override;

    void setShowAbsoluteValues(bool absoluteValues) override;

    inline ViewDataType type() const override
    {
        return ViewDataType::BP_Average;
    }

    void updateView() override;

private:
    void setupView();

private:
    AbsFormatProxyModel* mAbsFormatModel = nullptr;
    HierarchicalHeaderView* mVerticalHeader = nullptr;
};

class BPScalingViewFrame final : public AbstractBPViewFrame
{
    Q_OBJECT

public:
    BPScalingViewFrame(QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    BPScalingViewFrame(QSharedPointer<AbstractModelInstance> modelInstance,
                       QSharedPointer<AbstractViewConfiguration> viewConfig,
                       QWidget *parent = nullptr, Qt::WindowFlags f = Qt::WindowFlags());

    AbstractTableViewFrame* clone(int view) override;

    ViewDataType type() const override
    {
        return ViewDataType::BP_Scaling;
    }

    void setupView(QSharedPointer<AbstractModelInstance> modelInstance) override;

    void updateView() override;

private:
    void setupView();

private:
    HierarchicalHeaderView* mVerticalHeader = nullptr;
};


}
}
}

#endif // BPVIEWFRAME_H
