#include "mainwindow.hpp"
#include "./ui_mainwindow.h"
#include <QGridLayout>
#include "sceneview.hpp"
#include <QTimer>
#include "fluid.h"
#include <QCheckBox>
#include "sceneview.hpp"
#include <cmath>

SimulationParameters params;
SceneView* mainWindowSceneView;

void MainWindow::handle_wind_tunnel_button()
{
    timer->start();
    params.sceneNr = 1;
    setup_scene();
    mainWindowSceneView->render_squares();
    mainWindowSceneView->update_scene();
}

void MainWindow::handle_tank_button()
{
    timer->start();
    params.sceneNr = 0;
    setup_scene();
    mainWindowSceneView->render_squares();
    mainWindowSceneView->update_scene();
}

void MainWindow::handle_paint_button()
{
    timer->start();
    params.sceneNr = 2;
    setup_scene();
    mainWindowSceneView->render_squares();
    mainWindowSceneView->update_scene();
}

void MainWindow::handle_pressure_checkbox(int state)
{
    if (state == Qt::Checked)
    {
        params.showPressure = true;
    }
    else
    {
        params.showPressure = false;
    }
}

void MainWindow::handle_smoke_checkbox(int state)
{
    if (state == Qt::Checked)
    {
        params.showSmoke = true;
    }
    else
    {
        params.showSmoke = false;
    }
}

void MainWindow::handle_overrelax_checkbox(int state)
{
    if (state == Qt::Checked)
    {
        params.overRelaxation = 1.9;
    }
    else
    {
        params.overRelaxation = 1.0;
    }
}

void MainWindow::combobox_current_index_changed(int index)
{
    params.shape = index;
    if (params.fluid != nullptr)
    {
        if (mainWindowSceneView->activated_by_moving_mouse)
        {
            set_obstacle(mainWindowSceneView->zeroX + params.offsetForObstacle[params.shape][0],mainWindowSceneView->zeroY + params.offsetForObstacle[params.shape][1],false);
        }
        else
        {
            set_obstacle(1.0, 0.5, true);
            mainWindowSceneView->blueCircleX = 0.0f;
            mainWindowSceneView->blueCircleY = 0.0f;
        }
    }
}

void MainWindow::create_timer()
{
    timer->setInterval(25);
    QObject::connect(timer, SIGNAL(timeout()), this, SLOT(on_timer()));
}

void MainWindow::simulate()
{
    params.fluid->simulate(params.dt, params.gravity, params.numIters);
    params.frameNr++;
}

void MainWindow::update()
{
    simulate();
    mainWindowSceneView->update_scene();
}

void MainWindow::action_exit_triggered()
{
    QApplication::quit();
}

void MainWindow::on_timer()
{
    update();
}

MainWindow::MainWindow(QWidget* parent)
  : QMainWindow(parent)
  , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    scene = new SceneView();
    mainWindowSceneView = scene;
    connect(ui->WindTunnel, SIGNAL(clicked()), this, SLOT(handle_wind_tunnel_button()));
    connect(ui->Tank, SIGNAL(clicked()), this, SLOT(handle_tank_button()));
    connect(ui->Paint, SIGNAL(clicked()), this, SLOT(handle_paint_button()));
    connect(ui->Pressure, SIGNAL(stateChanged(int)), this, SLOT(handle_pressure_checkbox(int)));
    connect(ui->Smoke, SIGNAL(stateChanged(int)), this, SLOT(handle_smoke_checkbox(int)));
    connect(ui->Overrelax, SIGNAL(stateChanged(int)), this, SLOT(handle_overrelax_checkbox(int)));
    connect(ui->ShapesBox, SIGNAL(currentIndexChanged(int)), this, SLOT(combobox_current_index_changed(int)));
    ui->ShapesBox->addItem("Circle");
    ui->ShapesBox->addItem("Square");
    ui->ShapesBox->addItem("Triangle");
    ui->ShapesBox->addItem("Oval");

    create_timer();

    QGridLayout* layout = new QGridLayout(ui->frame);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(scene);
    ui->frame->setLayout(layout);

    timer->start();
    setup_scene();
    mainWindowSceneView->update_scene();
}

MainWindow::~MainWindow()
{
    delete scene;
    scene = nullptr;
    delete ui;
}

void MainWindow::set_obstacle(float x, float y, bool reset)
{
    float vx{0.0};
    float vy{0.0};
    if (!reset)
    {
        vx = (x - params.obstacleX) / params.dt;
        vy = (y - params.obstacleY) / params.dt;
    }
    params.obstacleX = x;
    params.obstacleY = y;

    double r = params.obstacleRadius;
    Fluid* f = params.fluid;
    size_t n = f->numY;

    for (int i{1}; i < f->numX - 2; i++)
    {
        for (int j{1}; j < f->numY - 2; j++)
        {
            f->s[i * n + j] = 1.0;

            float dx = (i + 0.5) * f->h - x;
            float dy = (j + 0.5) * f->h - y;

            if (params.shape == 0)
            {
                set_obstacle_for_circle(f, i, j, n, dx, dy, r, vx, vy);
            }
            else if (params.shape == 1)
            {
                set_obstacle_for_square(f, i, j, n, dx, dy, r, vx, vy);
            }
            else if (params.shape == 2)
            {
                set_obstacle_for_triangle(f, i, j, n, dx, dy, r, vx, vy);
            }
            else if (params.shape == 3)
            {
                set_obstacle_for_oval(f, i, j, n, dx, dy, r, vx, vy);
            }

        }
    }
    params.showObstacle = true;
}

void MainWindow::setup_scene()
{
    params.overRelaxation = 1.9;
    ui->Overrelax->setChecked(true);
    params.dt = 1.0 / 60.0;
    params.numIters = 40;
    int res{100};

    if (params.sceneNr == 0)
    {
        res = 50;
    }

    double domainHeight{1.0};
    double simHeight{1.0};
    double simWidth{1.0};
    double domainWidth{domainHeight / simHeight * simWidth};
    double h{domainHeight / res};
    int numX{static_cast<int>(std::floor(2 * domainWidth / h))};
    int numY{static_cast<int>(std::floor(domainHeight / h))};
    double density{1000.0};

    params.fluid = new Fluid(density, numX, numY, h);
    size_t n = params.fluid->numY;

    if (params.sceneNr == 0)
    {
        set_scene_for_wind_tunnel(n);
    }
    else if (params.sceneNr == 1)
    {
        set_scene_for_pressure_tank(n);
    }
    else if (params.sceneNr == 2)
    {
        set_scene_for_paint(n);
    }
}

void MainWindow::set_obstacle_for_circle(Fluid* f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy)
{
    if (dx * dx + dy * dy < r * r)
    {
        f->s[i * n + j] = 0.0;
        if (params.sceneNr == 2)
        {
            f->m[i * n + j] = 0.5 + 0.5 * std::sin(0.1 * params.frameNr);
        }
        else
        {
            f->m[i * n + j] = 1.0;
            f->u[i * n + j] = vx;
            f->u[(i + 1) * n + j] = vx;
            f->v[i * n + j] = vy;
            f->v[i * n + j + 1] = vy;
        }
    }
}

void MainWindow::set_obstacle_for_square(Fluid *f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy)
{
    if (std::abs(dx) < r && std::abs(dy) < r)
    {
        f->s[i * n + j] = 0.0;
        if (params.sceneNr == 2)
        {
            f->m[i * n + j] = 0.5 + 0.5 * std::sin(0.1 * params.frameNr);
        }
        f->m[i * n + j] = 1.0;
        f->u[i * n + j] = vx;
        f->u[(i + 1) * n + j] = vx;
        f->v[i * n + j] = vy;
        f->v[i * n + j + 1] = vy;
    }
}

void MainWindow::set_obstacle_for_triangle(Fluid *f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy)
{
    if (std::abs(dx) < r && std::abs(dy) < r)
    {
        if ((dx >= 0 && dy >= 0 && dx - dy >= 0) || (dx >= 0 && dy <= 0 && dx + dy >= 0))
        {
            f->s[i * n + j] = 0.0;
            if (params.sceneNr == 2)
            {
                f->m[i * n + j] = 0.5 + 0.5 * std::sin(0.1 * params.frameNr);
            }


            f->m[i * n + j] = 1.0;
            f->u[i * n + j] = vx;
            f->u[(i + 1) * n + j] = vx;
            if (dx >= 0 && dy >= 0 && dx - dy >= 0)
            {
                f->v[i * n + j] = vy;
                f->v[i * n + j + 1] = vy;
            }
            else
            {
                f->v[i * n + j] = -vy;
                f->v[i * n + j + 1] = -vy;
            }
        }
    }
}

void MainWindow::set_obstacle_for_oval(Fluid *f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy)
{
    double ovalRadiusX{r * 1.5};
    double ovalRadiusY{r * 1.0};

    if ((dx * dx) / (ovalRadiusX * ovalRadiusX) + (dy * dy) / (ovalRadiusY * ovalRadiusY) < 1)
    {
        f->s[i * n + j] = 0.0;
        if (params.sceneNr == 2)
        {
            f->m[i * n + j] = 0.5 + 0.5 * std::sin(0.1 * params.frameNr);
        }
        f->m[i * n + j] = 1.0;
        f->u[i * n + j] = vx;
        f->u[(i + 1) * n + j] = vx;
        f->v[i * n + j] = vy;
        f->v[i * n + j + 1] = vy;
    }
}

void MainWindow::set_scene_for_wind_tunnel(size_t n)
{
    for (int i{0}; i < params.fluid->numX; ++i)
    {
        for (int j{0}; j < params.fluid->numY; ++j)
        {
            double s = 1.0;
            if (i == 0 || i == params.fluid->numX - 1 || j == 0)
            {
                s = 0.0;
            }
            params.fluid->s[i * n + j] = s;
        }
    }

    params.gravity = -9.81;
    params.showPressure = true;
    ui->Pressure->setChecked(true);
    params.showSmoke = false;
    ui->Smoke->setChecked(false);
}

void MainWindow::set_scene_for_pressure_tank(size_t n)
{
    double inVel = 2.0;
    for (int i{0}; i < params.fluid->numX; ++i)
    {
        for (int j{0}; j < params.fluid->numY; ++j)
        {
            double s = 1.0;
            if (i == 0 || j == 0 || j == params.fluid->numY - 1)
            {
                s = 0.0;
            }
            params.fluid->s[i * n + j] = s;

            if (i == 1)
            {
                params.fluid->u[i * n + j] = inVel;
            }
        }
    }

    double pipeH = 0.1 * params.fluid->numY;
    int minJ = static_cast<int>(std::floor(0.5 * params.fluid->numY - 0.5 * pipeH));
    int maxJ = static_cast<int>(std::floor(0.5 * params.fluid->numY + 0.5 * pipeH));

    for (int j{minJ}; j < maxJ; ++j)
    {
        params.fluid->m[j] = 0.0;
    }

    set_obstacle(1.0, 0.5, true);
    params.gravity = 0.0;
    params.showPressure = false;
    ui->Pressure->setChecked(false);
    params.showSmoke = true;
    ui->Smoke->setChecked(true);

}

void MainWindow::set_scene_for_paint(size_t n)
{
    params.gravity = 0.0;
    params.overRelaxation = 1.0;
    ui->Overrelax->setChecked(false);
    params.showPressure = false;
    ui->Pressure->setChecked(false);
    params.showSmoke = true;
    ui->Smoke->setChecked(true);
}




