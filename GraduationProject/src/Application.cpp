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

#include "imgui/imgui.h"
#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "tests/TestPathTracingPhoto.h"
#include "tests/TestClearColor.h"
#include "tests/TestRealTimePathTracing.h"

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

    /*
    Shader shader("res/shaders/pathTracing.shader");
    shader.Bind();
    shader.SetUniform1i("lastFrame", 2);
    shader.SetUniform1i("width", SCR_WIDTH);
    shader.SetUniform1i("height", SCR_HEIGHT);
    shader.SetUniform1i("spp", 16);
    shader.UnBind();

    Shader fbShader("res/shaders/lastframe.shader");
    fbShader.Bind();

    fbShader.SetUniform1i("texture0", 0);

    fbShader.UnBind();
    
    Scene myScene;

    Material LightMaterial({ 1,1,1 });
    LightMaterial.emissive = glm::vec3(1.0f);

    GREEN.roughness = 0.1f;
    GREEN.metallic = 0.1f;

    WHITE.roughness = 0.5f;
    WHITE.metallic = 0.1f;
    WHITE.specular = 0.8f;

    CYAN.roughness = 0.2f;

    Plane PlaneLeft(Left, { -1,0,0 }, RED);
    Plane PlaneRight(Right, { -1,0,0 }, GREEN);
    Plane PlaneUp(Up, { 0,-1,0 }, WHITE);
    Plane PlaneDown(Down, { 0,-1,0 }, WHITE);
    Plane PlaneBack(Back, { 0,0,-1 }, WHITE_MIRROR);
    Plane PlaneLight(LightUp, { 0,-1,0 }, LightMaterial);

    Cube textCube1({ 0.3, -1.5, -7.0 }, CYAN);
    Cube textCube2({ -1.0, -1.2, -5.1 }, WHITE, 0.5, 0.8, 0.5, PI / 4.0f);


    Material mySphere({ 1.0, 0.6, 0.6 }, 1.0f, 0.0f);

    Sphere textSphere1({ 0,0.5,-5.5 }, 0.5, WHITE_MIRROR);
    Sphere textSphere2({ 1.5, -1,-6 }, 0.3, { 1.0, 1.0, 1.0 });
    Sphere textSphere3( {-1.3, -1, -6.5}, 0.4, { 0.3, 0.5, 1.0 });
    
    myScene.push(&PlaneBack);
    myScene.push(&PlaneDown);
    myScene.push(&PlaneLeft);
    myScene.push(&PlaneRight);
    myScene.push(&PlaneUp);
    myScene.push(&PlaneLight);

    myScene.push(&textCube1);
    myScene.push(&textCube2);

    FrameBuffer myfbo;
    */

    {
        glEnable(GL_DEPTH_TEST);

        ImGui::CreateContext();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui::StyleColorsDark();

        test::Test* currentTest = nullptr;
        test::TestMenu* TestMenu = new test::TestMenu(currentTest);
        currentTest = TestMenu;

        TestMenu->RegisterTest<test::TestClearColor>("Clear Color");
        TestMenu->RegisterTest<test::TestRenderPhoto>("pathTracing");
        TestMenu->RegisterTest<test::TestRenderRealTime>("Real Time");

        const char* glsl_version = "#version 330";
        ImGui_ImplOpenGL3_Init(glsl_version);

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
            // input
            // -----
            processInput(window);

            /*
            myfbo.Bind();

            glm::mat4 projection = glm::perspective(glm::radians(camera.GetFov()), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
            glm::mat4 view = camera.GetViewMatrix();

            shader.Bind();
            shader.SetUniformMat4f("projection", projection);
            shader.SetUniformMat4f("view", view);
            shader.SetUniform1i("frameCount", frameCount++);

            // 渲染数据读入帧缓冲区
            myfbo.BindTexture(2);
            myScene.Render(shader);
            myfbo.DrawBuffer(1);

            // 绘制缓冲区内容
            myfbo.UnBind();
            myfbo.Draw(fbShader);
            */


            
            ImGui_ImplOpenGL3_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();

            if (currentTest)
            {
                currentTest->OnUpdate(0.0f);
                currentTest->OnRender();
                ImGui::Begin("Test");

                if (currentTest != TestMenu && ImGui::Button("<-"))
                {
                    delete currentTest;
                    currentTest = TestMenu;
                }

                currentTest->OnImGuiRender();
                ImGui::End();
            }

            ImGui::Render();
            ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

            // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
            // -------------------------------------------------------------------------------
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        delete currentTest;
        if (currentTest != TestMenu)
            delete TestMenu;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}