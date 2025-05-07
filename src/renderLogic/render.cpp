#include <vector>

#include <renderLogic/render.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Constants
constexpr int longitudinalDivisor = 18;
constexpr int latitudinalDivisor = 18;
constexpr int dimensionality = 3;
constexpr float PI = 3.14;

// Planet
std::vector<float> planetVertices;
glm::vec4 planetNormal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

unsigned int planetVBO;
unsigned int planetVAO;

void initializeObjects() {
    planetVertices.push_back(planetNormal.x);
    planetVertices.push_back(planetNormal.y);
    planetVertices.push_back(planetNormal.z);
    for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
        auto longNormal = glm::rotate(glm::mat4(1.0f), longitude * PI / 18.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * planetNormal;
        for (int latitude = 0; latitude < latitudinalDivisor; ++latitude) {
            if (longitude == 0 && latitude == 0) continue;
            auto rotNormal = glm::rotate(glm::mat4(1.0f), latitude * PI / 18.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * longNormal;
            planetVertices.push_back(rotNormal.x);
            planetVertices.push_back(rotNormal.y);
            planetVertices.push_back(rotNormal.z);
        }
    }
    planetVertices.push_back(-planetNormal.x);
    planetVertices.push_back(-planetNormal.y);
    planetVertices.push_back(-planetNormal.z);
    // Planet
    glGenBuffers(1, &planetVBO);
    glGenVertexArrays(1, &planetVAO);
    glBindVertexArray(planetVAO);
    glBindBuffer(GL_ARRAY_BUFFER, planetVBO);
    glBufferData(GL_ARRAY_BUFFER, planetVertices.size() * sizeof(float), planetVertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);  
    glBindVertexArray(planetVAO);
}

void renderSimulation(unsigned int shaderProgram) {
    // Render planet
    glUseProgram(shaderProgram);
    glBindVertexArray(planetVAO);
    glDrawArrays(GL_TRIANGLES, 0, dimensionality * latitudinalDivisor * longitudinalDivisor);
}