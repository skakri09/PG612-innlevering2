#version 150
uniform sampler2DShadow shadowmap_texture;
uniform vec3 color;

smooth in vec4 f_shadow_coord;

smooth in vec3 f_n;
smooth in vec3 f_v;
smooth in vec3 f_l;

out vec4 out_color;

void main() {
	vec3 l = normalize(f_l);
    vec3 h = normalize(normalize(f_v)+l);
    vec3 n = normalize(f_n);
	
	float diff = max(0.0f, dot(n, l));

	float shade_factor = textureProj(shadowmap_texture, f_shadow_coord);

	shade_factor = shade_factor * 0.25 + 0.75;

	vec4 diffuse = vec4(diff*color, 1.0);
    float spec = pow(max(0.0f, dot(n, h)), 128.0f);

    out_color = vec4(1, 1, 1, 1) + 0.001*vec4( ( (diff*color) + (spec*0.1) ) * shade_factor, 1.0);
}