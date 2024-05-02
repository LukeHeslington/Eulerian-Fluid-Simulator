#include "fluid.h"
#include <algorithm>
#include <cmath>
#include <iostream>

Fluid::Neighbours neighbours;

Fluid::Fluid(float density, int numX, int numY, float h)
    : density(density), numX(numX + 2), numY(numY + 2), numCells(this->numX * this->numY),
    h(h), u(numCells, 0.0), v(numCells, 0.0), newU(numCells, 0.0), newV(numCells, 0.0),
    p(numCells, 0.0), s(numCells, 0.0), m(numCells, 1.0), newM(numCells, 0.0) {
}

void Fluid::integrate(float dt, float gravity)
{
    int n = this->numY;
    for (int i = 1; i < this->numX; i++)
    {
        for (int j = 1; j < this->numY - 1; j++)
        {
            if (this->s[i * n + j] != 0.0 && this->s[i * n + j - 1] != 0.0)
                this->v[i * n + j] += gravity * dt;
        }
    }
}

void Fluid::solve_incompressibility(size_t numIters, float dt)
{
    int n = this->numY;
    float cp = this->density * this->h / dt;

    for (int iter = 0; iter < numIters; iter++)
    {
        for (int i = 1; i < this->numX - 1; i++)
        {
            for (int j = 1; j < this->numY - 1; j++)
            {
                if (this->s[i * n + j] == 0.0)
                    continue;

                sumOfAllNeighbours = sum_of_all_neighbours(i, j, n);

                if (sumOfAllNeighbours == 0.0)
                    continue;

                update_solve_incompressibilitys_vectors(cp, i,j,n);
            }
        }
    }
}

float Fluid::sum_of_all_neighbours(int i, int j, int n)
{
    neighbours.cellAboveOfCurrentCell = this->s[(i - 1) * n + j];
    neighbours.cellBelowOfCurrentCell = this->s[(i + 1) * n + j];
    neighbours.cellLeftOfCurrentCell = this->s[i * n + j - 1];
    neighbours.cellRightOfCurrentCell = this->s[i * n + j + 1];
    return neighbours.cellAboveOfCurrentCell + neighbours.cellBelowOfCurrentCell + neighbours.cellLeftOfCurrentCell + neighbours.cellRightOfCurrentCell;
}

void Fluid::update_solve_incompressibilitys_vectors(float cp, int i, int j, int n)
{
    float div = this->u[(i + 1) * n + j] - this->u[i * n + j] + this->v[i * n + j + 1] - this->v[i * n + j];
    float p = -div / sumOfAllNeighbours;
    p *= overRelaxation;
    this->p[i * n + j] += cp * p;
    this->u[i * n + j] -= neighbours.cellAboveOfCurrentCell * p;
    this->u[(i + 1) * n + j] += neighbours.cellBelowOfCurrentCell * p;
    this->v[i * n + j] -= neighbours.cellLeftOfCurrentCell * p;
    this->v[i * n + j + 1] += neighbours.cellRightOfCurrentCell * p;
}

void Fluid::extrapolate()
{
    int gridSizeY = this->numY;

    for (int i = 0; i < this->numX; i++)
    {
        extrapolate_horizontal_velocity(i, gridSizeY);
    }

    for (int j = 0; j < this->numY; j++)
    {
        extrapolate_vertical_velocity(j, gridSizeY);
    }
}

void Fluid::extrapolate_horizontal_velocity(int i, int gridSizeY)
{
    this->u[i * gridSizeY] = this->u[i * gridSizeY + 1];
    this->u[i * gridSizeY + this->numY - 1] = this->u[i * gridSizeY + this->numY - 2];
}

void Fluid::extrapolate_vertical_velocity(int j, int gridSizeY)
{
    this->v[j] = this->v[gridSizeY + j];
    this->v[(this->numX - 1) * gridSizeY + j] = this->v[(this->numX - 2) * gridSizeY + j];
}

float Fluid::sample_field(float x, float y, int field)
{
    int gridSizeY = this->numY;
    float h = this->h;
    float h1 = 1.0f / h;
    float h2 = 0.5f * h;

    x = std::max(std::min(x, this->numX * h), h);
    y = std::max(std::min(y, this->numY * h), h);

    float dx = 0.0f;
    float dy = 0.0f;

    const float* f;

    switch (field)
    {
        case U_FIELD: f = this->u.data(); dy = h2; break;
        case V_FIELD: f = this->v.data(); dx = h2; break;
        case S_FIELD: f = this->m.data(); dx = h2; dy = h2; break;
        default: return 0.0f;
    }

    int x0 = std::min(static_cast<int>(std::floor((x - dx) * h1)), this->numX - 1);
    float tx = ((x - dx) - x0 * h) * h1;
    int x1 = std::min(x0 + 1, this->numX - 1);

    int y0 = std::min(static_cast<int>(std::floor((y - dy) * h1)), this->numY - 1);
    float ty = ((y - dy) - y0 * h) * h1;
    int y1 = std::min(y0 + 1, this->numY - 1);

    float sx = 1.0f - tx;
    float sy = 1.0f - ty;

    float val = sx * sy * f[x0 * gridSizeY + y0] +
                tx * sy * f[x1 * gridSizeY + y0] +
                tx * ty * f[x1 * gridSizeY + y1] +
                sx * ty * f[x0 * gridSizeY + y1];

    return val;
}

float Fluid::avg_u(size_t i, size_t j)
{
    size_t n = this->numY;
    float u = (this->u[i*n + j-1] + this->u[i*n + j] + this->u[(i+1)*n + j-1] + this->u[(i+1)*n + j]) * 0.25;
    return u;
}

float Fluid::avg_v(size_t i, size_t j)
{
    size_t n = this->numY;
    float v = (this->v[(i-1)*n + j] + this->v[i*n + j] + this->v[(i-1)*n + j+1] + this->v[i*n + j+1]) * 0.25;
    return v;
}

void Fluid::advect_vel(float dt)
{
    tempU = this->u;
    tempV = this->v;

    int gridSizeY = this->numY;
    float h = this->h;
    float h2 = 0.5f * h;

    for (int i = 1; i < this->numX; i++)
    {
        for (int j = 1; j < this->numY; j++)
        {
            if (this->s[i * gridSizeY + j] != 0.0 && this->s[(i - 1) * gridSizeY + j] != 0.0 && j < this->numY - 1)
            {
                compute_u_for_advect_velocity(i, j, h2, gridSizeY, dt);
            }
            if (this->s[i * gridSizeY + j] != 0.0 && this->s[i * gridSizeY + j - 1] != 0.0 && i < this->numX - 1)
            {
                compute_v_for_advect_velocity(i, j, h2, gridSizeY, dt);
            }
        }
    }

    this->u = tempU;
    this->v = tempV;
}

void Fluid::compute_u_for_advect_velocity(int i, int j, float h2, int gridSizeY, float dt)
{
    float x = i * h;
    float y = j * h + h2;
    float u = this->u[i * gridSizeY + j];
    float v = this->avg_v(i, j);
    x = x - dt * u;
    y = y - dt * v;
    u = this->sample_field(x, y, U_FIELD);
    tempU[i * gridSizeY + j] = u;
}

void Fluid::compute_v_for_advect_velocity(int i, int j, float h2, int gridSizeY, float dt)
{
    float x = i * h + h2;
    float y = j * h;
    float u = this->avg_u(i, j);
    float v = this->v[i * gridSizeY + j];
    x = x - dt * u;
    y = y - dt * v;
    v = this->sample_field(x, y, V_FIELD);
    tempV[i * gridSizeY + j] = v;
}

void Fluid::advect_smoke(float dt)
{
    tempM = this->m;

    int gridSizeY = this->numY;
    float h = this->h;
    float h2 = 0.5f * h;

    for (int i = 1; i < this->numX - 1; i++)
    {
        for (int j = 1; j < this->numY - 1; j++)
        {
            if (this->s[i * gridSizeY + j] != 0.0) {
                compute_m_for_advect_smoke(i, j, h2, gridSizeY, dt);
            }
        }
    }

    this->m = tempM;
}

void Fluid::compute_m_for_advect_smoke(int i, int j, float h2, int gridSizeY, float dt)
{
    float u = (this->u[i * gridSizeY + j] + this->u[(i + 1) * gridSizeY + j]) * 0.5f;
    float v = (this->v[i * gridSizeY + j] + this->v[i * gridSizeY + j + 1]) * 0.5f;
    float x = i * h + h2 - dt * u;
    float y = j * h + h2 - dt * v;

    tempM[i * gridSizeY + j] = this->sample_field(x, y, S_FIELD);
}

void Fluid::set_v_velocity(size_t i, size_t j, float value)
{
    this->v[i*this->numY +j]= value;
}

void Fluid::set_u_velocity(size_t i, size_t j, float value)
{
    this->u[i*this->numY +j]= value;
}

void Fluid::set_s_velocity(size_t i, size_t j, float value)
{
    this->m[i*this->numY +j]= value;
}

void Fluid::simulate(float dt, float gravity, size_t numIters)
{
    this->integrate(dt, gravity);
    this->p.assign(this->p.size(), 0.0);
    this->solve_incompressibility(numIters, dt);
    this->extrapolate();
    this->advect_vel(dt);
    this->advect_smoke(dt);
}
