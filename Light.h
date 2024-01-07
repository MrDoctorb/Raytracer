#pragma once
#include "Vec.h"

class Light 
{
public:
	enum class LightType
	{
		Ambient,
		Directional,
		Point
	};

	LightType light_type;
	Vec3f colored_intensity;
	Vec3f position; //This is direction for directional lights and pos for point

	Light(LightType light_type, const Vec3f& colored_intensity, const Vec3f& position = {0.0,0.0,0.0})
		: light_type(light_type),
		colored_intensity(colored_intensity),
		position(position){}


};


/*
#pragma once
#include "Vec.h"

class Light
{
public:
	enum class LightType
	{
		Ambient,
		Directional,
		Point
	};

	LightType light_type;
	float intensity;//could change this to a Vec3 to convert to color
	Vec3f position; //This is direction for directional lights and pos for point

	Light(LightType light_type, float intensity, const Vec3f& position = { 0.0,0.0,0.0 })
		: light_type(light_type),
		intensity(intensity),
		position(position) {}


};*/