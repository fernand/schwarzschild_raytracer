void ck() {
    GLenum err = glGetError();
    if(err == GL_NO_ERROR) {
        return;
    }
    char* msg;
    switch(err) {
        case GL_INVALID_ENUM: msg = "INVALID_ENUM"; break;
        case GL_INVALID_VALUE: msg = "INVALID_VALUE"; break;
        case GL_INVALID_OPERATION: msg = "INVALID_OPERATION"; break;
        case GL_INVALID_FRAMEBUFFER_OPERATION: msg = "INVALID_FRAME_BUFFER_OPERATION"; break;
        case GL_OUT_OF_MEMORY: msg = "OUT_OF_MEMORY"; break;
        case GL_STACK_UNDERFLOW: msg = "STACK_UNDERFLOW"; break;
        case GL_STACK_OVERFLOW: msg = "STACK_OVERFLOW";
        default: msg = "Unknown";
    }
    printf("GL error: %s\n", msg);
    exit(-1);
}

GLuint shaderFromSource(char* name, char* path) {
    GLuint shaderId = glCreateShader(GL_COMPUTE_SHADER); ck();
    char source[10240];
    int len = 10240;
    getFileContents(path, source, &len);
    const GLchar* sourceAddr = &source[0];
    glShaderSource(shaderId, 1, &sourceAddr, &len); ck();
    glCompileShader(shaderId); ck();

    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus); ck();
    if(compileStatus != GL_TRUE) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog); ck();
        printf("Shader %s compilation failed: %s", name, infoLog);
        exit(-1);
    }

    return shaderId;
}

GLuint shaderProgramFromShader(GLuint shaderId) {
    GLuint programId = glCreateProgram(); ck();
    glAttachShader(programId, shaderId); ck();
    glLinkProgram(programId); ck();
    GLint programStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &programStatus); ck();
    if(programStatus != 1) {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, NULL, infoLog); ck();
        printf("Shader program link failed: %s\n", infoLog); ck();
        exit(-1);
    }
    glDetachShader(programId, shaderId); ck();
    return programId;
}

void printWorkgroupInfo() {
    GLint xCnt, yCnt, zCnt;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 0, &xCnt);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 1, &yCnt);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_COUNT, 2, &zCnt);
    printf("max work group sizes %d,%d,%d\n", xCnt, yCnt, zCnt);

    GLint xSize, ySize, zSize;
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 0, &xSize);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 1, &ySize);
    glGetIntegeri_v(GL_MAX_COMPUTE_WORK_GROUP_SIZE, 2, &zSize);
    printf("max local work group sizes %d,%d,%d\n", xSize, ySize, zSize);
    
    GLint workInvs;
    glGetIntegerv(GL_MAX_COMPUTE_WORK_GROUP_INVOCATIONS, &workInvs);
    printf("max local invocations %d", workInvs);
}
