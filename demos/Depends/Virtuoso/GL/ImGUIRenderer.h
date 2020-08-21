//
//  ImguiRender.h
//  IMGUITEST
//
//  Created by Chris Pugh on 3/31/16.
//  Copyright © 2016 VirtuosoEngine. All rights reserved.
//

#ifndef ImguiRender_h
#define ImguiRender_h

#include <imgui.h>
#include <Virtuoso/GL/ShaderProgramLib.h>
#include <Virtuoso/GL/TextureLoader.h>
#include <array>

struct ImguiRenderState
{
    gl::Program imguiProg;
    gl::Buffer vertexBuffer;
    gl::Buffer indexBuffer;
    gl::Texture fontTex;
    gl::VertexArray fontVAO;
    
    const static std::string DEFAULT_API_VERSION;
    
    ImguiRenderState(const std::string& apiVersion = DEFAULT_API_VERSION);
    
    void renderGUI(ImDrawData* data);
};

const std::string imguiVert =

R"STRING(

precision highp float;

layout (location=0) in vec2 pos;
layout (location=1) in vec2 tcs;
layout (location=2) in vec4 col;

out vec2 coords;
out vec4 color;

uniform vec2 scale;
uniform mat4 mvp;

void main()
{
    coords = tcs;
    color = col;
    
    vec2 pos2 = fma(pos, scale, vec2(-1.0, 1.0));
    
    gl_Position = mvp * vec4(pos2, 0.0, 1.0);
}

)STRING";


const std::string imguiFrag =
R"STRING(

precision highp float;

in vec2 coords;
in vec4 color;
out vec4 col;

uniform sampler2D tex;

void main(void)
{
    col = color * texture(tex, coords).r;
}

)STRING";

#endif /* ImguiRender_h */


#ifdef ImguiRender_h_IMPLEMENTATION

const std::string ImguiRenderState::DEFAULT_API_VERSION = "#version 410 core\n";

void ImguiRenderState::renderGUI(ImDrawData* data)
{
    ImVec2 clip_off = data->DisplayPos;         // (0,0) unless using multi-viewports
    ImVec2 clip_scale = data->FramebufferScale;
    int fb_height = (int)(data->DisplaySize.y * data->FramebufferScale.y);

    ImGuiIO& io = ImGui::GetIO();

    imguiProg.Use();

    imguiProg.Uniform("scale", 2.0f / io.DisplaySize.x, -2.0f / io.DisplaySize.y);

    fontVAO.Bind();

    glActiveTexture(GL_TEXTURE0);
    fontTex.Bind(GL_TEXTURE_2D);

    bool scissorEnabled = glIsEnabled(GL_SCISSOR_TEST);

    std::array<GLint, 4> scissorBoxOld;
    glGetIntegerv(GL_SCISSOR_BOX, &scissorBoxOld[0]);

    if (!scissorEnabled)
    {
        glEnable(GL_SCISSOR_TEST);
    }

    for (int i = 0; i < data->CmdListsCount; i++)
    {
        ImDrawList& drawList = *data->CmdLists[i];

        indexBuffer.Data (sizeof(ImDrawIdx)  * drawList.IdxBuffer.size(),  drawList.IdxBuffer.Data, GL_STATIC_DRAW);

        vertexBuffer.Data(sizeof(ImDrawVert) * drawList.VtxBuffer.size() , drawList.VtxBuffer.Data, GL_STATIC_DRAW);

        GLushort* offset = 0;

        for (int cmds =0; cmds< drawList.CmdBuffer.size(); cmds++)
        {
          ImDrawCmd& cmd = drawList.CmdBuffer[cmds];

          ImDrawCmd* pcmd = &cmd;
          
          ImVec4 clip_rect;
          clip_rect.x = (pcmd->ClipRect.x - clip_off.x) * clip_scale.x;
          clip_rect.y = (pcmd->ClipRect.y - clip_off.y) * clip_scale.y;
          clip_rect.z = (pcmd->ClipRect.z - clip_off.x) * clip_scale.x;
          clip_rect.w = (pcmd->ClipRect.w - clip_off.y) * clip_scale.y;
          
          // Apply scissor/clipping rectangle
          glScissor((int)clip_rect.x, (int)(fb_height - clip_rect.w), (int)(clip_rect.z - clip_rect.x), (int)(clip_rect.w - clip_rect.y));
          
          glDrawElements(GL_TRIANGLES, cmd.ElemCount, GL_UNSIGNED_SHORT, (void*)offset);

          offset += cmd.ElemCount;
        }
    }

    // reset state

    if (!scissorEnabled)
    {
        glDisable(GL_SCISSOR_TEST);
    }

    glScissor(scissorBoxOld[0], scissorBoxOld[1], scissorBoxOld[2], scissorBoxOld[3]);
}

ImguiRenderState::ImguiRenderState(const std::string& apiVersion)
: imguiProg(
        Virtuoso::GL::Program(
                {Virtuoso::GL::Shader(GL_VERTEX_SHADER, apiVersion + imguiVert),
                Virtuoso::GL::Shader(GL_FRAGMENT_SHADER, apiVersion + imguiFrag)}
        ))
{
    ImGuiIO& io = ImGui::GetIO();
    
    /*** render font atlas ***/
    unsigned char* pixels;
    int width, height;
    
    io.Fonts->GetTexDataAsAlpha8(&pixels, &width, &height);
    
    fontTex = Virtuoso::GL::allocateTexture(width, height, GL_R8);
    fontTex.SubImage2D(0, 0, 0, width, height, GL_RED, GL_UNSIGNED_BYTE, pixels);

    fontTex.Bind(GL_TEXTURE_2D);
    fontTex.GenerateMipmap(GL_TEXTURE_2D);
    
    Virtuoso::GL::setFilterTrilinear(fontTex);
    
    fontVAO.Attrib(0, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), vertexBuffer, 0);
    fontVAO.Attrib(1, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert), vertexBuffer, 2 * sizeof(GL_FLOAT));
    fontVAO.Attrib(2, 4, GL_UNSIGNED_BYTE, GL_TRUE, sizeof(ImDrawVert),vertexBuffer, 4 * sizeof(GL_FLOAT));
    
    fontVAO.Elements(indexBuffer);

    std::array < float, 16 > id = {
            1.0f, 0.0f, 0.0f, 0.0f,
            0.0f, 1.0f, 0.0f, 0.0f,
            0.0f, 0.0f, 1.0f, 0.0f,
            0.0f, 0.0f, 0.0f, 1.0f
    };

    imguiProg.Uniform("tex", 0);
    
    imguiProg.UniformMatrix<float, 4, 4>("mvp", id.data());
}


#endif
