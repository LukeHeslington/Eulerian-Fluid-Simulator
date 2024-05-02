#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QTimer>
#include "fluid.h"
#include <QCheckBox>
class SceneView;

struct SimulationParameters {
    double gravity{-9.81};
    double dt{1.0 / 120.0};
    int numIters{100};
    int frameNr{0};
    double overRelaxation{1.9};
    double obstacleX{0.0};
    double obstacleY{0.0};
    double obstacleRadius{0.085};
    bool paused{false};
    int sceneNr{1};
    bool showObstacle{false};
    bool showPressure{false};
    bool showSmoke{true};
    Fluid* fluid{nullptr};
    int shape{0};
    float offsetForObstacle[4][2]{
        {0.01, 0.015}, // 0 is for Circle
        {0.03, 0.015}, // 1 is for Square
        {0.03, 0.015}, // 2 is for Triangle
        {0.01, 0.015} // 2 is for Oval
    };
};

extern SimulationParameters params;

QT_BEGIN_NAMESPACE
namespace Ui
{
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();
  void set_obstacle(float, float, bool);

public slots:
  void action_exit_triggered();
  void handle_wind_tunnel_button();
  void handle_tank_button();
  void handle_paint_button();
  void on_timer();
  void handle_pressure_checkbox(int state);
  void handle_smoke_checkbox(int state);
  void handle_overrelax_checkbox(int state);
  void combobox_current_index_changed(int index);

protected:
  void create_scene();
  void create_timer();
  void simulate();
  void update();

private:
  Ui::MainWindow* ui;
  SceneView* scene;
  QTimer* timer{new QTimer(this)};

  float x;
  float y;
  bool reset;

  void setup_scene();
  void set_obstacle_for_circle(Fluid* f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy);
  void set_obstacle_for_square(Fluid* f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy);
  void set_obstacle_for_triangle(Fluid* f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy);
  void set_obstacle_for_oval(Fluid* f, int i, int j, size_t n, float dx, float dy, double r, float vx, float vy);
  void set_scene_for_wind_tunnel(size_t n);
  void set_scene_for_pressure_tank(size_t n);
  void set_scene_for_paint(size_t n);
};
#endif // MAINWINDOW_HPP
