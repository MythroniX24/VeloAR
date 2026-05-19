#pragma once
// renderer/shader_manager.h — GLSL shader compilation and program caching
#include <GLES3/gl3.h>
#include <string>
#include <unordered_map>
#include "../core/logger.h"

namespace arracing {

class ShaderManager {
public:
    ShaderManager() = default;
    ~ShaderManager();

    // Compile and link from source strings.
    // Returns program ID or 0 on failure.
    [[nodiscard]] GLuint CreateProgram(
        const std::string& vert_src,
        const std::string& frag_src,
        const std::string& name = "unnamed");

    // Get cached program by name
    [[nodiscard]] GLuint GetProgram(const std::string& name) const;

    // Delete all programs
    void Cleanup();

private:
    [[nodiscard]] GLuint CompileShader(GLenum type, const std::string& src,
                                       const std::string& label);

    std::unordered_map<std::string, GLuint> programs_;
};

} // namespace arracing
