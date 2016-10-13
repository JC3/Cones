//=============================================================================
/*
 * https://github.com/JC3/Cones
 *
 * MIT License
 *
 * Copyright (c) 2016, Jason Cipriani
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
//=============================================================================

#ifndef SIMULATORVIEW_H
#define SIMULATORVIEW_H

#include <QFrame>
#include "simulator.h"

#define AUTO_BOUNDS 1 /**< If 1, viewport bounds are set automatically. */


//-----------------------------------------------------------------------------
/**
 * A widget that draws the current simulation. See paintEvent().
 */
//-----------------------------------------------------------------------------

class SimulatorView : public QFrame {
    Q_OBJECT

public:

    explicit SimulatorView (QWidget *parent = 0);
    
    void setSimulator (const Simulator *sim) {
        sim_ = sim;
        update();
    }

#if !AUTO_BOUNDS
    void setViewBounds (double xmin, double xmax) {
        viewXmin_ = xmin;
        viewXmax_ = xmax;
        update();
    }
#endif

protected:

    void paintEvent (QPaintEvent *);

private:

    const Simulator *sim_;  /**< Simulator being drawn. */
    double viewXmin_;       /**< Minimum visible belt position. */
    double viewXmax_;       /**< Maximum visible belt position. */
    
};


#endif // SIMULATORVIEW_H
