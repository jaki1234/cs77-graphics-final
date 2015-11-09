#ifndef _GL_H_
#define _GL_H_

#ifdef __APPLE__
#   include <glew.h>
#else
#   ifdef _WIN32
#       pragma warning (disable: 4224)
#       include <windows.h>
#       include "ext/glew/glew.h"
#       include "ext/glew/wglew.h"
#       include "GL/gl.h"
#   else
#       include "ext/glew/glew.h"
#   endif
#endif

#define GLFW_INCLUDE_GLU
#include "GLFW/glfw3.h"


#define GLS_CHECK_ERROR 1

#if GLS_CHECK_ERROR
inline void _checkGlError() {
    auto errcode = glGetError();
    if(not (errcode == GL_NO_ERROR)) {
        //warning_va("gl error: %s", gluErrorString(errcode));
    } 
}
#else
inline void _checkGlError() { }
#endif


// check if an OpenGL error
void error_if_glerror() {
    auto error = glGetError();
    error_if_not(error == GL_NO_ERROR, "gl error");
    
}

// check if an OpenGL shader is compiled
void error_if_shader_not_valid(int shader_id) {
    int iscompiled;
    glGetShaderiv(shader_id, GL_COMPILE_STATUS, &iscompiled);
    if(not iscompiled) {
        char buf[10000];
        glGetShaderInfoLog(shader_id, 10000, 0, buf);
        error("shader not compiled\n\n%s\n\n",buf);
    }
}

// check if an OpenGL program is valid
void error_if_program_not_valid(int prog_id) {
    int islinked;
    glGetProgramiv(prog_id, GL_LINK_STATUS, &islinked);
    if(not islinked) {
        char buf[10000];
        glGetProgramInfoLog(prog_id, 10000, 0, buf);
        error("program not linked\n\n%s\n\n",buf);        
    }
    
    int isvalid;
    glValidateProgram(prog_id);
    glGetProgramiv(prog_id, GL_VALIDATE_STATUS, &isvalid);
    if(not isvalid) {
        char buf[10000];
        glGetProgramInfoLog(prog_id, 10000, 0, buf);
        error("program not validated\n\n%s\n\n",buf);
    }
}



#endif
