#version 460 core

#define PI 3.14

out vec4 out_color;

in vec2 fragUV;

in vec3 normal_vector;

in float h;

in vec3 fragment_position;

in vec4 shadowMapCoord;

uniform vec3 light_position;

uniform vec3 view_position;

layout(binding = 1) uniform sampler2DShadow shadow;

const vec3 grass_color = vec3(0.0f, 0.7f, 0.2f);
const float grass_reflectivity = 0.005f;
const float grass_metalness = 0.0f;
const float grass_fresnel = 0.95f;
const float grass_shininess = 64.0f;

const vec3 rock_color = vec3(0.5f, 0.45f, 0.4f);
const float rock_reflectivity = 0.05f;
const float rock_metalness = 0.4f;
const float rock_fresnel = 0.80f;
const float rock_shininess = 32.0f;

const vec3 snow_color = vec3(0.85f, 0.9f, 1.0f);
const float snow_reflectivity = 0.3f;
const float snow_metalness = 0.0f;
const float snow_fresnel = 0.2f;
const float snow_shininess = 256.0f;

const vec3 water_color = vec3(0.125f, 0.5f, 1.0f);
const float water_reflectivity = 0.3f;
const float water_metalness = 0.0f;
const float water_fresnel = 0.2f;
const float water_shininess = 256.0f;

uniform vec3 light_color;
const float light_intensity = 350000000.0f;

uniform vec3 irradience_color;// = vec3(0.5f, 0.7f, 1.0f);
const float irradience_intensity = 0.75f;

// Fresnel term
float F(float R0, vec3 wi, vec3 wh){
	return mix(pow(1 - dot(wh, wi), 5), 1, R0); 
}

// Microfacet Distribution Function
float D(float s, vec3 n, vec3 wh){
	return (s + 2) / (2 * PI) * pow(dot(n, wh), s);
}

// Masking function
float G(vec3 n, vec3 wi, vec3 wo, vec3 wh){
	return min(1, 2 * dot(n, wh) * min(dot(n, wo), dot(n, wi)) / dot(wo, wh));
}

float BRDF(float f, float d, float g, vec3 n, vec3 wi, vec3 wo){
	return f * d * g / (4 * dot(n, wo) * dot(n, wi));
}

vec3 calcDirectIllumination(vec3 wo, vec3 wi){

	vec3 n = normalize(normal_vector);

	if(dot(n, wi) <= 0.0)
		return vec3(0.0f);

	vec3 wh = normalize(wi + wo);

	float dist = distance(light_position, fragment_position);

	vec3 Li = light_intensity * light_color * 1.0/(dist*dist);
	
	vec3 object_color         = mix(mix(mix(grass_color, rock_color, clamp(n.y, 0.0f, 1.0f)), snow_color, clamp(2*h, 0.0f, 1.0f)), water_color, clamp(8*-h-4, 0.0f, 1.0f));
	float object_fresnel      = mix(mix(mix(grass_fresnel, rock_fresnel, clamp(n.y, 0.0f, 1.0f)), snow_fresnel, clamp(2*h, 0.0f, 1.0f)), water_fresnel, clamp(8*-h-4, 0.0f, 1.0f));
	float object_shininess    = mix(mix(mix(grass_shininess, rock_shininess, clamp(n.y, 0.0f, 1.0f)), snow_shininess, clamp(2*h, 0.0f, 1.0f)), water_shininess, clamp(8*-h-4, 0.0f, 1.0f));
	float object_metalness    = mix(mix(mix(grass_metalness, rock_metalness, clamp(n.y, 0.0f, 1.0f)), snow_metalness, clamp(2*h, 0.0f, 1.0f)), water_metalness, clamp(8*-h-4, 0.0f, 1.0f));
	float object_reflectivity = mix(mix(mix(grass_reflectivity, rock_reflectivity, clamp(n.y, 0.0f, 1.0f)), snow_reflectivity, clamp(2*h, 0.0f, 1.0f)), water_reflectivity, clamp(8*-h-4, 0.0f, 1.0f));

	float f = F(object_fresnel, wi, wh);
	float d = D(object_shininess, n, wh);
	float g = G(n, wi, wo, wh);

	float brdf = BRDF(f, d, g, n, wi, wo);

	if(isnan(brdf))
		brdf = 0;

	vec3 diffuse_term = object_color * (1.0/PI) * abs(dot(n, wi)) * Li;

	vec3 dielectric_term = brdf * dot(n, wi) * Li + (1 - f) * diffuse_term;

	vec3 metal_term = brdf * object_color * dot(n, wi) * Li;

	vec3 microfacet_term = mix(dielectric_term, metal_term, object_metalness);


	return mix(diffuse_term, microfacet_term, object_reflectivity);
}

vec3 calcIndirectIllumination(vec3 wo, vec3 wi){

	vec3 n = normalize(normal_vector);
	
	wi = vec3(normalize(reflect(-wo, n)));

	vec3 object_color         = mix(mix(mix(grass_color, rock_color, clamp(n.y, 0.0f, 1.0f)), snow_color, clamp(2*h, 0.0f, 1.0f)), water_color, clamp(8*-h-4, 0.0f, 1.0f));
	float object_fresnel      = mix(mix(mix(grass_fresnel, rock_fresnel, clamp(n.y, 0.0f, 1.0f)), snow_fresnel, clamp(2*h, 0.0f, 1.0f)), water_fresnel, clamp(8*-h-4, 0.0f, 1.0f));
	float object_shininess    = mix(mix(mix(grass_shininess, rock_shininess, clamp(n.y, 0.0f, 1.0f)), snow_shininess, clamp(2*h, 0.0f, 1.0f)), water_shininess, clamp(8*-h-4, 0.0f, 1.0f));
	float object_metalness    = mix(mix(mix(grass_metalness, rock_metalness, clamp(n.y, 0.0f, 1.0f)), snow_metalness, clamp(2*h, 0.0f, 1.0f)), water_metalness, clamp(8*-h-4, 0.0f, 1.0f));
	float object_reflectivity = mix(mix(mix(grass_reflectivity, rock_reflectivity, clamp(n.y, 0.0f, 1.0f)), snow_reflectivity, clamp(2*h, 0.0f, 1.0f)), water_reflectivity, clamp(8*-h-4, 0.0f, 1.0f));
	
	vec3 Li = irradience_intensity * irradience_color;

	vec3 diffuse_term = object_color * (1.0/PI) * Li;

	vec3 wh = normalize(wo + wi);

	float f = F(object_fresnel, wi, wh);

	vec3 dielectric_term = mix(diffuse_term, Li, f);

	vec3 metal_term = f * object_color * Li;

	vec3 microfacet_term = mix(dielectric_term, metal_term, object_metalness);

	return mix(diffuse_term, microfacet_term, object_reflectivity);
}

void main(){

	vec3 light_direction = normalize(fragment_position - light_position);
	vec3 view_direction = -normalize(fragment_position - view_position);

	vec3 direct_illumination = vec3(0.0f);
	vec3 indirect_illumination = vec3(0.0f);

	float visibility = textureProj(shadow, shadowMapCoord);

	if(light_position.y <= 0.0f)
		visibility = clamp(visibility + light_position.y/500.0f, 0.0f, 1.0f);

	direct_illumination = visibility * calcDirectIllumination(view_direction, -light_direction);	

	indirect_illumination = calcIndirectIllumination(view_direction, -light_direction);
	
	vec3 color = direct_illumination + indirect_illumination;

	out_color = vec4(color, 1.0f);

	//out_color = vec4(vec3(visibility), 1.0f);
	
}