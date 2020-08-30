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

// push glsl state
// multiline comment
// wishlist
// autocomplete contexts (eg for glsl)
// regex filters
// allow processing of rule groups for output - eg. int <identifier> lets us color the identifer separately


// built in variables are gl_*
using namespace Virtuoso;

struct ConsoleApplication : public GLFWApplication
{
    IMGUIQuakeConsole console3;
    IMGUIOstream console2;
    
    IMGUIInputLine cis;

    ConsoleApplication()
    {
        MultiStream ms;
        ms.addStream(console2);
        
        ms << "-- Message from ConsoleApplication() constructor" << std::endl;
        ms << "[warning] - This thing supports color!\n";
        ms << "\033\[1mAND ANSI COLOR CODES TOO" << std::endl;
        
        ms << "REGULAR COLORS"<<std::endl;
        ms << "\u001b[30m A \u001b[31m B \u001b[32m C \u001b[33m D \u001b[0m";
        ms << "\u001b[34m E \u001b[35m F \u001b[36m G \u001b[37m H \u001b[0m\n"<<std::endl;
        
        ms << "BRIGHT COLORS"<<std::endl;
        ms << "\u001b[30;1m A \u001b[31;1m B \u001b[32;1m C \u001b[33;1m D \u001b[0m";
        ms << "\u001b[34;1m E \u001b[35;1m F \u001b[36;1m G \u001b[37;1m H \u001b[0m\n"<<std::endl;
        
        ms << "BACKGROUNDS COLORS"<<std::endl;
        ms << "\u001b[40m A \u001b[41m B \u001b[42m C \u001b[43m D \u001b[0m";
        ms << "\u001b[44m A \u001b[45m B \u001b[46m C \u001b[47m D \u001b[0m\n";
        
        ms << "BACKGROUND + BRIGHT COLOR"<<std::endl;
        ms << "\u001b[40;1m A \u001b[41;1m B \u001b[42;1m C \u001b[43;1m D \u001b[0m";
        ms << "\u001b[44;1m A \u001b[45;1m B \u001b[46;1m C \u001b[47;1m D \u001b[0m";

        ms << TEXT_COLOR_RED << "\nRED TEXT\n";
        ms << TEXT_COLOR_RED_BRIGHT << "BRIGHT RED TEXT " << std::endl;
        
        ImGuiContext* ctx = ImGui::CreateContext();
        ImGui::SetCurrentContext(ctx);
        ImGui_ImplGlfw_Init(window, true, GlfwClientApi_OpenGL);
        
        //console.bindMemberCommand("glslTest", *this, &ConsoleApplication::doGLSLTest);
        
    }
    
    void console2Draw()
    {
        static bool open  = true;
        
        if (!open) return;
            
        console2.renderInWindow(open, "ostream test");
    }
    
    void render()
    {
        if (!fpsCounter.frames)
        {
            console2 << " FPS : " << fpsCounter.fps <<'\n';
        }
        
        static ImguiRenderState imguiRenderer;
        ImGui_ImplGlfw_NewFrame();

        glClearColor(0.0f,0.0f,0.0f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);

        ImGui::NewFrame();

        console2Draw();
        
        bool open2 = true;
        
        if (cis.renderInWindow(open2, "Input Line"))
        {
            console3 << cis.getInput() << std::endl;
        }
        
        bool pople = true;
        console3.render("console 2 implementation", &pople);
        
        ImGui::EndFrame();
        ImGui::Render();
        imguiRenderer.renderGUI(ImGui::GetDrawData());
    }
    
    std::string makeKeywordsRegexStr(const std::string keywords[], std::size_t numKeywords)
    {
        std::stringstream sstr;
        
        sstr << "\\b" << keywords[0] << "\\b";
        for (int i = 1 ; i < numKeywords; i++)
        {
            sstr << '|' << "\\b"<< keywords[i] <<"\\b";
        }
        
        return (sstr.str());
    }
    
    void extraHighlightingRules()
    {
        /*Rule r;
        r.rule = std::regex("<glsl>");
        r.hasColor = true;
        r.color =  ImVec4(1.0,0.0,1.0,1.0);*/
    }
    
    void doGLSLTest()
    {
        const std::string& printSrc = R"STRING(

# this is meant to show off some functionality in the console.
<glsl>

        precision highp float;

        in vec2 coords;
        in vec4 color;
        out vec4 col;

        uniform sampler2D tex;

        /***
            This is a multiline comment
        ***/

        void main(void)
        {
        // this just sets the color!
            col = color * texture(tex, coords).r;
        }

</glsl>

        )STRING";
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


