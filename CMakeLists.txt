# Copyright (C) 2022 The Qt Company Ltd.
# SPDX-License-Identifier: BSD-3-Clause

cmake_minimum_required(VERSION 3.28)
project(hellogles3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 

if(NOT DEFINED INSTALL_EXAMPLESDIR)
    set(INSTALL_EXAMPLESDIR "examples")
endif()

set(INSTALL_EXAMPLEDIR "${INSTALL_EXAMPLESDIR}/opengl/hellogles3")

set(Qt6_DIR "C:/Qt/Qt6.6.1/6.6.1/msvc2019_64/lib/cmake/Qt6")


find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL)

qt_standard_project_setup()
 
qt_add_executable(hellogles3 WIN32
    glwindow-splat.h glwindow-splat.cpp
    main.cpp
)

add_library(gllib)
target_sources(gllib
  PUBLIC
    FILE_SET cxx_modules TYPE CXX_MODULES FILES
    
)

set_target_properties(hellogles3 PROPERTIES
    WIN32_EXECUTABLE TRUE
)

target_link_libraries(hellogles3 PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    gllib
)

# Resources:
set(hellogles3_resource_files
    ""
)

qt_add_resources(hellogles3 "hellogles3"
    PREFIX
        "/"
    FILES
        ${hellogles3_resource_files}
)

install(TARGETS hellogles3
    RUNTIME DESTINATION "${INSTALL_EXAMPLEDIR}"
    BUNDLE DESTINATION "${INSTALL_EXAMPLEDIR}"
    LIBRARY DESTINATION "${INSTALL_EXAMPLEDIR}"
)
