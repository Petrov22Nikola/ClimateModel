#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <core/coordHandler.h>

// Render Earth & associated objects
void renderSimulation(unsigned int shaderProgram, Coords cityCoords, bool thermalView);
void initializeObjects();