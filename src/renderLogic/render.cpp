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
constexpr float PI = 3.14f;
constexpr float longitudeCorrection = -90.0f;

// Planet
std::vector<float> planetVertices;
std::vector<unsigned int> planetIndices;
glm::vec4 planetNormal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

unsigned int planetVBO;
unsigned int planetVAO;
unsigned int planetEBO;
unsigned int thermalTexture;
unsigned int physicalTexture;

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
        for (int longitude = 0; longitude <= longitudinalDivisor; ++longitude) {
            int lonWrapped = longitude % longitudinalDivisor;
            auto rotNormal = glm::rotate(glm::mat4(1.0f), (float)lonWrapped * 2 * PI / 18.0f, glm::vec3(0.0f, 1.0f, 0.0f)) * latNormal;
            planetVertices.push_back(rotNormal.x);
            planetVertices.push_back(rotNormal.y);
            planetVertices.push_back(rotNormal.z);
            // Texture Coords
            float u = (longitude == longitudinalDivisor)
                ? 1.0f
                : (float)(longitude) / (float)(longitudinalDivisor);
            planetVertices.push_back(u);
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
    for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
        int next = (longitude + 1) % longitudinalDivisor;
        planetIndices.push_back(0);
        planetIndices.push_back(1 + longitude);
        planetIndices.push_back(1 + next);
    }
    // \- Sphere
    int ringWidth = longitudinalDivisor + 1;
    for (int latitude = 0; latitude < latitudinalDivisor - 2; ++latitude) {
        for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
            planetIndices.push_back(1 + latitude * ringWidth + longitude);
            planetIndices.push_back(1 + (latitude + 1) * ringWidth + longitude);
            planetIndices.push_back(1 + latitude * ringWidth + longitude + 1);

            planetIndices.push_back(1 + (latitude + 1) * ringWidth + longitude + 1);
            planetIndices.push_back(1 + latitude * ringWidth + longitude + 1);
            planetIndices.push_back(1 + (latitude + 1) * ringWidth + longitude);
        }
    }
    // \- Bottom
    int numVert = planetVertices.size() / 5, botVert = numVert - 1, firstOnRing = botVert - (longitudinalDivisor + 1);
    for (int longitude = 0; longitude < longitudinalDivisor; ++longitude) {
        int next = (longitude + 1) % longitudinalDivisor;
        planetIndices.push_back(botVert);
        planetIndices.push_back(firstOnRing + next);
        planetIndices.push_back(firstOnRing + longitude);
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

    // Thermal Texture
    glGenTextures(1, &thermalTexture);
    glBindTexture(GL_TEXTURE_2D, thermalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    int width, height, nrChannels;
    unsigned char *thermalData = stbi_load("thermalImage.png", &width, &height, &nrChannels, 0);
    if (thermalData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, thermalData);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(thermalData);

    // Physical Texture
    glGenTextures(1, &physicalTexture);
    glBindTexture(GL_TEXTURE_2D, physicalTexture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    unsigned char *physData = stbi_load("physicalWorldMap.jpg", &width, &height, &nrChannels, 0);
    if (physData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, physData);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(physData);
}

void renderSimulation(unsigned int shaderProgram, Coords cityCoords) {
    // Render planet
    glUseProgram(shaderProgram);
    glm::mat4 mvp(1.0f);
    // mvp = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    mvp = glm::rotate(glm::mat4(1.0f), (float)((-cityCoords.longitude + longitudeCorrection) * (PI / 180.0) * std::min(1.0f, (float)glfwGetTime() / 2.0f)), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    mvp = glm::rotate(glm::mat4(1.0f), (float)(cityCoords.latitude * (PI / 180.0) * std::min(1.0f, (float)glfwGetTime() / 2.0f)), glm::vec3(1.0f, 0.0f, 0.0f)) * mvp;
    if ((float)glfwGetTime() / 2.0f >= 1.0f && (float)glfwGetTime() / 2.0f >= 1.0f) mvp = glm::scale(glm::mat4(1.0f), glm::vec3(std::min((float)glfwGetTime() / 2.0f, 3.0f), std::min((float)glfwGetTime() / 2.0f, 3.0f), 1.0f)) * mvp;
    GLuint mvpMatrix = glGetUniformLocation(shaderProgram, "MVP");
    glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, thermalTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "thermalTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, physicalTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "physicalTexture"), 1);
    glBindVertexArray(planetVAO);
    glDrawElements(GL_TRIANGLES, planetIndices.size(), GL_UNSIGNED_INT, 0);
}
