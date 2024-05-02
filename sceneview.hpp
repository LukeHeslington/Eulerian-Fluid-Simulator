#ifndef SCENEVIEW_HPP
#define SCENEVIEW_HPP

#include <QOpenGLWidget>
#include <QOpenGLExtraFunctions>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QMouseEvent>
#include "mainwindow.hpp"
#include "fluid.h"


class SceneView : public QOpenGLWidget, protected QOpenGLExtraFunctions
{
  Q_OBJECT

public:

  SceneView(QWidget* parent=nullptr);
  ~SceneView();

  float blueCircleX{0.0f};
  float blueCircleY{0.0f};
  float zeroX;
  float zeroY;
  bool activated_by_moving_mouse {false};

  void render_squares();
  void update_scene();

protected:

  void initializeGL() override;
  void paintGL() override;
  void resizeGL(int width, int height) override;
  void teardownGL();
  void mouseMoveEvent(QMouseEvent *event) override;

private:

  unsigned int shaderProgram{0};
  unsigned int shaderProgramForGrid{0};
  unsigned int squareVAO, squareVBO, squareEBO;
  unsigned int CircleVBO{0}, CircleVAO{0}, CircleEBO{0};
  unsigned int SquareObjectVBO, SquareObjectVAO, SquareObjectEBO;
  unsigned int TriangleObjectVBO, TriangleObjectVAO, TriangleObjectEBO;
  unsigned int OvalObjectVBO{0}, OvalObjectVAO{0}, OvalObjectEBO{0};
  unsigned int objectTypeLoc;
  unsigned int viewLoc;
  unsigned int colorLocForCircle;
  unsigned int colorLocForSquares;
  unsigned int texture;

  const int numSegments{100};
  int ballColour[3];
  int screenWidth{1200};
  int screenHeight{605};
  float squareSize;
  float cScale{1.0};
  float r, g, b;
  float x,y;

  std::vector<double> colour{255, 255, 255, 255};
  std::vector<std::vector<std::vector<double>>> colourValues;
  std::vector<glm::vec3> cellColors;
  std::string vertexShaderCode;
  std::string fragmentShaderCode;
  QImage textureImage;
  GLuint colorBuffer;

  void print_context_information();
  void draw();
  void paint_squares();
  void draw_circle_to_screen();
  void draw_square_to_screen();
  void draw_triangle_to_screen();
  void draw_oval_to_screen();
  void render_circle_object();
  void render_square_object();
  void render_triangle_object();
  void render_oval_object();
  void paint_show_pressure(Fluid* f, int i, int j, size_t n, float minP, float maxP);
  void paint_show_smoke(Fluid* f, int i, int j, size_t n, float minP, float maxP);
  float c_x(float);

  static std::vector<double> get_sci_color(double val, double minVal, double maxVal);
  std::string read_shader_code_from_resource_file(QString filepath);
  QImage read_texture_from_resource_file(QString filepath);
};

#endif
