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

#ifndef SIMULATOR_H
#define SIMULATOR_H

#include <QObject>
#include <QList>
#include <QRect>
#include <QPoint>
#include <QVector2D>


//-----------------------------------------------------------------------------
/**
 * Simulator. Does all the things. Currently MainWindow initializes the
 * simulator and is responsible for calling update(), and SimulatorView just
 * grabs data and draws it.
 */
//-----------------------------------------------------------------------------

class Simulator : public QObject {
    Q_OBJECT

public:

    /** Simulation parameters. Distance units are arbitrary but see the
     *  SimulatorView comments for details. Belt moves in +X direction. */
    struct Parameters {
        double timestep;        /**< In seconds. 1/FPS makes sense. */
        double beltWidth;       /**< Width of belt. */
        double beltSpeed;       /**< Speed of belt (units / second). */
        double coneRate;        /**< Average spawn rate (cones / second). */
        QRectF coneDrop;        /**< Cone spawn area. */
        QRectF hoseRange;       /**< Hose head movement range. */
        double hoseFillRate;    /**< Cone fill rate (full fills / second). */
        double hoseSpeed;       /**< Hose head movement speed (units / second). */
        double urgentTime;      /**< Time margin for cones to be urgent (seconds). */
    };

    /** A cone. */
    struct Cone {
        QPointF pos;    /**< Position. */
        double fill;    /**< Amount of ice cream (0 to 1). */
        Cone (double x, double y) : pos(x, y), fill(0), status(Boring) { }
        // Some stuff used by updateHose():
        enum Status { Boring, AlreadyFull, CantFill, Urgent };
        double totaltime;
        double timelimit;
        QVector2D fillpoint;
        Status status; // read by SimulatorView *only*!
    };

#if 0 // work in progress
    class HoseDrive {
    public:
        virtual ~HoseDrive () { }
        virtual void moveTo (const QVector2D &pos, bool instant) = 0;
        virtual bool arrived () const = 0;
        virtual double calcTime (const QVector2D &pos) const = 0;
        virtual QVector2D calcIntercept
    };
#endif

    /** A hose head. */
    struct Hose {
        QPointF pos;    /**< Position. */
        Cone *target;   /**< Current target Cone, or NULL. */
        explicit Hose (const QPointF &pos) : pos(pos), target(NULL), state(Idle), arrived(false), urgentmode(false) { }
        // Some stuff used by updateHose():
        enum State { Idle, Approaching, Filling };
        State state;    /**< Current state. */
        QVector2D dest; /**< Current movement destination (Idle, Approaching). */
        bool arrived;   /**< Arrived at destination? (Idle, Approaching) */
        bool urgentmode;/**< Handling "urgent" cones? */
    };

    explicit Simulator (const Parameters &p, QObject *parent = 0);
    ~Simulator ();

    /** @return Current list of cones. Do not delete these. */
    const QList<Cone *> & cones () const { return cones_; }

    /** @return Current parameters. */
    const Parameters & params () const { return p_; }

    /** @return Current hose head info. */
    const Hose & hose () const { return hose_; }

public slots:

    void update ();

    // These all change various parameters and are called by the GUI. I should
    // probably add accessors for these as well since some of them don't directly
    // correspond to Parameter fields but whatever. Currently that logic is all
    // in MainWindow::showOptions().

    void setBeltSpeed (double v) {
        p_.beltSpeed = v;
    }

    /** Also adjusts the hose movement range and cone drop area. */
    void setBeltWidth (double v) {
        p_.hoseRange.adjust(0, 0, 0, v - p_.beltWidth);
        p_.coneDrop.adjust(0, 0, 0, v - p_.beltWidth);
        p_.beltWidth = v;
    }

    void setConeRate (double v) {
        p_.coneRate = v;
        // eliminate lag when rate increasing
        double next = 1.0 / p_.coneRate;
        if (newconet_ - t_ > next)
            newconet_ = t_ + next;
    }

    /** This "variance" is the width (via -X edge) of the drop area. */
    void setConeVariance (double v) {
        p_.coneDrop.setLeft(p_.coneDrop.right() - v);
    }

    /** Adjusts the width (via +X edge) of the hose area. */
    void setHoseRange (double v) {
        p_.hoseRange.setWidth(v);
    }

    void setHoseSpeed (double v) {
        p_.hoseSpeed = v;
    }

    void setFillRate (double v) {
        p_.hoseFillRate = v;
    }

    void setUrgentTime (double v) {
        p_.urgentTime = v;
    }

private:

    Parameters p_;          /**< Current parameters. */
    double t_;              /**< Current timestamp. */
    double newconet_;       /**< Timestamp of next cone creation. */
    QList<Cone *> cones_;   /**< All the cones. */
    Hose hose_;             /**< The hose head. */

    void updateCones ();
    void updateHose (Hose &h);

};

#endif // SIMULATOR_H
