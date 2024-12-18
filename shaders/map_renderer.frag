const char* map_renderer_frag = R"(
#version 430 core


in vec2 texCoord; // [-1, 1]
uniform float g_time;
uniform float g_fov;
uniform float g_frame_width, g_frame_height;
uniform mat4 g_trans_mat;

uniform ivec2 g_selected;

//uniform vec3 g_plane_u;
//uniform vec3 g_plane_v;
//uniform vec3 g_plane_pos;

uniform mat4 g_model_trans_mat;
uniform mat4 g_model_trans_mat_inv;
uniform ivec2 g_map_size;
struct RegionData{
	vec2 cell_center;
	float identity;
	float padding_1;
};
RegionData empty_region(){
	return RegionData(vec2(1000000), 0, 0);	
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

vec3 get_identity_color(float identity){
	if(identity == 0) return vec3(1,1,1);
	if(identity == 1) return mix(vec3(1),vec3(1,0,0),1 - abs(sin(2 * g_time)));
	if(identity == 2) return vec3(0,1,0);
	if(identity == 3) return vec3(0,0,1);
	return vec3(1,1,1);
}



vec3 getRayDir(float x, float y){ // x, y in [-1, 1]
	float aspect = g_frame_width / g_frame_height;
	y /= aspect;
	vec3 dir = vec3(x, y, g_fov);
	dir = normalize(dir);

	return (g_trans_mat * vec4(dir, 0)).xyz;
}

struct PlaneIntersection{
	vec3 pos;
	vec2 uv;
	float t;
};


vec3 cell(vec2 cell_uv, vec2 cell_center[3][3]){
    float min_dist = 1e6;
    vec2 idx = vec2(0);
    for(int i = 0; i < 3; i++){
        for(int j = 0; j < 3; j++){
            vec2 test_cell_uv = cell_center[i][j] + vec2(i, j) - 1;
            float dist = length(cell_uv - test_cell_uv);
            idx = mix(idx, vec2(i, j), float(dist < min_dist));
            min_dist = min(min_dist, dist);
        }
    }
    return vec3(ivec2(idx) - 1, min_dist);    
}

float cell_sdf(vec2 cell_uv, vec2 cell_center[5][5]){
    float sdf = 1e6;
    for(int i = 0; i < 5; i++){
        for(int j = 0; j < 5; j++){
			if(i==2 && j==2) continue;
			vec2 N = normalize(cell_center[i][j] - cell_center[2][2]);
			sdf = min(sdf, abs(dot(N,0.5*(cell_center[i][j]+cell_center[2][2])-cell_uv)));
        }
    }
    return sdf;    
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
    #line 0
	if( tN>tF || tF<0.0) return vec2(-1.0); // no intersection
    outNormal = (tN>0.0) ? step(vec3(tN),t1): // ro ouside the box
                           step(t2,vec3(tF));  // ro inside the box
    outNormal *= -sign(rd);
    return vec2( tN, tF );
}

vec4 doPlaneColoring(vec2 uv){
	vec2 cell_center[3][3];
	vec4 cell_position = uv_to_cell_position(uv);
	vec2 cell_uv = cell_position.zw;
	ivec2 cell_idx = ivec2(cell_position.xy);
	for(int i = -1; i <= 1; i++){
		for(int j = -1; j <= 1; j++){
			cell_center[i+1][j+1] = getRegion(cell_idx.x + i, cell_idx.y + j).cell_center;
		}
	}
	vec3 cell_data = cell(cell_uv, cell_center);
	ivec2 real_cell_idx = cell_idx + ivec2(cell_data.xy);
	vec2 cell_center_5x5[5][5];
	for(int i = -2; i <= 2; i++){
		for(int j = -2; j <= 2; j++){
			cell_center_5x5[i+2][j+2] = getRegion(real_cell_idx.x + i, real_cell_idx.y + j).cell_center + cell_data.xy + vec2(i,j);
		}
	}
	float sdf = cell_sdf(cell_uv, cell_center_5x5);



	vec3 color = vec3(0);
	RegionData region = getRegion(real_cell_idx.x, real_cell_idx.y);
	color = get_identity_color(region.identity) * float(cell_data.z * 0.5);
	color = mix(color, vec3(1,0,0), float(cell_data.z < 0.05));
	color = mix(vec3(0,0,0) * float(sdf<0.0125), color, 0.5 + 0.95 * float(sdf>=0.0125));

	if(real_cell_idx.x == g_selected.x && real_cell_idx.y == g_selected.y){
	color =  vec3(0,1,0)*( 0.5 + 0.5 * sin(g_time*5));
	}

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

vec4 render(){
	vec3 rayDir = getRayDir(texCoord.x, texCoord.y);
	vec3 rayOrigin = (g_trans_mat * vec4(0, 0, 0, 1)).xyz;
	vec2 max_padding_board = 1.0/g_map_size;
	vec4 color = vec4(0);

	vec3 box_normal = vec3(0);
	vec3 transformed_rayOrigin = (g_model_trans_mat_inv * vec4(rayOrigin, 1)).xyz;
	vec3 transformed_rayDir = (g_model_trans_mat_inv * vec4(rayDir, 0)).xyz;
	vec2 base_box = boxIntersection(transformed_rayOrigin + vec3(0,0.875+1e-3,0), transformed_rayDir, vec3(1 + max_padding_board.x ,0.25, 1 + max_padding_board.y), box_normal);
	if(base_box.x > 0){
		vec3 p = transformed_rayOrigin + base_box.x * transformed_rayDir;
		vec4 box_color = vec4(1)* pow(clamp(1.5+p.y,0.,1.),4);
		color = mix(color, box_color, box_color.a);
	}
	
	
	vec3 g_plane_u = (g_model_trans_mat*vec4(1,0,0,0)).xyz;
	vec3 g_plane_v = (g_model_trans_mat*vec4(0,0,1,0)).xyz;
	vec3 g_plane_pos = (g_model_trans_mat*vec4(0,0,0,1)).xyz;

	PlaneIntersection pi = intersectPlane(rayOrigin, rayDir, g_plane_u, g_plane_v, g_plane_pos);

	vec4 map = doPlaneColoring(pi.uv) * float(pi.t > 0) * float(abs(pi.uv.x)<=1 + max_padding_board.x &&abs(pi.uv.y)<=1 + max_padding_board.y);

	float board = max(abs(pi.uv.x),abs(pi.uv.y));
	map += step(1 + max_padding_board.x,board)*pow(smoothstep(1.5, 1. + max_padding_board.x,board),2)*vec4(1+sin(g_time*2)) * 0.25;

	if(pi.t <= base_box.x || base_box.x < 0)
		color = mix(color, map, map.a);


	return color;
}


out vec4 fragColor;
void main(){
	fragColor = render();
}
)";