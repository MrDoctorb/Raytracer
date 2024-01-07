#include <math.h>
#include "misc.h"
#include"RayTracingCanvas.h"


int main(int argc, char* argv[])
{
	RayTracingCanvas c("Raytracing!!!??!?!", 600, 600, 3);
	Sphere red{ {0.0f, -1.0f, 3.0f}, 1.0f };
	Sphere green = { {-2.0f, 0.0f, 4.0f}, 1.0f };
	Sphere blue = { {2.0f, 0.0f, 4.0f}, 1.0f };
	Sphere yellow = { {0.0f, -8, 0.0}, 7 };//Floor

	c.add_light({ Light::LightType::Ambient, {.2f,.2f,.2f} });
	c.add_light({ Light::LightType::Directional, {.2f,.2f,.2f}, {1, 4, 4} });
	c.add_light({ Light::LightType::Point, {.6f,.2f,.2f}, {2, 1, 0} });
	//c.add_light({ Light::LightType::Point, {-.6f,-.6f,-.6f}, {0, 1, 3} });


	c.add_sphere(red, Color::red, 500, 0.2f);
	c.add_sphere(green, Color::green, 10, 0.4f);
	c.add_sphere(blue, Color::blue, 500, 0.3f);
	c.add_sphere(yellow, Color::yellow, 1000, 0.5f);


	//float degrees = 0;

	float camera_angle = 0.0f;
	float light_1_angle = 0.0f;
	float light_2_angle = 0.0f;
	float sphere_1_angle = 0.0f;
	float sphere_2_angle = 0.0f;

	c.set_camera_orientation(Mat::get_rotation_matrix(35, { 1,0,0 }));

	while (should_keep_rendering())
	{
		c.clear();

		auto sphere_1_x_pos = 4.0f * std::sin(sphere_1_angle) - 2.0f;
		auto sphere_1_z_pos = 4.0f * std::cos(sphere_1_angle) + 4.0f;

		auto sphere_2_x_pos = 5.0f * std::sin(sphere_2_angle) + 2.0f;
		auto sphere_2_z_pos = 4.0f * std::cos(sphere_2_angle) + 4.0f;

		auto camera_x_pos = 2.0f * std::sin(camera_angle);
		auto camera_z_pos = 2.0f * std::cos(camera_angle) - 15;

		auto light_1_x_pos = 2.0f * std::sin(light_1_angle);
		auto light_1_z_pos = 2.0f * std::cos(light_1_angle);
			 
		auto light_2_x_pos = 5.0f * std::sin(light_2_angle);
		auto light_2_z_pos = 5.0f * std::cos(light_2_angle);


		c.get_spheres()[1].center = { sphere_1_x_pos, 1, sphere_1_z_pos };
		c.get_spheres()[2].center = { sphere_2_x_pos, 2, sphere_2_z_pos };
		c.set_camera_position({ camera_x_pos, 8.0f, camera_z_pos });
		//c.set_camera_orientation(Mat::get_rotation_matrix(-camera_angle, { 0,1,0 }));
		c.get_lights()[1].position = { light_1_x_pos, 3, light_1_z_pos };
		c.get_lights()[2].position = { light_2_x_pos, 4, light_2_z_pos };




		//c.get_spheres()[0].center.y = std::sin(degrees) - 1;
		c.draw();
		c.present();
		//degrees += 1;

		camera_angle += 0.1f;
		light_1_angle += 0.2f;
		light_2_angle += 0.3f;
		sphere_1_angle += 0.15f;
		sphere_2_angle += 0.25f;
	}
	return 0;
}