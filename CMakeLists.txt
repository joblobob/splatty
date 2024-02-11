cmake_minimum_required(VERSION 3.28)
project(splatty LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 


set(Qt6_DIR "C:/Qt/Qt6.6.1/6.6.1/msvc2019_64/lib/cmake/Qt6")

find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

add_executable(splatty WIN32
    glwindow-splat.h glwindow-splat.cpp
    main.cpp
)

add_subdirectory(splatlib)

target_link_libraries(splatty PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    splatlib
)

set(INSTALL_EXAMPLEDIR "examples/opengl/splatty")

install(TARGETS splatty
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
