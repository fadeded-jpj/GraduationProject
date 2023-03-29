#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <iostream>
#include <vector>
#include <omp.h>
#include <algorithm>

#include "Callback.h"
#include "Shader.h"
#include "Scene.h"

int main()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

   
    Shader basic("res/shaders/basic.shader");
    Shader shader("res/shaders/pathTracing.shader");
    
    //=====================  test  =========================

    //=====================================================
    Scene myScene;

    Sphere textSphere1({ 0,0,-3 }, 1, { 1.0, 1.0, 1.0 });
    Sphere textSphere2({ 1.2, 1,-3 }, 0.5, { 0.1, 0.7, 0.1 });
    Sphere textSphere3( {-1.2, 1, -3}, 0.5, { 0.1, 0.1, 0.7 });
    Sphere lightSphere({ 0, 0, 3 }, 1, { 1,1,1 });
    textSphere1.setEmissive(glm::vec3(1));
    lightSphere.setEmissive(glm::vec3(1.0));

    myScene.push(&textSphere1);
    //myScene.push(&textSphere2);
    //myScene.push(&textSphere3);
    //myScene.push(&lightSphere);


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

        shader.Bind();
        shader.SetUniformMat4f("projection", projection);
        shader.SetUniformMat4f("view", view);
        shader.SetUniform1ui("frameCount", frameCount++);
        
        shader.SetUniform1i("width", SCR_WIDTH);
        shader.SetUniform1i("height", SCR_HEIGHT);

        myScene.Render(shader);
        /*basic.Bind();
        basic.SetUniformMat4f("projection", projection);
        basic.SetUniformMat4f("view", view);
        textSphere1.Draw(basic);*/

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

