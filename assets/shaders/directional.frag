#version 330 core

in Interpolators {
    vec2 uv;
    vec3 view;
    vec3 normal;
} fs_in;
in float visibility;
uniform vec3 skyColour;

struct Material {
    sampler2D ambient_occlusion;
    sampler2D emissive;
    sampler2D roughness;
    sampler2D specular;
    sampler2D albedo;
    vec3 albedo_tint;
    vec3 specular_tint;
    float roughness_scale; 
};
uniform Material material;
uniform mat4 trans;
uniform bool fog;
struct DirectionalLight {
    vec3 color;
    vec3 direction;
};
uniform DirectionalLight light;

uniform vec3 ambient;
out vec4 color;

float lambert(vec3 n, vec3 l){
    return max(0, dot(n,l));
}

float blinn_phong(vec3 n, vec3 l, vec3 v, float shininess){
    return pow(max(0, dot(n,normalize(l+v))), shininess);
}

void main()
{
    float ao = texture(material.ambient_occlusion, fs_in.uv).r;
    vec4 color1,color2,color3;

    vec3 albedo = material.albedo_tint * texture(material.albedo, fs_in.uv).rgb;
    vec3 specular = material.specular_tint * texture(material.specular, fs_in.uv).rgb;
    float roughness = material.roughness_scale * texture(material.roughness, fs_in.uv).r;
    vec3 n = normalize(fs_in.normal);
    vec3 v = normalize(fs_in.view);
    vec3 l = -light.direction;
    float shininess = 2/pow(max(0.01f,roughness), 2) - 2;
    color =trans* vec4(
        albedo*ao*ambient + 
        albedo*light.color*lambert(n, l) + 
        specular*light.color*blinn_phong(n, l, v, shininess),
        1.0f
    );
	if(fog)
		color.rgb=mix(skyColour,color.rgb,visibility-0.05);

}