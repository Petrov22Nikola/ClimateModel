#define STB_IMAGE_IMPLEMENTATION

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
unsigned int texture;

void initializeObjects() {
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    // Vertices
    planetVertices.push_back(planetNormal.x);
    planetVertices.push_back(planetNormal.y);
    planetVertices.push_back(planetNormal.z);
    // Texture Coords
    planetVertices.push_back(0.5f);
    planetVertices.push_back(0.0f);
    for (int latitude = 1; latitude < latitudinalDivisor; ++latitude) {
        auto latNormal = glm::rotate(glm::mat4(1.0f), (float)latitude * PI / 18.0f, glm::vec3(0.0f, 0.0f, 1.0f)) * planetNormal;
        for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
            auto rotNormal = glm::rotate(glm::mat4(1.0f), (float)longitude * 2 * PI / 18.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * latNormal;
            planetVertices.push_back(rotNormal.x);
            planetVertices.push_back(rotNormal.y);
            planetVertices.push_back(rotNormal.z);
            // Texture Coords
            planetVertices.push_back((float)(longitude + 0.5f) / (float)longitudinalDivisor);
            planetVertices.push_back((float)latitude / (float)latitudinalDivisor);
        }
    }
    planetVertices.push_back(-planetNormal.x);
    planetVertices.push_back(-planetNormal.y);
    planetVertices.push_back(-planetNormal.z);
    // Texture Coords
    planetVertices.push_back(0.5f);
    planetVertices.push_back(1.0f);
    // Indices
    // \- Top
    for (int longitude = 0; longitude < longitudinalDivisor - 1; ++longitude) {
        planetIndices.push_back(0);
        planetIndices.push_back((longitude + 1) % longitudinalDivisor);
        planetIndices.push_back((longitude + 2) % longitudinalDivisor);
    }
    // \- Sphere
    for (int latitude = 0; latitude < latitudinalDivisor - 2; ++latitude) {
        for (int longitude = 0; longitude < longitudinalDivisor - 1; ++longitude) {
            planetIndices.push_back(1 + latitude * longitudinalDivisor + longitude);
            planetIndices.push_back(1 + (latitude + 1) * longitudinalDivisor + longitude);
            planetIndices.push_back(1 + latitude * longitudinalDivisor + ((longitude + 1) % longitudinalDivisor));
            
            planetIndices.push_back(1 + (latitude + 1) * longitudinalDivisor + ((longitude + 1) % longitudinalDivisor));
            planetIndices.push_back(1 + latitude * longitudinalDivisor + ((longitude + 1) % longitudinalDivisor));
            planetIndices.push_back(1 + (latitude + 1) * longitudinalDivisor + longitude);
        }
    }
    // \- Bottom
    int numVert = planetVertices.size() / 5, botVert = numVert - 1;
    for (int longitude = 0; longitude < longitudinalDivisor - 1; ++longitude) {
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
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glGenBuffers(1, &planetEBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, planetIndices.size() * sizeof(unsigned int), planetIndices.data(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, planetEBO);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    int width, height, nrChannels;
    unsigned char *data = stbi_load("thermalImage.png", &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else
    {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(data);
}

void renderSimulation(unsigned int shaderProgram) {
    // Render planet
    glUseProgram(shaderProgram);
    glm::mat4 mvp(1.0f);
    mvp = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    GLuint mvpMatrix = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
    glBindTexture(GL_TEXTURE_2D, texture);
    glUniform1i(glGetUniformLocation(shaderProgram, "Texture"), 0);
    glBindVertexArray(planetVAO);
    glDrawElements(GL_TRIANGLES, planetIndices.size(), GL_UNSIGNED_INT, 0);
}