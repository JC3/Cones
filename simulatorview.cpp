#include "simulatorview.h"
#include <QPainter>
#include <QtGlobal>

#define BACKGROUND_COLOR    Qt::blue
#define BELT_COLOR          Qt::lightGray
#define HOSE_AREA_COLOR     Qt::yellow
#define HOSE_BORDER_COLOR   Qt::black
#define HOSE_FILL_NORMAL    Qt::magenta
#define HOSE_FILL_IDLE      Qt::gray
#define HOSE_FILL_URGENT    Qt::cyan
#define HOSE_RADIUS         0.5
#define CONE_AREA_COLOR     Qt::white
#define CONE_EMPTY_COLOR    Qt::red
#define CONE_UEMPTY_COLOR   Qt::black
#define CONE_XEMPTY_COLOR   Qt::red
#define CONE_FULL_COLOR     Qt::green
#define CONE_BORDER_COLOR   Qt::black
#define CONE_TARGETED_COLOR Qt::white
#define CONE_WIDTH          1.5
#define CONE_HEIGHT         2.0


//-----------------------------------------------------------------------------
/**
 * Get cone fill color from cone targetting status.
 */
//-----------------------------------------------------------------------------

static QColor coneColor (Simulator::Cone::Status s) {
    switch (s) {
    case Simulator::Cone::Urgent: return CONE_UEMPTY_COLOR;
    case Simulator::Cone::CantFill: return CONE_XEMPTY_COLOR;
    default: return CONE_EMPTY_COLOR;
    }
}


//-----------------------------------------------------------------------------
/**
 * Constructor.
 */
//-----------------------------------------------------------------------------

SimulatorView::SimulatorView (QWidget *parent) :
    QFrame(parent),
    sim_(NULL),
    viewXmin_(-12.0),
    viewXmax_(36.0)
{
}


//-----------------------------------------------------------------------------
/**
 * Draw everything. A QTransform is used to put everything in belt coordinates.
 * The view is scaled to ensure visibility of the entire belt width and the
 * min/max positions.
 *
 * Cones are drawn as 1.5 x 2 rectangles, the hose head is a circle with radius
 * 0.5. The units I use in the initial simulation parameters are therefore
 * based roughly on that (I was thinking the units kind of vaguely resemble
 * inches).
 *
 * The belt moves in the +X direction, the view draws +X to the *left* so cones
 * moving on +X are animated right to left. My initial simulation parameters as
 * well as the math in some of the parameter setter slots in Simulator (and the
 * location at which cones are destroyed in Simulator::updateCones()) was
 * chosen based on the leading edge of the hose range being at X=0 and the cone
 * spawn area being somewhere in X<0. So you should probably use that same
 * system when picking initial simulation parameters to keep the view looking
 * OK.
 */
//-----------------------------------------------------------------------------

void SimulatorView::paintEvent (QPaintEvent *) {

    if (!sim_)
        return;

    const QList<Simulator::Cone *> &cones = sim_->cones();
    const Simulator::Parameters &sp = sim_->params();
    const Simulator::Hose &hose = sim_->hose();

#if AUTO_BOUNDS
    viewXmin_ = sp.coneDrop.left();
    viewXmax_ = sp.hoseRange.right() + (sp.hoseRange.left() - sp.coneDrop.right());
#endif

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);

    // background
    p.fillRect(rect(), BACKGROUND_COLOR);

    // set transform so we can draw in belt coords from here on down
    QRectF view(viewXmin_, 0.0, viewXmax_ - viewXmin_, sp.beltWidth);
    double scale = qMin(rect().width() / view.width(), rect().height() / view.height());
    QTransform t = QTransform()
            .translate(rect().center().x(), rect().center().y())
            .scale(-1, 1)
            .scale(scale, scale)
            .translate(-view.center().x(), -view.center().y())
            ;
    p.setWorldTransform(t);

    // belt
    p.fillRect(view, BELT_COLOR);

    // spawn area
    p.fillRect(sp.coneDrop, CONE_AREA_COLOR);

    // hose range
    p.fillRect(sp.hoseRange, HOSE_AREA_COLOR);

    // cones
    p.setBrush(Qt::NoBrush);
    foreach (const Simulator::Cone *cone, cones) {
        p.setPen(QPen(cone == hose.target ? CONE_TARGETED_COLOR : CONE_BORDER_COLOR, 0));
        QRectF rccone(0.0, 0.0, CONE_WIDTH, CONE_HEIGHT);
        QRectF rcfill(0.0, 0.0, rccone.width(), rccone.height() * cone->fill);
        rccone.moveCenter(cone->pos);
        rcfill.moveBottomLeft(rccone.bottomLeft());
        p.fillRect(rccone, coneColor(cone->status));
        p.fillRect(rcfill, CONE_FULL_COLOR);
        p.drawRect(rccone);
    }

    // hose
    p.setPen(QPen(HOSE_BORDER_COLOR, 0));
    if (hose.state == Simulator::Hose::Idle)
        p.setBrush(HOSE_FILL_IDLE);
    else if (hose.urgentmode)
        p.setBrush(HOSE_FILL_URGENT);
    else
        p.setBrush(HOSE_FILL_NORMAL);
    p.drawLine(QPointF(sp.hoseRange.left(), hose.pos.y()), QPointF(sp.hoseRange.right(), hose.pos.y()));
    p.drawLine(QPointF(hose.pos.x(), sp.hoseRange.top()), QPointF(hose.pos.x(), sp.hoseRange.bottom()));
    p.drawEllipse(hose.pos, HOSE_RADIUS, HOSE_RADIUS);

}
