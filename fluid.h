#ifndef TMP_IMPL_HPP
#define TMP_IMPL_HPP
#include <vector>


class Fluid
{
public:
    Fluid(float density, int numX, int numY, float h);

    constexpr static int U_FIELD{0};
    constexpr static int V_FIELD{1};
    constexpr static int S_FIELD{2};
    float density;
    float h;
    int numX;
    int numY;
    int numCells;
    float sumS;
    float sumOfAllNeighbours;
    float overRelaxation{1.9};

    std::vector<float> u;
    std::vector<float> v;
    std::vector<float> newU;
    std::vector<float> newV;
    std::vector<float> p;
    std::vector<float> s;
    std::vector<float> m;
    std::vector<float> newM;

    std::vector<float> tempU;
    std::vector<float> tempV;
    std::vector<float> tempM;

    struct Neighbours {
        float cellAboveOfCurrentCell;
        float cellBelowOfCurrentCell;
        float cellLeftOfCurrentCell;
        float cellRightOfCurrentCell;
    }neighbours;

    void integrate(float dt, float gravity);
    void solve_incompressibility(size_t numIters, float dt);
    float sum_of_all_neighbours(int i, int j, int n);
    void update_solve_incompressibilitys_vectors(float cp, int i, int j, int n);
    void extrapolate();
    void extrapolate_horizontal_velocity(int i, int gridSizeY);
    void extrapolate_vertical_velocity(int j, int gridSizeY);
    float sample_field(float x, float y, int field);
    float avg_u(size_t i, size_t j);
    float avg_v(size_t i, size_t j);
    void advect_vel(float dt);
    void compute_u_for_advect_velocity(int i, int j, float h2, int gridSizeY, float dt);
    void compute_v_for_advect_velocity(int i, int j, float h2, int gridSizeY, float dt);
    void advect_smoke(float dt);
    void compute_m_for_advect_smoke(int i, int j, float h2, int gridSizeY, float dt);
    void simulate(float dt, float gravity, size_t numIters);
    void set_v_velocity(size_t i, size_t j, float value);
    void set_u_velocity(size_t i, size_t j, float value);
    void set_s_velocity(size_t i, size_t j, float value);
};
#endif // TMP_IMPL_HPP
