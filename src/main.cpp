#include <iostream>
#include <fstream>

#include <core/fileReader.h>
#include <core/dataScanner.h>
#include <renderLogic/render.h>

// Globals
const std::string filePath = __FILE__;
bool thermalView = true;

// Keyboard input
void processInput(GLFWwindow *window) {
    if(glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
    if(glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS) {
        thermalView = false;
    } else {
        thermalView = true;
    }
}

// Framebuffer resizing
void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(int argc, char **argv) {
    // Initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    // Window
    GLFWwindow* window = glfwCreateWindow(1200, 900, "Climate Simulation", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);

    // GLAD
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }    

    // Register framebuffer resizing callback & set framebuffer size
    glViewport(0, 0, 1200, 900);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // Vertex Shader
    const std::string vsFile = filePath + "\\..\\renderLogic\\vertexShader.vert";
    const std::string vertexShaderSource = extractFileContents(vsFile);
    const char *vsShaderSource = vertexShaderSource.c_str();
    unsigned int vertexShader;
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vsShaderSource, NULL);
    glCompileShader(vertexShader);
    
    int  success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else std::cout << "Compiled vertex Shader." << std::endl;

    // Fragment Shader
    const std::string fsFile = filePath + "\\..\\renderLogic\\fragmentShader.frag";
    const std::string fragmentShaderSource = extractFileContents(fsFile);
    const char *fsShaderSource = fragmentShaderSource.c_str();
    unsigned int fragmentShader;
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fsShaderSource, NULL);
    glCompileShader(fragmentShader);

    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
    } else std::cout << "Compiled fragment Shader." << std::endl;

    // Shader program
    unsigned int shaderProgram;
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if(!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKAGE_FAILED\n" << infoLog << std::endl;
    } else std::cout << "Linked shader program." << std::endl;

    glUseProgram(shaderProgram);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    Coords cityCoords;
    // Data collection
    if (argc > 1) {
        std::string location = "";
        for (int i = 1; i < argc; ++i) {
            location += std::string(argv[i]);
            if (i != argc - 1) location += " ";
        }
        cityCoords = initializeData(location);
    } else {
        // Default location
        cityCoords = initializeData("Oakville Canada");
    }

    // Initialize objects
    initializeObjects(cityCoords);

    localeWeatherData(cityCoords.latitude, cityCoords.longitude);

    // Event loop
    while(!glfwWindowShouldClose(window))
    {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f );
        glClear(GL_COLOR_BUFFER_BIT);
        processInput(window);
        glfwPollEvents();
        renderSimulation(shaderProgram, cityCoords, thermalView);
        glfwSwapBuffers(window);
    }
  
    glfwTerminate();
    return 0;
}