/***************************************************************************************
 * Copyright (c) 2013 Robert Riemann <robert@riemann.cc>                                *
 *                                                                                      *
 * This program is free software; you can redistribute it and/or modify it under        *
 * the terms of the GNU General Public License as published by the Free Software        *
 * Foundation; either version 2 of the License, or (at your option) any later           *
 * version.                                                                             *
 *                                                                                      *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY      *
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A      *
 * PARTICULAR PURPOSE. See the GNU General Public License for more details.             *
 *                                                                                      *
 * You should have received a copy of the GNU General Public License along with         *
 * this program.  If not, see <http://www.gnu.org/licenses/>.                           *
 ****************************************************************************************/

#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#include "settings.h"

#include <cmath>

real f_eval(real x) {
    if(x < -0.5) {
        return 0;
    } else if (x < 0) {
        return 1;
    } else if (x < 0.5) {
        return 4*(x-0.5)*(x-0.5);
    } else {
        return 2*(x-0.5);
    }
}

real f_eval2(real x) {
    if(x > 0.25 && x < 0.75) {
        return 1.0;
    } else {
        return 0.0;
    }
}

// quadric
real f_eval3(real x) {
    return 2*pow(x,4)-2*pow(x,2)-0.2*pow(x,3)+0.3*x+1;
}

real f_eval4(real x) {
    return (x-1)*(x-1);
}

// sin
const real pi2 = 2*std::atan(1.0);

real f_eval5(real x) {
    if(x > pi2/3) {
        return 1;
    }
    return std::sin(3*x)*0.5+0.5;
}

#endif // FUNCTIONS_H
