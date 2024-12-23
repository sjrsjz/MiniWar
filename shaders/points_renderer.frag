#ifndef __glsl_point_renderer_frag__
#define __glsl_point_renderer_frag__
static const char* point_renderer_frag = R"(
#version 430 core

in vec4 color;
in vec3 world_pos;
out vec4 fragColor;
    
void main() {
    // 将坐标转换到[-1,1]范围
    vec2 pc = gl_PointCoord * 2.0 - 1.0;
    float dist = length(pc);
    
    // 基础圆形
    float circle = 1.0 - smoothstep(0.8, 1.0, dist);
    
    // 发光效果
    float glow = exp(-dist * 3.0) * 0.5;
    
    // 十字形
    float cross = max(
        1.0 - abs(pc.x) * 8.0,
        1.0 - abs(pc.y) * 8.0
    );
    
    // 组合效果
    float final = circle + glow + cross * 0.3;
    
    // 输出最终颜色
    fragColor = color * final * 10;
    
    // 输出深度
    float pdist = length(world_pos);
    gl_FragDepth = pdist / (1 + pdist);

}
)";
#endif