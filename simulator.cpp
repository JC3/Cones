#include "simulator.h"
#include <cmath>
#include <QtGlobal>
#include <QVector2D>
#include <QDebug>
#include <QDateTime>


//-----------------------------------------------------------------------------
/**
 * @return  A random number between min and max.
 */
//-----------------------------------------------------------------------------

static double randf (double min, double max) {

    return (double)qrand() / (double)RAND_MAX * (max - min) + min;

}


//-----------------------------------------------------------------------------
/**
 * Given cone position and velocity, and hose position and speed, calculates
 * the point that the hose can intercept the cone and the time it will take to
 * get there. Math is from http://stackoverflow.com/a/2249237.
 *
 * @param   cone        Cone position.
 * @param   coneVel     Cone velocity (per second).
 * @param   hose        Hose head position.
 * @param   hoseSpeed   Hose head speed (per second).
 * @param   tout        If not NULL, will contain movement time.
 * @return  The interception point, or a null vector if no solution exists. The
 *          hose direction and velocity can be calculated from this, hose, and
 *          tout.
 */
//-----------------------------------------------------------------------------

static QVector2D intercept (const QVector2D &cone,
                            const QVector2D &coneVel,
                            const QVector2D &hose,
                            double hoseSpeed,
                            double *tout)
{

    /* from http://stackoverflow.com/a/2249237
    a := sqr(target.velocityX) + sqr(target.velocityY) - sqr(projectile_speed)
    b := 2 * (target.velocityX * (target.startX - cannon.X)
              + target.velocityY * (target.startY - cannon.Y))
    c := sqr(target.startX - cannon.X) + sqr(target.startY - cannon.Y)
    disc := sqr(b) - 4 * a * c
    t1 := (-b + sqrt(disc)) / (2 * a)
    t2 := (-b - sqrt(disc)) / (2 * a)
    aim.X := t * target.velocityX + target.startX
    aim.Y := t * target.velocityY + target.startY
    */

    QVector2D hoseToCone = cone - hose;

    double a = coneVel.lengthSquared() - hoseSpeed * hoseSpeed;
    double b = 2.0 * QVector2D::dotProduct(coneVel, hoseToCone);
    double c = hoseToCone.lengthSquared();
    double disc = b * b - 4 * a * c;

    if (disc < 0.0)
        return QVector2D();

    double sqrt_disc = sqrt(disc);
    double t1 = (-b + sqrt_disc) / (2.0 * a);
    double t2 = (-b - sqrt_disc) / (2.0 * a);
    double t;

    if (t1 < 0.0)
        t = t2;
    else if (t2 < 0.0)
        t = t1;
    else
        t = qMin(t1, t2);

    if (t < 0.0)
        return QVector2D();

    if (tout)
        *tout = t;

    return coneVel * t + cone;

}


//-----------------------------------------------------------------------------
/**
 * Construct a Simulator from the given configuration. Everything is ready to
 * go after this, just start calling update().
 *
 * @param   p   Initial simulation parameters. These are not validated, don't
 *              break anything.
 */
//-----------------------------------------------------------------------------

Simulator::Simulator (const Parameters &p, QObject *parent) :
    QObject(parent),
    p_(p),
    t_(0),
    newconet_(0),
    hose_(p.hoseRange.center())
{

    qsrand(QDateTime::currentMSecsSinceEpoch());

}


//-----------------------------------------------------------------------------
/**
 * Destructor. Will invalidate all Cone pointers.
 */
//-----------------------------------------------------------------------------

Simulator::~Simulator () {

    qDeleteAll(cones_);

}


//-----------------------------------------------------------------------------
/**
 * Calculates one simulation frame. Updates cone and hose states and increments
 * the current timestamp. May invalidate some Cone pointers (as they die).
 */
//-----------------------------------------------------------------------------

void Simulator::update () {

    updateCones();
    updateHose(hose_);
    t_ += p_.timestep;

}


//-----------------------------------------------------------------------------
/**
 * Updates cones for this frame. Moves the cones, creates new ones, kills old
 * ones. May invalidate some Cone pointers (as they die). Currently the
 * the position on the belt at which cones die is chosen to correspond with a
 * location just beyond the end of the view bounds that I use in SimulatorView.
 */
//-----------------------------------------------------------------------------

void Simulator::updateCones () {

    // kinda arbitrary, based on view auto bounds
    double diepos = p_.hoseRange.right() + (p_.hoseRange.left() - p_.coneDrop.right()) + 2.0;

    // move / kill cones
    for (QList<Cone *>::iterator i = cones_.begin(); i != cones_.end(); ) {
        if ((*i)->pos.x() > diepos) {
            delete *i;
            i = cones_.erase(i);
        } else {
            (*i)->pos.rx() += p_.beltSpeed * p_.timestep;
            ++ i;
        }
    }

    // spawn new cones
    while (t_ >= newconet_) {
        newconet_ += 1.0 / p_.coneRate;
        cones_.push_back(new Cone(randf(p_.coneDrop.left(), p_.coneDrop.right()),
                                  randf(p_.coneDrop.top(), p_.coneDrop.bottom())));
    }

}


//-----------------------------------------------------------------------------
/**
 * Update hose position. This is where the filling algorithm is implemented,
 * and is the function you'd want to play with when implementing a new
 * algorithm. It is responsible for:
 *
 * - Analyzing current cone positions.
 * - Moving the hose.
 * - Filling the cones (by modifying Cone::fill).
 *
 * This is the only place in the Simulator code that really uses the member
 * fields of Hose (and currently stores a few things in Cone as well). So you
 * can add/remove members to those structs as you see fit except you will also
 * probably have to make changes in SimulatorView to match. Other than that
 * caveat this can all be modified as needed.
 *
 * The reason h is passed as a parameter instead of just using hose_ is that I
 * was originally thinking of supporting multiple hose heads. Might be a fun
 * feature to add.
 */
//-----------------------------------------------------------------------------

void Simulator::updateHose (Hose &h) {

    if (h.state == Hose::Idle && !h.target) {

        QList<Cone *> urgent;
        double closesttime = 0.0;

        // find the closest cone that we can move to and fill up in time
        foreach (Cone *cone, cones_) {
            cone->status = Cone::Boring;
            if (cone->fill >= 1.0) {
                cone->status = Cone::AlreadyFull;
                continue;
            }
            // time cone has before it moves out of range
            double timelimit = (p_.hoseRange.right() - cone->pos.x()) / p_.beltSpeed;
            // time cone will require to fill up
            double filltime = (1.0 - cone->fill) / p_.hoseFillRate;
            if (filltime > timelimit) {
                cone->status = Cone::CantFill;
                continue;
            }
            // time it will take hose to get to cone, predicting where the cone will be
            double movetime;
            QVector2D fillpoint = intercept(QVector2D(cone->pos),
                                            QVector2D(p_.beltSpeed, 0),
                                            QVector2D(hose_.pos),
                                            p_.hoseSpeed, &movetime);
            if (fillpoint.isNull() || !p_.hoseRange.contains(fillpoint.toPointF())) {
                cone->status = Cone::CantFill;
                continue;
            }
            double totaltime = filltime + movetime;
            if (totaltime > timelimit) {
                cone->status = Cone::CantFill;
                continue;
            }
            // ok so its a candidate
            if (!h.target || totaltime < closesttime) {
                closesttime = totaltime;
                h.target = cone;
                h.state = Hose::Approaching;
                h.arrived = false;
                h.dest = fillpoint;
            }
            // stragglers
            if (timelimit - totaltime < p_.urgentTime) {
                cone->totaltime = totaltime;
                cone->fillpoint = fillpoint;
                cone->timelimit = timelimit;
                cone->status = Cone::Urgent;
                urgent.push_back(cone);
            }
        }

        // stragglers
        if (!urgent.isEmpty()) {
            closesttime = 0.0;
            h.target = NULL;
            foreach (Cone *cone, urgent) {
                if (h.urgentmode) {
                    if (!h.target || cone->totaltime < closesttime) {
                        closesttime = cone->totaltime;
                        h.target = cone;
                        h.dest = cone->fillpoint;
                    }
                } else {
                    if (!h.target || cone->totaltime < closesttime) {
                        closesttime = cone->totaltime;
                        h.target = cone;
                        h.dest = cone->fillpoint;
                    }
                    h.urgentmode = true;
                }
            }
        } else {
            h.urgentmode = false;
        }

    }

    // if idle, drift towards inlet center
    // note: lazy logic will immediately set arrived = true again if we're already
    // there but who cares.
    if (h.state == Hose::Idle) {
        h.arrived = false;
        h.dest = QVector2D(p_.hoseRange.left(), p_.hoseRange.center().y());
    }

    if (h.state == Hose::Idle || h.state == Hose::Approaching) {
        if (!h.arrived) {
            QVector2D todest = h.dest - QVector2D(h.pos);
            double dist = p_.hoseSpeed * p_.timestep;
            if (dist > todest.length()) {
                h.pos = h.dest.toPointF();
                h.arrived = true;
            } else {
                h.pos += (todest.normalized() * dist).toPointF();
            }
        }
    }

    if (h.state == Hose::Approaching && h.arrived) {
        h.state = Hose::Filling;
    }

    if (h.state == Hose::Filling) {
        h.pos = h.target->pos;
        h.target->fill += p_.hoseFillRate * p_.timestep;
        if (h.target->fill >= 1.0) {
            h.target->fill = 1.0;
            h.target = NULL;
            h.state = Hose::Idle;
        }
    }

}
