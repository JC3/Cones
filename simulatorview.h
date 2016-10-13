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
