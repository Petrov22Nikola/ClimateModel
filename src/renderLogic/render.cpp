#include <vector>
#include <iostream>

#include <renderLogic/render.h>
#include <renderLogic/stb_image.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Constants
constexpr int longitudinalDivisor = 18;
constexpr int latitudinalDivisor = 18;
constexpr int dimensionality = 3;
constexpr float PI = 3.14;

// Planet
std::vector<float> planetVertices;
std::vector<unsigned int> planetIndices;
glm::vec4 planetNormal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

unsigned int planetVBO;
unsigned int planetVAO;
unsigned int planetEBO;

void initializeObjects() {
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    // Vertices
    planetVertices.push_back(planetNormal.x);
    planetVertices.push_back(planetNormal.y);
    planetVertices.push_back(planetNormal.z);
    for (int latitude = 1; latitude < latitudinalDivisor; ++latitude) {
        auto latNormal = glm::rotate(glm::mat4(1.0f), (float)latitude * PI / 18.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * planetNormal;
        for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
            auto rotNormal = glm::rotate(glm::mat4(1.0f), (float)longitude * 2 * PI / 18.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * latNormal;
            planetVertices.push_back(rotNormal.x);
            planetVertices.push_back(rotNormal.y);
            planetVertices.push_back(rotNormal.z);
        }
    }
    planetVertices.push_back(-planetNormal.x);
    planetVertices.push_back(-planetNormal.y);
    planetVertices.push_back(-planetNormal.z);
    // Indices
    // \- Top
    for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
        planetIndices.push_back(0);
        planetIndices.push_back((longitude + 1) % longitudinalDivisor);
        planetIndices.push_back((longitude + 2) % longitudinalDivisor);
    }
    // \- Sphere
    for (int latitude = 0; latitude < latitudinalDivisor - 2; ++latitude) {
        for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
            planetIndices.push_back(1 + latitude * latitudinalDivisor + longitude);
            planetIndices.push_back(1 + (latitude + 1) * latitudinalDivisor + longitude);
            planetIndices.push_back(1 + latitude * latitudinalDivisor + ((longitude + 1) % longitudinalDivisor));
            planetIndices.push_back(1 + latitude * latitudinalDivisor + ((longitude + 1) % longitudinalDivisor));
            planetIndices.push_back(1 + (latitude + 1) * latitudinalDivisor + ((longitude + 1) % longitudinalDivisor));
            planetIndices.push_back(1 + (latitude + 1) * latitudinalDivisor + longitude);
        }
    }
    // \- Bottom
    int numVert = planetVertices.size() / 3, botVert = numVert - 1;
    for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
        planetIndices.push_back(botVert);
        planetIndices.push_back(botVert + ((-longitude - 1) % longitudinalDivisor));
        planetIndices.push_back(botVert + ((-longitude - 2) % longitudinalDivisor));
    }

    // Planet
    glGenBuffers(1, &planetVBO);
    glGenVertexArrays(1, &planetVAO);
    glBindVertexArray(planetVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planetVBO);
    glBufferData(GL_ARRAY_BUFFER, planetVertices.size() * sizeof(float), planetVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &planetEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planetIndices.size() * sizeof(unsigned int), planetIndices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetEBO);
}

void renderSimulation(unsigned int shaderProgram) {
    // Render planet
    glUseProgram(shaderProgram);
    glm::mat4 mvp(1.0f);
    mvp = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    GLuint mvpMatrix = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
    glBindVertexArray(planetVAO);
    glDrawElements(GL_TRIANGLES, planetIndices.size(), GL_UNSIGNED_INT, 0);
}