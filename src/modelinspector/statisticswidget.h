/*
 * This file is part of the GAMS Studio project.
 *
 * Copyright (c) 2017-2019 GAMS Software GmbH <support@gams.com>
 * Copyright (c) 2017-2019 GAMS Development Corp. <support@gams.com>
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
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef STATISTICSWIDGET_H
#define STATISTICSWIDGET_H

#include <QSharedPointer>
#include <QWidget>

#include "modelinstance.h"

namespace gams {
namespace studio {
namespace modelinspector {

class ModelInstance;

namespace Ui {
class StatisticsWidget;
}

// TODO if this is kept remove the intermediate widget?
class StatisticsWidget : public QWidget
{
    Q_OBJECT

public:
    StatisticsWidget(QWidget *parent = nullptr);

    void showStatistic(const QSharedPointer<ModelInstance> &modelInstance);

private:
    Ui::StatisticsWidget* ui;
};

}
}
}

#endif // STATISTICSWIDGET_H
