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

class QCustomPlot;
class QCPColorMap;
class QCPBars;
class QSpinBox;
class QTimer;

class multires_grid_t;
class monores_grid_t;
class theory_t;
class node_t;

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

    multires_grid_t *m_grid_multi = nullptr;
    monores_grid_t  *m_grid_mono  = nullptr;
    theory_t *m_theory = nullptr;

    QCustomPlot *customPlot;
    QCPColorMap *colorMap;

    QCustomPlot *customPlotTheory;
    QCPColorMap *colorMapTheory;

    std::array<QCPBars*,1> bars;
    size_t count_nodes_packed;
    QSpinBox *spinBox;
    QTimer *timer;

    size_t N;
    size_t N2;

private slots:

    void actionRun();
    void replot();
    void rescale();
    void autoPlayToggled(bool checked);

    void deleteGrids();
    void initializeGrids();
};

#endif // MAINWINDOW_HPP
