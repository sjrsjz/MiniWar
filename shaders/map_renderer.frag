#ifndef __glsl_map_renderer_frag__
#define __glsl_map_renderer_frag__
static const char* map_renderer_frag = R"(
#version 430 core


in vec2 texCoord; // [-1, 1]
uniform float g_time;
uniform float g_fov;
uniform float g_frame_width, g_frame_height;
uniform mat4 g_trans_mat;

uniform ivec2 g_selected;
uniform ivec2 g_mouse_selected;

uniform vec4 g_radioactive_selected; // [center_x, center_y, radius, selected]
uniform vec3 g_attack_target; // [center_x, center_y, selected]
uniform vec4 g_scatter_target; // [center_x, center_y, radius, selected]


uniform mat4 g_model_trans_mat;
uniform mat4 g_model_trans_mat_inv;
uniform ivec2 g_map_size;


layout(binding = 0) uniform sampler2D g_tex_radioactive;
layout(binding = 1) uniform sampler2D g_tex_attack_target;
layout(binding = 2) uniform sampler2D g_tex_scatter_target;

struct RegionData{
	vec2 cell_center;
    vec2 army_position; // 标记的军队位置，用于显示军队，当 x = -1e6 时表示没有军队
	float identity;
	float padding_1;
};
RegionData empty_region(){
	return RegionData(vec2(1000000), vec2(-1e6), 0, 0);	
}
layout(std430, binding = 0) buffer MapBuffer{
	RegionData region[];
} mapBuffer;

RegionData getRegion(int x, int y){
	if(x < 0 || x >= g_map_size.x || y < 0 || y >= g_map_size.y) return empty_region();
	return mapBuffer.region[x + y * g_map_size.x];
}
vec4 uv_to_cell_position(vec2 uv){
	// 转换到网格坐标
	uv = 0.5 * uv + 0.5;
	uv *= vec2(g_map_size);
	return vec4(floor(uv), fract(uv));
}

float hexagon(vec2 position) {    
    position /= vec2(2.0, sqrt(3.0));
    position.y -= 0.5;
    position.x -= fract(floor(position.y) * 0.5);
    position = abs(fract(position) - 0.5);
    return abs(1.0 - max(position.x + position.y * 1.5, position.x * 2.0));
}

vec3 get_identity_color(float identity, vec2 uv, vec3 region_normal, vec3 sky_color){
    vec3 region_color = vec3(region_normal.y * 0.1);
	if(identity == 0) return region_color;
	if(identity == 1) return mix(region_color, vec3(10,0,0), pow(1 - 0.65 * abs(sin(2 * g_time)),4));
	if(identity == 2) return mix(region_color, vec3(0,1,1) * max(0.1 - hexagon(uv*g_map_size.x), 0) * 20, 0.5 * pow(1 - 0.65 * abs(sin(2 * g_time)),4));
	if(identity == 3) return vec3(0,0,1);
	return vec3(1,1,1);
}

const float PI = 3.1415926535897932384626433832795;
float luma(vec3 c) {
    return dot(c, vec3(0.299, 0.587, 0.114));
}

// 大气渲染
float S_R = 0.05;
float cosD_S = 1 / sqrt(1 + S_R * S_R);
vec3 b_P = vec3(300000); //atmosphere thickness
float b_k = 0.25; //mix
vec3 Mie = vec3(0.1);
vec3 Rayleigh = 5e9 * pow(vec3(1. / 700, 1. / 520, 1. / 450), vec3(4));
vec3 b_k0 = mix(Rayleigh, Mie, b_k);
vec3 b_Q = b_k0 / (b_P * b_P); //absorption
vec3 b_g0 = mix(Rayleigh * 0.01, vec3(luma(Rayleigh)), b_k); //single scatter

vec3 getSkyColor(vec3 b_Sun, vec3 b_Moon, in vec3 pos, in vec3 n, in vec3 lightDir) {
    mediump vec3 n0 = n;
    n.y = max(n.y, 1e-5);
    mediump float dot_n_L = dot(n, lightDir);
    mediump vec3 b_g0_2 = b_g0 * b_g0;
    mediump vec3 tmp_x=1. + b_g0_2 - 2. * b_g0 * dot_n_L;
    tmp_x *= tmp_x * tmp_x;
    mediump vec3 g = 3. / (8. * PI) * (1. + dot_n_L*dot_n_L) * (1. - b_g0_2) / (2. + b_g0_2) * inversesqrt(tmp_x);
    mediump vec3 t = b_Q * 0.5 * (b_P - pos.y) * (b_P - pos.y);
    mediump vec3 c = b_Sun * g * (exp(-t / n.y) - exp(-t / lightDir.y)) / (n.y - lightDir.y) * max(lightDir.y, 0.);
	float a = abs(min(dot(n0, lightDir) - cosD_S, 0));
    c += exp(-t / n.y) * b_Sun * exp(-sqrt(a * 6000)) * 10;
    c += exp(-t / n.y) * b_Moon * exp(-sqrt(a * 15000)) * 10;
    return max(c,0);
}
vec3 getFogColor(vec3 b_Sun, vec3 b_Moon, in vec3 pos, in vec3 n, in vec3 lightDir, float s, vec3 col) {
    mediump vec3 n0 = n;
    if (n.y > 0) s = min((b_P.x - pos.y) / n.y, s);
    float dot_n_L = dot(n, lightDir);
    mediump vec3 b_g0_2 = b_g0 * b_g0;
    mediump vec3 tmp_x = 1. + b_g0_2 - 2. * b_g0 * dot_n_L;
    tmp_x *= tmp_x * tmp_x;
    mediump vec3 g = 3. / (8. * PI) * (1. + dot_n_L * dot_n_L) * (1. - b_g0_2) / (2. + b_g0_2) * inversesqrt(tmp_x);
    mediump vec3 t = b_Q * 0.5 * (b_P - pos.y) * (b_P - pos.y);
    mediump vec3 s1 = exp(b_Q * s * (0.5 * s * n.y - (b_P - pos.y)) * (1 - n.y / lightDir.y));
    mediump vec3 c =  b_Sun * g * exp(-t / lightDir.y) * (1 - s1) / (-n.y + lightDir.y) * max(lightDir.y, 0.);
    c += exp(b_Q * 0.5 * n.y * s * s - b_Q * (b_P - pos.y) * s) * col;
    return c;
}

vec3 getRayDir(float x, float y){ // x, y in [-1, 1]
	float aspect = g_frame_width / g_frame_height;
	x *= aspect;
	vec3 dir = vec3(x, y, g_fov);
	dir = normalize(dir);

	return normalize((g_trans_mat * vec4(dir, 0)).xyz);
}

struct PlaneIntersection{
	vec3 pos;
	vec2 uv;
	float t;
};


mat2x3 cell(vec2 cell_uv, vec2 cell_center[3][3], vec2 army_center[3][3]){
    float min_dist = 1e6;
    vec2 idx = vec2(0);

    float army_dist = 1e6;
    vec2 army_idx = vec2(0);
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            vec2 test_cell_uv = cell_center[i][j] + vec2(i, j) - 1;
            float dist = length(cell_uv - test_cell_uv);
            idx = mix(idx, vec2(i, j), float(dist < min_dist));
            min_dist = min(min_dist, dist);

            vec2 test_army_uv = army_center[i][j] + vec2(i, j) - 1;
            float dist_army = length(cell_uv - test_army_uv);
            army_idx = mix(army_idx, vec2(i, j), float(dist_army < army_dist));
            army_dist = min(army_dist, dist_army);

        }
    }
    return mat2x3(vec3(ivec2(idx) - 1, min_dist), vec3(ivec2(army_idx) - 1, army_dist));
}

vec2 cell_sdf(vec2 cell_uv, vec3 cell_center[5][5]){
    float sdf = 1e6;
	float diff_identity = 0;
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
			if(i==2 && j==2) continue;
			vec2 N = normalize(cell_center[i][j].xy - cell_center[2][2].xy);
			float d = abs(dot(N,0.5*(cell_center[i][j].xy+cell_center[2][2].xy)-cell_uv));
			diff_identity = float(d<sdf) * float(cell_center[i][j].z != cell_center[2][2].z) + diff_identity * (1 - float(d<sdf));
			sdf = min(sdf, d);
        }
    }
    return vec2(sdf, diff_identity);    
}


PlaneIntersection intersectPlane(vec3 rayOrigin, vec3 rayDir, vec3 planeU, vec3 planeV, vec3 planePos){
	vec3 n = cross(planeU, planeV);
	float t = dot(n, planePos - rayOrigin) / dot(n, rayDir);
	if (t<0) return PlaneIntersection(vec3(0), vec2(0), -1);
	vec3 pos = rayOrigin + t*rayDir;
	vec3 posLocal = pos - planePos;
	vec2 uv;
	uv.x = dot(posLocal, planeU);
	uv.y = dot(posLocal, planeV);
	return PlaneIntersection(pos,uv,t);
}

vec2 boxIntersection( in vec3 ro, in vec3 rd, vec3 boxSize, out vec3 outNormal ) 
{
    vec3 m = 1.0/rd; // can precompute if traversing a set of aligned boxes
    vec3 n = m*ro;   // can precompute if traversing a set of aligned boxes
    vec3 k = abs(m)*boxSize;
    vec3 t1 = -n - k;
    vec3 t2 = -n + k;
    float tN = max( max( t1.x, t1.y ), t1.z );
    float tF = min( min( t2.x, t2.y ), t2.z );
	if( tN>tF || tF<0.0) return vec2(-1.0); // no intersection
    outNormal = (tN>0.0) ? step(vec3(tN),t1): // ro ouside the box
                           step(t2,vec3(tF));  // ro inside the box
    outNormal *= -sign(rd);
    return vec2( tN, tF );
}

struct CellIndex {
    ivec2 base_idx;      // 基础网格索引 
    ivec2 real_idx;      // 实际区块索引
    vec2 cell_uv;        // 区块内UV坐标
    vec3 cell_data;      // 区块数据(偏移xy + 距离)
    float sdf;           // 距离场
    float diff_identity; // 身份差异
    vec3 army_data; // 军队数据
};

CellIndex calculateCellIndex(vec2 uv) {
    CellIndex result;
    
    // 基础索引计算
    vec4 cell_position = uv_to_cell_position(uv);
    result.cell_uv = cell_position.zw;
    result.base_idx = ivec2(cell_position.xy);
    
    // 3x3网格中心点计算
    vec2 cell_center[3][3];
    vec2 army_center[3][3];
    for(int i = -1; i <= 1; i++){
        for(int j = -1; j <= 1; j++){
            cell_center[i+1][j+1] = getRegion(result.base_idx.x + i, result.base_idx.y + j).cell_center;
            army_center[i+1][j+1] = getRegion(result.base_idx.x + i, result.base_idx.y + j).army_position;
        }
    }
    mat2x3 cell_ = cell(result.cell_uv, cell_center, army_center);
    result.cell_data = cell_[0];
    result.army_data = cell_[1];
    result.real_idx = result.base_idx + ivec2(result.cell_data.xy);

    // 5x5网格SDF计算
    vec3 cell_center_5x5[5][5];
    for(int i = -2; i <= 2; i++){
        for(int j = -2; j <= 2; j++){
            RegionData r = getRegion(result.real_idx.x + i, result.real_idx.y + j);
            cell_center_5x5[i+2][j+2] = vec3(r.cell_center + result.cell_data.xy + vec2(i,j), r.identity);
        }
    }
    
    vec2 sdf_diff = cell_sdf(result.cell_uv, cell_center_5x5);
    result.sdf = sdf_diff.x;
    result.diff_identity = sdf_diff.y;
    
    return result;
}

vec3 sun_light_dir = normalize(vec3(-1,0.03,-1));

vec4 doPlaneColoring(vec2 uv, vec3 sky_color){
    CellIndex cell_idx = calculateCellIndex(uv);
    
    vec3 color = vec3(0);
    RegionData region = getRegion(cell_idx.real_idx.x, cell_idx.real_idx.y);

    vec3 region_normal = normalize(vec3(region.cell_center.x - 0.5, 1, region.cell_center.y-0.5));

    color = get_identity_color(region.identity, uv, region_normal, sky_color);

    //color *= sky_color * max(dot(region_normal,sun_light_dir),0.1);
    
    // 区块中心
    // color = mix(color, vec3(1,0,0), float(cell_idx.cell_data.z < 0.05));

    // 军队位置
    color = mix(color, vec3(10,10,10), float(cell_idx.army_data.z < 0.05));

    // 当前选中
    bool is_selected = cell_idx.real_idx.x == g_selected.x && cell_idx.real_idx.y == g_selected.y;
    
    if(g_radioactive_selected.w < 0.5 && g_attack_target.z < 0.5 && g_scatter_target.w < 0.5){
        is_selected = is_selected || cell_idx.real_idx.x == g_mouse_selected.x && cell_idx.real_idx.y == g_mouse_selected.y;
    }
    if(is_selected){
        color = vec3(0,10,0) * (0.5 + 0.5 * sin(g_time * 15));
    }

    // 圆形选中
    if(g_radioactive_selected.w > 0.5){
        vec2 center = g_radioactive_selected.xy;
        float radius = g_radioactive_selected.z;
        float dist = length(vec2(cell_idx.real_idx) - center);
        if(dist < radius){
            float weight = pow(0.65 + 0.35 * sin(g_time*5),3) * exp(-7*dist/(radius+1e-3));
            color = mix(color, vec3(20,20,0), weight);
            vec2 region_uv = (vec2(g_mouse_selected) + 0.5) / g_map_size * 2 - 1;
            vec2 d_uv = (uv - region_uv) * g_map_size / radius * 1.25 * 0.25 + 0.5;

            vec4 color_radioactive = texture(g_tex_radioactive, d_uv);
            color_radioactive *= float(d_uv.x>=0 && d_uv.x <= 1 && d_uv.y >= 0 && d_uv.y <= 1);
            color = mix(color, vec3(100,100,0) * color_radioactive.a, color_radioactive.a * weight);
        }
    }

    if(g_attack_target.z > 0.5){
        vec2 center = g_attack_target.xy;
		float radius = 5;
		float dist = length(vec2(cell_idx.real_idx) - center);
		if(dist < radius){
			vec2 region_uv = (vec2(g_mouse_selected) + 0.5) / g_map_size * 2 - 1;
			vec2 d_uv = (uv - region_uv) * g_map_size / radius * 2 * 0.25 + 0.5;

			vec4 color_attack_target = texture(g_tex_attack_target, d_uv);
			color_attack_target *= float(d_uv.x>=0 && d_uv.x <= 1 && d_uv.y >= 0 && d_uv.y <= 1);
			color = mix(color, vec3(0,10,0) * color_attack_target.a, color_attack_target.a * pow(0.55 + 0.45 * sin(g_time*5),3));
        }
     }

     if(g_scatter_target.w > 0.5){
         vec2 center = g_scatter_target.xy;
         float radius = g_scatter_target.z;
         float dist = length(vec2(cell_idx.real_idx) - center);
         if(dist < radius){
		     float weight = pow(0.65 + 0.35 * sin(g_time*5),3);//* exp(-7*dist/(radius+1e-3));
		     color = mix(color, vec3(0.5,0,0), weight);
		     vec2 region_uv = (vec2(g_mouse_selected) + 0.5) / g_map_size * 2 - 1;
		     vec2 d_uv = (uv - region_uv) * g_map_size / radius * 1.25 * 0.25 + 0.5;

		     vec4 color_scatter_target = texture(g_tex_scatter_target, d_uv);
		     color_scatter_target *= float(d_uv.x>=0 && d_uv.x <= 1 && d_uv.y >= 0 && d_uv.y <= 1);
		     color = mix(color, vec3(50,0,0) * color_scatter_target.a, color_scatter_target.a * weight);
	     }
    }
    // 边界线
    color = mix(mix(vec3(0,0,0),vec3(0,500,0),cell_idx.diff_identity) * float(cell_idx.sdf<0.025), 
                color, 
                0.5 + 0.95 * float(cell_idx.sdf>=0.0125));
    return vec4(color, 1);
}

vec2 rot2D(vec2 v, float angle){
	return vec2(v.x*cos(angle) - v.y*sin(angle), v.x*sin(angle) + v.y*cos(angle));
}

vec3 rot3D(vec3 v, vec3 axis, float angle){
	axis = normalize(axis);
	float s = sin(angle);
	float c = cos(angle);
	float oc = 1 - c;
	mat3 rotMat = mat3(
		oc*axis.x*axis.x + c, oc*axis.x*axis.y - axis.z*s, oc*axis.x*axis.z + axis.y*s,
		oc*axis.x*axis.y + axis.z*s, oc*axis.y*axis.y + c, oc*axis.y*axis.z - axis.x*s,
		oc*axis.x*axis.z - axis.y*s, oc*axis.y*axis.z + axis.x*s, oc*axis.z*axis.z + c
	);
	return rotMat * v;
}

vec4 render(out float depth){
	vec3 rayDir = getRayDir(texCoord.x, texCoord.y);
	vec3 rayOrigin = (g_trans_mat * vec4(0, 0, 0, 1)).xyz;
	vec2 max_padding_board = 1.0 / g_map_size;


	float dist = 1e6;

	vec3 box_normal = vec3(0);
	vec3 transformed_rayOrigin = (g_model_trans_mat_inv * vec4(rayOrigin, 1)).xyz;
	vec3 transformed_rayDir = (g_model_trans_mat_inv * vec4(rayDir, 0)).xyz;

    vec3 sky_color = getSkyColor(vec3(10),vec3(0.1),transformed_rayOrigin,transformed_rayDir,sun_light_dir);
	vec4 color = vec4(sky_color,0);


	vec2 base_box = boxIntersection(transformed_rayOrigin + vec3(0,0.875+1e-3,0), transformed_rayDir, vec3(1 + max_padding_board.x ,0.25, 1 + max_padding_board.y), box_normal);
	if(base_box.x > 0){
		vec3 p = transformed_rayOrigin + base_box.x * transformed_rayDir;
		vec4 box_color = vec4(0.1,0.1,0.1,1)* pow(clamp(1.55+p.y,0.,1.),4);
		color = mix(color, box_color, box_color.a);
		dist = min(dist, base_box.x);
	}
	
	
	vec3 g_plane_u = vec3(1,0,0);
	vec3 g_plane_v = vec3(0,0,1);
	vec3 g_plane_pos = vec3(0,-0.625,0);

	PlaneIntersection pi = intersectPlane(transformed_rayOrigin, transformed_rayDir, g_plane_u, g_plane_v, g_plane_pos);

    bool in_plane = pi.t > 0 && abs(pi.uv.x)<=1 + max_padding_board.x && abs(pi.uv.y) <= 1 + max_padding_board.y;

	vec4 map = doPlaneColoring(pi.uv, sky_color) * float(in_plane);

	//float board = max(abs(pi.uv.x),abs(pi.uv.y));
	//map += step(1 + max_padding_board.x,board)*pow(smoothstep(1.5, 1. + max_padding_board.x,board),2)*vec4(0.125);

	if((pi.t <= base_box.x || base_box.x < 0) && in_plane){
		color = mix(color, map, map.a);
		dist = min(dist, pi.t);
    }
    // 计算depth
    depth = dist;

	color.xyz = getFogColor(vec3(10),vec3(0.1),rayOrigin,rayDir,sun_light_dir,min(dist,5)*50000,color.xyz);

	return color;
}


layout (location = 0) out vec4 fragColor;
void main(){
    float depth;
   	fragColor = render(depth);
    gl_FragDepth = depth/(1+depth);
}
)";
#endif