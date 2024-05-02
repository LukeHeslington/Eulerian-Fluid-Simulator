#include "sceneview.hpp"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <QFile>
#include <QDebug>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <iostream>
#include "mainwindow.hpp"
#include "fluid.h"
#include <algorithm>

extern SimulationParameters params;
MainWindow* mainW;

SceneView::SceneView(QWidget* parent): QOpenGLWidget(parent)
{
    setMinimumSize(screenWidth, screenHeight);
}

std::string SceneView::read_shader_code_from_resource_file(QString filepath)
{
    QString data;
    QFile file(filepath);

    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug()<<"file not opened";
    }
    else
    {
        data = file.readAll();
    }
    return data.toStdString();
}

SceneView::~SceneView()
{
    teardownGL();
}

void SceneView::initializeGL()
{
    initializeOpenGLFunctions();
    print_context_information();
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    vertexShaderCode = read_shader_code_from_resource_file(":/shaders/pass_through.vert");
    fragmentShaderCode = read_shader_code_from_resource_file(":/shaders/simple.frag");

    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    const char* vertexShaderString = vertexShaderCode.c_str();
    glShaderSource(vertexShader, 1, &vertexShaderString, NULL);
    glCompileShader(vertexShader);

    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        qDebug() << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog;
    }

    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    const char* fragmentShaderString = fragmentShaderCode.c_str();
    glShaderSource(fragmentShader, 1, &fragmentShaderString, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        qDebug() << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog;
    }

    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        qDebug() << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog;
    }

    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    render_squares();
    render_circle_object();
    render_square_object();
    render_triangle_object();
    render_oval_object();

}

void SceneView::paintGL()
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
    glUseProgram(shaderProgram);

    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)screenWidth / (float)screenHeight, 0.1f, 100.0f);
    unsigned int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    objectTypeLoc = glGetUniformLocation(shaderProgram, "objectType");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, &projection[0][0]);
    colorLocForCircle = glGetUniformLocation(shaderProgram, "currentColorForCircle");

    if (params.fluid != nullptr)
    {
        Fluid* f = params.fluid;
        size_t n = f->numY;
        float h = f->h;
        float minP = f->p[0];
        float maxP = f->p[0];

        for (int i = 0; i < params.fluid->numCells; i++)
        {
            minP = std::min(minP, f->p[i]);
            maxP = std::max(maxP, f->p[i]);
        }
        colourValues.resize(f->numX, std::vector<std::vector<double>>(f->numY, std::vector<double>(3, 0)));
        for (int i{0}; i < f->numX; i++)
        {
            for (int j{0}; j < f->numY; j++)
            {
                if(params.showPressure)
                {
                    paint_show_pressure(f, i, j, n, minP, maxP);
                }
                else if (params.showSmoke)
                {
                    paint_show_smoke(f, i, j, n, minP, maxP);
                }
                else if (f->s[i*n + j] == 0.0)
                {
                    colour[0] = 0;
                    colour[1] = 0;
                    colour[2] = 0;
                    colourValues[i][j] = colour;
                }

                x = std::floor(c_x(i * h));
                r = colour[0];
                g = colour[1];
                b = colour[2];
                colourValues[i][j] = colour;
            }
        }

        if (params.showObstacle)
        {
            if (params.showPressure)
            {
                ballColour[0] = 255;
                ballColour[1] = 255;
                ballColour[2] = 255;
            }
            else
            {
                ballColour[0] = 201;
                ballColour[1] = 36;
                ballColour[2] = 201;
            }
        }
    }

    if (params.shape == 0)
    {
        draw_circle_to_screen();
    }
    else if (params.shape == 1)
    {
        draw_square_to_screen();
    }
    else if (params.shape == 2)
    {
        draw_triangle_to_screen();
    }
    else if (params.shape == 3)
    {
        draw_oval_to_screen();
    }

    paint_squares();
    glBindVertexArray(0);
}


void SceneView::resizeGL(int width, int height)
{
}

void SceneView::teardownGL()
{
  makeCurrent();
}

void SceneView::print_context_information()
{
    QString glType{(context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL"};
    QString glVersion{reinterpret_cast<const char*>(glGetString(GL_VERSION))};
    QString glProfile;

    #define CASE(c) case QSurfaceFormat::c: glProfile = #c; break

    switch (format().profile())
    {
        CASE(NoProfile);
        CASE(CoreProfile);
        CASE(CompatibilityProfile);
    }

    #undef CASE
    qDebug() << qPrintable(glType) << qPrintable(glVersion) << "(" << qPrintable(glProfile) << ")";
}

void SceneView::paint_squares()
{
    if (params.fluid != nullptr)
    {
        size_t numXValue = params.fluid->numX;
        size_t numYValue = params.fluid->numY;

        for (int i{0}; i < numXValue; ++i)
        {
            for (int j{0}; j < numYValue; ++j)
            {
                glm::mat4 viewSquare = glm::mat4(1.0f);
                float xPos = (i - (0.5f) * (numXValue - 1)) * squareSize;
                float yPos = (j - (0.5f) * (numYValue - 1)) * squareSize;
                viewSquare = glm::translate(viewSquare, glm::vec3(xPos, yPos, -3.0f));
                unsigned int viewLocSquare = glGetUniformLocation(shaderProgram, "viewSquare");
                glUniformMatrix4fv(viewLocSquare, 1, GL_FALSE, glm::value_ptr(viewSquare));
                unsigned int colorLocForSquares = glGetUniformLocation(shaderProgram, "currentColorForSquares");
                glUniform3f(colorLocForSquares, colourValues[i][j][0] / 255.0f, colourValues[i][j][1] / 255.0f, colourValues[i][j][2] / 255.0f);
                glUniform1i(objectTypeLoc, 2);
                glBindVertexArray(squareVAO);
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
            }
        }
    }
}

void SceneView::draw_circle_to_screen()
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(blueCircleX, blueCircleY, -3.0f));
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3f(colorLocForCircle, ballColour[0] / 255.0f, ballColour[1] / 255.0f, ballColour[2] / 255.0f);
    glUniform1i(objectTypeLoc, 0);
    glBindVertexArray(CircleVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
}

void SceneView::draw_square_to_screen()
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(blueCircleX, blueCircleY, -3.0f));
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3f(colorLocForCircle, ballColour[0] / 255.0f, ballColour[1] / 255.0f, ballColour[2] / 255.0f);
    glUniform1i(objectTypeLoc, 0);

    glUseProgram(shaderProgram);
    glBindVertexArray(SquareObjectVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SquareObjectEBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SceneView::draw_triangle_to_screen()
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(blueCircleX, blueCircleY, -3.0f));
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3f(colorLocForCircle, ballColour[0] / 255.0f, ballColour[1] / 255.0f, ballColour[2] / 255.0f);
    glUniform1i(objectTypeLoc, 0);

    glUseProgram(shaderProgram);
    glBindVertexArray(TriangleObjectVAO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleObjectEBO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void SceneView::draw_oval_to_screen()
{
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(blueCircleX, blueCircleY, -3.0f));
    viewLoc = glGetUniformLocation(shaderProgram, "view");
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, &view[0][0]);
    glUniform3f(colorLocForCircle, ballColour[0] / 255.0f, ballColour[1] / 255.0f, ballColour[2] / 255.0f);
    glUniform1i(objectTypeLoc, 0);
    glBindVertexArray(OvalObjectVAO);
    glDrawArrays(GL_TRIANGLE_FAN, 0, 360);
}


float SceneView::c_x(float x)
{
    return x * cScale;
}

std::vector<double> SceneView::get_sci_color(double val, double minVal, double maxVal)
{
    val = std::min(std::max(val, minVal), maxVal - 0.0001);
    double d{maxVal - minVal};
    val = (d == 0.0) ? 0.5 : (val - minVal) / d;
    double m{0.25};
    int num{static_cast<int>(val / m)};
    double s{(val - num * m) / m};
    double r, g, b;

    switch (num) {
        case 0: r = 0.0; g = s; b = 1.0; break;
        case 1: r = 0.0; g = 1.0; b = 1.0 - s; break;
        case 2: r = s; g = 1.0; b = 0.0; break;
        case 3: r = 1.0; g = 1.0 - s; b = 0.0; break;
    }
    return {static_cast<double>(255 * r), static_cast<double>(255 * g), static_cast<double>(255 * b), 255};
}

void SceneView::render_squares()
{
    float squareVertices[]
    {
        -0.01f, -0.01f, 0.0f,
        0.01f, -0.01f, 0.0f,
        0.01f,  0.01f, 0.0f,
        -0.01f,  0.01f, 0.0f
    };

    unsigned int squareIndices[]
    {
        0, 1, 2,
        0, 2, 3
    };

    glGenVertexArrays(1, &squareVAO);
    glGenBuffers(1, &squareVBO);
    glGenBuffers(1, &squareEBO);
    glBindVertexArray(squareVAO);
    glBindBuffer(GL_ARRAY_BUFFER, squareVBO);

    float scaler{1.22};
    if (params.sceneNr == 0)
    {
        scaler *= 2;
        squareSize = 0.0240 * 2;
    }
    else if (params.sceneNr == 2 || params.sceneNr == 1){
        squareSize = 0.0243f;
    }
    else
    {
        scaler /= 2;
        squareSize = 0.0243 / 2;
    }

    for (int i{0}; i < 12; ++i)
    {
        squareVertices[i] *= scaler;
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(squareVertices), squareVertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, squareEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(squareIndices), squareIndices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SceneView::render_circle_object()
{
    unsigned int indices[]
    {
        0, 1, 3,
        1, 2, 3
    };

    float vertices[360 * 5];
    float scale{0.2f};

    for (int i{0}; i < 360; ++i)
    {
        float angle = glm::radians(static_cast<float>(i));
        vertices[i * 5] = scale * cos(angle);
        vertices[i * 5 + 1] = scale * sin(angle);
        vertices[i * 5 + 2] = 0.1f;
        vertices[i * 5 + 3] = (vertices[i * 5] + 1.0f) / 2.0f;
        vertices[i * 5 + 4] = (vertices[i * 5 + 1] + 1.0f) / 2.0f;
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &CircleVAO);
    glGenBuffers(1, &CircleVBO);
    glGenBuffers(1, &CircleEBO);
    glBindVertexArray(CircleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, CircleVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, CircleEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void SceneView::render_square_object()
{
    float vertices[]
    {
        0.2f,  0.2f, 0.0f,
        0.2f, -0.2f, 0.0f,
        -0.2f, -0.2f, 0.0f,
        -0.2f,  0.2f, 0.0f
    };

    unsigned int indices[]
    {
        0, 1, 3,
        1, 2, 3
    };

    glGenVertexArrays(1, &SquareObjectVAO);
    glGenBuffers(1, &SquareObjectVBO);
    glGenBuffers(1, &SquareObjectEBO);
    glBindVertexArray(SquareObjectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, SquareObjectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, SquareObjectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SceneView::render_triangle_object()
{
    float vertices[]
    {
        0.2f,  0.2f, 0.0f,
        0.2f, -0.2f, 0.0f,
        -0.2f, -0.2f, 0.0f,
    };

    unsigned int indices[]
    {
        0, 1, 3
    };

    glGenVertexArrays(1, &TriangleObjectVAO);
    glGenBuffers(1, &TriangleObjectVBO);
    glGenBuffers(1, &TriangleObjectEBO);
    glBindVertexArray(TriangleObjectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, TriangleObjectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, TriangleObjectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

void SceneView::render_oval_object()
{
    unsigned int indices[359 * 3];
    float vertices[360 * 5];
    float scale_x{0.3f};
    float scale_y{0.2f};


    for (int i{0}; i < 360; ++i)
    {
        float angle = glm::radians(static_cast<float>(i));
        vertices[i * 5] = scale_x * cos(angle);
        vertices[i * 5 + 1] = scale_y * sin(angle);
        vertices[i * 5 + 2] = 0.1f;
        vertices[i * 5 + 3] = (vertices[i * 5] + 1.0f) / 2.0f;
        vertices[i * 5 + 4] = (vertices[i * 5 + 1] + 1.0f) / 2.0f;

        if (i > 0)
        {
            indices[(i - 1) * 3] = 0;
            indices[(i - 1) * 3 + 1] = i - 1;
            indices[(i - 1) * 3 + 2] = i;
        }
    }

    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glGenVertexArrays(1, &OvalObjectVAO);
    glGenBuffers(1, &OvalObjectVBO);
    glGenBuffers(1, &OvalObjectEBO);
    glBindVertexArray(OvalObjectVAO);
    glBindBuffer(GL_ARRAY_BUFFER, OvalObjectVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, OvalObjectEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
}

void SceneView::paint_show_pressure(Fluid* f, int i, int j, size_t n, float minP, float maxP)
{
    float p = f->p[i*n + j];
    float s = f->m[i*n + j];
    std::vector<double> sciColor = get_sci_color(p, minP, maxP);
    for (int k{0}; k < 4; ++k)
    {
        colour[k] = static_cast<double>(sciColor[k]);
    }
    if (params.showSmoke)
    {
        colour[0] = std::max(0.0, colour[0] - 255*s);
        colour[1] = std::max(0.0, colour[1] - 255*s);
        colour[2] = std::max(0.0, colour[2] - 255*s);
    }
    colourValues[i][j] = colour;
}

void SceneView::paint_show_smoke(Fluid *f, int i, int j, size_t n, float minP, float maxP)
{
    float s = f->m[i*n + j];
    colour[0] = 255*s;
    colour[1] = 255*s;
    colour[2] = 255*s;
    if (params.sceneNr == 2)
    {
        std::vector<double> sciColor = get_sci_color(s, 0.0, 1.0);
        for (int i = 0; i < 4; ++i)
        {
            colour[i] = static_cast<double>(sciColor[i]) * 255;
        }
    }
    colourValues[i][j] = colour;
}

void SceneView::update_scene()
{
    update();
}

void SceneView::mouseMoveEvent(QMouseEvent *event)
{

    if (event->buttons() & Qt::LeftButton) {

        activated_by_moving_mouse = true;
        QPointF localPos = event->position();
        float mouseX = localPos.x();
        float mouseY = screenHeight - localPos.y();

        zeroX = (mouseX / screenWidth) * 2;
        zeroY = (mouseY / screenHeight);

        mainW->set_obstacle(zeroX + params.offsetForObstacle[params.shape][0],zeroY + params.offsetForObstacle[params.shape][1],false);


        blueCircleX = (((mouseX - 40) / 1120) * 4.34 - 2.17);
        blueCircleY = ((mouseY - 40) / 520) * 2 - 1;

        update();
    }
}
