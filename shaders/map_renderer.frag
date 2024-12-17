const char* map_renderer_frag = R"(
#version 430 core
layout(std140, binding = 0) buffer MapBuffer{
	vec4 mapData[];
} mapBuffer;


in vec2 texCoord; // [-1, 1]
uniform float g_time;
uniform float g_fov;
uniform float g_frame_width, g_frame_height;
uniform mat4 g_trans_mat;

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
};


PlaneIntersection intersectPlane(vec3 rayOrigin, vec3 rayDir, vec3 planeU, vec3 planeV, vec3 planePos){
	vec3 n = cross(planeU, planeV);
	float t = dot(n, planePos - rayOrigin) / dot(n, rayDir);
	vec3 pos = rayOrigin + t*rayDir;
	vec3 posLocal = pos - planePos;
	vec2 uv;
	uv.x = dot(posLocal, planeU);
	uv.y = dot(posLocal, planeV);
	return PlaneIntersection(pos,uv);
}

vec4 doPlaneColoring(vec2 uv){
	return vec4(uv.x*10,uv.y*10, 0, 1) * float(abs(uv.x) <= 1 && abs(uv.y) <= 1);
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
	const float PI = 3.1415936535;
	vec3 planeU = normalize(vec3(1,0,0));
	vec3 planeV = normalize(vec3(0,1,0));
	planeU = rot3D(planeU, vec3(0,0,1), PI-0.5);
	planeV = rot3D(planeV, vec3(0,0,1), PI-0.5);
	
	planeU = rot3D(planeU, vec3(1,0,0), 1);
	planeV = rot3D(planeV, vec3(1,0,0), 1);
	vec3 planePos = vec3(0, 0, 3);
	PlaneIntersection pi = intersectPlane(rayOrigin, rayDir, planeU, planeV, planePos);
	return doPlaneColoring(pi.uv);
}


out vec4 fragColor;
void main(){
	fragColor = render() + mapBuffer.mapData[0];
}
)";