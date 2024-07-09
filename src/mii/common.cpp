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
#include "common.h"

namespace gams {
namespace studio {
namespace mii {

const QString AttributeHelper::InfeasibilityText = "Infeasibility";
const QString AttributeHelper::LevelText         = "Level";
const QString AttributeHelper::LowerText         = "Lower";
const QString AttributeHelper::MarginalText      = "Marginal";
const QString AttributeHelper::MarginalNumText   = "MarginalNum";
const QString AttributeHelper::RangeText         = "Range";
const QString AttributeHelper::ScaleText         = "Scale";
const QString AttributeHelper::SlackText         = "Slack";
const QString AttributeHelper::SlackLBText       = "Slack lower bound";
const QString AttributeHelper::SlackUBText       = "Slack upper bound";
const QString AttributeHelper::UpperText         = "Upper";
const QString AttributeHelper::TypeText          = "Type";

const QString ValueHelper::NAText     = "NA";
const QString ValueHelper::EPSText    = "EPS";
const QString ValueHelper::INFText    = "INF";
const QString ValueHelper::PINFText   = "+INF";
const QString ValueHelper::NINFText   = "-INF";
const unsigned char ValueHelper::Plus   = '+';
const unsigned char ValueHelper::Minus  = '-';
const unsigned char ValueHelper::Mixed  = 'm';
const QString ValueHelper::PlusMinus    = "+/-";

const QString ViewHelper::AttributeHeaderText = "Attributes";
const QString ViewHelper::EquationHeaderText  = "Equations";
const QString ViewHelper::VariableHeaderText  = "Variables";

const QString ViewHelper::ModelInstance   = "Model Instance";
const QString ViewHelper::PredefinedViews = "Predefined Views";
const QString ViewHelper::CustomViews     = "Custom Views";
const QString ViewHelper::Blockpic        = "Blockpic";
const QString ViewHelper::SymbolView      = "Symbol View";

const QString ViewHelper::Jacobian      = "Jacobian";
const QString ViewHelper::BPScaling     = "Scaling";
const QString ViewHelper::BPOverview    = "Overview";
const QString ViewHelper::BPCount       = "Count";
const QString ViewHelper::BPAverage     = "Average";
const QString ViewHelper::Postopt       = "Postopt";
const QString ViewHelper::Preopt        = "Preopt";
const QStringList ViewHelper::PredefinedViewTexts = {
                                                Jacobian,
                                                BPOverview,
                                                BPCount,
                                                BPAverage,
                                                BPScaling,
                                                Postopt
                                            };

const QString FileHelper::GamsCntr = "gamscntr.dat";
const QString FileHelper::GamsDict = "gamsdict.dat";
const QString FileHelper::Gamsmatr = "gamsmatr.dat";
const QString FileHelper::GamsSolu = "gamssolu.dat";
const QString FileHelper::GamsStat = "gamsstat.dat";
}
}
}
