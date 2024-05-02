#include "gtest/gtest.h"
#include "fluid.h"
#include <iostream>
#include "fluid.cpp"

const float density{1.5};
const size_t numX{1};
const size_t numY{1};
const float h{1.0};

Fluid Create_Fluid_Instance() {
    return Fluid(density, numX, numY, h);
}

TEST(Fluid, GivenInitialisedConditions_WhenCreatingAFluidInstance_CheckFluidClassIsMadeCorrectly)
{
    Fluid fluid = Create_Fluid_Instance();

    EXPECT_FLOAT_EQ(1.5, fluid.density);
    EXPECT_FLOAT_EQ(1.0, fluid.h);
}

TEST(Fluid, GivenAVector_WhenTestingToSeeIfValuesAreInputtedCorrectly_ExpectValuePutInVectorInCorrectLocation)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.set_u_velocity(0, 0, 2.5);
    fluid.set_v_velocity(1, 1, 3.0);
    fluid.set_s_velocity(2, 2, 4.5);

    EXPECT_FLOAT_EQ(fluid.u[0 * fluid.numY + 0], 2.5);
    EXPECT_FLOAT_EQ(fluid.v[1 * fluid.numY + 1], 3.0);
    EXPECT_FLOAT_EQ(fluid.m[2 * fluid.numY + 2], 4.5);
}

TEST(Fluid, GivenASVectorWithValues_WhenTestingTheIntegrateFunction_ExpectTheVVectorToBeUpdatedWithInitialValueMultipliedByGravityAndDt)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.s[1 * fluid.numY + 0]= 1.0;
    fluid.s[1 * fluid.numY + 1]= 1.0;
    fluid.set_v_velocity(1, 1, 2.0);
    fluid.integrate(0.1, 9.8);

    EXPECT_FLOAT_EQ(fluid.v[1 * fluid.numY + 1], 2.0 + 9.8 * 0.1);
}

TEST(Fluid, GivenASVectorWithValues_WhenTestingPartOfTheSolveIncompressability_CheckNeighbouringCellsAreDetectedAndSummatedCorrectlyWhenSurroundedByCells)
{
    Fluid fluid = Create_Fluid_Instance();
    fluid.s = {1,2,3,4,5,6,7,8,9};
    float result = fluid.sum_of_all_neighbours(1, 1, 3);
    ASSERT_EQ(result, 20);
}

TEST(Fluid, GivenASVectorWithValues_WhenTestingPartOfTheSolveIncompressability_CheckNeighbouringCellsAreDetectedAndSummatedCorrectlyWhenSurroundedBy3CellsAndAWall)
{
    Fluid fluid = Create_Fluid_Instance();
    fluid.s = {1,2,3,4,5,6,7,8,9};
    float result = fluid.sum_of_all_neighbours(0, 1, 3);
    ASSERT_EQ(result, 9);
}

TEST(Fluid, GivenASVectorWithValues_WhenTestingPartOfTheSolveIncompressability_CheckNeighbouringCellsAreDetectedAndSummatedCorrectlyWhenInACorner)
{
    Fluid fluid = Create_Fluid_Instance();
    fluid.s = {1,2,3,4,5,6,7,8,9};
    int i{0}; int j{0}; size_t n = 3;
    float result = fluid.sum_of_all_neighbours(i, j, n);
    ASSERT_EQ(result, 6);
}

TEST(Fluid, GivenAUVector_WhenTestingExtrapolate_CheckExtrapolateHorizontalVelocityIsWorkingAsExpected){

    Fluid fluid = Create_Fluid_Instance();
    fluid.u = {1,2,3,4,5,6,7,8,9};
    std::vector<float> expected = {2,2,2,5,5,5,8,8,8};

    for (int i{0}; i < fluid.numX; ++i) {
        fluid.extrapolate_horizontal_velocity(i, fluid.numY);
    }

    ASSERT_EQ(fluid.u, expected);
}

TEST(Fluid, GivenAUVector_WhenTestingExtrapolate_CheckExtrapolateVerticalVelocityIsWorkingAsExpected){

    Fluid fluid = Create_Fluid_Instance();
    fluid.v = {1,2,3,4,5,6,7,8,9};
    std::vector<float> expected = {4,5,6,4,5,6,4,5,6};

    for (int i{0}; i < fluid.numX; ++i) {
        fluid.extrapolate_vertical_velocity(i, fluid.numY);
    }

    ASSERT_EQ(fluid.v, expected);
}

TEST(Fluid, GivenAUAndAVVector_WhenTestingsCompleteExtrapolateFunction_ExpectCorrectValuesInsideVectors)
{
    float density = 1.5;
    size_t numX = 3;
    size_t numY = 3;
    float h = 0.1;
    Fluid fluid(density, numX, numY, h);

    fluid.set_u_velocity(0, 0, 2.0);
    fluid.set_u_velocity(fluid.numX-1, 0, 3.0);

    fluid.set_v_velocity(0, 0, 4.0);
    fluid.set_v_velocity(fluid.numX-1, 0, 5.0);

    fluid.extrapolate();

    EXPECT_EQ(fluid.u[0], fluid.u[1]);
    EXPECT_EQ(fluid.u[numX-1], fluid.u[numX-2]);

    EXPECT_EQ(fluid.v[0 * numY], fluid.v[1 * numY]);
    EXPECT_EQ(fluid.v[(numX-1) * numY], fluid.v[(numX-2) * numY]);
}

TEST(Fluid, GivenAThreeByThreeGridInitialisedFromOneToNine_WhenTestingSampleField_ExpectCorrectVectorIsUsedAndReturnValue)
{
    Fluid fluid = Create_Fluid_Instance();

    int count = 0;
    for (int i{0}; i < 3; ++i) {
        for (int j{0}; j < 3; ++j) {
            fluid.set_u_velocity(i, j, count);
            fluid.set_v_velocity(i, j, count+1);
            fluid.set_s_velocity(i, j, count+2);
            count++;
        }
    }

    EXPECT_FLOAT_EQ(fluid.sample_field(0.5, 0.5, Fluid::U_FIELD), 3.5);
    EXPECT_FLOAT_EQ(fluid.sample_field(1.0, 1.0, Fluid::V_FIELD), 3.5);
    EXPECT_FLOAT_EQ(fluid.sample_field(1.5, 1.5, Fluid::S_FIELD), 6.0);
}

TEST(Fluid, GivenAThreeByThreeGridInitialisedFromOneToNine_WhenTestingAvgU_ExpectAveragedValue)
{
    Fluid fluid = Create_Fluid_Instance();

    int count{0};
    for (int i{0}; i < 3; ++i) {
        for (int j{0}; j < 3; ++j) {
            fluid.set_u_velocity(i, j, count);
            count++;
        }
    }

    EXPECT_FLOAT_EQ(fluid.avg_u(1, 1), 5.0);
    EXPECT_FLOAT_EQ(fluid.avg_u(1, 2), 6.0);
    EXPECT_FLOAT_EQ(fluid.avg_u(2, 2), 3.75);
}

TEST(Fluid, GivenAThreeByThreeGridInitialisedFromOneToNine_WhenTestingAvgV_ExpectAveragedValue)
{
    Fluid fluid = Create_Fluid_Instance();

    int count{0};
    for (int i{0}; i < 3; ++i) {
        for (int j{0}; j < 3; ++j) {
            fluid.set_v_velocity(i, j, count);
            count++;
        }
    }

    EXPECT_FLOAT_EQ(fluid.avg_v(1, 1), 3.0);
    EXPECT_FLOAT_EQ(fluid.avg_v(1, 2), 4.0);
    EXPECT_FLOAT_EQ(fluid.avg_v(2, 2), 4.75);
}

TEST(Fluid, GivenInitialUVector_WhenCalculatingVelocityForAdvection_ExpectCorrectUVector)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.tempU = {1,5,1,1,5,1,1,5,1};
    fluid.u = {1,5,1,1,5,1,1,5,1};
    std::vector<float> expected = {1,5,1,3,5,1,1,5,1};

    fluid.compute_u_for_advect_velocity(1, 0, 0.5f * fluid.h, fluid.numY, 0.1);
    ASSERT_EQ(fluid.tempU, expected);
}

TEST(Fluid, GivenInitialVVector_WhenCalculatingVelocityForAdvection_ExpectCorrectVVectorAndExpectItToBeSymmetricVersionOfUVectorAsInputsAreSymmetricToAboveTest)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.tempV = {1,1,1,5,5,5,1,1,1};
    fluid.v = {1,1,1,5,5,5,1,1,1};
    std::vector<float> expected = {1,3,1,5,5,5,1,1,1};

    fluid.compute_v_for_advect_velocity(0, 1, 0.5f * fluid.h, fluid.numY, 0.1);
    ASSERT_EQ(fluid.tempV, expected);
}

TEST(Fluid, GivenInitialMVector_WhenCalculatingSmokeForAdvectionWhenNextToAWall_ExpectCorrectMVector)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.tempM = {1,1,1,5,5,5,1,1,1};
    fluid.m = {1,1,1,5,5,5,1,1,1};
    std::vector<float> expected = {1,1,3,5,5,5,1,1,1};

    fluid.compute_m_for_advect_smoke(0, 2, 0.5f * fluid.h, fluid.numY, 0.1);
    ASSERT_EQ(fluid.tempM, expected);
}

TEST(Fluid, GivenInitialMVector_WhenCalculatingSmokeForAdvectionWhenNextToNoWalls_ExpectCorrectMVector)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.tempM = {1,1,1,5,5,5,1,1,1};
    fluid.m = {1,1,1,5,5,5,1,1,1};
    std::vector<float> expected = {1,1,1,5,5,5,1,1,1};

    fluid.compute_m_for_advect_smoke(1, 1, 0.5f * fluid.h, fluid.numY, 0.1);
    ASSERT_EQ(fluid.tempM, expected);
}

TEST(Fluid, GivenInitialMVector_WhenCalculatingSmokeForAdvectionAcrossEntireGrid_ExpectCorrectMVector)
{
    Fluid fluid = Create_Fluid_Instance();

    fluid.m = {1,1,1,5,5,5,1,1,1};
    std::vector<float> expected = {1,1,1,5,5,5,1,1,1};

    fluid.advect_smoke(0.1);
    ASSERT_EQ(fluid.m, expected);
}
