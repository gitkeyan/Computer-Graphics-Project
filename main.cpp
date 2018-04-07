/***********************************************************
	
	Starter code for Assignment 3

***********************************************************/

#include <cstdlib>
#include <iostream>
#include <string>
#include "raytracer.h"

#include <thread>         // std::thread
#include <mutex>          // std::mutex
std::mutex mtx0;           // mutex for critical section

using namespace std;

/*  
	render multiple images where 
	specified object is moving for 
	random amount specified to produce 
	the same effect of motion blur 

	height: height of the image rendered
	width: width of the image rendered
	maxTranslate: the max amount that x, y and z coordinate can be randomly translated
	minTranslate: the min amount that x, y and z coordinate can be randomly translated
	obj: scene object to be used to produce motion blur effect
	entry: all render styles to be applied

*/
void produce_motion_blur_img(Camera camera1, Camera camera2, Raytracer raytracer,
							LightList light_list, Scene scene, int height, int width, double maxTranslate, double minTranslate,
							SceneNode * obj, string entry){
	
	/* color channel for the first image */
	double rBuf[height * width];
	double gBuf[height * width];
	double bBuf[height * width];

	/* color channel for the second image */	
	double rBuf2[height * width];
	double gBuf2[height * width];
	double bBuf2[height * width];

	for(int i = 0; i < (height * width); i++){//initialize all color channels to 0
		rBuf[i] = 0.0;
		gBuf[i] = 0.0;
		bBuf[i] = 0.0;
		
		rBuf2[i] = 0.0;
		gBuf2[i] = 0.0;
		bBuf2[i] = 0.0;
	}
	
	Image image3(width, height);
	Image image4(width, height);
	int motion_count = 20; // the amount of times the object will move and the image to be rendered to approximate motion blur
	

	double xDisplacement = 0.0; //to translate the object back to the position before motion blur is called
	double yDisplacement = 0.0;
	double zDisplacement = 0.0;
	
	int movement_count = 0; // the number of times the object moved
	
	for(int t = 0; t < motion_count; t++){
		mtx0.lock();
		
		// to determine the amount to translate
		double randX = (rand() / RAND_MAX) * (maxTranslate - minTranslate) + minTranslate;
		double randY = (rand() / RAND_MAX) * (maxTranslate - minTranslate) + minTranslate;
		double randZ = 0.1 * ((rand() / RAND_MAX) * (maxTranslate - minTranslate) + minTranslate);
		
		xDisplacement += randX;
		yDisplacement += randY;
		zDisplacement += randZ;
		
		obj->translate(Vector3D(randX, randY, randZ));
		
		movement_count++;
		
		// Render the images
		Image imageFrame(width, height);
		Image imageFrame2(width, height);
		raytracer.render(camera1, scene, light_list, imageFrame, entry);
		raytracer.render(camera2, scene, light_list, imageFrame2, entry);
		
		
		
		/* to sum up the color value of the images produced for imageFrame and imageFrame2 */
		#pragma omp parallel for
		for(int i = 0; i < height; i++){
			#pragma omp parallel for
			for(int j = 0; j < width; j++){
				rBuf[i*width+j] += imageFrame.rbuffer[i*width+j];
				gBuf[i*width+j] += imageFrame.gbuffer[i*width+j];
				bBuf[i*width+j] += imageFrame.bbuffer[i*width+j];
				
				rBuf2[i*width+j] += imageFrame2.rbuffer[i*width+j];
				gBuf2[i*width+j] += imageFrame2.gbuffer[i*width+j];
				bBuf2[i*width+j] += imageFrame2.bbuffer[i*width+j];
			}
		}

		mtx0.unlock();
		
	}
	
	/* to take average of the sum */
	#pragma omp parallel for
	for(int i = 0; i < height; i++){
		#pragma omp parallel for
		for(int j = 0; j < width; j++){
			image3.rbuffer[i*width+j] = int(rBuf[i*width+j]/(movement_count * 1.0));
			image3.gbuffer[i*width+j] = int(gBuf[i*width+j]/(movement_count * 1.0));
			image3.bbuffer[i*width+j] = int(bBuf[i*width+j]/(movement_count * 1.0));
			
			image4.rbuffer[i*width+j] = int(rBuf2[i*width+j]/(movement_count * 1.0));
			image4.gbuffer[i*width+j] = int(gBuf2[i*width+j]/(movement_count * 1.0));
			image4.bbuffer[i*width+j] = int(bBuf2[i*width+j]/(movement_count * 1.0));
		}
	}

	obj->translate(Vector3D(-xDisplacement, -yDisplacement, -zDisplacement)); // to untranslate the object movement
	image3.flushPixelBuffer("view1.bmp");
	image4.flushPixelBuffer("view2.bmp");
	
	
}


int main(int argc, char* argv[])
{
	//-----
	// Ask the user for the shading style to apply
	
	string entry = "";
	int maxChoice = 0;   // part A can apply up to 3 styles, part B have 8 
	int count = 0;
	while(true){
		std::cout << "Currently availiable styles:\n";
		std::cout << "(A1)   Ambient only\n";
		std::cout << "(A12)  Ambient + Diffuse\n";
		std::cout << "(A123) Phong Illumination (Ambient + Diffuse + Specular)\n";
		std::cout << "(B1)   Recursive ray tracing\n";
		std::cout << "(B2)   Hard shadows\n";
		std::cout << "(B3)   Add a cylinder to the scene\n";
		std::cout << "(B4)   Anti-Aliasing\n";
		std::cout << "(B5)   Soft-shadow\n";
		std::cout << "(B6)   Motion-Blur\n";
		std::cout << "(B7)   Texture mapping\n";
		std::cout << "(B8)   Depth of field\n";
		std::cout << "(C)    Bonus\n";
		std::cout << "Please enter a string listed above in the bracket to apply the rendering styles:\n";
		std::cin >> entry;
		
		maxChoice = 0;
		if(entry.find("A") != std::string::npos){
			maxChoice = 3;
		}else if(entry.find("B") != std::string::npos){
			maxChoice = 8;
		}
		
		for(int i = 1; i < maxChoice + 1; i++){
			if(entry.find(std::to_string(i)) != std::string::npos){
				count++;
			}
		}
		
		if(entry.find("C") != std::string::npos){
			count++;
		}

		if(count > 0){
			break;
		}
	}
	
	//-----
	
	
	// Build your scene and setup your camera here, by calling 
	// functions from Raytracer.  The code here sets up an example
	// scene and renders it from two different view points, DO NOT
	// change this if you're just implementing part one of the 
	// assignment.  
	Raytracer raytracer;
	LightList light_list;
	Scene scene;   

	int width = 320;
	int height = 240;

	if (argc == 3) {
		width = atoi(argv[1]);
		height = atoi(argv[2]);
	}
	
// -------------- bonus --------------
	/* 
		the following code produce a scene consisting of the following things
		blue ping pong table 
		texture mapped net for the ping pong table
		two ping pong bats
		a motion blured ping pong ball
		a texture mapped scoreboard
	 */
	if(entry.find("C") != std::string::npos){
		entry.append("B457");//we used anti-aliasing(B4), soft-shadow(B5), and texture mapping (B7)

		Material blue(Color(0.0, 0.0, 0.75), Color(0.0,0.4,0.1),    // table
			Color(0.628281, 0.555802, 0.366065),
			12);

		Material netColor(Color(0.3, 0.3, 0.3), Color(0.1,0.8,0.5),    // net
			Color(0.628281, 0.555802, 0.366065),
			51.2);

		Material middleLineColor(Color(1, 1, 1), Color(0.8,0.7,0.3),    // middle white line on the table
			Color(0.628281, 0.555802, 0.366065),
			51.2);

		Material scoreboardColor(Color(0.3, 0.3, 0.3), Color(0.75164,0.60648,0.22648),    // scoreboard
			Color(0.628281, 0.555802, 0.366065),
			51.2);

		Material red(Color(0.0, 0.0, 0.75), Color(0.8,-0.5,-0.5),    // ping pong pad
			Color(0.628281, 0.555802, 0.366065),
			12);

		Material padHandleColor(Color(0.0, 0.0, 0.75), Color(0.8,0.1,0.3),    // ping pong pad handle
			Color(0.628281, 0.555802, 0.366065),
			12);		

		Material ballColor(Color(0.5, 0.5, 0.5), Color(0.75164,0.60648,0.22648),     // ball
		Color(0.628281, 0.555802, 0.366065),
		51.2);

		/* add the objects described for the bonus into the scene */
		SceneNode* table = new SceneNode(new UnitCube(), &blue);
		scene.push_back(table);

		SceneNode* middleLine = new SceneNode(new UnitCube(), &middleLineColor);
		scene.push_back(middleLine);

		SceneNode* line1 = new SceneNode(new UnitCube(), &middleLineColor);
		scene.push_back(line1);

		SceneNode* line2 = new SceneNode(new UnitCube(), &middleLineColor);
		scene.push_back(line2);

		SceneNode* line3 = new SceneNode(new UnitCube(), &middleLineColor);
		scene.push_back(line3);

		SceneNode* line4 = new SceneNode(new UnitCube(), &middleLineColor);
		scene.push_back(line4);

		SceneNode* net = new SceneNode(new UnitCube(), &netColor);
		scene.push_back(net);

		SceneNode* scoreboard = new SceneNode(new UnitCube(), &scoreboardColor);
		scene.push_back(scoreboard);
		scoreboardColor.setTexture("scoreboard.bmp", 676, 407);

		SceneNode* pad1 = new SceneNode(new UnitCylinder(), &red);
		scene.push_back(pad1);

		SceneNode* padHandle1 = new SceneNode(new UnitCube(), &padHandleColor);
		scene.push_back(padHandle1);
		padHandleColor.setTexture("pingpongHandle.bmp", 222, 812);

		SceneNode* pad2 = new SceneNode(new UnitCylinder(), &red);
		scene.push_back(pad2);

		SceneNode* padHandle2 = new SceneNode(new UnitCube(), &padHandleColor);
		scene.push_back(padHandle2);
		padHandleColor.setTexture("pingpongHandle.bmp", 222, 812);


		SceneNode* ball = new SceneNode(new UnitSphere(), &ballColor);
		scene.push_back(ball);

		/* apply transformation to desplay the scene properly */
		//for table
		double factorTable[3] = { 6.2, 3.6, 0.3 };
		table->translate(Vector3D(0, 0, -4));

		//for middle white line on the table
		double factorMiddleLine[3] = { 6.15, 0.05, 0.3 };
		middleLine->translate(Vector3D(0, 0.01, -4));

		//for 4 border lines
		double factorLine1[3] = { 6.15, 0.05, 0.3 };
		line1->translate(Vector3D(0, 0.01, -4));

		double factorLine2[3] = { 6.15, 0.05, 0.3 };
		line2->translate(Vector3D(0, 0.01, -4));

		double factorLine3[3] = { 0.05, 3.55, 0.3 };
		line3->translate(Vector3D(0, 0.01, -4));
		
		double factorLine4[3] = { 0.05, 3.55, 0.3 };
		line4->translate(Vector3D(0, 0.01, -4));
		
		//for net
		double factorNet[3] = { 0.05, 3.6, 0.6 };
		net->translate(Vector3D(0, 0.2, -3.7));

		//for scoreboard
		double factorScoreboard[3] = { 2.3, 0.015, 1.2 };
		scoreboard->translate(Vector3D(0, 0.01, -4));

		//for ping pong bats
		double factorPad1[3] = { 0.28,0.33,0.12 };
		pad1->translate(Vector3D(0, 0, -3));		

		double factorPadHandle1[3] = { 0.1,0.5,.09 };
		padHandle1->translate(Vector3D(0, 0, -3));	

		double factorPad2[3] = { 0.28,0.33,0.12 };
		pad2->translate(Vector3D(0, 0, -3));		

		double factorPadHandle2[3] = { 0.1,0.5,0.09 };
		padHandle2->translate(Vector3D(0, 0, -3.0));	

		//for ball
		double factorBall[3] = { 0.08, 0.08, 0.08};
		ball->translate(Vector3D(-0.5, 0.1, -3));

		// apply rotation
		for(int i = 0; i < scene.size(); i++){
			scene[i]->rotate('z', 40);
			scene[i]->rotate('y', 17);
			scene[i]->rotate('x', -25);
		}

		/* apply scaling, transtation, or rotation if needed */
		table->scale(Point3D(0, 0, 0), factorTable);

		middleLine->scale(Point3D(0, 0, 0), factorMiddleLine);

		line1->translate(Vector3D(0, 1.75, 0));
		line1->scale(Point3D(0, 0, 0), factorLine1);

		line2->translate(Vector3D(0, -1.75, 0));
		line2->scale(Point3D(0, 0, 0), factorLine2);

		line3->translate(Vector3D(3.05, 0, 0));
		line3->scale(Point3D(0, 0, 0), factorLine3);	

		line4->translate(Vector3D(-3.05, 0, 0));
		line4->scale(Point3D(0, 0, 0), factorLine4);

		net->scale(Point3D(0, 0, 0), factorNet);

		scoreboard->translate(Vector3D(0, 2.6, 0.5));
		scoreboard->scale(Point3D(0, 0, 0), factorScoreboard);

		pad1->translate(Vector3D(-2.2,-0.7, 0));
		pad1->rotate('y', -10);
		pad1->scale(Point3D(0, 0, 0), factorPad1);


		padHandle1->translate(Vector3D(-2.2, -0.7, 0));
		padHandle1->translate(Vector3D(0, -0.3, 0));
		padHandle1->rotate('y', -10);
		padHandle1->scale(Point3D(0, 0, 0), factorPadHandle1);

		pad2->translate(Vector3D(2,0.5, 0));
		pad2->translate(Vector3D(0,1, 0.11));
		pad2->rotate('y', 30);
		pad2->rotate('x', -20);
		pad2->scale(Point3D(0, 0, 0), factorPad2);


		padHandle2->translate(Vector3D(2, 0.5, 0));
		padHandle2->translate(Vector3D(0, 0.3, 0));
		padHandle2->translate(Vector3D(0, 1, 0));
		padHandle2->rotate('y', 30);
		padHandle2->rotate('x', -20);
		padHandle2->scale(Point3D(0, 0, 0), factorPadHandle2);

		ball->scale(Point3D(0,0,0), factorBall);

		//light
		PointLight* pLight = new PointLight(Point3D(-5,10,4), Color(0.75,0.75,0.75));
		light_list.push_back(pLight);


		Camera camera1(Point3D(0, -0.5, 3), Vector3D(0, 0, -1), Vector3D(0, 1, 0), 50.0);
		Camera camera2(Point3D(3, 0, 1), Vector3D(-4, 0, -8), Vector3D(-1, 12, 25), 60.0);

		/* //for debugging purpose, this portion of code exclude motion blur
		Image image1(width, height);
		raytracer.render(camera1, scene, light_list, image1, entry); //render 3D scene to image
		image1.flushPixelBuffer("view1.bmp"); //save rendered image to file

		// Render it from a different point of view.
		Image image2(width, height);
		raytracer.render(camera2, scene, light_list, image2, entry);
		image2.flushPixelBuffer("view2.bmp");*/

		double maxTranslate = 0.075;
		double minTranslate = -0.075;

		// apply motion blur for the ping pong ball
		produce_motion_blur_img(camera1, camera2, raytracer, light_list, scene, height, width, maxTranslate, minTranslate, ball, entry);
	
		// Free memory
		for (size_t i = 0; i < scene.size(); ++i) {
			delete scene[i];
		}

		for (size_t i = 0; i < light_list.size(); ++i) {
			delete light_list[i];
		}
	
		return 0;
	}

// ----------------------------


	// Define materials for shading.
	Material gold(Color(0.3, 0.3, 0.3), Color(0.75164,0.60648,0.22648),     // Ellipsoid
		Color(0.628281, 0.555802, 0.366065),
		51.2);
	Material gold2(Color(0.3, 0.3, 0.3), Color(0.75164,0.60648,0.22648),    // Sphere
		Color(0.628281, 0.555802, 0.366065),
		51.2);
	Material gold3(Color(0.3, 0.3, 0.3), Color(0.75164,0.60648,0.22648),    // Cylinder
		Color(0.628281, 0.555802, 0.366065),
		51.2);
	Material jade(Color(0, 0, 0), Color(0.54,0.89,0.63),                    // Plane
		Color(0.316228,0.316228,0.316228),
		12.8);
		
	Material gold4(Color(0.3, 0.3, 0.3), Color(0.75164,0.60648,0.22648),    // Cube
		Color(0.628281, 0.555802, 0.366065),
		51.2);

	
	
	//-----
	if(entry.find(std::to_string(7)) != std::string::npos){ // set texture mapping for cube, plane and sphere if requested
		jade.setTexture("./textureUsed/beach.bmp", 2880, 1748);
		gold.setTexture("./textureUsed/earthmap.bmp", 1000, 500);
		gold2.setTexture("./textureUsed/earthmap.bmp", 1000, 500);
		gold4.setTexture("./textureUsed/star.bmp", 500, 500);
	}
		
	
	// enable soft shadow if requested
	int soft_shadow_enabled = 0;
	if(entry.find("B") != std::string::npos){
		if(entry.find("5") != std::string::npos){
			soft_shadow_enabled = 1;
		}	
	}
	
	if(!soft_shadow_enabled){
		// Defines a point light source.
		PointLight* pLight = new PointLight(Point3D(0,0,5), Color(0.9,0.9,0.9));
		light_list.push_back(pLight);
	}else{		
		double gap = 0.3;
		int xRange = 5;
		int yRange = 5;
		// approximate area light by uniformly distributing light sources
		for(int n = -xRange; n <= xRange; n++){
			for(int m = -yRange; m <= yRange; m++){
				// Defines a point light source.
				PointLight* pLight = new PointLight(Point3D(gap * n, gap * m,5), Color(0.9,0.9,0.9));
				light_list.push_back(pLight);
			}
		}				
	}
	//-----
	
	// Add a unit square into the scene with material mat.
	SceneNode* sphere = new SceneNode(new UnitSphere(), &gold);
	if(entry.find("7") == std::string::npos){
		scene.push_back(sphere);
	}
	SceneNode* plane = new SceneNode(new UnitSquare(), &jade);
	scene.push_back(plane);

	// Apply some transformations to the sphere and unit square.
	double factor1[3] = { 1.0, 2.0, 1.0 };
	sphere->translate(Vector3D(0, 0, -5));
	sphere->rotate('x', -45);
	sphere->rotate('z', 45);
	sphere->scale(Point3D(0, 0, 0), factor1);

	double factor2[3] = { 6.0, 6.0, 6.0 };
	plane->translate(Vector3D(0, 0, -7));
	plane->rotate('z', 45);
	plane->scale(Point3D(0, 0, 0), factor2);
	
	
	//----- 
	//Add unit cylinder into the scene if it is requested by the user 
	if(entry.find("B") != std::string::npos){
		if(entry.find("3") != std::string::npos){
			
			SceneNode* cylinder = new SceneNode(new UnitCylinder(), &gold3);
			scene.push_back(cylinder);		
			double factor3[3] = { 0.8, 0.9, 0.9 };
			cylinder->translate(Vector3D(-2, 2, -4.5));	
			cylinder->scale(Point3D(0, 0, 0), factor3);

		}
	}
	
	//-----
	
	// the undeformed unit sphere
	SceneNode* sphere2 = new SceneNode(new UnitSphere(), &gold2);
	if(entry.find("B") != std::string::npos){
		scene.push_back(sphere2);
	}

	double factor4[3] = { 1.0, 1.0, 1.0 };
	sphere2->translate(Vector3D(2, 2.5, -5));
	sphere2->scale(Point3D(0, 0, 0), factor4);	
	
	//----- 
	//Add unit cube into the scene if the user request texture mapping
	if(entry.find("B") != std::string::npos){
		if(entry.find("7") != std::string::npos){
			SceneNode* cube = new SceneNode(new UnitCube(), &gold4);
			scene.push_back(cube);
			double factor5[3] = { 2.0, 2.0, 2.0 };

			cube->translate(Vector3D(0, 0, -5));	
			cube->rotate('y', -30);
			cube->rotate('x', 20);
			cube->scale(Point3D(0, 0, 0), factor5);
	
		}
	}
	
	
	// Render the scene, feel free to make the image smaller for
	// testing purposes.
	Camera camera1(Point3D(0, 0, 1), Vector3D(0, 0, -1), Vector3D(0, 1, 0), 60.0);
	Camera camera2(Point3D(4, 2, 1), Vector3D(-4, -2, -6), Vector3D(0, 1, 0), 60.0);
	
	if(entry.find("6") == std::string::npos){
		
		Image image1(width, height);
		raytracer.render(camera1, scene, light_list, image1, entry); //render 3D scene to image
		image1.flushPixelBuffer("view1.bmp"); //save rendered image to file

		// Render it from a different point of view.
		Image image2(width, height);
		raytracer.render(camera2, scene, light_list, image2, entry);
		image2.flushPixelBuffer("view2.bmp");
	}
		
	
	/* motion_blur_enabled */
	if(entry.find("6") != std::string::npos){  		
		double maxTranslate = 0.025;
		double minTranslate = -0.025;
		if((entry.find("B") != std::string::npos) && (entry.find("7") != std::string::npos)){
			// produce images for the sphere texture mapped with world map when texture mapping is enabled
			produce_motion_blur_img(camera1, camera2, raytracer, light_list, scene, height, width, maxTranslate, minTranslate, sphere2, entry);
		}else{
			// produce images with motion blur enabled for the ellipsoid when texture mapping is enabled
			produce_motion_blur_img(camera1, camera2, raytracer, light_list, scene, height, width, maxTranslate, minTranslate, sphere, entry);

		}
		
	}
		

	// Free memory
	for (size_t i = 0; i < scene.size(); ++i) {
		delete scene[i];
	}

	for (size_t i = 0; i < light_list.size(); ++i) {
		delete light_list[i];
	}
	
	return 0;
}