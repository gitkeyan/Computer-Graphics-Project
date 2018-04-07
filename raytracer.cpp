/***********************************************************
	
	Starter code for Assignment 3

	Implementations of functions in raytracer.h, 
	and the main function which specifies the scene to be rendered.	

***********************************************************/


#include "raytracer.h"
#include <cmath>
#include <iostream>
#include <cstdlib>
#include <string>

#include <thread>         // std::thread
#include <mutex>          // std::mutex
std::mutex mtx;           // mutex for critical section

using namespace std;

void Raytracer::traverseScene(Scene& scene, Ray3D& ray)  {
	for (size_t i = 0; i < scene.size(); ++i) {
		SceneNode* node = scene[i];

		if (node->obj->intersect(ray, node->worldToModel, node->modelToWorld)) {
			ray.intersection.mat = node->mat;
		}
	}
}

void Raytracer::computeTransforms(Scene& scene) {
	// right now this method might seem redundant. But if you decide to implement 
	// scene graph this is where you would propagate transformations to child nodes
		
	for (size_t i = 0; i < scene.size(); ++i) {
		SceneNode* node = scene[i];

		node->modelToWorld = node->trans;
		node->worldToModel = node->invtrans; 
	}
}

void Raytracer::computeShading(Ray3D& ray, Scene& scene, LightList& light_list) {
	//reference: http://ray-tracing-conept.blogspot.ca/2015/01/hard-and-soft-shadows.html
	Color color(0.0, 0.0, 0.0);
	for (size_t  i = 0; i < light_list.size(); ++i) {
		LightSource* light = light_list[i];
		
		// Each lightSource provides its own shading function.
		// Implement shadows here if needed.		
		
		if(!(ray.hard_shadow_enabled || ray.soft_shadow_enabled)){
			// local illumination only, no shadow
			light->shade(ray);  
			color = color + ray.col;
		}else{		
			double epsilon = 0.001;
			
			// create ray from intersection to light
			Vector3D direction = light->get_position() - ray.intersection.point;
			Ray3D shadowRay = Ray3D(ray.intersection.point + (epsilon * direction), direction);
			
			// determine if shadow ray intersects
			traverseScene(scene, shadowRay); 
			if (shadowRay.intersection.none) {
				light->shade(ray);  // compute phong illumination
				color = color + ray.col;
			}else{
				// only include ambient term when not visible to light
				ray.part = 'A';
				ray.diffuse_enabled = 0;
				ray.specular_enabled = 0;
				
				light->shade(ray);   // compute shadow with only ambient term
				
				ray.part = 'B';
				ray.diffuse_enabled = 1;
				ray.specular_enabled = 1;
				
				color = color + ray.col;
			}
			
		}
		      
	}
	if(!ray.soft_shadow_enabled){
		ray.col = color;
	}else{
		ray.col = (1 / (1.0 * light_list.size())) * color;
	}
	
	
}

Color Raytracer::shadeRay(Ray3D& ray, Scene& scene, LightList& light_list, int depth) {
	Color col(0.0, 0.0, 0.0); 
	traverseScene(scene, ray); 

	// Don't bother shading if the ray didn't hit 
	// anything.
	if (!ray.intersection.none) {
		computeShading(ray, scene, light_list); 
		col = ray.col; 		
		
		// You'll want to call shadeRay recursively (with a different ray, 
		// of course) here to implement reflection/refraction effects.  
		
		if((depth > 0) && (ray.recursive_ray_enabled)){  // create a reflected ray at the point of intersection
			double epsilon = 0.01;
			Intersection intersection_obj = ray.intersection;
			Material m = *(intersection_obj.mat);

			// get unit normal and unit light direction
			Vector3D n = intersection_obj.normal;
			n.normalize();
			
			Vector3D l = -ray.dir;
			double magnitude = l.length();
			l.normalize();
			
			// get unit reflection direction and unit direction of view
			Vector3D r = ((2.0 * l.dot(n)) * n) - l;
			r.normalize();
			//r = magnitude * r;
			
			Ray3D reflectRay = Ray3D(ray.intersection.point + (epsilon * r), r);
			reflectRay.setRayStyle(ray.part, ray.ambient_enabled, ray.diffuse_enabled, ray.specular_enabled,
				ray.recursive_ray_enabled, ray.hard_shadow_enabled, ray.anti_aliasing_enabled,
				ray.soft_shadow_enabled);
			
			// compute global illumination 
			Color reflectCol = shadeRay(reflectRay, scene, light_list, --depth);
			
			// check if point of intersection is valid and set global illumination's contribution value
			double t = std::max(0.0, reflectRay.intersection.t_value);  
			double contribution = 0.0;
			if(t > 0.0){
				// set the reflection to contribute less the further away reflection ray's intersection is
				contribution = std::min(0.5, 1.0 / t);
			}
			
			col = col + contribution * m.specular * reflectCol;

		}
		col.clamp();
	}
	
	return col; 
}	

void Raytracer::render(Camera& camera, Scene& scene, LightList& light_list, Image& image, string entry) {
	
	//-----
	// determine all rendering styles to enable
	char part;   // 'A' or 'B'
	int ambient_enabled = 0;
	int diffuse_enabled = 0;
	int specular_enabled = 0;
	
	int recursive_ray_enabled = 0;
	int hard_shadow_enabled = 0;
	int soft_shadow_enabled = 0;
	int anti_aliasing_enabled = 0;
	int motion_blur_enabled = 0;
	int depth_of_field_enabled = 0;

	if(entry.find("A") != std::string::npos){
		part = 'A';
	
		if(entry.find("1") != std::string::npos){
			ambient_enabled = 1;
		}
		
		if(entry.find("2") != std::string::npos){
			diffuse_enabled = 1;
		}
		
		if(entry.find("3") != std::string::npos){
			specular_enabled = 1;
		}

	}else if(entry.find("B") != std::string::npos){
		part = 'B';
		
		ambient_enabled = 1;
		diffuse_enabled = 1;
		specular_enabled = 1;
		
		if(entry.find("1") != std::string::npos){
			recursive_ray_enabled = 1;
		}
		
		
		if(entry.find("5") != std::string::npos){  // extra point light sources added in main to simulate soft shadow effect
			soft_shadow_enabled = 1;
		}else if(entry.find("2") != std::string::npos){  
			hard_shadow_enabled = 1;    // hard shadow enabled, only a single point light source in main function
		}
		
		if(entry.find("4") != std::string::npos){
			anti_aliasing_enabled = 1;
		}
		
		if(entry.find("6") != std::string::npos){
			motion_blur_enabled = 0;
		}
		
		if(entry.find("8") != std::string::npos){
			depth_of_field_enabled = 1;
		}
	}
	
	//-----
	
	computeTransforms(scene);

	Matrix4x4 viewToWorld;
	double factor = (double(image.height)/2)/tan(camera.fov*M_PI/360.0);

	viewToWorld = camera.initInvViewMatrix();
	
	// Construct a ray for each pixel.
	#pragma omp parallel for
	for (int i = 0; i < image.height; i++) {
		#pragma omp parallel for
		for (int j = 0; j < image.width; j++) {
			// lines to use to see if multi-threading is enabled
			//std::cout << j;   
			//std::cout << "\n";
			Color col(0.0, 0.0, 0.0);
			
			if(!anti_aliasing_enabled){
				int k = !depth_of_field_enabled ? 1 : 30;    // sampling factor   4
				int depth = 2;
				
				// Sets up ray origin and direction in view space, 
				// image plane is at z = -1.
				Point3D origin(0, 0, 0);
				Point3D imagePlane;
				imagePlane[0] = (-double(image.width)/2 + 0.5 + j)/factor;
				imagePlane[1] = (-double(image.height)/2 + 0.5 + i)/factor;
				imagePlane[2] = -1;

				Ray3D ray;
				// TODO: Convert ray to world space  
				//-----
				Vector3D direction = viewToWorld * (imagePlane - origin); 
				origin = viewToWorld * origin;
				
				
				for(int m = 0; m < k; m++){
					ray = Ray3D(origin, direction);
				
					
					if(depth_of_field_enabled){   // offset ray position to induce field of depth effect
						double F = 5.0;
						Point3D focusPoint = ray.origin + (F * ray.dir);
						
						ray.origin[0] = ray.origin[0] + ((double)rand() / RAND_MAX);
						ray.origin[1] = ray.origin[1] + ((double)rand() / RAND_MAX);
						ray.origin[2] = ray.origin[2] + ((double)rand() / RAND_MAX);
						
						ray.dir = focusPoint - ray.origin;
					}
					
					
					ray.setRayStyle(part, ambient_enabled, diffuse_enabled, specular_enabled,
						recursive_ray_enabled, hard_shadow_enabled, anti_aliasing_enabled,
						soft_shadow_enabled);
					col = col + (1/(k * 1.0)) * shadeRay(ray, scene, light_list, depth);
				}
				
				
				
				//-----
				
				
				image.setColorAtPixel(i, j, col);

			}else{
				//-----
				// Raytracing with uniform supersampling to create anti-aliasing effect
				
				
				int k = !depth_of_field_enabled ? 4 : 16;    // sampling factor
				double contribution = 1/(k * k * 1.0);
				for(int m = 0; m < k; m++){
					for(int n = 0; n < k; n++){
						// Sets up ray origin and direction in view space, 
						// image plane is at z = -1.
						Point3D origin(0, 0, 0);
						Point3D imagePlane;
						
						imagePlane[0] = (-double(image.width)/2 + 0.5 + j + m / (k * 1.0))/factor;
						imagePlane[1] = (-double(image.height)/2 + 0.5 + i + n / (k * 1.0))/factor;
						imagePlane[2] = -1;
						
						Ray3D ray;
						// TODO: Convert ray to world space  
						//-----
						Vector3D direction = viewToWorld * (imagePlane - origin); 
						origin = viewToWorld * origin;
						ray = Ray3D(origin, direction);
						
						if(depth_of_field_enabled){   // offset ray position to induce field of depth effect
							double F = 5.0;
							Point3D focusPoint = ray.origin + (F * ray.dir);
							
							ray.origin[0] = ray.origin[0] + ((double)rand() / RAND_MAX);
							ray.origin[1] = ray.origin[1] + ((double)rand() / RAND_MAX);
							ray.origin[2] = ray.origin[2] + ((double)rand() / RAND_MAX);
							
							ray.dir = focusPoint - ray.origin;
						}
						
						
						ray.setRayStyle(part, ambient_enabled, diffuse_enabled, specular_enabled,
							recursive_ray_enabled, hard_shadow_enabled, anti_aliasing_enabled,
							soft_shadow_enabled);
						
						int depth = 2;
						//-----
						
						col = col + (contribution * shadeRay(ray, scene, light_list, depth)); 
						image.setColorAtPixel(i, j, col);
					}
				}
			}
						
		}
	}
}
