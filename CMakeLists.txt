cmake_minimum_required(VERSION 3.23)

project(FinalProject VERSION 0.1 LANGUAGES CXX)
configure_file(FinalProjectConfig.h.in FinalProjectConfig.h)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets OpenGL OpenGLWidgets)
find_package(glm REQUIRED)
find_package(GTest REQUIRED)
add_executable(FinalProject_unittests)
target_sources(FinalProject_unittests PRIVATE FinalProject_unittests.cpp)
target_include_directories(FinalProject_unittests PRIVATE "${GTEST_INCLUDE_DIRS}")

target_link_libraries(FinalProject_unittests
    PRIVATE
        ${GTEST_LIBRARIES}
        ${GTEST_MAIN_LIBRARIES}

)


add_executable(${PROJECT_NAME}

    fluid.h fluid.cpp

)
target_sources(${PROJECT_NAME}
    PRIVATE
    main.cpp
    mainwindow.cpp
    mainwindow.hpp
    mainwindow.ui
    sceneview.hpp
    sceneview.cpp
    shaders.qrc
)

target_compile_features(${PROJECT_NAME} PUBLIC cxx_std_17)
target_include_directories(${PROJECT_NAME} PRIVATE)

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::OpenGL Qt::OpenGLWidgets glm::glm)
target_include_directories(${PROJECT_NAME} PRIVATE /usr/local/Cellar/glm/0.9.9.8/include)
target_include_directories(${PROJECT_NAME} PRIVATE /usr/local/Cellar/glfw/3.3.8/include)

set_target_properties(${PROJECT_NAME} PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(
    FILES
        "FinalProject Config.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/FinalProject ConfigVersion.cmake"
    DESTINATION lib/cmake/FinalProject
)

