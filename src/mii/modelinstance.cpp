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
#include "modelinstance.h"
#include "datahandler.h"
#include "datamatrix.h"
#include "labeltreeitem.h"
#include "numerics.h"

#include <QAbstractItemModel>
#include <QVector>

#include <QDebug>

namespace gams {
namespace studio {
namespace mii {

ModelInstance::ModelInstance(bool useOutput,
                             const QString &workspace,
                             const QString &systemDir,
                             const QString &scratchDir)
    : AbstractModelInstance(workspace, systemDir, scratchDir)
    , mDataHandler(new DataHandler(*this))
{
    setUseOutput(useOutput);
    initialize();
    loadScratchData();
}

ModelInstance::~ModelInstance()
{
    if (mGMO) gmoFree(&mGMO);
    if (mGEV) gevFree(&mGEV);
    // don't delete mDCT... it is handled in GMO
    delete mDataHandler;
    qDeleteAll(mEquations);
    qDeleteAll(mVariables);
}

QString ModelInstance::modelName() const
{
    if (mGMO) {
        char name[GMS_SSSIZE];
        gmoNameModel(mGMO, name);
        return name;
    }
    return QString();
}

Symbol* ModelInstance::equation(int sectionIndex) const
{
    if (sectionIndex >= vSectionIndexToSymbol.size())
        return nullptr;
    return vSectionIndexToSymbol[sectionIndex];
}

const QVector<Symbol*>& ModelInstance::equations() const
{
    return mEquations;
}

int ModelInstance::equationCount() const
{
    return mEquations.count();
}

int ModelInstance::equationCount(ValueHelper::EquationType type) const
{
    switch (type) {
    case ValueHelper::EquationType::E:
        return gmoGetEquTypeCnt(mGMO, gmoequ_E);
    case ValueHelper::EquationType::G:
        return gmoGetEquTypeCnt(mGMO, gmoequ_G);
    case ValueHelper::EquationType::L:
        return gmoGetEquTypeCnt(mGMO, gmoequ_L);
    case ValueHelper::EquationType::N:
        return gmoGetEquTypeCnt(mGMO, gmoequ_N);
    case ValueHelper::EquationType::X:
        return gmoGetEquTypeCnt(mGMO, gmoequ_X);
    case ValueHelper::EquationType::C:
        return gmoGetEquTypeCnt(mGMO, gmoequ_C);
    case ValueHelper::EquationType::B:
        return gmoGetEquTypeCnt(mGMO, gmoequ_B);
    default:
        return 0;
    }
}

unsigned char ModelInstance::equationType(int row) const
{
    char buffer[GMS_SSSIZE];
    gmoGetEquTypeTxt(mGMO, row, buffer);
    return QString(buffer).replace('=', "").trimmed().at(0).toLatin1();
}

int ModelInstance::equationRowCount() const
{
    return gmoM(mGMO);
}

int ModelInstance::variableCount() const
{
    return mVariables.count();
}

int ModelInstance::variableCount(ValueHelper::VariableType type) const
{
    switch (type) {
    case ValueHelper::VariableType::X:
        return gmoGetVarTypeCnt(mGMO, gmovar_X);
    case ValueHelper::VariableType::B:
        return gmoGetVarTypeCnt(mGMO, gmovar_B);
    case ValueHelper::VariableType::I:
        return gmoGetVarTypeCnt(mGMO, gmovar_I);
    case ValueHelper::VariableType::S1:
        return gmoGetVarTypeCnt(mGMO, gmovar_S1);
    case ValueHelper::VariableType::S2:
        return gmoGetVarTypeCnt(mGMO, gmovar_S2);
    case ValueHelper::VariableType::SC:
        return gmoGetVarTypeCnt(mGMO, gmovar_SC);
    case ValueHelper::VariableType::SI:
        return gmoGetVarTypeCnt(mGMO, gmovar_SI);
    default:
        return 0;
    }
}

char ModelInstance::variableType(int column) const
{
    char buffer[GMS_SSSIZE];
    gmoGetVarTypeTxt(mGMO, column, buffer);
    return QString(buffer).at(0).toLatin1();
}

int ModelInstance::variableRowCount() const
{
    return gmoN(mGMO);
}

QString ModelInstance::longestEquationText() const
{
    return mLongestEqnText;
}

QString ModelInstance::longestVariableText() const
{
    return mLongestVarText;
}

QString ModelInstance::longestLabelText() const
{
    return mLongestLabel;
}

Symbol* ModelInstance::variable(int sectionIndex) const
{
    if (sectionIndex >= hSectionIndexToSymbol.size())
        return nullptr;
    return hSectionIndexToSymbol[sectionIndex];
}

const QVector<Symbol*>& ModelInstance::variables() const
{
    return mVariables;
}

int ModelInstance::symbolCount() const {
    return dctNLSyms(mDCT);
}

int ModelInstance::maximumEquationDimension() const
{
    return mMaxEquationDimension;
}

int ModelInstance::maximumVariableDimension() const
{
    return mMaxVariableDimension;
}

double ModelInstance::modelMinimum() const
{
    return mDataHandler->modelMinimum();
}

double ModelInstance::modelMaximum() const
{
    return mDataHandler->modelMaximum();
}

void ModelInstance::loadScratchData()
{
    if (mState == Error)
        return;
    mLogMessages << "Model Workspace: " + mWorkspace;
    QString ctrlFile = mScratchDir + "/" + FileHelper::GamsCntr;
    mLogMessages << "CTRL File: " + ctrlFile;
    if (gevInitEnvironmentLegacy(mGEV, ctrlFile.toStdString().c_str())) {
        mLogMessages << "ERROR: Could not initialize model instance";
        mState = Error;
        return;
    }

    char msg[GMS_SSSIZE];
    gmoRegisterEnvironment(mGMO, mGEV, msg);
    if (gmoLoadDataLegacy(mGMO, msg)) {
        mLogMessages << "ERROR: Could not load model instance (input): " + QString(msg);
        mState = Error;
        return;
    }

    if (mUseOutput) {
        QString solFile = mScratchDir + "/" + FileHelper::GamsSolu;
        mLogMessages << "Solution File: " + solFile;
        gmoNameSolFileSet(mGMO, solFile.toStdString().c_str());
        if (gmoLoadSolutionLegacy(mGMO)) {
            mLogMessages << "ERROR: Could not load model instance (output): " + QString(msg);
            mState = Error;
            return;
        }
    }

    mDCT = (dctHandle_t)gmoDict(mGMO);
    if (!mDCT) {
        mLogMessages << "ERROR: Could not load dictionary file.";
        mState = Error;
        return;
    }

    mLogMessages << "Absolute Scratch Path: " + mScratchDir;
}

void ModelInstance::loadEvaluationPoint(double *evalPoint, int size)
{
    int i = 0;
    double value;
    for (auto var : variables()) {
        if (i >= size)
            continue;
        if (var->isScalar()) {
            value = variableAttribute(AttributeHelper::LevelText, var->firstSection(), 0, false).toDouble();
            evalPoint[i++] = value;
        } else {
            for (auto e=var->firstSection(); e<var->entries(); ++e) {
                value = variableAttribute(AttributeHelper::LevelText, var->firstSection(), e, false).toDouble();
                evalPoint[i++] = value;
            }
        }
    }
}

void ModelInstance::loadSymbols()
{
    int eqnIndex = 0, varIndex = 0;
    int sectionIndexEqn = 0, sectionIndexVar = 0;
    for (int i=1; i<=symbolCount(); ++i) {
        auto sym = loadSymbol(i);
        if (Symbol::Equation == sym->type()) {
            mMaxEquationDimension = std::max(mMaxEquationDimension, sym->dimension());
            sym->setFirstSection(sectionIndexEqn);
            sym->setLogicalIndex(eqnIndex++);
            sectionIndexEqn += sym->entries();
            loadEquationDimensions(sym);
            sym->setLabelTree(QSharedPointer<LabelTreeItem>(new LabelTreeItem));
            mEquations.append(sym);
            for (int i=sym->firstSection(); i<=sym->lastSection(); ++i) {
                vSectionIndexToSymbol.append(sym);
            }
            if (sym->name().size() > mLongestEqnText.size()) {
                mLongestEqnText = sym->name();
            }
        } else if (Symbol::Variable == sym->type()) {
            mMaxVariableDimension = std::max(mMaxVariableDimension, sym->dimension());
            sym->setFirstSection(sectionIndexVar);
            sym->setLogicalIndex(varIndex++);
            sectionIndexVar += sym->entries();
            loadVariableDimensions(sym);
            sym->setLabelTree(QSharedPointer<LabelTreeItem>(new LabelTreeItem));
            mVariables.append(sym);
            for (int i=sym->firstSection(); i<=sym->lastSection(); ++i) {
                hSectionIndexToSymbol.append(sym);
            }
            if (sym->name().size() > mLongestVarText.size()) {
                mLongestVarText = sym->name();
            }
        } else {
            delete sym;
        }
    }
}

Symbol* ModelInstance::loadSymbol(int index)
{
    auto sym = new Symbol;
    if (index > symbolCount())
        return sym;

    sym->setOffset(dctSymOffset(mDCT, index));
    sym->setDimension(dctSymDim(mDCT, index));
    sym->setEntries(dctSymEntries(mDCT, index));

    char symbolName[GMS_SSSIZE];
    if (dctSymName(mDCT, index, symbolName, GMS_SSSIZE)) {
        sym->setName("##ERROR##");
    } else {
        sym->setName(symbolName);
    }

    auto type = dctSymType(mDCT, index);
    if (type == dcteqnSymType) {
        sym->setType(Symbol::Equation);
    } else if (type == dctvarSymType) {
        sym->setType(Symbol::Variable);
    } else {
        mLogMessages << QString("ERROR: loadSymbol() >> Unknown symbol type (%1) found in ModelInstance::loadSymbol()").arg(sym->name());
        sym->setType(Symbol::Unknown);
        return sym;
    }

    int count = 0;
    gdxStrIndex_t gdxIndex;
    gdxStrIndexPtrs_t gdxIndexPtr;
    GDXSTRINDEXPTRS_INIT(gdxIndex, gdxIndexPtr);
    if (dctSymDomNames(mDCT, index, gdxIndexPtr, &count)) {
        mLogMessages << QString("ERROR: Could not load symbol (%1) domains.").arg(sym->name());
    } else {
        for (int i=0; i<count; ++i) {
            sym->appendDomainLabel(gdxIndexPtr[i]);
        }
    }

    return sym;
}

void ModelInstance::loadEquationDimensions(Symbol *symbol)
{
    char quote;
    int nDomains = 0;
    char labelName[GMS_SSSIZE];
    int domains[GLOBAL_MAX_INDEX_DIM];
    symbol->dimLabels() = QVector<QSet<QString>>(symbol->dimension());
    for (int j=0; j<symbol->entries(); ++j) {
        if (gmoGetiSolverQuiet(mGMO, symbol->offset() + j) < 0) {
            mLogMessages << "ERROR: calling gmoGetiSolverQuiet() in ModelInstance::loadDimensions()";
            continue;
        }
        int symIndex;
        if (dctRowUels(mDCT, symbol->offset()+j, &symIndex, domains, &nDomains)) {
            mLogMessages << "ERROR: calling dctRowUels() in ModelInstance::loadDimensions()";
            continue;
        }
        QStringList labels(symbol->dimension());
        // TODO AF: use the universe and lazy load the uels... use int indicies everywhere
        for (int k=0; k<nDomains; ++k) {
            dctUelLabel(mDCT, domains[k], &quote, labelName, GMS_SSSIZE);
            labels[k] = labelName;
            symbol->dimLabels()[k].insert(labelName);
        }
        symbol->sectionLabels()[symbol->firstSection()+j] = std::move(labels);
    }
}

void ModelInstance::loadVariableDimensions(Symbol *symbol)
{
    char quote;
    int nDomains = 0;
    char labelName[GMS_SSSIZE];
    int domains[GLOBAL_MAX_INDEX_DIM];
    symbol->dimLabels() = QVector<QSet<QString>>(symbol->dimension());
    for (int j=0; j<symbol->entries(); ++j) {
        if (gmoGetjSolverQuiet(mGMO, symbol->offset() + j) < 0) {
            mLogMessages << "ERROR: calling gmoGetjSolverQuiet() in ModelInstance::loadDimensions()";
            continue;
        }
        int symIndex;
        if (dctColUels(mDCT, symbol->offset()+j, &symIndex, domains, &nDomains)) {
            mLogMessages << "ERROR: calling dctColUels() in ModelInstance::loadDimensions()";
            continue;
        }
        QStringList labels(symbol->dimension());
        // TODO AF: use the universe and lazy load the uels... use int indicies everywhere
        for (int k=0; k<nDomains; ++k) {
            dctUelLabel(mDCT, domains[k], &quote, labelName, GMS_SSSIZE);
            labels[k] = labelName;
            symbol->dimLabels()[k].insert(labelName);
        }
        symbol->sectionLabels()[symbol->firstSection()+j] = std::move(labels);
    }
}

const QVector<Symbol*>& ModelInstance::symbols(Symbol::Type type) const
{
    return type == Symbol::Equation ? mEquations : mVariables;
}

void ModelInstance::loadBaseData()
{
    loadSymbols();
    loadLabels();
    mDataHandler->loadJacobian();
}

void ModelInstance::variableLowerBounds(double *bounds)
{
    if (gmoGetVarLower(mGMO, bounds)) {
        mLogMessages << "variableLowerBounds() -> Something went wrong!";
    }
}

void ModelInstance::variableUpperBounds(double *bounds)
{
    if (gmoGetVarUpper(mGMO, bounds)) {
        mLogMessages << "variableUpperBounds() -> Something went wrong!";
    }
}

double ModelInstance::rhs(int row) const
{
    return gmoGetRhsOne(mGMO, row);
}

int ModelInstance::rowCount(int viewId) const
{
    return mDataHandler->rowCount(viewId);
}

int ModelInstance::rowEntryCount(int row, int viewId) const
{
    return mDataHandler->rowEntryCount(row, viewId);
}

int ModelInstance::columnCount(int viewId) const
{
    return mDataHandler->columnCount(viewId);
}

int ModelInstance::columnEntryCount(int column, int viewId) const
{
    return mDataHandler->columnEntryCount(column, viewId);
}

const QList<int> &ModelInstance::rowIndices(int viewId, int row) const
{
    return mDataHandler->rowIndices(viewId, row);
}

const QList<int> &ModelInstance::columnIndices(int viewId, int column) const
{
    return mDataHandler->columnIndices(viewId, column);
}

int ModelInstance::symbolRowCount(int viewId) const
{
    return mDataHandler->symbolRowCount(viewId);
}

int ModelInstance::symbolColumnCount(int viewId) const
{
    return mDataHandler->symbolColumnCount(viewId);
}

QSharedPointer<AbstractViewConfiguration> ModelInstance::clone(int viewId, int newViewId)
{
    return mDataHandler->clone(viewId, newViewId);
}

void ModelInstance::loadViewData(const QSharedPointer<AbstractViewConfiguration>& viewConfig)
{
    return mDataHandler->loadData(viewConfig);
}

void ModelInstance::loadLabels()
{
    char q;
    char label[GMS_SSSIZE];
    for (int i=1; i<=dctNUels(mDCT); ++i) {
        dctUelLabel(mDCT, i, &q, label, GMS_SSSIZE);
        mLabels << label;
    }
    const QString ttlblk = "ttlblk";
    const QString mincolcnt = "mincolcnt";
    const QString minrowcnt = "minrowcnt";
    for (int i=0; i<3 && !mLabels.isEmpty(); ++i) {
        auto label = mLabels.last();
        if (label == ttlblk || label == mincolcnt || label == minrowcnt)
            mLabels.removeLast();
    }
    for (const auto& label : std::as_const(mLabels)) {
        if (label.size() > mLongestLabel.size())
            mLongestLabel = label;
    }
}

QVariant ModelInstance::data(int row, int column, int viewId) const
{
    return mDataHandler->data(row, column, viewId);
}

int ModelInstance::nlFlag(int row, int column, int viewId)
{
    return mDataHandler->nlFlag(row, column, viewId);
}

QSharedPointer<PostoptTreeItem> ModelInstance::dataTree(int viewId) const
{
    return mDataHandler->dataTree(viewId);
}

QVariant ModelInstance::headerData(int logicalIndex,
                                   Qt::Orientation orientation,
                                   int viewId,
                                   int role) const
{
    if (role == ViewHelper::IndexDataRole) {
        return mDataHandler->headerData(logicalIndex, orientation, viewId);
    }
    if (role == ViewHelper::LabelDataRole) {
        return mDataHandler->plainHeaderData(orientation, viewId, logicalIndex, 0);
    }
    if (role == ViewHelper::SectionLabelRole) {
        return mDataHandler->sectionLabels(orientation, viewId, logicalIndex);
    }
    return QVariant();
}

QVariant ModelInstance::plainHeaderData(Qt::Orientation orientation,
                                        int viewId,
                                        int logicalIndex,
                                        int dimension) const
{
    return mDataHandler->plainHeaderData(orientation, viewId, logicalIndex, dimension);
}

void ModelInstance::initialize()
{
    gevSetExitIndicator(0); // switch of lib exit() call
    gevSetScreenIndicator(0); // switch off std lib output
    gevSetErrorCallback(ModelInstance::errorCallback);

    mLogMessages << "GAMS System Dir: " + mSystemDir;

    char msg[GMS_SSSIZE];
    if (!gevCreateD(&mGEV,
                    mSystemDir.toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        mLogMessages << "ERROR: " + QString(msg);
        mState = Error;
        return;
    }

    gmoSetExitIndicator(0); // switch of lib exit() call
    gmoSetScreenIndicator(0); // switch off std lib output
    gmoSetErrorCallback(ModelInstance::errorCallback);

    if (!gmoCreateD(&mGMO,
                    mSystemDir.toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        mLogMessages << "ERROR: " + QString(msg);
        mState = Error;
        return;
    }

    if (gmoHaveBasis(mGMO)){
        specialMarginalEquValuePtr = std::bind(&ModelInstance::specialMarginalEquValueBasis,
                                               this, std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);
        specialMarginalVarValuePtr = std::bind(&ModelInstance::specialMarginalVarValueBasis,
                                               this, std::placeholders::_1,
                                               std::placeholders::_2,
                                               std::placeholders::_3);
    } else {
        specialMarginalEquValuePtr = std::bind(&ModelInstance::specialValuePostopt,
                                               this, std::placeholders::_1,
                                               std::placeholders::_2);
        specialMarginalVarValuePtr = std::bind(&ModelInstance::specialValuePostopt,
                                               this, std::placeholders::_1,
                                               std::placeholders::_2);
    }

    dctSetExitIndicator(0); // switch of lib exit() call
    dctSetScreenIndicator(0); // switch off std lib output
    dctSetErrorCallback(ModelInstance::errorCallback);

    if (!dctCreateD(&mDCT,
                    mSystemDir.toStdString().c_str(),
                    msg,
                    sizeof(msg))) {
        mLogMessages << "ERROR: " + QString(msg);
        mState = Error;
        return;
    }
}

DataMatrix* ModelInstance::jacobianData()
{
    int nz = 0, nlnz = 0, unused1 = 0;
    auto matrix = new DataMatrix(equationRowCount(), variableRowCount(), gmoNLM(mGMO));
    loadEvaluationPoint(matrix->evalPoint(), matrix->columnCount());
    if (matrix->isLinear()) {
        for (int row=0; row<equationRowCount(); ++row) {
            if (gmoGetRowStat(mGMO, row, &nz, &unused1, &nlnz))
                continue;
            auto* dataRow = matrix->row(row);
            dataRow->setEntries(nz);
            dataRow->setEntriesNl(nlnz);
            dataRow->setColIdx(new int[nz]);
            dataRow->setInputData(new double[nz]);
            dataRow->setNlFlags(new int[nz]);
            if (gmoGetRowSparse(mGMO, row, dataRow->colIdx(), dataRow->inputData(),
                                dataRow->nlFlags(), &unused1, &nlnz)) {
                continue;
            }
        }
    } else {
        double* scratch = new double[matrix->columnCount()];
        for (int row=0; row<equationRowCount(); ++row) {
            if (gmoGetRowStat(mGMO, row, &nz, &unused1, &nlnz))
                continue;
            auto* dataRow = matrix->row(row);
            dataRow->setEntries(nz);
            dataRow->setEntriesNl(nlnz);
            dataRow->setColIdx(new int[nz]);
            dataRow->setInputData(new double[nz]);
            dataRow->setOutputData(new double[nz]);
            dataRow->setNlFlags(new int[nz]);
            if (gmoGetRowSparse(mGMO, row, dataRow->colIdx(), dataRow->inputData(),
                                dataRow->nlFlags(), &unused1, &nlnz)) {
                continue;
            }
            std::copy(dataRow->inputData(), dataRow->inputData()+dataRow->entries(), dataRow->outputData());
            if (!dataRow->entriesNl()) {
                continue;
            }
            int numerr = 0;
            double fnl = 0, gxnl = 0; // not needed
            if (gmoEvalGradNL(mGMO, row, matrix->evalPoint(), &fnl, scratch, &gxnl, &numerr)) {
                mLogMessages << QString("Gradient evaluation in Line %1 failed. Please check your model").arg(row);
                mState = Error;
                continue;
            }
            for (int c=0; c<dataRow->entries(); ++c) {
                if (dataRow->nlFlags()[c]) {
                    dataRow->outputData()[c] = scratch[dataRow->colIdx()[c]];
                }
            }
        }
        delete[] scratch;
    }
    return matrix;
}

QVariant ModelInstance::equationAttribute(const QString &header, int index, int entry, bool abs) const
{
    double value = 0.0;
    int absoluteIndex = index + entry;
    if (!header.compare(AttributeHelper::LevelText, Qt::CaseInsensitive)) {
        value = gmoGetEquLOne(mGMO, absoluteIndex);
    } else if (!header.compare(AttributeHelper::LowerText, Qt::CaseInsensitive)) {
        auto bounds = equationBounds(absoluteIndex);
        value = bounds.first;
    } else if (!header.compare(AttributeHelper::MarginalText, Qt::CaseInsensitive)) {
        value = gmoGetEquMOne(mGMO, absoluteIndex);
        return specialMarginalEquValuePtr(value, absoluteIndex, abs);
    } else if (!header.compare(AttributeHelper::MarginalNumText, Qt::CaseInsensitive)) {
        value = specialValue(gmoGetEquMOne(mGMO, absoluteIndex));
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::ScaleText, Qt::CaseInsensitive)) {
        value = gmoGetEquScaleOne(mGMO, absoluteIndex);
    } else if (!header.compare(AttributeHelper::UpperText, Qt::CaseInsensitive)) {
        auto bounds = equationBounds(absoluteIndex);
        value = bounds.second;
    } else if (!header.compare(AttributeHelper::InfeasibilityText, Qt::CaseInsensitive)) {
        double a = specialValue(equationBounds(absoluteIndex).first);
        double b = specialValue(gmoGetEquLOne(mGMO, absoluteIndex));
        double v1 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        a = specialValue(gmoGetEquLOne(mGMO, absoluteIndex));
        b = specialValue(equationBounds(absoluteIndex).second);
        double v2 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        value = std::max(0.0, std::max(v1, v2));
        value = abs ? std::abs(value) : value;
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::RangeText, Qt::CaseInsensitive)) {
        double a = specialValue(equationBounds(absoluteIndex).second);
        double b = specialValue(equationBounds(absoluteIndex).first);
        value = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
    } else if (!header.compare(AttributeHelper::SlackText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetEquLOne(mGMO, absoluteIndex));
        double b = specialValue(equationBounds(absoluteIndex).first);
        double v1 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        v1 = std::max(0.0, v1);
        v1 = abs ? std::abs(v1) : v1;
        a = specialValue(equationBounds(absoluteIndex).second);
        b = specialValue(gmoGetEquLOne(mGMO, absoluteIndex));
        double v2 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        v2 = std::max(0.0, v2);
        v2 = abs ? std::abs(v2) : v2;
        value = std::min(v1, v2);
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::SlackLBText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetEquLOne(mGMO, absoluteIndex));
        double b = specialValue(equationBounds(absoluteIndex).first);
        value = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        value = std::max(0.0, value);
        value = abs ? std::abs(value) : value;
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::SlackUBText, Qt::CaseInsensitive)) {
        double a = specialValue(equationBounds(absoluteIndex).second);
        double b = specialValue(gmoGetEquLOne(mGMO, absoluteIndex));
        value = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        value = std::max(0.0, value);
        value = abs ? std::abs(value) : value;
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::TypeText, Qt::CaseInsensitive)) {
        return QChar(equationType(index));
    } else {
        return "## Undefined ##";
    }
    value = abs ? std::abs(value) : value;
    return specialValuePostopt(value, abs);
}

QVariant ModelInstance::variableAttribute(const QString &header, int index, int entry, bool abs) const
{
    double value = 0.0;
    int absoluteIndex = index + entry;
    if (!header.compare(AttributeHelper::LevelText, Qt::CaseInsensitive)) {
        value = gmoGetVarLOne(mGMO, absoluteIndex);
    } else if (!header.compare(AttributeHelper::LowerText, Qt::CaseInsensitive)) {
        value = gmoGetVarLowerOne(mGMO, absoluteIndex);
    } else if (!header.compare(AttributeHelper::MarginalText, Qt::CaseInsensitive)) {
        value = gmoGetVarMOne(mGMO, absoluteIndex);
        return specialMarginalVarValuePtr(value, absoluteIndex, abs);
    } else if (!header.compare(AttributeHelper::ScaleText, Qt::CaseInsensitive)) {
        value = gmoGetVarScaleOne(mGMO, absoluteIndex);
    } else if (!header.compare(AttributeHelper::UpperText, Qt::CaseInsensitive)) {
        value = gmoGetVarUpperOne(mGMO, absoluteIndex);
    } else if (!header.compare(AttributeHelper::InfeasibilityText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetVarLowerOne(mGMO, absoluteIndex));
        double b = specialValue(gmoGetVarLOne(mGMO, absoluteIndex));
        double v1 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        a = specialValue(gmoGetVarLOne(mGMO, absoluteIndex));
        b = specialValue(gmoGetVarUpperOne(mGMO, absoluteIndex));
        double v2 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        value = std::max(0.0, std::max(v1, v2));
        value = abs ? std::abs(value) : value;
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::RangeText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetVarUpperOne(mGMO, absoluteIndex));
        double b = specialValue(gmoGetVarLowerOne(mGMO, absoluteIndex));
        value = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
    } else if (!header.compare(AttributeHelper::SlackText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetVarLOne(mGMO, absoluteIndex));
        double b = specialValue(gmoGetVarLowerOne(mGMO, absoluteIndex));
        double v1 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        v1 = std::max(0.0, v1);
        v1 = abs ? std::abs(v1) : v1;
        a = specialValue(gmoGetVarUpperOne(mGMO, absoluteIndex));
        b = specialValue(gmoGetVarLOne(mGMO, absoluteIndex));
        double v2 = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        v2 = std::max(0.0, v2);
        v2 = abs ? std::abs(v2) : v2;
        value = std::min(v1, v2);
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::SlackLBText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetVarLOne(mGMO, absoluteIndex));
        double b = specialValue(gmoGetVarLowerOne(mGMO, absoluteIndex));
        value = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        value = std::max(0.0, value);
        value = abs ? std::abs(value) : value;
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::SlackUBText, Qt::CaseInsensitive)) {
        double a = specialValue(gmoGetVarUpperOne(mGMO, absoluteIndex));
        double b = specialValue(gmoGetVarLOne(mGMO, absoluteIndex));
        value = AttributeHelper::attributeValue(a, b, isInf(a), isInf(b));
        value = std::max(0.0, value);
        value = abs ? std::abs(value) : value;
        if (isInf(value))
            return specialValuePostopt(value, abs);
        return DoubleFormatter::format(value, DoubleFormatter::g, 6, true);
    } else if (!header.compare(AttributeHelper::TypeText, Qt::CaseInsensitive)) {
        auto type = QChar(variableType(index));
        if (type == 'x') { // x = continuous
            if (gmoGetVarLowerOne(mGMO, absoluteIndex) >= 0 && gmoGetVarUpperOne(mGMO, absoluteIndex) >= 0) {
                return QChar('+');
            } else if (gmoGetVarLowerOne(mGMO, absoluteIndex) <= 0 && gmoGetVarUpperOne(mGMO, absoluteIndex) <= 0) {
                return QChar('-');
            } else {
                return QChar('u');
            }
        }
        return QChar(variableType(index));
    } else {
        return "## Undefined ##";
    }
    value = abs ? std::abs(value) : value;
    return specialValuePostopt(value, abs);
}

int ModelInstance::maxSymbolDimension(int viewId, Qt::Orientation orientation) const
{
    return mDataHandler->maxSymbolDimension(viewId, orientation);
}

void ModelInstance::removeViewData(int viewId)
{
    mDataHandler->removeViewData(viewId);
}

void ModelInstance::removeViewData()
{
    mDataHandler->removeViewData();
}

QPair<double, double> ModelInstance::equationBounds(int row) const
{
    QPair<double, double> bounds;
    switch (gmoGetEquTypeOne(mGMO, row)) {
    case gmoequ_B:
    case gmoequ_E:
        bounds.first = gmoGetRhsOne(mGMO, row);
        bounds.second = gmoGetRhsOne(mGMO, row);
        break;
    case gmoequ_C:
    case gmoequ_G:
        bounds.first = gmoGetRhsOne(mGMO, row);
        bounds.second = gmoPinf(mGMO);
        break;
    case gmoequ_L:
        bounds.first = gmoMinf(mGMO);
        bounds.second = gmoGetRhsOne(mGMO, row);
        break;
    case gmoequ_N:
        bounds.first = gmoMinf(mGMO);
        bounds.second = gmoPinf(mGMO);
        break;
    case gmoequ_X:
        bounds.first = 0.0;
        bounds.second = 0.0;
        break;
    }
    return bounds;
}

bool ModelInstance::isInf(double value) const
{
    return gmoPinf(mGMO) == value || gmoMinf(mGMO) == value;
}

double ModelInstance::specialValue(double value) const
{
    return GMS_SV_EPS == value ? 0.0 : value;
}

QVariant ModelInstance::specialValuePostopt(double value, bool abs) const
{
    if (gmoPinf(mGMO) == value)
        return ValueHelper::PINFText;
    else if (gmoMinf(mGMO) == value)
        return ValueHelper::NINFText;
    else if (GMS_SV_EPS == value)
        return ValueHelper::EPSText;
    auto val = abs ? std::abs(value) : value;
    return DoubleFormatter::format(val, DoubleFormatter::g, 6, true);
}

bool ModelInstance::isSpecialValue(double value) const
{
    return gmoPinf(mGMO) == value || gmoMinf(mGMO) == value || GMS_SV_EPS == value;
}

QVariant ModelInstance::specialMarginalEquValueBasis(double value, int rIndex, bool abs)
{
    if (gmoGetEquStatOne(mGMO, rIndex) != gmoBstat_Basic && value == 0.0)
        return ValueHelper::EPSText;
    return specialValuePostopt(value, abs);
}

QVariant ModelInstance::specialMarginalVarValueBasis(double value, int cIndex, bool abs)
{
    if (gmoGetVarStatOne(mGMO, cIndex) != gmoBstat_Basic && value == 0.0)
        return ValueHelper::EPSText;
    return specialValuePostopt(value, abs);
}

int ModelInstance::errorCallback(int count, const char *message)
{
    Q_UNUSED(count)
    qDebug()<< message;
    return 0;
}

}
}
}
