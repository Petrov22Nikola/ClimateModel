cmake_minimum_required(VERSION 3.15...4.0)
project(
  ClimateSimulation
  VERSION 1.0
  LANGUAGES C CXX)

# GLFW
add_library(glfw3 STATIC IMPORTED)
set_target_properties(glfw3 PROPERTIES
  IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw3.lib"
  INTERFACE_INCLUDE_DIRECTORIES "${CMAKE_CURRENT_SOURCE_DIR}/include"
)
# GLAD
add_library(glad STATIC src/glad.c)
target_include_directories(glad PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

set(SOURCES 
  src/main.cpp
  src/renderLogic/render.cpp
  src/fileReader.cpp
  src/dataScanner.cpp
  src/coordHandler.cpp)
add_executable(Simulation ${SOURCES})
target_include_directories(Simulation PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
find_package(OpenGL REQUIRED)
find_package(CURL CONFIG REQUIRED)
target_link_libraries(Simulation glad glfw3 CURL::libcurl OpenGL::GL)