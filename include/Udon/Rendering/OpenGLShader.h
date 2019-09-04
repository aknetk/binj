#ifndef OPENGLSHADER_H
#define OPENGLSHADER_H

#define PUBLIC
#define PRIVATE
#define PROTECTED
#define STATIC
#define VIRTUAL


#include <Udon/Standard.h>

class OpenGLShader {
public:
    GLuint ProgramID;
    GLuint VertexProgramID;
    GLuint FragmentProgramID;
    GLint  LocProjectionMatrix;
    GLint  LocModelViewMatrix;
    GLint  LocPosition;
    GLint  LocTexCoord;
    GLint  LocTexture;
    GLint  LocTextureU;
    GLint  LocTextureV;
    GLint  LocColor;
    GLint  LocTextureSize;
    GLint  LocDirectionVector;

    OpenGLShader(const GLchar** vertexShaderSource, size_t vsSZ, const GLchar** fragmentShaderSource, size_t fsSZ);
    OpenGLShader(GLuint vertPID, const GLchar** fragmentShaderSource);
    static bool CheckGLError(int line);
    bool CheckShaderError(GLuint shader);
    bool CheckProgramError(GLuint prog);
    GLuint Use();
    GLint  GetAttribLocation(const GLchar* identifier);
    GLint  GetUniformLocation(const GLchar* identifier);
    void Dispose();
};

#endif /* OPENGLSHADER_H */
