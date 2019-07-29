void ck() {
    GLenum err = glGetError();
    if (err == GL_NO_ERROR) {
        return;
    }
    char* msg;
    switch (err) {
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

GLuint createAndBindEmptyTexture(const GLuint texUnit, const int nx, const int ny) {
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx, ny, 0, GL_RGBA, GL_FLOAT, NULL);
    glBindImageTexture(texUnit, texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    return texture;
}

GLuint createAndBindTextureFromImage(const GLuint texUnit, const int nx, const int ny, const u8 *data) {
    GLuint texture;
    glGenTextures(1, &texture);
    glActiveTexture(GL_TEXTURE0 + texUnit);
    glBindTexture(GL_TEXTURE_2D, texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA32F, nx, ny, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glBindImageTexture(texUnit, texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    return texture;
}

GLuint createAndBindSSBO(GLuint ssboLocation, size_t bufferSize, void *buffer) {
    GLuint ssbo;
    glGenBuffers(1, &ssbo);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    glBufferData(GL_SHADER_STORAGE_BUFFER, bufferSize, buffer, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, 0);
    glBindBufferBase(GL_SHADER_STORAGE_BUFFER, ssboLocation, ssbo);
    return ssbo;
}

void updateSSBO(GLuint ssbo, size_t bufferSize, void *buffer) {
    glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo);
    GLvoid *p = glMapBuffer(GL_SHADER_STORAGE_BUFFER, GL_WRITE_ONLY);
    memcpy(p, buffer, bufferSize);
    glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

GLuint shaderFromSource(char* name, char* path) {
    GLuint shaderId = glCreateShader(GL_COMPUTE_SHADER);
    char source[10240];
    int len = 10240;
    getFileContents(path, source, &len);
    const GLchar* sourceAddr = &source[0];
    glShaderSource(shaderId, 1, &sourceAddr, &len);
    glCompileShader(shaderId);

    GLint compileStatus;
    glGetShaderiv(shaderId, GL_COMPILE_STATUS, &compileStatus);
    if (compileStatus != GL_TRUE) {
        char infoLog[512];
        glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
        printf("Shader %s compilation failed: %s", name, infoLog);
        exit(-1);
    }

    return shaderId;
}

GLuint shaderProgramFromShader(GLuint shaderId) {
    GLuint programId = glCreateProgram();
    glAttachShader(programId, shaderId);
    glLinkProgram(programId);
    GLint programStatus;
    glGetProgramiv(programId, GL_LINK_STATUS, &programStatus);
    if (programStatus != 1) {
        char infoLog[512];
        glGetProgramInfoLog(programId, 512, NULL, infoLog);
        printf("Shader program link failed: %s\n", infoLog);
        exit(-1);
    }
    glDetachShader(programId, shaderId);
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
