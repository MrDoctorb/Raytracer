#pragma once

#include <cmath>
#include <limits>
#include <vector>
#include <tuple>

#include "Sphere.h"
#include "Color.h"
#include "CanvasBase.h"
#include "misc.h"
#include "Light.h"
#include "Mat.h"

class RayTracingCanvas final : public CanvasBase
{
	static constexpr float viewport_size = 1.0f;
	static constexpr float projection_plane_z = 1.0f;

public:
	RayTracingCanvas(const char* window_title, size_t width, size_t height, unsigned int max_ray_recursion)
		:CanvasBase(window_title, width, height)
		, _camera_position({ 0.0f,0.0f,0.0f })
		, _camera_orientation(Mat::get_identity_matrix())
		, _max_ray_recursion(max_ray_recursion)
	{
	}

	void set_camera_position(const Vec3f& position) 
	{
		_camera_position = position;
	}

	void set_camera_orientation(const Mat& orientation)
	{
		_camera_orientation = orientation;
	}

	void add_sphere(const Sphere& sphere, const Color& color, float specular, float reflectivity)
	{
		_spheres.push_back(sphere);
		_sphere_colors.push_back(color);
		_sphere_speculars.push_back(specular);
		_sphere_reflectivities.push_back(reflectivity);
	}

	void add_light(const Light& light)
	{
		_lights.push_back(light);
	}

	std::vector<Sphere>& get_spheres()
	{
		return _spheres;
	}

	std::vector<Light>& get_lights()
	{
		return _lights;
	}

	void draw() const
	{
		auto min_t{ projection_plane_z };
		auto half_width = static_cast<int>(_width / 2);
		auto half_height = static_cast<int>(_height / 2);

		for (auto x = -half_width; x < half_width; ++x)
		{
			for (auto y = -half_height; y < half_height; ++y)
			{
				Vec2i ray_canvas_pos{ x,y };

				//This is D in our equation
				//auto ray_dir = canvas_to_viewport(ray_canvas_pos);

				auto ray_dir_v4 = _camera_orientation * canvas_to_viewport(ray_canvas_pos);
				auto ray_dir_v3 = Vec3f{ ray_dir_v4.x, ray_dir_v4.y , ray_dir_v4.z };
				auto c = trace_ray(_camera_position, ray_dir_v3, min_t, _max_ray_recursion);

				put_pixel(ray_canvas_pos, c);

			}
		}
	}

private:
	Vec3f _camera_position;
	Mat _camera_orientation;
	unsigned int _max_ray_recursion;
	std::vector<Light> _lights;
	std::vector<Sphere> _spheres;
	std::vector<Color> _sphere_colors;
	std::vector<float> _sphere_speculars;
	std::vector<float> _sphere_reflectivities;

	Vec3f canvas_to_viewport(const Vec2i& pt) const
	{
		return
		{
			static_cast<float>(pt.x) * viewport_size / static_cast<float>(_width),
			static_cast<float>(pt.y) * viewport_size / static_cast<float>(_height),
			projection_plane_z
		};
	}

	Vec3f compute_lighting(const Vec3f& point, const Vec3f& normal, const Vec3f& ray_dir, float specular) const
	{
		//float intensity = 0.0;
		Vec3f colored_intensity = { 0.0,0.0,0.0 };

		for (auto& light : _lights)
		{
			if (light.light_type == Light::LightType::Ambient)
			{
				//intensity += light.intensity;
				colored_intensity = colored_intensity + light.colored_intensity;
			}
			else
			{
				//could be position or direction, depends on what type it is
				auto vec_l = light.position;
				if (light.light_type == Light::LightType::Point)
				{
					//reduce based on distance
					vec_l = vec_l - point;
				}
				//Shadow
				auto intersection = find_closest_sphere_intersection(
					point, vec_l, 0.01f, std::numeric_limits<float>::max());
				if (std::get<0>(intersection) >= 0) continue;


				//Diffuse
				auto n_dot_l = compute_dot_product(normal, vec_l);
				if (n_dot_l > 0)
				{
					colored_intensity = colored_intensity + (n_dot_l * light.colored_intensity / compute_vector_length(vec_l));
				}

				//Specular
				auto vec_r = reflect_ray(normal, vec_l);
				auto r_dot_v = compute_dot_product(vec_r, -1 * ray_dir);
				if (r_dot_v > 0)
				{
					auto a = compute_vector_length(ray_dir);
					auto b = compute_vector_length(vec_r);
					auto c = r_dot_v / (b * a);
					auto s = std::pow(c, specular);

					colored_intensity.x += light.colored_intensity.x * s;
					colored_intensity.y += light.colored_intensity.y * s;
					colored_intensity.z += light.colored_intensity.z * s;
				}
			}
		}

		return colored_intensity;
	}

	static Vec3f reflect_ray(const Vec3f& normal, const Vec3f& ray)
	{
		return ((2.0f * compute_dot_product(normal, ray)) * normal) - ray;
	}

	static Color combine_color_and_intensity(const Color& color, const Vec3f& intensity)
	{
		auto r_uint = static_cast<unsigned int>(static_cast<float>(color.r) * intensity.x);
		auto g_uint = static_cast<unsigned int>(static_cast<float>(color.g) * intensity.y);
		auto b_uint = static_cast<unsigned int>(static_cast<float>(color.b) * intensity.z);

		r_uint = r_uint < 0 ? 0 : r_uint > 255 ? 255 : r_uint;
		g_uint = g_uint < 0 ? 0 : g_uint > 255 ? 255 : g_uint;
		b_uint = b_uint < 0 ? 0 : b_uint > 255 ? 255 : b_uint;

		return Color::custom(
			static_cast<uint8_t>(r_uint),
			static_cast<uint8_t>(g_uint),
			static_cast<uint8_t>(b_uint));
	}

	Color trace_ray(const Vec3f& camera_pos, const Vec3f& ray_dir,
		float min_t, unsigned int recursion_depth) const
	{
		auto intersection = find_closest_sphere_intersection(
			camera_pos, ray_dir, min_t, std::numeric_limits<float>::max());
		auto closest_sphere_index = std::get<0>(intersection);
		auto closest_t = std::get<1>(intersection);
		if (closest_sphere_index < 0) return Color::black;

		auto sphere = _spheres[closest_sphere_index];
		auto sphere_color = _sphere_colors[closest_sphere_index];
		auto sphere_specular = _sphere_speculars[closest_sphere_index];
		auto sphere_reflectivity = _sphere_reflectivities[closest_sphere_index];

		//Intersection point of the current ray
		auto point = camera_pos + (closest_t * ray_dir);
		//We can assume this because we're using spheres
		auto normal = point - sphere.center;
		normal = normal / compute_vector_length(normal);

		auto intensity = compute_lighting(point, normal, ray_dir, sphere_specular);

		auto local_color = sphere_color * intensity;

		if (sphere_reflectivity <= 0.0f || recursion_depth <= 0)
		{
			return local_color;
		}

		auto reflected_ray = reflect_ray(normal, -ray_dir);
		auto reflected_color = trace_ray(point, reflected_ray, 0.1f, recursion_depth - 1);

		auto c1 = local_color * (1.0f - sphere_reflectivity);
		auto c2 = reflected_color * sphere_reflectivity;
		return c1 + c2;
	}

	std::tuple<int, float> find_closest_sphere_intersection(const Vec3f& camera_pos, const Vec3f& ray_dir,
		float min_t, float max_t) const
	{
		auto closest_t = max_t;
		auto closest_sphere_index = -1;

		//If thi wasn't spheres this would be harder and more general
		for (int i = 0; i < static_cast<int>(_spheres.size()); ++i)
		{
			Vec2f intersections; //T1 and T2 in our equation. Intersections is changed by the following line
			auto intersects = intersect_ray_sphere(camera_pos, ray_dir, _spheres[i], intersections);

			if (!intersects) continue;

			if (intersections.x < closest_t && intersections.x > min_t)
			{
				closest_t = intersections.x;
				closest_sphere_index = i;
			}

			if (intersections.y < closest_t && intersections.y > min_t)
			{
				closest_t = intersections.y;
				closest_sphere_index = i;
			}
		}
		return { closest_sphere_index, closest_t };
	}


	//We are going to set the value of intersections inside of here
	static bool intersect_ray_sphere(const Vec3f& camera_pos, const Vec3f& ray_dir,
		const Sphere& sphere, Vec2f& intersections)
	{
		auto camera_to_sphere_center = camera_pos - sphere.center;
		auto k1 = compute_dot_product(ray_dir, ray_dir); //a
		auto k2 = 2 * compute_dot_product(camera_to_sphere_center, ray_dir); //b
		auto k3 = compute_dot_product(camera_to_sphere_center, camera_to_sphere_center) //c
			- sphere.radius * sphere.radius;

		auto discriminate = k2 * k2 - 4 * k1 * k3;
		if (discriminate < 0)
		{
			return false;
		}

		auto t1 = (-k2 + std::sqrt(discriminate)) / (2 * k1);
		auto t2 = (-k2 - std::sqrt(discriminate)) / (2 * k1);

		intersections = { t1, t2 };
		return true;
	}

};
