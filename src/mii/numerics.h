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
#ifndef NUMERICS_H
#define NUMERICS_H

#include <QString>

/**
 *  doubleFormat.h
 * Feb 2020: Formatting routines for displaying doubles
 */
#ifdef __cplusplus
extern "C" {
#endif

char *x2fixed (double v, int nDecimals, int squeeze, char outBuf[], int *outLen, char decSep);

char *x2efmt (double v, int nSigFigs, int squeeze, char outBuf[], int *outLen, char decSep);

char *x2gfmt (double v, int nSigFigs, int squeeze, char outBuf[], int *outLen, char decSep);

#ifdef __cplusplus
};
#endif

namespace gams {
namespace studio {
namespace mii {

class DoubleFormatter
{
public:
    enum Format {
        g = 0,
        f = 1,
        e = 2
    };
    static const int gFormatFull;
    static QString format(double v, Format format, int precision, int squeeze, QChar decSep = '.');

private:
    DoubleFormatter() {};
};

}
}
}

#endif // NUMERICS_H
