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

#include "symbol.h"
#include "labeltreeitem.h"

using namespace gams::studio::mii;

Q_DECLARE_METATYPE(LabelTreeItem)
Q_DECLARE_METATYPE(Symbol)

class TestSymbol : public QObject
{
    Q_OBJECT

private slots:
    void test_default();
    void test_getSet();
    void test_comparison();
    void test_appendDomainLabels();
};

void TestSymbol::test_default()
{
    Symbol symbol;
    QCOMPARE(symbol.offset(), -1);
    QCOMPARE(symbol.entries(), -1);
    QCOMPARE(symbol.name(), QString());
    QCOMPARE(symbol.dimension(), -1);
    QCOMPARE(symbol.type(), Symbol::Unknown);
    QCOMPARE(symbol.sectionLabels(), SectionLabels());
    QCOMPARE(symbol.label(0,0), QString());
    QCOMPARE(symbol.contains(0), false);
    QCOMPARE(symbol.isScalar(), false);
    QVERIFY(symbol.lastOffset()<0);
    QCOMPARE(symbol.firstSection(), -1);
    QVERIFY(symbol.lastSection()<0);
    QCOMPARE(symbol.labelTree(),  nullptr);
    QCOMPARE(symbol.isEquation(), false);
    QCOMPARE(symbol.isVariable(), false);
    QCOMPARE(symbol.domainLabels(), DomainLabels());
}

void TestSymbol::test_getSet()
{
    int i = 0;
    const int value = 42;
    Symbol symbol;

    symbol.setOffset(value+i);
    QCOMPARE(symbol.offset(), value+i);

    ++i;
    symbol.setEntries(value+i);
    QCOMPARE(symbol.entries(), value+i);

    symbol.setName("lala");
    QCOMPARE(symbol.name(), "lala");

    ++i;
    symbol.setDimension(value+i);
    QCOMPARE(symbol.dimension(), value+i);

    ++i;
    symbol.setType(Symbol::Equation);
    QCOMPARE(symbol.type(), Symbol::Equation);
    QVERIFY(symbol.isEquation());
    symbol.setType(Symbol::Variable);
    QCOMPARE(symbol.type(), Symbol::Variable);
    QVERIFY(symbol.isVariable());

    ++i;
    symbol.setFirstSection(value+i);
    QCOMPARE(symbol.firstSection(), value+i);
    QCOMPARE(symbol.lastSection(), value+i+symbol.entries()-1);
    QVERIFY(symbol.contains(value+i));
    QVERIFY(symbol.contains(symbol.lastSection()));

    ++i;
    QSharedPointer<LabelTreeItem> labelTree(new LabelTreeItem);
    symbol.setLabelTree(labelTree);
    QVERIFY(symbol.labelTree() == labelTree);

    DomainLabels dLabels { "i", "j"};
    symbol.setDomainLabels(dLabels);
    QCOMPARE(symbol.domainLabels(), dLabels);
}

void TestSymbol::test_comparison()
{
    QCOMPARE(Symbol(), Symbol());
    Symbol symbol1;
    symbol1.setName("lala");
    Symbol symbol2;
    symbol2.setName("lala");
    QCOMPARE(symbol1, symbol2);
    symbol2.setDimension(42);
    QVERIFY(symbol1 != symbol2);
    QVERIFY(symbol2 != Symbol());
}

void TestSymbol::test_appendDomainLabels()
{
    Symbol symbol;
    symbol.appendDomainLabel("j");
    symbol.appendDomainLabel("k");
    DomainLabels dLabels { "j", "k" };
    QCOMPARE(symbol.domainLabels(), dLabels);
}

QTEST_APPLESS_MAIN(TestSymbol)

#include "tst_testsymbol.moc"
