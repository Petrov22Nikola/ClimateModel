#define STB_IMAGE_IMPLEMENTATION

#include <vector>
#include <iostream>

#include <renderLogic/render.h>
#include <renderLogic/stb_image.h>
#include <core/coordHandler.h>
#include <core/dataScanner.h>
#include <glm/glm.hpp>
#include <glm/ext.hpp>

// Constants
constexpr int longitudinalDivisor = 18;
constexpr int latitudinalDivisor = 18;
constexpr int dimensionality = 3;
constexpr float PI = 3.14f;
constexpr float longitudeCorrection = -89.75f;
constexpr float loadTime = 24.0f;
std::vector<Coords> apiCoords;

// Planet
std::vector<float> planetVertices;
std::vector<unsigned int> planetIndices;
glm::vec4 planetNormal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f);

unsigned int planetVBO;
unsigned int planetVAO;
unsigned int planetEBO;
unsigned int thermalTexture;
unsigned int physicalTexture;
unsigned int pointVAO, pointVBO;

void initializeObjects(Coords cityCoords) {
    // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    glEnable(GL_CULL_FACE);
    glEnable(GL_PROGRAM_POINT_SIZE);
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
    const int imgHeight = 512, imgWidth = 1024;
    unsigned char *thermalData = stbi_load("thermalImage.png", &width, &height, &nrChannels, 0);
    int targetRow = -cityCoords.latitude / 90.0 * imgHeight + imgHeight, targetCol = cityCoords.longitude / 180.0 * imgWidth + imgWidth, targetPixelRadius = 5;
    for (int row = 0; row < 1024; ++row) {
        for (int col = 0; col < 2048; ++col) {
            unsigned char* pixOffset = thermalData + (row * 2048 + col) * nrChannels;
            if (pow(abs(targetRow - row), 2) + pow(abs(targetCol - col), 2) < pow(targetPixelRadius, 2)) {
                Coords coords{(double)(-row + imgHeight) / imgHeight * 90.0, (double)(col - imgWidth) / imgWidth * 180};
                apiCoords.push_back(coords);
                pixOffset[0] = 255;
                pixOffset[1] = 255;
                pixOffset[2] = 255;
            }
        }    
    }
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
    unsigned char *physData = stbi_load("physicalMap.jpg", &width, &height, &nrChannels, 0);
    if (physData) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, physData);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture" << std::endl;
    }
    stbi_image_free(physData);

    // Point
    // float pointPos[] = { 0.0f, 0.0f, 0.0f};
    // glGenVertexArrays(1, &pointVAO);
    // glGenBuffers(1, &pointVBO);
    // glBindVertexArray(pointVAO);
    // glBindBuffer(GL_ARRAY_BUFFER, pointVBO);
    // glBufferData(GL_ARRAY_BUFFER, sizeof(pointPos), pointPos, GL_STATIC_DRAW);
    // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    // glEnableVertexAttribArray(0);
}

void renderSimulation(unsigned int shaderProgram, Coords cityCoords, bool thermalView) {
    // Render planet
    glUseProgram(shaderProgram);
    glm::mat4 mvp(1.0f);
    // mvp = glm::rotate(glm::mat4(1.0f), (float)glfwGetTime(), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    mvp = glm::rotate(glm::mat4(1.0f), (float)((-cityCoords.longitude + longitudeCorrection) * (PI / 180.0) * std::min(1.0f, (float)glfwGetTime() / 10.0f)), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    mvp = glm::rotate(glm::mat4(1.0f), (float)(cityCoords.latitude * (PI / 180.0) * std::min(1.0f, (float)glfwGetTime() / 10.0f)), glm::vec3(1.0f, 0.0f, 0.0f)) * mvp;
    if ((float)glfwGetTime() / 10.0f >= 1.0f && (float)glfwGetTime() / 10.0f >= 1.0f) mvp = glm::scale(glm::mat4(1.0f), glm::vec3(std::min((float)glfwGetTime() - 9.0f, 15.0f), std::min((float)glfwGetTime() - 9.0f, 15.0f), 1.0f)) * mvp;
    // mvp = glm::rotate(glm::mat4(1.0f), (float)((-cityCoords.longitude + longitudeCorrection) * (PI / 180.0)), glm::vec3(0.0f, 1.0f, 0.0f)) * mvp;
    // mvp = glm::rotate(glm::mat4(1.0f), (float)(cityCoords.latitude * (PI / 180.0)), glm::vec3(1.0f, 0.0f, 0.0f)) * mvp;
    // mvp = glm::scale(glm::mat4(1.0f), glm::vec3(15.0f, 15.0f, 1.0f)) * mvp;
    GLuint mvpMatrix = glGetUniformLocation(shaderProgram, "MVP");
    glUniform1i(glGetUniformLocation(shaderProgram, "isPoint"), false);
    glUniformMatrix4fv(mvpMatrix, 1, GL_FALSE, &mvp[0][0]);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, thermalTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "thermalTexture"), 0);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, physicalTexture);
    glUniform1i(glGetUniformLocation(shaderProgram, "physicalTexture"), 1);
    glUniform1i(glGetUniformLocation(shaderProgram, "thermalView"), thermalView);
    glBindVertexArray(planetVAO);
    glDrawElements(GL_TRIANGLES, planetIndices.size(), GL_UNSIGNED_INT, 0);

    // if ((float)glfwGetTime() > loadTime) {
    //     glUniform1i(glGetUniformLocation(shaderProgram, "isPoint"), true);
    //     glBindVertexArray(pointVAO);
    //     glDrawArrays(GL_POINTS, 0, 2);
    // }
}
