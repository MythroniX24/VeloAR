// car/car_entity.cc — Car system orchestration
#include "car_entity.h"
#include "../renderer/shader_manager.h"
#include "../core/logger.h"
#include <glm/gtc/matrix_transform.hpp>

namespace arracing {

// ─── Car PBR shader ───────────────────────────────────────────────────────
static const char* kCarVert = R"(#version 300 es
layout(location = 0) in vec3 a_Pos;
layout(location = 1) in vec3 a_Normal;
layout(location = 2) in vec2 a_UV;

uniform mat4 u_MVP;
uniform mat4 u_Model;

out vec3 v_WorldPos;
out vec3 v_Normal;
out vec2 v_UV;

void main() {
    vec4 worldPos = u_Model * vec4(a_Pos, 1.0);
    v_WorldPos    = worldPos.xyz;
    v_Normal      = mat3(transpose(inverse(u_Model))) * a_Normal;
    v_UV          = a_UV;
    gl_Position   = u_MVP * vec4(a_Pos, 1.0);
}
)";

static const char* kCarFrag = R"(#version 300 es
precision mediump float;

in  vec3 v_WorldPos;
in  vec3 v_Normal;
in  vec2 v_UV;
out vec4 fragColor;

uniform vec3  u_Albedo;
uniform float u_Roughness;
uniform float u_Metallic;
uniform vec3  u_LightDir;
uniform vec3  u_CamPos;
uniform sampler2D u_AlbedoTex;
uniform int   u_HasTexture;

const float PI = 3.14159265;
const vec3  kAmbient = vec3(0.15);

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

void main() {
    vec3 albedo  = (u_HasTexture == 1)
                   ? texture(u_AlbedoTex, v_UV).rgb
                   : u_Albedo;

    vec3 N = normalize(v_Normal);
    vec3 L = normalize(-u_LightDir);
    vec3 V = normalize(u_CamPos - v_WorldPos);
    vec3 H = normalize(L + V);

    // Simple PBR diffuse + specular
    float NdotL  = max(dot(N, L), 0.0);
    float NdotH  = max(dot(N, H), 0.0);
    float rough2 = u_Roughness * u_Roughness;

    // GGX specular
    float denom  = NdotH * NdotH * (rough2 - 1.0) + 1.0;
    float D      = rough2 / (PI * denom * denom + 0.0001);
    vec3  F0     = mix(vec3(0.04), albedo, u_Metallic);
    vec3  F      = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3  specular = D * F * 0.25;

    // Diffuse (Lambertian, reduced for metallic)
    vec3  diffuse = albedo * (1.0 - u_Metallic) * (1.0 / PI);

    vec3 color = (diffuse + specular) * NdotL + kAmbient * albedo;
    // Simple gamma correction
    color = pow(color, vec3(1.0 / 2.2));

    fragColor = vec4(color, 1.0);
}
)";

bool CarEntity::Init(PhysicsWorld& world, const glm::vec3& spawn_pos,
                     AssetManager& /*assets*/, MeshRenderer& renderer) {
    // Build geometry
    car_mesh_.Build(0.9f, 0.2f, 1.8f);   // chassis half extents
    wheel_mesh_.Build(0.25f, 0.2f, 20);  // radius, width, segments

    // Compile shader
    ShaderManager sm;
    car_program_ = sm.CreateProgram(kCarVert, kCarFrag, "car_pbr");
    if (!car_program_) {
        LOGE("CarEntity: Failed to compile PBR shader");
        return false;
    }

    renderer.Init(car_program_);

    // Chassis material: red metallic paint
    chassis_mat_.albedo     = glm::vec3(0.85f, 0.12f, 0.12f);
    chassis_mat_.roughness  = 0.25f;
    chassis_mat_.metallic   = 0.85f;

    // Wheel material: dark rubber
    wheel_mat_.albedo     = glm::vec3(0.1f, 0.1f, 0.1f);
    wheel_mat_.roughness  = 0.9f;
    wheel_mat_.metallic   = 0.0f;

    // Initialize physics
    bool ok = vehicle_.Init(world, spawn_pos);
    if (!ok) {
        LOGE("CarEntity: Failed to initialize vehicle physics");
        return false;
    }

    LOGI("CarEntity: Initialized");
    return true;
}

void CarEntity::UpdatePhysics(const InputState& input, float dt) {
    vehicle_.Update(input, dt);
}

void CarEntity::Render(MeshRenderer& renderer,
                       const glm::mat4& view_proj,
                       const glm::vec3& light_dir,
                       const glm::vec3& cam_pos,
                       const glm::mat4& anchor_mat) {
    if (!car_mesh_.IsReady() || !wheel_mesh_.IsReady()) return;

    // ── Chassis ────────────────────────────────────────────────────────
    // Physics transform is in Bullet world space; we offset by AR anchor
    glm::mat4 chassis_physics = vehicle_.ChassisTransform();
    glm::mat4 chassis_model   = anchor_mat * chassis_physics;
    glm::mat4 chassis_mvp     = view_proj * chassis_model;

    renderer.Draw(car_mesh_.Mesh(), chassis_mat_,
                  chassis_mvp, chassis_model, light_dir, cam_pos);

    // ── Wheels ─────────────────────────────────────────────────────────
    for (int w = 0; w < 4; ++w) {
        glm::mat4 wheel_physics = vehicle_.WheelTransform(w);
        glm::mat4 wheel_model   = anchor_mat * wheel_physics;
        glm::mat4 wheel_mvp     = view_proj * wheel_model;

        renderer.Draw(wheel_mesh_.Mesh(), wheel_mat_,
                      wheel_mvp, wheel_model, light_dir, cam_pos);
    }
}

void CarEntity::ResetToAnchor(const glm::mat4& anchor_mat) {
    // Extract position from anchor matrix
    glm::vec3 anchor_pos = glm::vec3(anchor_mat[3]);
    vehicle_.SetPosition(anchor_pos);
}

} // namespace arracing
