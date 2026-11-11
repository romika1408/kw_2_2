cmake_minimum_required(VERSION 3.16)

project(Control_Work_02_Polymorphism VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Quick QuickWidgets Quick3D)

qt_standard_project_setup()

qt_add_executable(Control_Work_02_Polymorphism
    main.cpp
    uav.h
    uav.cpp
    sensor.h
    sensor.cpp
    uav_scene.h
    uav_scene.cpp
    sensor_map_widget.h
    sensor_map_widget.cpp
    mainwindow.h
    mainwindow.cpp
)

# 3D-сцена задания 1 хранится внутри программы как ресурс qrc:/uav_scene.qml
qt_add_resources(Control_Work_02_Polymorphism "scene"
    PREFIX "/"
    FILES uav_scene.qml
)

target_link_libraries(Control_Work_02_Polymorphism PRIVATE
    Qt6::Widgets
    Qt6::Quick
    Qt6::QuickWidgets
    Qt6::Quick3D
)

set_target_properties(Control_Work_02_Polymorphism PROPERTIES
    WIN32_EXECUTABLE ON
)
