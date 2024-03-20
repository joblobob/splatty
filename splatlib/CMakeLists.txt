cmake_minimum_required(VERSION 3.28)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON) 


set(Qt6_DIR "C:/Qt/Qt6.6.1/6.6.1/msvc2019_64/lib/cmake/Qt6")

find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL)

#embed!
include(FetchContent)
FetchContent_Declare(
  battery-embed
  GIT_REPOSITORY https://github.com/batterycenter/embed.git
  GIT_TAG        v1.2.8
)
FetchContent_MakeAvailable(battery-embed)

add_library(splatlib)
target_sources(splatlib
  PUBLIC
    FILE_SET my_modules TYPE CXX_MODULES FILES 
        compiletimestuff.ixx
        glsplat.ixx
        splatdata.ixx
        splatreader.ixx
        splatty.ixx
        shaders.ixx
)


#files to embed!
b_embed(splatlib res/vertexShader.vert)
b_embed(splatlib res/fragmentShader.frag)

target_link_libraries(splatlib PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
)