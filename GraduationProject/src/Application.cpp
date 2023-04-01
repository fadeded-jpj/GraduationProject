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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Path Tracing", NULL, NULL);
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
    shader.Bind();
    shader.SetUniform1i("lastFrame", 2);
    shader.UnBind();

    Shader fbShader("res/shaders/lastframe.shader");
    fbShader.Bind();
    for (int i = 0; i < 6; i++) {
        fbShader.SetUniform1i("texture" + std::to_string(i), i);
    }
    fbShader.UnBind();
    
    //=====================FrameBuffer  test  =========================
    float vertices[] = {
        -1.0f,  1.0f, 
        -1.0f, -1.0f, 
         1.0f, -1.0f, 
         1.0f,  1.0f 
    };

    std::vector<GLuint> indices = {
        0, 1, 3,   // first triangle
        1, 2, 3    // second triangle
    };
    unsigned int quadVAO, quadVBO, quadEBO;
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);
    
    glBindVertexArray(quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), &vertices, GL_STATIC_DRAW);
    
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);


    unsigned int framebuffer;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    
    // create a color attachment texture
    std::vector<GLuint> textureColorbuffer(6);
    std::vector<GLuint> attachment;
    for (int i = 0; i < 6; i++) {
        glGenTextures(1, &textureColorbuffer[i]);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, SCR_WIDTH, SCR_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + i, GL_TEXTURE_2D, textureColorbuffer[i], 0);
        attachment.push_back(GL_COLOR_ATTACHMENT0 + i);
    }
    glDrawBuffers(attachment.size(), &attachment[0]);
    // create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
    //unsigned int rbo;
    //glGenRenderbuffers(1, &rbo);
    //glBindRenderbuffer(GL_RENDERBUFFER, rbo);
    //glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, SCR_WIDTH, SCR_HEIGHT); // use a single renderbuffer object for both a depth AND stencil buffer.
    //glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo); // now actually attach it
    
    // now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now    
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //=====================================================
    Scene myScene;

    Material LightMaterial({ 1,1,1 });
    LightMaterial.emissive = glm::vec3(1.0f);

    Plane PlaneLeft(Left, { -1,0,0 }, RED);
    Plane PlaneRight(Right, { -1,0,0 }, GREEN);
    Plane PlaneUp(Up, { 0,-1,0 }, WHITE);
    Plane PlaneDown(Down, { 0,-1,0 }, WHITE);
    Plane PlaneBack(Back, { 0,0,-1 }, WHITE);
    Plane PlaneLight(LightUp, { 0,-1,0 }, LightMaterial);

    Sphere textSphere1({ 0,0,-6 }, 0.8, { 1.0, 0.5, 0.2 });
    Sphere textSphere2({ 1.3, 1,-3 }, 0.3, { 0.3, 1.0, 0.1 });
    Sphere textSphere3( {-1.3, 1, -4}, 0.3, { 0.3, 0.1, 1.0 });
    //Sphere lightSphere({ 0, 2, -4 }, 1, { 1,1,1 });

    //textSphere1.setEmissive(glm::vec3(1));
    //lightSphere.setEmissive(glm::vec3(1.0));

    myScene.push(&textSphere1);
    myScene.push(&textSphere2);
    myScene.push(&textSphere3);
    //myScene.push(&lightSphere);
    
    myScene.push(&PlaneBack);
    myScene.push(&PlaneDown);
    myScene.push(&PlaneLeft);
    myScene.push(&PlaneRight);
    myScene.push(&PlaneUp);
    myScene.push(&PlaneLight);

    // You can unbind the VAO afterwards so other VAO calls won't accidentally modify this VAO, but this rarely happens. Modifying other
    // VAOs requires a call to glBindVertexArray anyways so we generally don't unbind VAOs (nor VBOs) when it's not directly necessary.
    glBindVertexArray(0);
    
    glEnable(GL_DEPTH_TEST);
    
    // 线框模型
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

    // 背面剔除
    //glEnable(GL_CULL_FACE);
    //glCullFace(GL_FRONT);
    //glCullFace(GL_BACK);

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

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        
        glEnable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_DEPTH_BUFFER_BIT);

        glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
        glm::mat4 view = camera.GetViewMatrix();

        shader.Bind();
        shader.SetUniformMat4f("projection", projection);
        shader.SetUniformMat4f("view", view);
        shader.SetUniform1i("frameCount", frameCount++);

        shader.SetUniform1i("width", SCR_WIDTH);
        shader.SetUniform1i("height", SCR_HEIGHT);
        shader.SetUniform1i("lastFrame", 2);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, textureColorbuffer[0]);
        myScene.Render(shader);
        glDrawBuffers(attachment.size(), &attachment[0]);
        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
        
        glDisable(GL_DEPTH_TEST);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        
        
        fbShader.Bind();
        glBindVertexArray(quadVAO);
        for (int i = 0; i < textureColorbuffer.size(); i++) {
            glActiveTexture(GL_TEXTURE0 + i);
            glBindTexture(GL_TEXTURE_2D, textureColorbuffer[i]);
        }
        glDrawElements(GL_TRIANGLE_STRIP, indices.size(), GL_UNSIGNED_INT, 0);
        fbShader.UnBind();
        

        /*
        basic.Bind();
        basic.SetUniformMat4f("projection", projection);
        basic.SetUniformMat4f("view", view);
        textSphere1.Draw(basic);
        */
        
        // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
        // -------------------------------------------------------------------------------
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glDeleteVertexArrays(1, &quadVAO);
    glDeleteBuffers(1, &quadVBO);
    //glDeleteRenderbuffers(1, &rbo);
    glDeleteFramebuffers(1, &framebuffer);

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

