/***********************************************************
	
	Starter code for Assignment 3

	Implements light_source.h

***********************************************************/

#include <cmath>
#include "light_source.h"

void PointLight::shade(Ray3D& ray) {
	// TODO: implement this function to fill in values for ray.col 
	// using phong shading.  Make sure your vectors are normalized, and
	// clamp colour values to 1.0.
	//
	// It is assumed at this point that the intersection information in ray 
	// is available.  So be sure that traverseScene() is called on the ray 
	// before this function.  

	//-----
	/*
	User will be asked for the type of shading style he/she wants to apply to the image.
	Each ray will be given an integer to indicate the type of rendering style to apply.
	Currently availiable styles:
	(A1) Ambient only
	(A12) Ambient + Diffuse
	(A123) Phong Illumination (Ambient + Diffuse + Specular)	
	(B1)   Recusive ray tracing
	(B2)   Hard shadows
	(B3)   Add a cylinder to the scene
	*/
	
	//-----
	// Look up ray's material to find ambient, diffuse, and specular coefficients 
	Intersection intersection_obj = ray.intersection;
	Material m = *(intersection_obj.mat);
	
	// ambient term
	Color ambientTerm = m.ambient * col_ambient;

	// get unit normal and unit light direction
	Vector3D n = intersection_obj.normal;
	n.normalize();
	
	Vector3D l = pos - intersection_obj.point;
	l.normalize();
	
	// diffuse term
	Color diffuseTerm = m.diffuse * (std::max(0.0, n.dot(l)) * col_diffuse);
	
	// get unit reflection direction and unit direction of view
	Vector3D r = ((2.0 * l.dot(n)) * n) - l;
	r.normalize();
	
	Vector3D v = -ray.dir;
	v.normalize();
	
	// specular term
	double v_r_alpha = std::pow(std::max(0.0, v.dot(r)), m.specular_exp);
	Color specularTerm = m.specular * (v_r_alpha * col_specular);
	
	//double v_r_alpha = std::pow(v.dot(r), m.specular_exp);
	//Color specularTerm = m.specular * (std::max(0.0, v_r_alpha) * col_specular);
	
	ray.col = ray.ambient_enabled * ambientTerm + ray.diffuse_enabled * diffuseTerm + ray.specular_enabled * specularTerm;
	ray.col.clamp();

	//-----
}
