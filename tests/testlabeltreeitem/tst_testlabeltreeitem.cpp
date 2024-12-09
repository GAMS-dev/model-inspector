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

#include "labeltreeitem.h"

using namespace gams::studio::mii;

class TestLabelTreeItem : public QObject
{
    Q_OBJECT

private slots:
    void test_constructor();
    void test_default();
    void test_getSet();
    void test_visibleChilds();
    void test_clone();
    void test_remove();
    void test_firstSectionIndex();
    void test_siblings();
    void test_sections();
    void test_unitedSections();
    void test_visibleSections();
    void test_visibleSectionsSorted();
    void test_sectionLabels_root_level();
    void test_sectionLabels_dimension_1();
    void test_sectionLabels_dimension_2();
    void test_sectionLabels_dimension_3();
    void test_isVisible();
    void test_sectionExtent();
    void test_visibleBranch();
};

void TestLabelTreeItem::test_constructor()
{
    LabelTreeItem defaultItem;
    LabelTreeItem item("lala", &defaultItem);
    QCOMPARE(item.text(), "lala");
    QCOMPARE(item.parent(), &defaultItem);
}

void TestLabelTreeItem::test_default()
{
    LabelTreeItem item;
    QCOMPARE(item.child(-1), nullptr);
    QCOMPARE(item.child(0), nullptr);
    QVERIFY(item.childs().isEmpty());
    QVERIFY(item.visibleChilds().isEmpty());
    auto clone = item.clone();
    QVERIFY(clone != nullptr);
    delete clone;
    QCOMPARE(item.hasChildren(), false);
    QVERIFY(item.isRoot());
    QCOMPARE(item.firstSectionIndex(), -1);
    QCOMPARE(item.sectionIndex(), -1);
    QCOMPARE(item.parent(), nullptr);
    QVERIFY(item.siblings().isEmpty());
    QCOMPARE(item.size(), 0);
    QVERIFY(item.sections().isEmpty());
    QVERIFY(item.unitedSections().isEmpty());
    QVERIFY(item.visibleSections().isEmpty());
    QVERIFY(item.visibleSectionsSorted().isEmpty());
    QVERIFY(item.sectionLabels(-1, -1).isEmpty());
    QVERIFY(item.sectionLabels(0, 0).isEmpty());
    QVERIFY(item.sectionLabels(1, 1).isEmpty());
    QCOMPARE(item.text(), QString());
    QVERIFY(item.isVisible());
    QCOMPARE(item.sectionExtent(), 0);
}

void TestLabelTreeItem::test_getSet()
{
    LabelTreeItem item;

    auto child = new LabelTreeItem;
    item.append(child);
    QCOMPARE(item.child(0), child);
    QVERIFY(item.hasChildren());
    item.remove(child);
    QVERIFY(!item.hasChildren());
    item.setChilds({child});
    QVERIFY(item.hasChildren());
    item.setChilds({});
    QVERIFY(item.childs().isEmpty());
    delete child;

    item.setSectionIndex(42);
    QCOMPARE(item.sectionIndex(), 42);

    auto parent = new LabelTreeItem;
    item.setParent(parent);
    QCOMPARE(item.parent(), parent);
    item.setParent(nullptr);
    delete parent;

    QSet<int> sections { 1, 2, 3 };
    item.setSections(sections);
    QCOMPARE(item.sections(), sections);

    item.setText("some item");
    QCOMPARE(item.text(), "some item");

    item.setVisible(true);
    QVERIFY(item.isVisible());
}

void TestLabelTreeItem::test_visibleChilds()
{
    auto root1 = new LabelTreeItem("root1");
    auto childs1 = root1->visibleChilds();
    QCOMPARE(childs1.size(), 0);
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    auto child2 = new LabelTreeItem("c1", root2);
    root2->append(child2);
    child2 = new LabelTreeItem("c2", root2);
    root2->append(child2);
    auto childs2 = root2->visibleChilds();
    QCOMPARE(childs2.size(), 2);
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child3 = new LabelTreeItem("c1", root3);
    child3->setVisible(false);
    root3->append(child3);
    child3 = new LabelTreeItem("c2", root3);
    root3->append(child3);
    auto childs3 = root3->visibleChilds();
    QCOMPARE(childs3.size(), 1);
    delete root3;

    auto root4 = new LabelTreeItem("root4");
    auto child4 = new LabelTreeItem("c1", root4);
    child4->setVisible(false);
    root4->append(child4);
    child4 = new LabelTreeItem("c2", root4);
    root4->append(child4);
    auto sub4 = new LabelTreeItem("sub0", child3);
    sub4->setVisible(false);
    child4->append(sub4);
    auto childs4 = root4->visibleChilds();
    QCOMPARE(childs4.size(), 0);
    delete root4;
}

void TestLabelTreeItem::test_clone()
{
    auto source1 = new LabelTreeItem("root1");
    auto clone_1 = source1->clone();
    QCOMPARE(clone_1->text(), "root1");
    QCOMPARE(clone_1->childs().size(), 0);
    QVERIFY(source1 != clone_1);
    delete source1;
    delete clone_1;

    auto source2 = new LabelTreeItem("root2");
    source2->append(new LabelTreeItem("c1", source2));
    source2->append(new LabelTreeItem("c2", source2));
    auto clone_2 = source2->clone();
    QCOMPARE(clone_2->text(), "root2");
    QCOMPARE(clone_2->childs().size(), 2);
    QVERIFY(source2 != clone_2);
    QCOMPARE(clone_2->childs().at(0)->text(), "c1");
    QCOMPARE(clone_2->childs().at(0)->childs().size(), 0);
    QVERIFY(source2->childs().at(0) != clone_2->childs().at(0));
    QCOMPARE(clone_2->childs().at(1)->text(), "c2");
    QCOMPARE(clone_2->childs().at(1)->childs().size(), 0);
    QVERIFY(source2->childs().at(1) != clone_2->childs().at(1));
    delete source2;
    delete clone_2;

    auto source3 = new LabelTreeItem("root3");
    auto child_1 = new LabelTreeItem("c1", source3);
    child_1->append(new LabelTreeItem("sub00", child_1));
    child_1->append(new LabelTreeItem("sub01", child_1));
    source3->append(child_1);
    auto child_2 = new LabelTreeItem("c2", source3);
    child_2->append(new LabelTreeItem("sub10", child_2));
    child_2->append(new LabelTreeItem("sub11", child_2));
    source3->append(child_2);
    auto clone_3 = source3->clone();
    QCOMPARE(clone_3->text(), "root3");
    QCOMPARE(clone_3->childs().size(), 2);
    QVERIFY(source3 != clone_3);
    QCOMPARE(clone_3->childs().at(0)->text(), "c1");
    QCOMPARE(clone_3->childs().at(0)->childs().size(), 2);
    QVERIFY(source3->childs().at(0) != clone_3->childs().at(0));
    QCOMPARE(clone_3->childs().at(0)->childs().at(0)->text(), "sub00");
    QCOMPARE(clone_3->childs().at(0)->childs().at(0)->childs().size(), 0);
    QVERIFY(source3->childs().at(0)->childs().at(0) != clone_3->childs().at(0)->childs().at(0));
    QCOMPARE(clone_3->childs().at(0)->childs().at(1)->text(), "sub01");
    QCOMPARE(clone_3->childs().at(0)->childs().at(1)->childs().size(), 0);
    QVERIFY(source3->childs().at(0)->childs().at(1) != clone_3->childs().at(0)->childs().at(1));
    QCOMPARE(clone_3->childs().at(1)->text(), "c2");
    QCOMPARE(clone_3->childs().at(1)->childs().size(), 2);
    QVERIFY(source3->childs().at(1) != clone_3->childs().at(1));
    QCOMPARE(clone_3->childs().at(1)->childs().at(0)->text(), "sub10");
    QCOMPARE(clone_3->childs().at(1)->childs().at(0)->childs().size(), 0);
    QVERIFY(source3->childs().at(1)->childs().at(0) != clone_3->childs().at(1)->childs().at(0));
    QCOMPARE(clone_3->childs().at(1)->childs().at(1)->text(), "sub11");
    QCOMPARE(clone_3->childs().at(1)->childs().at(1)->childs().size(), 0);
    QVERIFY(source3->childs().at(1)->childs().at(1) != clone_3->childs().at(1)->childs().at(1));
    delete source3;
    delete clone_3;
}

void TestLabelTreeItem::test_remove()
{
    LabelTreeItem item;
    item.remove(nullptr);
    LabelTreeItem *child = new LabelTreeItem;
    item.remove(child);
    item.append(child);
    QCOMPARE(item.size(), 1);
    item.remove(child);
    QCOMPARE(item.size(), 0);
    QCOMPARE(child->parent(), nullptr);
}

void TestLabelTreeItem::test_firstSectionIndex()
{
    auto root1 = new LabelTreeItem("root1");
    auto section1 = root1->firstSectionIndex();
    QCOMPARE(section1, -1);
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    root2->setSections({8,7});
    auto section2 = root2->firstSectionIndex();
    QCOMPARE(section2, 7);
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child = new LabelTreeItem("c1", root3);
    child->setSections({8});
    root3->append(child);
    child = new LabelTreeItem("c2", root3);
    child->setSections({3,1});
    root3->append(child);
    auto section3 = root3->firstSectionIndex();
    QCOMPARE(section3, 1);
    delete root3;
}

void TestLabelTreeItem::test_siblings()
{
    auto root1 = new LabelTreeItem("root1");
    auto siblings1 = root1->siblings();
    QCOMPARE(siblings1, QList<LabelTreeItem*>());
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    root2->append(new LabelTreeItem("c1", root2));
    auto child = new LabelTreeItem("c2", root2);
    root2->append(child);
    auto siblings2 = child->siblings();
    QCOMPARE(siblings2.size(), 2);
    delete root2;
}

void TestLabelTreeItem::test_sections()
{
    auto root1 = new LabelTreeItem("root1");
    auto sections1 = root1->sections();
    QCOMPARE(sections1, QSet<int>());
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    auto child2 = new LabelTreeItem("c1", root2);
    child2->setSections({0});
    root2->append(child2);
    child2 = new LabelTreeItem("c2", root2);
    child2->setSections({1,2});
    root2->append(child2);
    auto sections2= root2->sections();
    QSet<int> expected2 {0, 1, 2};
    QCOMPARE(sections2, expected2);
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child3 = new LabelTreeItem("c1", root3);
    child3->setSections({0});
    root3->append(child3);
    child3 = new LabelTreeItem("c2", root3);
    // valid sections are defined by children
    child3->setSections({1,2});
    root3->append(child3);
    auto subChild = new LabelTreeItem("sub1", child3);
    subChild->setSections({3, 4, 5});
    child3->append(subChild);
    subChild = new LabelTreeItem("sub2", child3);
    subChild->setSections({6, 7});
    child3->append(subChild);
    auto sections3 = root3->sections();
    QSet<int> expected3 {0, 3, 4, 5, 6, 7};
    QCOMPARE(sections3, expected3);
    delete root3;
}

void TestLabelTreeItem::test_unitedSections()
{
    auto root1 = new LabelTreeItem("root1");
    auto united1 = root1->unitedSections();
    QCOMPARE(united1, UnitedSections());
    delete root1;

    auto root2 = new LabelTreeItem("root1");
    root2->setSections({0, 1});
    auto united2 = root2->unitedSections();
    UnitedSections expected2 { {0, 1} };
    QCOMPARE(united2, expected2);
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child = new LabelTreeItem("c1", root3);
    child->setSections({8});
    child->setVisible(false);
    root3->append(child);
    child = new LabelTreeItem("c2", root3);
    child->setSections({3,1});
    root3->append(child);
    child = new LabelTreeItem("c3", root3);
    child->setSections({7});
    root3->append(child);
    auto united3 = root3->unitedSections();
    UnitedSections expected3 { {1, 3}, {7} };
    QCOMPARE(united3, expected3);
    delete root3;
}

void TestLabelTreeItem::test_visibleSections()
{
    auto root1 = new LabelTreeItem("root1");
    root1->setSections({0});
    auto sections1 = root1->visibleSections();
    QList<int> expected1 {0};
    QCOMPARE(sections1, expected1);
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    root2->setSections({1});
    root2->setVisible(false);
    auto sections2 = root2->visibleSections();
    QCOMPARE(sections2, QList<int>());
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child = new LabelTreeItem("c1", root3);
    child->setSections({8});
    child->setVisible(false);
    root3->append(child);
    child = new LabelTreeItem("c2", root3);
    child->setSections({3,1});
    root3->append(child);
    auto sections3 = root3->visibleSections();
    QList<int> expected3 {1,3};
    std::sort(sections3.begin(), sections3.end());
    QCOMPARE(sections3, expected3);
    delete root3;
}

void TestLabelTreeItem::test_visibleSectionsSorted()
{
    auto root1 = new LabelTreeItem("root1");
    root1->setSections({0});
    auto sections1 = root1->visibleSectionsSorted();
    QList<int> expected1 {0};
    QCOMPARE(sections1, expected1);
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    root2->setSections({1});
    root2->setVisible(false);
    auto sections2 = root2->visibleSectionsSorted();
    QCOMPARE(sections2, QList<int>());
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child = new LabelTreeItem("c1", root3);
    child->setSections({8});
    child->setVisible(false);
    root3->append(child);
    child = new LabelTreeItem("c2", root3);
    child->setSections({3,1});
    root3->append(child);
    auto sections3 = root3->visibleSectionsSorted();
    QList<int> expected3 {1,3};
    QCOMPARE(sections3, expected3);
    delete root3;
}

void TestLabelTreeItem::test_sectionLabels_root_level()
{
    LabelTreeItem item("root");
    auto labels = item.sectionLabels(0, 0);
    QCOMPARE(labels, SectionLabels());
    labels = item.sectionLabels(0, 1);
    QCOMPARE(labels, SectionLabels());
    labels = item.sectionLabels(1, 2);
    QCOMPARE(labels, SectionLabels());
    labels = item.sectionLabels(-1, -2);
    QCOMPARE(labels, SectionLabels());
}

void TestLabelTreeItem::test_sectionLabels_dimension_1()
{
    auto root1 = new LabelTreeItem("root1");
    auto labels1 = root1->sectionLabels(0, 0);
    QCOMPARE(labels1, SectionLabels());
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    auto labels2 = root2->sectionLabels(0, 3);
    QCOMPARE(labels2, SectionLabels());
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto labels3 = root3->sectionLabels(3, 3);
    QCOMPARE(labels3, SectionLabels());
    delete root3;

    auto root4 = new LabelTreeItem("root4");
    root4->append(new LabelTreeItem("Sum - 1", root4));
    root4->append(new LabelTreeItem("Sum - 1", root4));
    auto labels4 = root4->sectionLabels(0, 2);
    SectionLabels expected4;
    expected4[0] = QStringList("Sum - 1");
    expected4[1] = QStringList("Sum - 1");
    QCOMPARE(labels4, expected4);
    labels4 = root4->sectionLabels(-4, -2);
    QCOMPARE(labels4, SectionLabels());
    labels4 = root4->sectionLabels(12, 42);
    SectionLabels expected4_2;
    expected4_2[12] = QStringList("Sum - 1");
    expected4_2[13] = QStringList("Sum - 1");
    QCOMPARE(labels4, expected4_2);
    delete root4;
}

void TestLabelTreeItem::test_sectionLabels_dimension_2()
{
    auto root1 = new LabelTreeItem("root1");
    auto dim1 = new LabelTreeItem("Sum - 1", root1);
    root1->append(dim1);
    auto dim2 = new LabelTreeItem("Sum - 2", dim1);
    dim1->append(dim2);
    auto labels1 = root1->sectionLabels(0, 2);
    SectionLabels expected;
    expected[0] = QStringList { "Sum - 1", "Sum - 2" };
    QCOMPARE(labels1, expected);
    dim1 = new LabelTreeItem("Sum - A", root1);
    root1->append(dim1);
    dim2 = new LabelTreeItem("Sum - B", dim1);
    dim1->append(dim2);
    expected[1] = QStringList { "Sum - A", "Sum - B" };
    labels1 = root1->sectionLabels(0, 2);
    QCOMPARE(labels1, expected);
    delete root1;
}

void TestLabelTreeItem::test_sectionLabels_dimension_3()
{
    auto root1 = new LabelTreeItem("root1");
    auto dim1 = new LabelTreeItem("Sum - 1", root1);
    root1->append(dim1);
    auto dim2 = new LabelTreeItem("Sum - 2", dim1);
    dim1->append(dim2);
    auto dim3 = new LabelTreeItem("Sum - 3", dim2);
    dim2->append(dim3);
    auto labels1 = root1->sectionLabels(0, 3);
    SectionLabels expected;
    expected[0] = QStringList { "Sum - 1", "Sum - 2", "Sum - 3" };
    QCOMPARE(labels1, expected);
    dim1 = new LabelTreeItem("Sum - A", root1);
    root1->append(dim1);
    dim2 = new LabelTreeItem("Sum - B", dim1);
    dim1->append(dim2);
    dim3 = new LabelTreeItem("Sum - C", dim2);
    dim2->append(dim3);
    expected[1] = QStringList { "Sum - A", "Sum - B", "Sum - C" };
    labels1 = root1->sectionLabels(0, 3);
    QCOMPARE(labels1, expected);
    delete root1;
}

void TestLabelTreeItem::test_isVisible()
{
    auto root1 = new LabelTreeItem("root1");
    QCOMPARE(root1->isVisible(), true);
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    root2->setVisible(false);
    QCOMPARE(root2->isVisible(), false);
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child1 = new LabelTreeItem("c1", root3);
    child1->setVisible(false);
    root3->append(child1);
    auto child2 = new LabelTreeItem("c2", root3);
    child2->setVisible(false);
    root3->append(child2);
    QCOMPARE(root3->isVisible(), false);
    child1->setVisible(true);
    child2->setVisible(false);
    QCOMPARE(root3->isVisible(), true);
    child1->setVisible(false);
    child2->setVisible(true);
    QCOMPARE(root3->isVisible(), true);
    child1->setVisible(true);
    child2->setVisible(true);
    QCOMPARE(root3->isVisible(), true);
    delete root3;
}

void TestLabelTreeItem::test_sectionExtent()
{
    auto root0 = new LabelTreeItem("root0");
    QCOMPARE(root0->sectionExtent(), 0);
    delete root0;

    auto root1 = new LabelTreeItem("root1");
    QCOMPARE(root1->sectionExtent(), 0);
    delete root1;

    auto root2 = new LabelTreeItem("root2");
    root2->setSections({0, 1, 2});
    QCOMPARE(root2->sectionExtent(), 1);
    delete root2;

    auto root3 = new LabelTreeItem("root3");
    auto child1 = new LabelTreeItem("c1", root3);
    child1->setSections({0,1});
    child1->setVisible(false);
    root3->append(child1);
    auto child2 = new LabelTreeItem("c2", root3);
    child2->setSections({2});
    child2->setVisible(false);
    root3->append(child2);
    QCOMPARE(root3->sectionExtent(), 0);
    child1->setVisible(true);
    child2->setVisible(false);
    QCOMPARE(root3->sectionExtent(), 1);
    child1->setVisible(false);
    child2->setVisible(true);
    QCOMPARE(root3->sectionExtent(), 1);
    child1->setVisible(true);
    child2->setVisible(true);
    QCOMPARE(root3->sectionExtent(), 2);
    delete root3;
}

void TestLabelTreeItem::test_visibleBranch()
{
    auto root1 = new LabelTreeItem("root");
    auto level1 = root1->childs();
    auto result1 = LabelTreeItem::visibleBranch(level1, "Test", 1);
    QCOMPARE(result1, nullptr);
    QCOMPARE(level1, QList<LabelTreeItem*>());
    delete root1;

    auto root2 = new LabelTreeItem("root");
    root2->append(new LabelTreeItem("c1", root2));
    auto level2 = root2->childs();
    auto result2 = LabelTreeItem::visibleBranch(level2, "Test", 1);
    QVERIFY(result2 != nullptr);
    QCOMPARE(result2->text(), "Test - 1");
    QCOMPARE(level2, QList<LabelTreeItem*>());
    delete root2;

    auto root3 = new LabelTreeItem("root");
    root3->append(new LabelTreeItem("c1", root3));
    root3->append(new LabelTreeItem("c2", root3));
    auto level3 = root3->childs();
    auto result3 = LabelTreeItem::visibleBranch(level3, "Test", 2);
    QVERIFY(result3 != nullptr);
    QCOMPARE(result3->text(), "Test - 2");
    QCOMPARE(level3.size(), 1);
    delete root3;

    auto root4 = new LabelTreeItem("root");
    auto child4 = new LabelTreeItem("c1", root4);
    child4->setVisible(false);
    root4->append(child4);
    child4 = new LabelTreeItem("c2", root4);
    child4->setVisible(false);
    root4->append(child4);
    auto level4 = root4->childs();
    auto result4 = LabelTreeItem::visibleBranch(level4, "Test", 2);
    QCOMPARE(root4->childs().size(), 0);
    QCOMPARE(result4, nullptr);
    QCOMPARE(level4, QList<LabelTreeItem*>());
    delete root4;

    auto root5 = new LabelTreeItem("root");
    auto child5 = new LabelTreeItem("c1", root5);
    child5->setVisible(false);
    root5->append(child5);
    child5 = new LabelTreeItem("c2", root5);
    child5->setVisible(false);
    root5->append(child5);
    child5 = new LabelTreeItem("c3", root5);
    child5->setVisible(true);
    root5->append(child5);
    auto level5 = root5->childs();
    auto result5 = LabelTreeItem::visibleBranch(level5, "Test", 5);
    QCOMPARE(root5->childs().size(), 1);
    QVERIFY(result5 != nullptr);
    QCOMPARE(level5, QList<LabelTreeItem*>());
    delete root5;

    auto root6 = new LabelTreeItem("root");
    auto child6 = new LabelTreeItem("c1", root6);
    child6->setVisible(true);
    root6->append(child6);
    child6 = new LabelTreeItem("c2", root6);
    child6->setVisible(false);
    root6->append(child6);
    child6 = new LabelTreeItem("c3", root6);
    child6->setVisible(false);
    root6->append(child6);
    auto level6 = root6->childs();
    auto result6 = LabelTreeItem::visibleBranch(level6, "Test", 6);
    QCOMPARE(root6->childs().size(), 3);
    QVERIFY(result6 != nullptr);
    QCOMPARE(level6.size(), 2);
    delete root6;
}

QTEST_APPLESS_MAIN(TestLabelTreeItem)

#include "tst_testlabeltreeitem.moc"
