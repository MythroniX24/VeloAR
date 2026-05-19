// renderer/shader_manager.cc — GLSL shader compilation
#include "shader_manager.h"
#include <vector>

namespace arracing {

GLuint ShaderManager::CompileShader(GLenum type, const std::string& src,
                                    const std::string& label) {
    GLuint shader = glCreateShader(type);
    if (!shader) {
        LOGE("ShaderManager: glCreateShader failed for '%s'", label.c_str());
        return 0;
    }

    const char* src_ptr = src.c_str();
    glShaderSource(shader, 1, &src_ptr, nullptr);
    glCompileShader(shader);

    // Check compile status
    GLint compiled = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
    if (!compiled) {
        GLint info_len = 0;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &info_len);
        std::vector<char> info_log(info_len + 1);
        glGetShaderInfoLog(shader, info_len, nullptr, info_log.data());
        LOGE("ShaderManager: Compile error in '%s':\n%s", label.c_str(), info_log.data());
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

GLuint ShaderManager::CreateProgram(const std::string& vert_src,
                                    const std::string& frag_src,
                                    const std::string& name) {
    GLuint vert = CompileShader(GL_VERTEX_SHADER,   vert_src, name + ".vert");
    GLuint frag = CompileShader(GL_FRAGMENT_SHADER, frag_src, name + ".frag");

    if (!vert || !frag) {
        if (vert) glDeleteShader(vert);
        if (frag) glDeleteShader(frag);
        return 0;
    }

    GLuint program = glCreateProgram();
    glAttachShader(program, vert);
    glAttachShader(program, frag);
    glLinkProgram(program);

    // Shaders can be detached after linking
    glDetachShader(program, vert);
    glDetachShader(program, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);

    // Check link status
    GLint linked = 0;
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (!linked) {
        GLint info_len = 0;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &info_len);
        std::vector<char> info_log(info_len + 1);
        glGetProgramInfoLog(program, info_len, nullptr, info_log.data());
        LOGE("ShaderManager: Link error in '%s':\n%s", name.c_str(), info_log.data());
        glDeleteProgram(program);
        return 0;
    }

    LOGI("ShaderManager: Program '%s' compiled OK (id=%u)", name.c_str(), program);
    programs_[name] = program;
    return program;
}

GLuint ShaderManager::GetProgram(const std::string& name) const {
    auto it = programs_.find(name);
    return (it != programs_.end()) ? it->second : 0;
}

void ShaderManager::Cleanup() {
    for (auto& [name, prog] : programs_) {
        glDeleteProgram(prog);
        LOGD("ShaderManager: Deleted program '%s'", name.c_str());
    }
    programs_.clear();
}

ShaderManager::~ShaderManager() { Cleanup(); }

} // namespace arracing
