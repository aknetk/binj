#if INTERFACE
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
};
#endif

#include <Udon/Rendering/OpenGLShader.h>

#include <Udon/Logging/Log.h>

#define CHECK_GL() OpenGLShader::CheckGLError(__LINE__)

PUBLIC OpenGLShader::OpenGLShader(const GLchar** vertexShaderSource, size_t vsSZ, const GLchar** fragmentShaderSource, size_t fsSZ) {
    GLint compiled = GL_FALSE;
    ProgramID = glCreateProgram(); CHECK_GL();

    VertexProgramID = glCreateShader(GL_VERTEX_SHADER); CHECK_GL();
    glShaderSource(VertexProgramID, vsSZ / sizeof(GLchar*), vertexShaderSource, NULL); CHECK_GL();
    glCompileShader(VertexProgramID); CHECK_GL();
    glGetShaderiv(VertexProgramID, GL_COMPILE_STATUS, &compiled); CHECK_GL();
    if (compiled != GL_TRUE) {
        Log::Print(Log::LOG_ERROR, "Unable to compile vertex shader %d!", VertexProgramID);
        CheckShaderError(VertexProgramID);
        CheckGLError(__LINE__);
        return;
    }

    FragmentProgramID = glCreateShader(GL_FRAGMENT_SHADER); CHECK_GL();
    glShaderSource(FragmentProgramID, fsSZ / sizeof(GLchar*), fragmentShaderSource, NULL); CHECK_GL();
    glCompileShader(FragmentProgramID); CHECK_GL();
    glGetShaderiv(FragmentProgramID, GL_COMPILE_STATUS, &compiled); CHECK_GL();
    if (compiled != GL_TRUE) {
        Log::Print(Log::LOG_ERROR, "Unable to compile fragment shader %d!", FragmentProgramID);
        CheckShaderError(FragmentProgramID);
        CheckGLError(__LINE__);
        return;
    }

    glAttachShader(ProgramID, VertexProgramID);                 CHECK_GL();
    glAttachShader(ProgramID, FragmentProgramID);               CHECK_GL();

    glBindAttribLocation(ProgramID, 0, "i_position");           CHECK_GL();
    glBindAttribLocation(ProgramID, 1, "i_uv");                 CHECK_GL();

    glLinkProgram(ProgramID);                                   CHECK_GL();

    GLint isLinked = GL_FALSE;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, (int*)&isLinked); CHECK_GL();
    if (isLinked != GL_TRUE) {
        CheckProgramError(ProgramID);
    }

    LocProjectionMatrix = GetUniformLocation("u_projectionMatrix");
    LocModelViewMatrix = GetUniformLocation("u_modelViewMatrix");

    LocPosition = GetAttribLocation("i_position");
    LocTexCoord = GetAttribLocation("i_uv");
    LocTexture = GetUniformLocation("u_texture");
    LocTextureU = GetUniformLocation("u_textureU");
    LocTextureV = GetUniformLocation("u_textureV");
    LocColor = GetUniformLocation("u_color");

    LocTextureSize = GetUniformLocation("u_textureSize");
    LocDirectionVector = GetUniformLocation("u_directionVector");

    // Log::Print(Log::LOG_IMPORTANT, "SHADER:");
    // Log::Print(Log::LOG_INFO, "u_projectionMatrix: %d", GetUniformLocation("u_projectionMatrix"));
    // Log::Print(Log::LOG_INFO, "u_modelViewMatrix:  %d", GetUniformLocation("u_modelViewMatrix"));
    // Log::Print(Log::LOG_INFO, "u_color:            %d", GetUniformLocation("u_color"));
    // Log::Print(Log::LOG_INFO, "u_texture:          %d", GetUniformLocation("u_texture"));
    // Log::Print(Log::LOG_INFO, "i_position:         %d", GetAttribLocation("i_position"));
    // Log::Print(Log::LOG_INFO, "i_uv:               %d", GetAttribLocation("i_uv"));
    // Log::Print(Log::LOG_INFO, "ProgramID:          %d", ProgramID);
}
PUBLIC OpenGLShader::OpenGLShader(GLuint vertPID, const GLchar** fragmentShaderSource) {
    GLint compiled = GL_FALSE;
    ProgramID = glCreateProgram();

    VertexProgramID = vertPID;

    FragmentProgramID = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FragmentProgramID, sizeof(fragmentShaderSource) / sizeof(GLchar*), fragmentShaderSource, NULL);
    glCompileShader(FragmentProgramID);
    glGetShaderiv(FragmentProgramID, GL_COMPILE_STATUS, &compiled);
    if (compiled != GL_TRUE) {
        Log::Print(Log::LOG_ERROR, "Unable to compile fragment OpenGLShader %d!", FragmentProgramID);
        CheckShaderError(FragmentProgramID);
        CheckGLError(__LINE__);
        return;
    }

    glAttachShader(ProgramID, VertexProgramID);
    glAttachShader(ProgramID, FragmentProgramID);

    glBindAttribLocation(ProgramID, 0, "i_position");
    glBindAttribLocation(ProgramID, 1, "i_uv");
    glBindAttribLocation(ProgramID, 2, "i_normals");
    CheckGLError(__LINE__);

    glLinkProgram(ProgramID);
    CheckGLError(__LINE__);

    GLint isLinked = GL_FALSE;
    glGetProgramiv(ProgramID, GL_LINK_STATUS, (int*)&isLinked);
    if (isLinked != GL_TRUE) {
        CheckProgramError(ProgramID);
    }

    glDetachShader(ProgramID, VertexProgramID); CheckGLError(__LINE__);
    glDetachShader(ProgramID, FragmentProgramID); CheckGLError(__LINE__);
    glDeleteShader(VertexProgramID); CheckGLError(__LINE__);
    glDeleteShader(FragmentProgramID); CheckGLError(__LINE__);

    LocProjectionMatrix = GetUniformLocation("u_projectionMatrix");
    LocModelViewMatrix = GetUniformLocation("u_modelViewMatrix");

    LocPosition = GetAttribLocation("i_position");
    LocTexCoord = GetAttribLocation("i_uv");
    LocTexture = GetUniformLocation("u_texture");
    LocTextureU = GetUniformLocation("u_textureU");
    LocTextureV = GetUniformLocation("u_textureV");
    LocColor = GetUniformLocation("u_color");

    // Log::Print(Log::LOG_INFO, "u_projectionMatrix: %d", GetUniformLocation("u_projectionMatrix"));
    // Log::Print(Log::LOG_INFO, "u_modelViewMatrix:  %d", GetUniformLocation("u_modelViewMatrix"));
    // Log::Print(Log::LOG_INFO, "u_color:            %d", GetUniformLocation("u_color"));
    // Log::Print(Log::LOG_INFO, "u_texture:          %d", GetUniformLocation("u_texture"));
    // Log::Print(Log::LOG_INFO, "i_position:         %d", GetAttribLocation("i_position"));
    // Log::Print(Log::LOG_INFO, "i_uv:               %d", GetAttribLocation("i_uv"));
    // Log::Print(Log::LOG_INFO, "ProgramID:          %d", ProgramID);
}

PUBLIC STATIC bool OpenGLShader::CheckGLError(int line) {
    const char* errstr = NULL;
    GLenum error = glGetError();
    switch (error) {
        case GL_NO_ERROR: errstr = "no error"; break;
        case GL_INVALID_ENUM: errstr = "invalid enumerant"; break;
        case GL_INVALID_VALUE: errstr = "invalid value"; break;
        case GL_INVALID_OPERATION: errstr = "invalid operation"; break;
        case GL_OUT_OF_MEMORY: errstr = "out of memory"; break;
        #ifdef GL_STACK_OVERFLOW
        case GL_STACK_OVERFLOW: errstr = "stack overflow"; break;
        case GL_STACK_UNDERFLOW: errstr = "stack underflow"; break;
        case GL_TABLE_TOO_LARGE: errstr = "table too large"; break;
        #endif
        #ifdef GL_EXT_framebuffer_object
        case GL_INVALID_FRAMEBUFFER_OPERATION_EXT: errstr = "invalid framebuffer operation"; break;
        #endif
        #if GLU_H
        case GLU_INVALID_ENUM: errstr = "invalid enumerant"; break;
        case GLU_INVALID_VALUE: errstr = "invalid value"; break;
        case GLU_OUT_OF_MEMORY: errstr = "out of memory"; break;
        case GLU_INCOMPATIBLE_GL_VERSION: errstr = "incompatible gl version"; break;
        // case GLU_INVALID_OPERATION: errstr = "invalid operation"; break;
        #endif
        default:
            errstr = "idk";
            break;
    }
    if (error != GL_NO_ERROR) {
        Log::Print(Log::LOG_ERROR, "OpenGL error on line %d: %s", line, errstr);
        return true;
    }
    return false;
}
PUBLIC bool OpenGLShader::CheckShaderError(GLuint shader) {
    int infoLogLength = 0;
    int maxLength = infoLogLength;

    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

    char* infoLog = new char[maxLength];
    glGetShaderInfoLog(shader, maxLength, &infoLogLength, infoLog);
    infoLog[strlen(infoLog) - 1] = 0;

    if (infoLogLength > 0)
        Log::Print(Log::LOG_ERROR, "%s", infoLog + 7);

    delete[] infoLog;
    return false;
}
PUBLIC bool OpenGLShader::CheckProgramError(GLuint prog) {
    int infoLogLength = 0;
    int maxLength = infoLogLength;

    glGetProgramiv(prog, GL_INFO_LOG_LENGTH, &maxLength);

    char* infoLog = new char[maxLength];
    glGetProgramInfoLog(prog, maxLength, &infoLogLength, infoLog);
    infoLog[strlen(infoLog) - 1] = 0;

    if (infoLogLength > 0)
        Log::Print(Log::LOG_ERROR, "%s", infoLog + 7);

    delete[] infoLog;
    return false;
}

PUBLIC GLuint OpenGLShader::Use() {
    glUseProgram(ProgramID);
    CheckGLError(__LINE__);

    // glEnableVertexAttribArray(1);

    CheckGLError(__LINE__);
    return ProgramID;
}

PUBLIC GLint  OpenGLShader::GetAttribLocation(const GLchar* identifier) {
    GLint value = glGetAttribLocation(ProgramID, identifier);
    CheckGLError(__LINE__);
    return value;
}
PUBLIC GLint  OpenGLShader::GetUniformLocation(const GLchar* identifier) {
    GLint value = glGetUniformLocation(ProgramID, identifier);
    CheckGLError(__LINE__);
    return value;
}

PUBLIC void OpenGLShader::Dispose() {
    glDeleteProgram(ProgramID);
}

#undef CHECK_GL
