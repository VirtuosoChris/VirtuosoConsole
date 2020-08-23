#include <iostream>
#include <cmath>

#define GLFW_INCLUDE_NONE // < so we can use the default glalt path here...

#include <gl_hpp/glalt/gl4.1.h>
#include <gl_hpp/glalt/glext.h>
#include <gl_hpp/opengl.hpp>
#include <GLFW/glfw3.h>

#include <Virtuoso/GL/GLFWApplication.h>

#define ImguiRender_h_IMPLEMENTATION
#define VIRTUOSO_TEXTURELOADER_IMPLEMENTATION
#define VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION
#include <Virtuoso/GL/ImGUIRenderer.h>
#undef ImguiRender_h_IMPLEMENTATION
#undef VIRTUOSO_TEXTURELOADER_IMPLEMENTATION
#undef VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION

#include <imgui/examples/imgui_impl_glfw.h>
#include <imgui/examples/imgui_impl_glfw.cpp>

#include "../IMGUIQuakeConsole.h"

struct ConsoleApplication : public GLFWApplication
{
    IMGUIQuakeConsole console;

    ConsoleApplication()
    {
        console << "-- Message from ConsoleApplication() constructor" << std::endl;
        console << "[warning] - This thing supports color!\n";
        
        ImGuiContext* ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplGlfw_Init(window, true, GlfwClientApi_OpenGL);
    }
    
    void render()
    {
        static ImguiRenderState imguiRenderer;
        ImGui_ImplGlfw_NewFrame();

        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        ImGui::NewFrame();

        bool open;
        console.Draw("Quake Style Console Demo", &open);
        
        ImGui::EndFrame();
        ImGui::Render();
        imguiRenderer.renderGUI(ImGui::GetDrawData());
    }
};


int main(void)
{
    if (!glfwInit())
        return -1;
    
    glfwSetErrorCallback(GLFWApplication::error_callback);

    ConsoleApplication app;
    
    if (!app)
    {
        glfwTerminate();
        return -1;
    }
    
    app.mainLoop();
   
    glfwTerminate();
    
    return 0;
}
