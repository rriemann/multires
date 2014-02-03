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

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>

#include "node/node_iterator.hpp"

#include "regular/regular_base.hpp"

class QCustomPlot;
class QCPBars;
struct node_base;
class QSpinBox;
class QTimer;

QT_FORWARD_DECLARE_CLASS(QGraphicsScene)

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    
private:
    Ui::MainWindow *ui;

    node_t::node_p m_root = nullptr;
    regular_t::regular_p m_root_regular = nullptr;

    QCustomPlot *customPlot;
    std::array<QCPBars*,4> bars;
    size_t count_nodes;
    size_t count_nodes_packed;
    QSpinBox *spinBox;
    QTimer *timer;

    QGraphicsScene *scene;

protected:
    void resizeEvent(QResizeEvent * event );

private slots:

    void actionRun();
    void replot();
    void rescale();
    void autoPlayToggled(bool checked);

    void initializeRoot();

    void blockBuilder(node_t::node_p node);
};

#endif // MAINWINDOW_HPP
