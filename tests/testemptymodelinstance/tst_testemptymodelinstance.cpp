/**
 * GAMS Model Instance Inspector (MII)
 *
 * Copyright (c) 2023 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2023 GAMS Development Corp. <support@gams.com>
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
 *
 */
#include <QtTest>

#include "abstractmodelinstance.h"

using namespace gams::studio::mii;

class TestEmptyModelInstance : public QObject
{
    Q_OBJECT

private slots:
    void test_constructor_initialize();
    void test_default();
    void test_getSet();
};

void TestEmptyModelInstance::test_constructor_initialize()
{
    const QString workspace = "myWorkDir";
    const QString systemDir = "mySysDir";
    const QString scratchDir = "myScratchDir";
    EmptyModelInstance instance(workspace, systemDir, scratchDir);
    auto realWorkspace = QDir(workspace).absolutePath();
    QCOMPARE(instance.workspace(), realWorkspace);
    QCOMPARE(instance.systemDirectory(), systemDir);
    QCOMPARE(instance.scratchDirectory(), scratchDir);
    QCOMPARE(instance.maxSymbolDimension(0, Qt::Horizontal), 0);
    QCOMPARE(instance.maxSymbolDimension(0, Qt::Vertical), 0);
}

void TestEmptyModelInstance::test_default()
{
    EmptyModelInstance instance;
    QCOMPARE(instance.workspace(), QDir(".").absolutePath());
    QCOMPARE(instance.systemDirectory(), QString());
    QCOMPARE(instance.scratchDirectory(), QString());
    QCOMPARE(instance.useOutput(), false);
    QCOMPARE(instance.modelMinimum(), std::numeric_limits<double>::lowest());
    QCOMPARE(instance.modelMaximum(), std::numeric_limits<double>::max());
    QCOMPARE(instance.logMessages(), QString());
    QCOMPARE(instance.modelName(), QString());
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::E), 0);
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::G), 0);
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::L), 0);
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::N), 0);
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::X), 0);
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::C), 0);
    QCOMPARE(instance.equationCount(ValueHelper::EquationType::B), 0);
    QCOMPARE(instance.equationRowCount(), 0);
    QCOMPARE(instance.equation(0), nullptr);
    QVERIFY(instance.equations().isEmpty());
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::X), 0);
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::B), 0);
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::I), 0);
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::S1), 0);
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::S2), 0);
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::SC), 0);
    QCOMPARE(instance.variableCount(ValueHelper::VariableType::SI), 0);
    QCOMPARE(instance.variableRowCount(), 0);
    QCOMPARE(instance.variable(0), nullptr);
    QVERIFY(instance.variables().isEmpty());
    QCOMPARE(instance.longestEquationText(), QString());
    QCOMPARE(instance.longestVariableText(), QString());
    QCOMPARE(instance.maximumEquationDimension(), 0);
    QCOMPARE(instance.maximumVariableDimension(), 0);
    QCOMPARE(instance.symbols(Symbol::Equation), QVector<Symbol*>());
    QCOMPARE(instance.symbols(Symbol::Variable), QVector<Symbol*>());
    QCOMPARE(instance.data(0, 1, 4), QVariant());
    QCOMPARE(instance.headerData(2, Qt::Horizontal, 42, ViewHelper::ItemDataRole::IndexDataRole), QVariant());
    QCOMPARE(instance.headerData(2, Qt::Vertical, 42, 128), QVariant());
    QCOMPARE(instance.plainHeaderData(Qt::Horizontal, 2, 0, 12), QVariant());
    QCOMPARE(instance.plainHeaderData(Qt::Vertical, 2, 0, 12), QVariant());
    QCOMPARE(instance.equationAttribute(QString(), -1, -4, false), QVariant());
    QCOMPARE(instance.variableAttribute(QString(), -1, -4, false), QVariant());
    QVERIFY(instance.dataTree(-42) != nullptr);
    QCOMPARE(instance.maxSymbolDimension(0, Qt::Horizontal), 0);
    QCOMPARE(instance.maxSymbolDimension(0, Qt::Vertical), 0);
    QCOMPARE(instance.rowEntryCount(0, 0), 0);
    QCOMPARE(instance.rowEntryCount(-1, -1), 0);
    QCOMPARE(instance.columnEntryCount(0, 0), 0);
    QCOMPARE(instance.columnEntryCount(-1, -1), 0);
}

void TestEmptyModelInstance::test_getSet()
{
    EmptyModelInstance instance;
    instance.setWorkspace("my_workspace");
    QCOMPARE(instance.workspace(), QDir("my_workspace").absolutePath());
    instance.setSystemDirectory("my_system_dir");
    QCOMPARE(instance.systemDirectory(), "my_system_dir");
    instance.setScratchDirectory("my_scratch_dir");
    QCOMPARE(instance.scratchDirectory(), "my_scratch_dir");
    instance.setUseOutput(true);
    QCOMPARE(instance.useOutput(), true);
}

QTEST_APPLESS_MAIN(TestEmptyModelInstance)

#include "tst_testemptymodelinstance.moc"
