#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <vector>
#include <omp.h>

#include "Callback.h"
#include "Shader.h"
#include "Scene.h"

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwSwapInterval(1);
    
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetScrollCallback(window, scroll_callback);

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }
    glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);

   
    Shader shader("res/shaders/PBR.shader");
    
    //=====================  test  =========================
    std::string testShaderPath = "res/shaders/whitted.shader";
    std::string lightShaderPath = "res/shaders/basic.shader";

    Shader testShader(testShaderPath);
    Shader lightShader(lightShaderPath);

    //=====================================================
    Scene myScene;

    Sphere textSphere1({ 0.0,0,0 }, 1, { 1.0, 1.0, 1.0 });
    Sphere textSphere2({ 1.0,1.0,0 }, 0.5, { 0.0, 1.0, 0.0 });
    Sphere textSphere3( {-1.0, 1.0, 0}, 0.5, { 0.0, 0.0, 1.0 });
    Sphere lightSphere({ 5, 5, 5 }, 0.1, { 1,1,1 });
    lightSphere.material.emissive = glm::vec3(1.0);

    myScene.push(&textSphere1, testShader);
    myScene.push(&textSphere2, testShader);
    myScene.push(&textSphere3, testShader);
    myScene.push(&lightSphere, lightShader);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
    
    // 线框模型
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 背面剔除
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);

    // render loop
    // -----------


    while (!glfwWindowShouldClose(window))
    {
        float currentFrame = static_cast<float>(glfwGetTime());
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        // input
        // -----
        processInput(window);

        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();
        
        testShader.Bind();
        testShader.SetUniformMat4f("projection", projection);
        testShader.SetUniformMat4f("view", view);
        testShader.SetUniform1ui("frameCount", frameCount++);
        testShader.SetUniform2uiv("screenSize", glm::vec2(SCR_WIDTH, SCR_HEIGHT));

        lightShader.Bind();
        lightShader.SetUniformMat4f("projection", projection);
        lightShader.SetUniformMat4f("view", view);

        myScene.Draw();

        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
  
    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

