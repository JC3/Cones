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

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QTimer>


MainWindow::MainWindow (QWidget *parent) :
    QMainWindow(parent),
    ui_(new Ui::MainWindow),
    frameskip_(1)
{

    ui_->setupUi(this);

#define FPS 50

    Simulator::Parameters p;
    p.timestep = 1.0 / FPS;
    p.beltWidth = 24.0;
    p.beltSpeed = 2.0;
    p.coneRate = 1.7;
    p.coneDrop = QRectF(-36, 0, 24, p.beltWidth).adjusted(0, 2, 0, -2);
    p.hoseRange = QRectF(12, 0, 36, p.beltWidth).adjusted(0, 1, 0, -1);
    p.hoseFillRate = 3.0;
    p.hoseSpeed = 20.0;
    p.urgentTime = 3.0;

    sim_ = new Simulator(p, this);
    ui_->view->setSimulator(sim_);
#if !AUTO_BOUNDS
    ui_->view->setViewBounds(-36, 72);
#endif

    connect(ui_->sbBeltSpeed, SIGNAL(valueChanged(double)), sim_, SLOT(setBeltSpeed(double)));
    connect(ui_->sbBeltWidth, SIGNAL(valueChanged(double)), sim_, SLOT(setBeltWidth(double)));
    connect(ui_->sbConeRate, SIGNAL(valueChanged(double)), sim_, SLOT(setConeRate(double)));
    connect(ui_->sbConeVariance, SIGNAL(valueChanged(double)), sim_, SLOT(setConeVariance(double)));
    connect(ui_->sbHoseWidth, SIGNAL(valueChanged(double)), sim_, SLOT(setHoseRange(double)));
    connect(ui_->sbHoseSpeed, SIGNAL(valueChanged(double)), sim_, SLOT(setHoseSpeed(double)));
    connect(ui_->sbFillRate, SIGNAL(valueChanged(double)), sim_, SLOT(setFillRate(double)));
    connect(ui_->sbUrgentTime, SIGNAL(valueChanged(double)), sim_, SLOT(setUrgentTime(double)));

    startTimer(1000 / FPS);
    showOptions();

}


MainWindow::~MainWindow () {

    delete ui_;

}


void MainWindow::timerEvent (QTimerEvent *) {

    for (int n = 0; n < frameskip_; ++ n)
        sim_->update();

    ui_->view->update();

}


void MainWindow::showOptions () {

    ui_->sbFrameSkip->setValue(frameskip_);
    ui_->sbBeltSpeed->setValue(sim_->params().beltSpeed);
    ui_->sbBeltWidth->setValue(sim_->params().beltWidth);
    ui_->sbConeRate->setValue(sim_->params().coneRate);
    ui_->sbConeVariance->setValue(sim_->params().coneDrop.width());
    ui_->sbHoseWidth->setValue(sim_->params().hoseRange.width());
    ui_->sbHoseSpeed->setValue(sim_->params().hoseSpeed);
    ui_->sbFillRate->setValue(sim_->params().hoseFillRate);
    ui_->sbUrgentTime->setValue(sim_->params().urgentTime);

}
