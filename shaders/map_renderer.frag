const char* map_renderer_frag = R"(
#version 430 core

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

#line 33
PlaneIntersection intersectPlane(vec3 rayOrigin, vec3 rayDir, vec3 planeNormal, vec3 planePos){
	float d = dot(planeNormal, planePos);
	float t = (d - dot(planeNormal, rayOrigin)) / dot(planeNormal, rayDir);
	vec3 pos = rayOrigin + rayDir * t;
	vec2 uv = pos.xy;
	return PlaneIntersection(pos,uv);
}

vec4 doPlaneColoring(vec2 uv){
	return vec4(sin(uv.x*10),cos(uv.y*10), 1, 1);
}

vec4 render(){
	vec3 rayDir = getRayDir(texCoord.x, texCoord.y);
	vec3 rayOrigin = (g_trans_mat * vec4(0, 0, 0, 1)).xyz;
	vec3 planeNormal = vec3(0, 0, 1);
	vec3 planePos = vec3(0, 0, 0);
	PlaneIntersection pi = intersectPlane(rayOrigin, rayDir, planeNormal, planePos);
	return vec4(rayDir,1);
	return doPlaneColoring(pi.uv);
}


out vec4 fragColor;
void main(){
	fragColor = render();
}
)";