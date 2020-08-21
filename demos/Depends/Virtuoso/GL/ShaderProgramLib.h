//  Copyright (c) 2013 Virtuoso Engine LLC. All rights reserved.
//
// defines:
// VIRTUOSO_SUPPRESS_OUTPUT to hide compilation logs, etc
// VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION for implementation
///#define VIRTUOSO_LOG_SHADERS

#ifndef _GL_SHADER_H_INCLUDED
#define _GL_SHADER_H_INCLUDED

#if !defined(GL_HPP)
#error "Missing dependency : include opengl.hpp before including ShaderProgramLib.h"
#endif

#include <iostream>

namespace Virtuoso
{
    namespace GL
    {
        gl::Shader Shader(GLenum shaderType, const std::string& src);
        gl::Program Program(std::initializer_list<gl::Shader> shaders);
    }
}
#endif

#ifdef VIRTUOSO_SHADERPROGRAMLIB_IMPLEMENTATION

#include <fstream>
#include <stdexcept>

namespace Virtuoso
{
    namespace GL
    {
        gl::Shader Shader(GLenum shaderType, const std::string& src)
        {
            static std::hash<std::string> hash_fn;

#ifdef VIRTUOSO_LOG_SHADERS
            std::clog<<"Shader SRC : "<< src << std::endl;

#else
            std::clog<< "\n\nShader with hash : " << hash_fn(src) << std::endl;
#endif
            gl::Shader rval(shaderType);

            try{
            rval.Source(src);
            std::string compileLog = rval.Compile();

            if (compileLog.length())
            {
                std::clog<<"\nShader Compile Log : \nStage : "<< (int)shaderType<<"\n" << compileLog << std::endl;
            }
            }
            catch (std::runtime_error& ex)
            {
                std::clog << ex.what() << std::endl;
                ///throw ex;
            }

            return rval;
        }

        gl::Program Program(std::initializer_list<gl::Shader> shaders)
        {
            gl::Program rval;

            for (const gl::Shader& sh: shaders)
            {
                rval.Attach(sh);
            }

            ///try
            {
            std::string linkLog = rval.Link();


            if (linkLog.length())
            {
               std::clog << " Program Link Log: \n" << linkLog << "\n\n";
            }
            }
            ///catch (...){}

            return rval;
        }
    }
}

#endif
