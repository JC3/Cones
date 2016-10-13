#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "simulator.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
    Q_OBJECT
    
public:

    explicit MainWindow (QWidget *parent = 0);
    ~MainWindow ();

protected:

    void timerEvent (QTimerEvent *);

private slots:

    void on_sbFrameSkip_valueChanged (int v) { frameskip_ = v; }

private:

    Ui::MainWindow *ui_;
    Simulator *sim_;
    int frameskip_;

    void showOptions ();

};

#endif // MAINWINDOW_H
