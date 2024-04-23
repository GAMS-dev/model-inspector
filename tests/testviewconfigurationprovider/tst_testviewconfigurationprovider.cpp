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
#include "viewconfigurationprovider.h"

using namespace gams::studio::mii;

class TestViewConfigurationProvider : public QObject
{
    Q_OBJECT

private slots:
    void test_defaultConfiguration();
    void test_configuration();

private:
    void test_viewConfiguration(AbstractViewConfiguration *viewConfig,
                                ViewHelper::ViewDataType type);
};

void TestViewConfigurationProvider::test_defaultConfiguration()
{
    auto viewConfig = ViewConfigurationProvider::defaultConfiguration();
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::Unknown);
}

void TestViewConfigurationProvider::test_configuration()
{
    AbstractViewConfiguration* viewConfig = nullptr;
    auto modelInstance = QSharedPointer<AbstractModelInstance>(new EmptyModelInstance);

    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::Unknown, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::Unknown);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::BP_Overview, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::BP_Overview);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::BP_Count, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::BP_Count);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::BP_Average, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::BP_Average);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::BP_Scaling, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::BP_Scaling);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::Symbols, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::Symbols);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::Postopt, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::Postopt);
    delete viewConfig;

    viewConfig = nullptr;
    viewConfig = ViewConfigurationProvider::configuration(ViewHelper::ViewDataType::BlockpicGroup, modelInstance);
    test_viewConfiguration(viewConfig, ViewHelper::ViewDataType::BlockpicGroup);
    delete viewConfig;
}

void TestViewConfigurationProvider::test_viewConfiguration(AbstractViewConfiguration *viewConfig,
                                                           ViewHelper::ViewDataType type)
{
    // test general properties
    if (viewConfig) {
        QVERIFY(true);
    } else {
        QVERIFY(false);
        return;
    }
    QVERIFY(viewConfig->modelInstance() != nullptr);
    QCOMPARE(viewConfig->viewType(), type);
    QCOMPARE(viewConfig->viewId(), (int)type);

    QVERIFY(viewConfig->filterDialogState() == AbstractViewConfiguration::None);

    // test label filters
    QCOMPARE(viewConfig->currentLabelFiler(), LabelFilter());
    QCOMPARE(viewConfig->defaultLabelFilter(), LabelFilter());
    viewConfig->currentLabelFiler().Any = true;
    QVERIFY(viewConfig->currentLabelFiler() != viewConfig->defaultLabelFilter());
    viewConfig->resetLabelFilter();
    QCOMPARE(viewConfig->currentLabelFiler(), viewConfig->defaultLabelFilter());

    // test identifier filters
    QCOMPARE(viewConfig->currentIdentifierFilter(), IdentifierFilter());
    QCOMPARE(viewConfig->defaultIdentifierFilter(), IdentifierFilter());
    IdentifierStates states;
    states[0] = IdentifierState();
    viewConfig->currentIdentifierFilter()[Qt::Horizontal] = states;
    QCOMPARE(viewConfig->currentIdentifierFilter().size(), 1);
    viewConfig->resetIdentifierFilter();
    QCOMPARE(viewConfig->currentIdentifierFilter(), viewConfig->defaultIdentifierFilter());

    // test value filters
    QCOMPARE(viewConfig->currentValueFilter(), ValueFilter());
    QCOMPARE(viewConfig->defaultValueFilter(), ValueFilter());
    QCOMPARE(viewConfig->currentValueFilter(), viewConfig->defaultValueFilter());
    viewConfig->currentValueFilter().ExcludeRange = true;
    QVERIFY(viewConfig->currentValueFilter() != viewConfig->defaultValueFilter());
    viewConfig->resetValueFilter();
    QCOMPARE(viewConfig->currentValueFilter(), viewConfig->defaultValueFilter());

    // test attribute filters
    if (type == ViewHelper::ViewDataType::Postopt) {
        QCOMPARE(viewConfig->currentAttributeFilter().size(), AttributeHelper::attributeTextList().size());
        QCOMPARE(viewConfig->defaultAttributeFilter().size(), AttributeHelper::attributeTextList().size());
        viewConfig->currentAttributeFilter().clear();
        QCOMPARE(viewConfig->currentAttributeFilter().size(), 0);
        viewConfig->resetAttributeFilter();
        QCOMPARE(viewConfig->currentAttributeFilter().size(), AttributeHelper::attributeTextList().size());
    } else {
        QVERIFY(viewConfig->currentAttributeFilter().isEmpty());
        QVERIFY(viewConfig->defaultAttributeFilter().isEmpty());
        viewConfig->currentAttributeFilter().clear();
        QVERIFY(viewConfig->currentAttributeFilter().isEmpty());
        viewConfig->resetAttributeFilter();
        QVERIFY(viewConfig->currentAttributeFilter().isEmpty());
    }

    // test default search result
    QCOMPARE(viewConfig->searchResult(), SearchResult());
}

QTEST_APPLESS_MAIN(TestViewConfigurationProvider)

#include "tst_testviewconfigurationprovider.moc"
