#ifndef __glsl_point_renderer_frag__
#define __glsl_point_renderer_frag__
static const char* point_renderer_frag = R"(
#version 430 core

in vec4 color;
in vec3 world_pos;
out vec4 fragColor;
    
void main() {
    vec2 pc = gl_PointCoord * 2.0 - 1.0;
    float dist = length(pc);
    
    // 圆形
    float circle = 1.0 - smoothstep(0.8, 1.0, dist);
    if(circle == 0) discard;
    fragColor = color * circle * 10;
    float pdist = length(world_pos);
    gl_FragDepth = pdist / (1 + pdist);

}
)";
#endif