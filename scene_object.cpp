/***********************************************************
	
	Starter code for Assignment 3
	
	Implements scene_object.h

***********************************************************/

#include <cmath>
#include "scene_object.h"

bool UnitSquare::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld) {
	// TODO: implement intersection code for UnitSquare, which is
	// defined on the xy-plane, with vertices (0.5, 0.5, 0), 
	// (-0.5, 0.5, 0), (-0.5, -0.5, 0), (0.5, -0.5, 0), and normal
	// (0, 0, 1).
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.

	//-----
	bool intersection_found = false;
	
	// Transform the ray (origin, direction) to object space
	ray.origin = worldToModel * ray.origin;
	ray.dir = worldToModel * ray.dir;

	double t = -ray.origin[2] / ray.dir[2];
	bool invalid_intersection = t < 0.0 || ray.dir[2] == 0.0;

	if(!invalid_intersection){
		Point3D p = ray.origin + t * ray.dir;
		Vector3D normal(0.0,0.0,1.0);
		
		// Check if point of intersection is in boundary
		if(p[0] >= -0.5 && p[0] <= 0.5 && p[1] >= -0.5 && p[1] <= 0.5){
			// use t value to determine if the unit square is currently the closest object
			// to the ray been shot
			if(ray.intersection.none || t < ray.intersection.t_value) {
				ray.intersection.t_value = t;
				ray.intersection.point = modelToWorld * p;
				ray.intersection.normal = transNorm(worldToModel, normal);
				ray.intersection.normal.normalize();   // normalize normal of intersection point
				ray.intersection.none = false;
				intersection_found = true;
			}
		}
	}
	
	// Transform the ray (origin, direction) back to world space
	ray.origin = modelToWorld * ray.origin;
	ray.dir = modelToWorld * ray.dir;
	
	return intersection_found;
	//-----
}

bool UnitSphere::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld) {
	// TODO: implement intersection code for UnitSphere, which is centred 
	// on the origin.  
	//
	// Your goal here is to fill ray.intersection with correct values
	// should an intersection occur.  This includes intersection.point, 
	// intersection.normal, intersection.none, intersection.t_value.   
	//
	// HINT: Remember to first transform the ray into object space  
	// to simplify the intersection test.

	//-----
	bool intersection_found = false;
	bool invalid_intersection = false;
	
	// Transform the ray (origin, direction) to object space
	ray.origin = worldToModel * ray.origin;
	ray.dir = worldToModel * ray.dir;

	Point3D c(0.0, 0.0, 0.0);  // center of of the unit sphere 
	Vector3D e_minus_c = ray.origin - c;
	
	// Compute for the coeffients of the quadratic equation to solve for t_value
	double A = ray.dir.dot(ray.dir);
	double B = 2.0 * ray.dir.dot(e_minus_c);
	double C = e_minus_c.dot(e_minus_c) - 1.0;
	
	double D = B * B - 4.0 * A * C;
	double t = -1.0;
	double t1 = -1.0;
	double t2 = -1.0;
	if(D < 0.0){   // no solution
		invalid_intersection = true;
	}else{
		t1 = (-B - sqrt(D)) / (2.0 * A);
		t2 = (-B + sqrt(D)) / (2.0 * A);
		
		if((t1 >= 0.0) && (t2 >= 0.0)){
			t = std::min(t1, t2);
		}else if(t1 >= 0.0){
			t = t1;
		}else if(t2 >= 0.0){
			t = t2;
		}else{
			invalid_intersection = true;
		}
	}
	

	if(!invalid_intersection){
		Point3D p = ray.origin + t * ray.dir;
		Vector3D normal(p[0], p[1], p[2]);
		normal.normalize();
		
		// use t value to determine if the unit square is currently the closest object
		// to the ray been shot
		if(ray.intersection.none || t < ray.intersection.t_value) {
			ray.intersection.t_value = t;
			ray.intersection.point = modelToWorld * p;
			ray.intersection.normal = transNorm(worldToModel, normal);
			ray.intersection.normal.normalize();   // normalize normal of intersection point
			ray.intersection.none = false;
			intersection_found = true;
		}

	}
	
	// Transform the ray (origin, direction) back to world space
	ray.origin = modelToWorld * ray.origin;
	ray.dir = modelToWorld * ray.dir;
	
	return intersection_found;
	//-----
}


bool UnitCylinder::intersect(Ray3D& ray, const Matrix4x4& worldToModel,
		const Matrix4x4& modelToWorld) {
	
	// source: https://www.cl.cam.ac.uk/teaching/1999/AGraphHCI/SMAG/node2.html#SECTION00024200000000000000
	// source: https://www.csie.ntu.edu.tw/~cyy/courses/rendering/pbrt-2.00/html/cylinder_8cpp_source.html
	
	// Determine if a valid intersection exist for a unit cylinder with (height = 1, radius = 1),
	// center point of top disk located at (0.0, 0.0, 0.5)
	// center point of bottom disk located at (0.0, 0.0, -0.5)
	
	
	/*  Intersection on none disk region:
	// Cylinder Equation:
	// x^2 + y^2 = 1, where -0.5 <= z <= 0.5
	
	ray: p = O + t * D
	
	    (Ox + t * Dx)^2 + (Oy + t * Dy)^2 = 1
	<=> (Dx^2 + Dy^2) * t^2 + (2 * Ox * Dx + 2 * Oy * Dy) * t + (Ox^2 + Oy^2 - 1) = 0
	<=> A * t^2 + B * t + C = 0
	Solve quadratic equation for t to find intersection on the none disk region
	If Z_t is not in the range of [-0.5, 0.5], there is no intersection on the none disk region 
	
	Normal of none disk region:
	df/dx = 2x,    df/dy = 2y,   df/dz = 0
	*/
	
	/* Intersection on disk region:
	// Disk Equation:
	// x^2 + y^2 <= 1, where z = -0.5 for bottom disk, and z = 0.5 for top disk
	//     z = O_z + t * D_z 
	// <=> t = (z - O_z)/D_z
	
	Normal of disk region:
	df/dx = 0,    df/dy = 0,   df/dz = 1   for top disk 
	df/dx = 0,    df/dy = 0,   df/dz = -1   for bottom disk 
	*/
			
	//-----
	int face = 0;
	bool intersection_found = false;
	bool intersect_nonDisk = true;
	bool intersect_topDisk = true;
	bool intersect_bottomDisk = true;
	
	// Transform the ray (origin, direction) to object space
	ray.origin = worldToModel * ray.origin;
	ray.dir = worldToModel * ray.dir;
	
	
	// ----- Compute for the closest point of intersection on the cylinder body -----
	double A = ray.dir[0] * ray.dir[0] + ray.dir[1] * ray.dir[1];
	double B = 2 * ray.origin[0] * ray.dir[0] + 2 * ray.origin[1] * ray.dir[1];
	double C = ray.origin[0] * ray.origin[0] + ray.origin[1] * ray.origin[1] - 1.0;
	
	double D = B * B - 4.0 * A * C;
	double t = -1.0;
	double t1 = -1.0;
	double t2 = -1.0;
	bool t_set = false;
	Vector3D normal;
	Point3D p;
	
	if(D < 0.0){   
		// no solution found on the infinite cylinder
		intersect_nonDisk = false;
	}else{
		t1 = (-B - sqrt(D)) / (2.0 * A);
		t2 = (-B + sqrt(D)) / (2.0 * A);
		
		if((t1 >= 0.0) && (t2 >= 0.0)){
			t = std::min(t1, t2);
		}else if(t1 >= 0.0){
			t = t1;
			
		}else if(t2 >= 0.0){
			t = t2;
		}else{
			intersect_nonDisk = false;
		}
		
		if(t >= 0.0){
			p = ray.origin + (t * ray.dir);
			
			if((-0.5 <= p[2]) && (p[2] <= 0.5) ){  // check if intersection point have Z in [-0.5, 0.5]
				normal[0] = p[0];
				normal[1] = p[1];
				normal[2] = 0.0;
				face = 0;
				
				t_set = true;
			}else{
				intersect_nonDisk = false;
			}
		}
		
	}
	
	// ----- Compute for the closest point of intersection on the top disk
	if(ray.dir[2] != 0.0){ // check if z-direction is 0 or not
		t1 = (0.5 - ray.origin[2])/ray.dir[2];
	}else{
		t1 = -1;
	}
	
	if(t1 >= 0.0){
		p = ray.origin + t1 * ray.dir;
		if(p[0] * p[0] + p[1] * p[1] <= 1){  // check to see if ray intersects disk
			if ((!t_set) || (t1 < t)){  // check to see if this disk is intersected first
				t = t1;
				normal[0] = 0.0;
				normal[1] = 0.0;
				normal[2] = 1.0;
				//normal.normalize();
				face = 1;
				
				t_set = true;
			}
			
		}else{
			intersect_topDisk = false;
		}
		
	}else{
		intersect_topDisk = false;
	}
	
	// ----- Compute for the closest point of intersection on the bottom disk
	if(ray.dir[2] != 0.0){  // check if z-direction is 0 or not
		t1 = (-0.5 - ray.origin[2])/ray.dir[2];
	}else{
		t1 = -1;
	}
	
	if(t1 >= 0.0){
		p = ray.origin + t1 * ray.dir;
		if(p[0] * p[0] + p[1] * p[1] <= 1){  // check to see if ray intersects disk
			if ((!t_set) || (t1 < t)){   // check to see if this disk is intersected first
				t = t1;
				normal[0] = 0.0;
				normal[1] = 0.0;
				normal[2] = -1.0;
				//normal.normalize();
				face = 2;
				
				t_set = true;
			}
			
		}else{
			intersect_bottomDisk = false;
		}
		
	}else{
		intersect_bottomDisk = false;
	}
	
	
	if(intersect_nonDisk || (intersect_topDisk || intersect_bottomDisk)){
		p = ray.origin + t * ray.dir;
		
		// use t value to determine if the unit square is currently the closest object
		// to the ray been shot
		if(ray.intersection.none || t < ray.intersection.t_value) {
			ray.intersection.t_value = t;
			ray.intersection.point = modelToWorld * p;
			ray.intersection.normal = transNorm(worldToModel, normal);
			ray.intersection.normal.normalize();   // normalize normal of intersection point
			ray.intersection.none = false;
			intersection_found = true;
		}

	}
	
	
	// Transform the ray (origin, direction) back to world space
	ray.origin = modelToWorld * ray.origin;
	ray.dir = modelToWorld * ray.dir;
	//-----
	
	return intersection_found;
}			

void SceneNode::rotate(char axis, double angle) {
	Matrix4x4 rotation;
	double toRadian = 2*M_PI/360.0;
	int i;
	
	for (i = 0; i < 2; i++) {
		switch(axis) {
			case 'x':
				rotation[0][0] = 1;
				rotation[1][1] = cos(angle*toRadian);
				rotation[1][2] = -sin(angle*toRadian);
				rotation[2][1] = sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'y':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][2] = sin(angle*toRadian);
				rotation[1][1] = 1;
				rotation[2][0] = -sin(angle*toRadian);
				rotation[2][2] = cos(angle*toRadian);
				rotation[3][3] = 1;
			break;
			case 'z':
				rotation[0][0] = cos(angle*toRadian);
				rotation[0][1] = -sin(angle*toRadian);
				rotation[1][0] = sin(angle*toRadian);
				rotation[1][1] = cos(angle*toRadian);
				rotation[2][2] = 1;
				rotation[3][3] = 1;
			break;
		}
		if (i == 0) {
			this->trans = this->trans*rotation; 	
			angle = -angle;
		} 
		else {
			this->invtrans = rotation*this->invtrans; 
		}	
	}
}

void SceneNode::translate(Vector3D trans) {
	Matrix4x4 translation;
	
	translation[0][3] = trans[0];
	translation[1][3] = trans[1];
	translation[2][3] = trans[2];
	this->trans = this->trans*translation; 	
	translation[0][3] = -trans[0];
	translation[1][3] = -trans[1];
	translation[2][3] = -trans[2];
	this->invtrans = translation*this->invtrans; 
}

void SceneNode::scale(Point3D origin, double factor[3] ) {
	Matrix4x4 scale;
	
	scale[0][0] = factor[0];
	scale[0][3] = origin[0] - factor[0] * origin[0];
	scale[1][1] = factor[1];
	scale[1][3] = origin[1] - factor[1] * origin[1];
	scale[2][2] = factor[2];
	scale[2][3] = origin[2] - factor[2] * origin[2];
	this->trans = this->trans*scale; 	
	scale[0][0] = 1/factor[0];
	scale[0][3] = origin[0] - 1/factor[0] * origin[0];
	scale[1][1] = 1/factor[1];
	scale[1][3] = origin[1] - 1/factor[1] * origin[1];
	scale[2][2] = 1/factor[2];
	scale[2][3] = origin[2] - 1/factor[2] * origin[2];
	this->invtrans = scale*this->invtrans; 
}


