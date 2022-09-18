/*
 * Program 3 base code - includes modifications to shape and initGeom in preparation to load
 * multi shape objects 
 * CPE 471 Cal Poly Z. Wood + S. Scueda + I. Dunn
 */

#include <iostream>
#include <glad/glad.h>

#include "GLSL.h"
#include "Program.h"
#include "Shape.h"
#include "MatrixStack.h"
#include "WindowManager.h"
#include "Texture.h"
#include "stb_image.h"
#include "Bezier.h"
#include "Spline.h"
#include "particleSys.h"


#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader/tiny_obj_loader.h>

// value_ptr for glm
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

using namespace std;
using namespace glm;

class Application : public EventCallbacks
{

public:
	WindowManager * windowManager = nullptr;

	// Our shader program - use this one for Blinn-Phong
	std::shared_ptr<Program> prog;

	//Our shader program for textures
	std::shared_ptr<Program> texProg;

	// Our shader program for particles
	std::shared_ptr<Program> partProg;

	//the partricle system
	particleSys *thePartSystem;

	//our geometry
	shared_ptr<Shape> sphere;
	shared_ptr<Shape> theBunny;
	std::vector<shared_ptr<Shape>> dummyMesh;
	std::vector<shared_ptr<Shape>> spiderMesh;
	std::vector<shared_ptr<Shape>> swordMesh;
	std::vector<shared_ptr<Shape>> terrainMesh;
	std::vector<shared_ptr<Shape>> mountainMesh;
	std::vector<shared_ptr<Shape>> stepMesh;
	std::vector<shared_ptr<Shape>> towerMesh;
	std::vector<shared_ptr<Shape>> eyeFireMesh;
	std::vector<shared_ptr<Shape>> eyePupilMesh;
	std::vector<shared_ptr<Shape>> cocoonMesh;

	int width, height;
	vec3 eyePosition = vec3(0, 0, 0);
	vec3 lookAtPoint;   //set based on scroll
	vec3 upVector;
	vec3 uvw;
	vec3 gaze = vec3(0, 0, -1);
	vec3 cameraRight;
	vec3 cameraUp = vec3(0, 1, 0);
	vec3 cameraStrafe;
	vec3 collider = vec3(0, 0, 0);

	double phi, theta;
	float movementSpeed = 0.15;
	float lastX, lastY;
	float pitch, yaw;
	bool firstMouse = true;
	double changeX, changeY, oldX, oldY;
	bool moveForward, moveLeft, moveRight, moveBackward, moveUp, moveDown = false;

	//spline camera
	vec3 view = vec3(0, 0, 1);
	vec3 strafe = vec3(1, 0, 0);
	vec3 g_eye = vec3(0, 1, 0);
	vec3 g_lookAt = vec3(0, 1, -10);
	Spline splinepath[3];
	bool goCamera = false;

	//PARTICLE SYSTEM
	// OpenGL handle to texture data used in particle
	shared_ptr<Texture> texture;
	bool keyToggles[256] = { false };
	//some particle variables
	float t = 0.0f; //reset in init
	float h = 0.01f;

	//global data for ground plane - direct load constant defined CPU data to GPU (not obj)
	GLuint GrndBuffObj, GrndNorBuffObj, GrndTexBuffObj, GIndxBuffObj;
	int g_GiboLen;
	//ground VAO
	GLuint GroundVertexArrayID;

	//the image to use as a texture (ground)
	shared_ptr<Texture> texture0;
	shared_ptr<Texture> texture1;

	//global data (larger program should be encapsulated)
	vec3 gMin;
	vec3 gMax;

	float gRot = 0;
	float gCamH = 0;
	//animation data
	float lightTrans = 0;
	float gTrans = -3;
	float sTheta = 0;
	float eTheta = 0;
	float hTheta = 0;
	float swordMaterial = 6;

	void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		{
			glfwSetWindowShouldClose(window, GL_TRUE);
		}
		//update global camera rotate
		if (key == GLFW_KEY_S && action == GLFW_PRESS) {
			moveBackward = true;
		}
		if (key == GLFW_KEY_S && action == GLFW_RELEASE) {
			moveBackward = false;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_PRESS) {
			moveUp = true;
		}
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) {
			moveUp = false;
		}
		if (key == GLFW_KEY_X && action == GLFW_PRESS) {
			moveDown = true;
		}
		if (key == GLFW_KEY_X && action == GLFW_RELEASE) {
			moveDown = false;
		}		
		// if (key == GLFW_KEY_X && action == GLFW_PRESS) {
		// 	gRot -= 0.2;
		// }
		if (key == GLFW_KEY_C && action == GLFW_PRESS) {
			gRot += 0.2;
		}
		//update camera height
		if (key == GLFW_KEY_W && action == GLFW_PRESS){
			moveForward = true;
		}
		if (key == GLFW_KEY_W && action == GLFW_RELEASE){
			moveForward = false;
		}
		if (key == GLFW_KEY_A && action == GLFW_PRESS) {
			moveLeft = true;
		}
		if (key == GLFW_KEY_A && action == GLFW_RELEASE) {
			moveLeft = false;
		}
		if (key == GLFW_KEY_D && action == GLFW_PRESS){
			moveRight = true;
		}
		if (key == GLFW_KEY_D && action == GLFW_RELEASE){
			moveRight = false;
		}
		if (key == GLFW_KEY_F && action == GLFW_PRESS){
			gCamH  -= 0.25;
		}
		if (key == GLFW_KEY_R && action == GLFW_PRESS)
		{
			thePartSystem->reSet();
		}
		if (key == GLFW_KEY_M && action == GLFW_PRESS) {
			swordMaterial = 1;
		}
		if (key == GLFW_KEY_M && action == GLFW_RELEASE) {
			swordMaterial = 6;
		}
		if (key == GLFW_KEY_Q && action == GLFW_PRESS){
			lightTrans += 0.25;
		}
		if (key == GLFW_KEY_E && action == GLFW_PRESS){
			lightTrans -= 0.25;
		}
		if (key == GLFW_KEY_Z && action == GLFW_PRESS) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
		}
		if (key == GLFW_KEY_Z && action == GLFW_RELEASE) {
			glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) {
			goCamera = !goCamera;
			//cout << "Eye position: " << g_eye.x << ", " << g_eye.y << ", " << g_eye.z << "\n";
		}
	}

	void mouseCallback(GLFWwindow *window, int button, int action, int mods)
	{
		double xpos, ypos;
		eyePosition = vec3(0, 0, 0);
		upVector = vec3(0, 1, 0);

		if (action == GLFW_PRESS)
		{
			 glfwGetCursorPos(window, &xpos, &ypos);
			 cout << "Pos X " << xpos <<  " Pos Y " << ypos << endl;
		}
	}
	
	void scrollCallback(GLFWwindow* window, double deltaX, double deltaY) {
		//code for pitch and yaw variable updates
		changeX = deltaX;
		changeY = -deltaY;
	}

	void resizeCallback(GLFWwindow *window, int width, int height)
	{
		glViewport(0, 0, width, height);
	}

	void init(const std::string& resourceDirectory)
	{
		GLSL::checkVersion();

		// Set background color.
		glClearColor(.45f, .2f, 0.2f, 1.0f);
		// Enable z-buffer test.
		CHECKED_GL_CALL(glEnable(GL_DEPTH_TEST));
		CHECKED_GL_CALL(glEnable(GL_BLEND));
		CHECKED_GL_CALL(glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA));
		CHECKED_GL_CALL(glPointSize(24.0f));


		// Initialize the GLSL program that we will use for local shading
		prog = make_shared<Program>();
		prog->setVerbose(true);
		prog->setShaderNames(resourceDirectory + "/simple_vert.glsl", resourceDirectory + "/simple_frag.glsl");
		prog->init();
		prog->addUniform("P");
		prog->addUniform("V");
		prog->addUniform("M");

		prog->addUniform("MatAmb");
		prog->addUniform("MatDif");
		prog->addUniform("MatSpec");
		prog->addUniform("MatShine");

		prog->addUniform("lightPos");
		prog->addAttribute("vertPos");
		prog->addAttribute("vertNor");
		prog->addAttribute("vertTex"); //silence error


		// Initialize the GLSL program that we will use for texture mapping
		texProg = make_shared<Program>();
		texProg->setVerbose(true);
		texProg->setShaderNames(resourceDirectory + "/tex_vert.glsl", resourceDirectory + "/tex_frag0.glsl");
		texProg->init();
		texProg->addUniform("P");
		texProg->addUniform("V");
		texProg->addUniform("M");
		texProg->addUniform("flip");
		texProg->addUniform("Texture0");
		texProg->addUniform("MatDiff");
		texProg->addUniform("MatSpec");
		texProg->addUniform("MatAmb");
		texProg->addUniform("MatShine");
		texProg->addUniform("lightPos");
		texProg->addAttribute("vertPos");
		texProg->addAttribute("vertNor");
		texProg->addAttribute("vertTex");

		//read in a load the texture
		texture0 = make_shared<Texture>();
  		texture0->setFilename(resourceDirectory + "/ground.jpg");
  		texture0->init();
  		texture0->setUnit(0);
  		texture0->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		texture1 = make_shared<Texture>();
  		texture1->setFilename(resourceDirectory + "/mordor3.jpeg");
  		texture1->init();
  		texture1->setUnit(1);
  		texture1->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);

		// Initialize PARTICLE the GLSL program.
		partProg = make_shared<Program>();
		partProg->setVerbose(true);
		partProg->setShaderNames(
			resourceDirectory + "/lab10_vert.glsl",
			resourceDirectory + "/lab10_frag.glsl");
		if (! partProg->init())
		{
			std::cerr << "One or more shaders failed to compile... exiting!" << std::endl;
			exit(1);
		}
		partProg->addUniform("P");
		partProg->addUniform("M");
		partProg->addUniform("V");
		partProg->addUniform("pColor");
		partProg->addUniform("alphaTexture");
		partProg->addAttribute("vertPos");
		partProg->addAttribute("vertCol");

		thePartSystem = new particleSys(vec3(0, 0, 0));
		thePartSystem->gpuSetup();

		// init splines up and down
       splinepath[0] = Spline(glm::vec3(20,15,30), glm::vec3(10,15,15), glm::vec3(-5, 5, 5), glm::vec3(2,0,-5), 5);
       splinepath[1] = Spline(glm::vec3(2,0,-5), glm::vec3(2, 3, -15), glm::vec3(10,8,-25), glm::vec3(10,15,-35), 5);
	   //splinepath[2] = Spline(glm::vec3(2,0,-5), glm::vec3(2, 5, -15), glm::vec3(10,15,-25), glm::vec3(10,20,-35), 5);
	}

	void initGeom(const std::string& resourceDirectory)
	{
		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;
		//EXAMPLE set up to read one shape from one obj file - convert to read several
		// Initialize mesh
		// Load geometry
 		// Some obj files contain material information.We'll ignore them for this assignment.
 		vector<tinyobj::shape_t> TOshapes;
 		vector<tinyobj::material_t> objMaterials;
 		string errStr;
		//load in the mesh and make the shape(s)
 		bool rc = tinyobj::LoadObj(TOshapes, objMaterials, errStr, (resourceDirectory + "/sphereWTex.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			sphere = make_shared<Shape>();
			sphere->createShape(TOshapes[0]);
			sphere->measure();
			sphere->init();
		}
		//read out information stored in the shape about its size - something like this...
		//then do something with that information.....
		gMin.x = sphere->min.x;
		gMin.y = sphere->min.y;



		// Initialize bunny mesh.
		vector<tinyobj::shape_t> TOshapesB;
 		vector<tinyobj::material_t> objMaterialsB;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesB, objMaterialsB, errStr, (resourceDirectory + "/bunnyNoNorm.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {
			
			theBunny = make_shared<Shape>();
			theBunny->createShape(TOshapesB[0]);
			theBunny->measure();
			theBunny->init();
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;


		// Initialize dummy mesh.
		vector<tinyobj::shape_t> TOshapesC;
 		vector<tinyobj::material_t> objMaterialsC;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesC, objMaterialsC, errStr, (resourceDirectory + "/dummy.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesC.size(); i++){
				dummyMesh.insert(dummyMesh.begin() + i, make_shared<Shape>());
				dummyMesh[i]->createShape(TOshapesC[i]);
				dummyMesh[i]->measure();
				dummyMesh[i]->init();

				if(dummyMesh[i]->min.x < gMin.x){
					gMin.x = dummyMesh[i]->min.x;
				}
				if(dummyMesh[i]->min.y < gMin.y){
					gMin.y = dummyMesh[i]->min.y;
				}
				if(dummyMesh[i]->max.x > gMax.x){
					gMax.x = dummyMesh[i]->max.x;
				}
				if(dummyMesh[i]->max.y > gMax.y){
					gMax.y = dummyMesh[i]->max.y;
				}
			
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize spider mesh.
		vector<tinyobj::shape_t> TOshapesD;
 		vector<tinyobj::material_t> objMaterialsD;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesD, objMaterialsD, errStr, (resourceDirectory + "/spider.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesD.size(); i++){
				spiderMesh.insert(spiderMesh.begin() + i, make_shared<Shape>());
				spiderMesh[i]->createShape(TOshapesD[i]);
				spiderMesh[i]->measure();
				spiderMesh[i]->init();

				if(spiderMesh[i]->min.x < gMin.x){
					gMin.x = spiderMesh[i]->min.x;
				}
				if(spiderMesh[i]->min.y < gMin.y){
					gMin.y = spiderMesh[i]->min.y;
				}
				if(spiderMesh[i]->max.x > gMax.x){
					gMax.x = spiderMesh[i]->max.x;
				}
				if(spiderMesh[i]->max.y > gMax.y){
					gMax.y = spiderMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize sword mesh.
		vector<tinyobj::shape_t> TOshapesE;
 		vector<tinyobj::material_t> objMaterialsE;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesE, objMaterialsE, errStr, (resourceDirectory + "/newSword.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesE.size(); i++){
				swordMesh.insert(swordMesh.begin() + i, make_shared<Shape>());
				swordMesh[i]->createShape(TOshapesE[i]);
				swordMesh[i]->measure();
				swordMesh[i]->init();

				if(swordMesh[i]->min.x < gMin.x){
					gMin.x = swordMesh[i]->min.x;
				}
				if(swordMesh[i]->min.y < gMin.y){
					gMin.y = swordMesh[i]->min.y;
				}
				if(swordMesh[i]->max.x > gMax.x){
					gMax.x = swordMesh[i]->max.x;
				}
				if(swordMesh[i]->max.y > gMax.y){
					gMax.y = swordMesh[i]->max.y;
				}
				
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize terrain mesh.
		vector<tinyobj::shape_t> TOshapesF;
 		vector<tinyobj::material_t> objMaterialsF;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesF, objMaterialsF, errStr, (resourceDirectory + "/terrain.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesF.size(); i++){
				terrainMesh.insert(terrainMesh.begin() + i, make_shared<Shape>());
				terrainMesh[i]->createShape(TOshapesF[i]);
				terrainMesh[i]->measure();
				terrainMesh[i]->init();

				if(terrainMesh[i]->min.x < gMin.x){
					gMin.x = terrainMesh[i]->min.x;
				}
				if(terrainMesh[i]->min.y < gMin.y){
					gMin.y = terrainMesh[i]->min.y;
				}
				if(terrainMesh[i]->max.x > gMax.x){
					gMax.x = terrainMesh[i]->max.x;
				}
				if(terrainMesh[i]->max.y > gMax.y){
					gMax.y = terrainMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize mountain mesh.
		vector<tinyobj::shape_t> TOshapesG;
 		vector<tinyobj::material_t> objMaterialsG;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesG, objMaterialsG, errStr, (resourceDirectory + "/mountain.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesG.size(); i++){
				mountainMesh.insert(mountainMesh.begin() + i, make_shared<Shape>());
				mountainMesh[i]->createShape(TOshapesG[i]);
				mountainMesh[i]->measure();
				mountainMesh[i]->init();

				if(mountainMesh[i]->min.x < gMin.x){
					gMin.x = mountainMesh[i]->min.x;
				}
				if(mountainMesh[i]->min.y < gMin.y){
					gMin.y = mountainMesh[i]->min.y;
				}
				if(mountainMesh[i]->max.x > gMax.x){
					gMax.x = mountainMesh[i]->max.x;
				}
				if(mountainMesh[i]->max.y > gMax.y){
					gMax.y = mountainMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize step mesh.
		vector<tinyobj::shape_t> TOshapesH;
 		vector<tinyobj::material_t> objMaterialsH;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesH, objMaterialsH, errStr, (resourceDirectory + "/step.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesH.size(); i++){
				stepMesh.insert(stepMesh.begin() + i, make_shared<Shape>());
				stepMesh[i]->createShape(TOshapesH[i]);
				stepMesh[i]->measure();
				stepMesh[i]->init();

				if(stepMesh[i]->min.x < gMin.x){
					gMin.x = stepMesh[i]->min.x;
				}
				if(stepMesh[i]->min.y < gMin.y){
					gMin.y = stepMesh[i]->min.y;
				}
				if(stepMesh[i]->max.x > gMax.x){
					gMax.x = stepMesh[i]->max.x;
				}
				if(stepMesh[i]->max.y > gMax.y){
					gMax.y = stepMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize tower mesh.
		vector<tinyobj::shape_t> TOshapesI;
 		vector<tinyobj::material_t> objMaterialsI;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesI, objMaterialsI, errStr, (resourceDirectory + "/tower.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesI.size(); i++){
				towerMesh.insert(towerMesh.begin() + i, make_shared<Shape>());
				towerMesh[i]->createShape(TOshapesI[i]);
				towerMesh[i]->measure();
				towerMesh[i]->init();

				if(towerMesh[i]->min.x < gMin.x){
					gMin.x = towerMesh[i]->min.x;
				}
				if(towerMesh[i]->min.y < gMin.y){
					gMin.y = towerMesh[i]->min.y;
				}
				if(towerMesh[i]->max.x > gMax.x){
					gMax.x = towerMesh[i]->max.x;
				}
				if(towerMesh[i]->max.y > gMax.y){
					gMax.y = towerMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize eye mesh.
		vector<tinyobj::shape_t> TOshapesJ;
 		vector<tinyobj::material_t> objMaterialsJ;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesJ, objMaterialsJ, errStr, (resourceDirectory + "/eye_fire.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesJ.size(); i++){
				eyeFireMesh.insert(eyeFireMesh.begin() + i, make_shared<Shape>());
				eyeFireMesh[i]->createShape(TOshapesJ[i]);
				eyeFireMesh[i]->measure();
				eyeFireMesh[i]->init();

				if(eyeFireMesh[i]->min.x < gMin.x){
					gMin.x = eyeFireMesh[i]->min.x;
				}
				if(eyeFireMesh[i]->min.y < gMin.y){
					gMin.y = eyeFireMesh[i]->min.y;
				}
				if(eyeFireMesh[i]->max.x > gMax.x){
					gMax.x = eyeFireMesh[i]->max.x;
				}
				if(eyeFireMesh[i]->max.y > gMax.y){
					gMax.y = eyeFireMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize pupil mesh.
		vector<tinyobj::shape_t> TOshapesK;
 		vector<tinyobj::material_t> objMaterialsK;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesK, objMaterialsK, errStr, (resourceDirectory + "/eye_pupil.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesK.size(); i++){
				eyePupilMesh.insert(eyePupilMesh.begin() + i, make_shared<Shape>());
				eyePupilMesh[i]->createShape(TOshapesK[i]);
				eyePupilMesh[i]->measure();
				eyePupilMesh[i]->init();

				if(eyePupilMesh[i]->min.x < gMin.x){
					gMin.x = eyePupilMesh[i]->min.x;
				}
				if(eyePupilMesh[i]->min.y < gMin.y){
					gMin.y = eyePupilMesh[i]->min.y;
				}
				if(eyePupilMesh[i]->max.x > gMax.x){
					gMax.x = eyePupilMesh[i]->max.x;
				}
				if(eyePupilMesh[i]->max.y > gMax.y){
					gMax.y = eyePupilMesh[i]->max.y;
				}
			}
		}

		gMin.x = gMin.y = 0;
		gMax.x = gMax.y = 0;

		// Initialize cocoon mesh.
		vector<tinyobj::shape_t> TOshapesL;
 		vector<tinyobj::material_t> objMaterialsL;
		//load in the mesh and make the shape(s)
 		rc = tinyobj::LoadObj(TOshapesL, objMaterialsL, errStr, (resourceDirectory + "/cocoon.obj").c_str());
		if (!rc) {
			cerr << errStr << endl;
		} else {

			for(int i = 0; i < TOshapesL.size(); i++){
				cocoonMesh.insert(cocoonMesh.begin() + i, make_shared<Shape>());
				cocoonMesh[i]->createShape(TOshapesL[i]);
				cocoonMesh[i]->measure();
				cocoonMesh[i]->init();

				if(cocoonMesh[i]->min.x < gMin.x){
					gMin.x = cocoonMesh[i]->min.x;
				}
				if(cocoonMesh[i]->min.y < gMin.y){
					gMin.y = cocoonMesh[i]->min.y;
				}
				if(cocoonMesh[i]->max.x > gMax.x){
					gMax.x = cocoonMesh[i]->max.x;
				}
				if(cocoonMesh[i]->max.y > gMax.y){
					gMax.y = cocoonMesh[i]->max.y;
				}
			}
		}
		
		//code to load in the ground plane (CPU defined data passed to GPU)
		initGround();
	}

	// Code to load in the PARTICLE texture
	void initTex(const std::string& resourceDirectory)
	{
		texture = make_shared<Texture>();
		texture->setFilename(resourceDirectory + "/alpha.bmp");
		texture->init();
		texture->setUnit(0);
		texture->setWrapModes(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE);
	}

	//directly pass quad for the ground to the GPU
	void initGround() {

		float g_groundSize = 20;
		float g_groundY = -0.25;

  		// A x-z plane at y = g_groundY of dimension [-g_groundSize, g_groundSize]^2
		float GrndPos[] = {
			-g_groundSize, g_groundY, -g_groundSize,
			-g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY,  g_groundSize,
			g_groundSize, g_groundY, -g_groundSize
		};

		float GrndNorm[] = {
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0,
			0, 1, 0
		};

		static GLfloat GrndTex[] = {
      		0, 0, // back
      		0, 1,
      		1, 1,
      		1, 0 };

      	unsigned short idx[] = {0, 1, 2, 0, 2, 3};

		//generate the ground VAO
      	glGenVertexArrays(1, &GroundVertexArrayID);
      	glBindVertexArray(GroundVertexArrayID);

      	g_GiboLen = 6;
      	glGenBuffers(1, &GrndBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndPos), GrndPos, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndNorBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndNorm), GrndNorm, GL_STATIC_DRAW);

      	glGenBuffers(1, &GrndTexBuffObj);
      	glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
      	glBufferData(GL_ARRAY_BUFFER, sizeof(GrndTex), GrndTex, GL_STATIC_DRAW);

      	glGenBuffers(1, &GIndxBuffObj);
     	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
      	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(idx), idx, GL_STATIC_DRAW);
      }

      //code to draw the ground plane
     void drawGround(shared_ptr<Program> curS) {
     	curS->bind();
     	glBindVertexArray(GroundVertexArrayID);
     	texture0->bind(curS->getUniform("Texture0"));
		//draw the ground plane 
  		SetModel(vec3(0, -2, 0), 0, 0, 1, curS);
  		glEnableVertexAttribArray(0);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndBuffObj);
  		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(1);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndNorBuffObj);
  		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

  		glEnableVertexAttribArray(2);
  		glBindBuffer(GL_ARRAY_BUFFER, GrndTexBuffObj);
  		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

   		// draw!
  		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GIndxBuffObj);
  		glDrawElements(GL_TRIANGLES, g_GiboLen, GL_UNSIGNED_SHORT, 0);

  		glDisableVertexAttribArray(0);
  		glDisableVertexAttribArray(1);
  		glDisableVertexAttribArray(2);
  		curS->unbind();
     }

     //helper function to pass material data to the GPU
	void SetMaterial(shared_ptr<Program> curS, int i) {

    	switch (i) {
    		case 0:
    			glUniform3f(curS->getUniform("MatAmb"), 0.089, 0.035, 0.013);
    			glUniform3f(curS->getUniform("MatDif"), 0.89, 0.35, 0.13);
    			glUniform3f(curS->getUniform("MatSpec"), 0.45, 0.17, 0.06);
    			glUniform1f(curS->getUniform("MatShine"), 20.0);
    		break;
    		case 1: 
    			glUniform3f(curS->getUniform("MatAmb"), 0.063, 0.75, 0.8);
    			glUniform3f(curS->getUniform("MatDif"), 0.63, 0.38, 1.0);
    			glUniform3f(curS->getUniform("MatSpec"), 0.9, 0.2, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 400.0);
    		break;
    		case 2:
    			glUniform3f(curS->getUniform("MatAmb"), 0.05, 0.01, 0.01);
    			glUniform3f(curS->getUniform("MatDif"), 0.5, 0.1, 0.1);
    			glUniform3f(curS->getUniform("MatSpec"), 0.25, 0.05, 0.05);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
			case 3:
    			glUniform3f(curS->getUniform("MatAmb"), 0.04, 0.04, 0.04);
    			glUniform3f(curS->getUniform("MatDif"), 0.4, 0.4, 0.4);
    			glUniform3f(curS->getUniform("MatSpec"), 0.2, 0.2, 0.2);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
			case 4:
    			glUniform3f(curS->getUniform("MatAmb"), 0.02, 0.02, 0.02);
    			glUniform3f(curS->getUniform("MatDif"), 0.2, 0.2, 0.2);
    			glUniform3f(curS->getUniform("MatSpec"), 0.1, 0.1, 0.1);
    			glUniform1f(curS->getUniform("MatShine"), 27.9);
    		break;
			case 5:
    			glUniform3f(curS->getUniform("MatAmb"), 0.01, 0.01, 0.01);
    			glUniform3f(curS->getUniform("MatDif"), 0.1, 0.1, 0.1);
    			glUniform3f(curS->getUniform("MatSpec"), 0.05, 0.05, 0.05);
    			glUniform1f(curS->getUniform("MatShine"), 227.9);
    		break;
			case 6:
    			glUniform3f(curS->getUniform("MatAmb"), 0.07, 0.07, 0.07);
    			glUniform3f(curS->getUniform("MatDif"), 0.7, 0.7, 0.7);
    			glUniform3f(curS->getUniform("MatSpec"), 0.035, 0.035, 0.035);
    			glUniform1f(curS->getUniform("MatShine"), 500.9);
    		break;
			case 7:
    			glUniform3f(curS->getUniform("MatAmb"), 0.095, 0.075, 0.041);
    			glUniform3f(curS->getUniform("MatDif"), 0.95, 0.75, 0.41);
    			glUniform3f(curS->getUniform("MatSpec"), 0.47, 0.37, 0.21);
    			glUniform1f(curS->getUniform("MatShine"), 500.9);
    		break;
			case 8:
    			glUniform3f(curS->getUniform("MatAmb"), 0.095, 0.095, 0.095);
    			glUniform3f(curS->getUniform("MatDif"), 0.95, 0.95, 0.95);
    			glUniform3f(curS->getUniform("MatSpec"), 0.5, 0.5, 0.5);
    			glUniform1f(curS->getUniform("MatShine"), 500.9);
    		break;
  		}
	}

	/* helper function to set model trasnforms */
  	void SetModel(vec3 trans, float rotY, float rotX, float sc, shared_ptr<Program> curS) {
  		mat4 Trans = glm::translate( glm::mat4(1.0f), trans);
  		mat4 RotX = glm::rotate( glm::mat4(1.0f), rotX, vec3(1, 0, 0));
  		mat4 RotY = glm::rotate( glm::mat4(1.0f), rotY, vec3(0, 1, 0));
  		mat4 ScaleS = glm::scale(glm::mat4(1.0f), vec3(sc));
  		mat4 ctm = Trans*RotX*RotY*ScaleS;
  		glUniformMatrix4fv(curS->getUniform("M"), 1, GL_FALSE, value_ptr(ctm));
  	}

	void setModel(std::shared_ptr<Program> prog, std::shared_ptr<MatrixStack>M) {
		glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(M->topMatrix()));
   	}
	
	/* camera controls - do not change */
	void SetView(shared_ptr<Program>  shader) {
  		glm::mat4 Cam = glm::lookAt(g_eye, g_lookAt, vec3(0, 1, 0));
  		glUniformMatrix4fv(shader->getUniform("V"), 1, GL_FALSE, value_ptr(Cam));
	}
	
	void drawSphere(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->scale(vec3(0.03, 0.03, 0.03));

			SetMaterial(prog, 2);
			setModel(prog, Model);
			sphere->draw(prog);
		Model->popMatrix();
	}

	void drawCocoon(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(4.42, -1.7, -9));
			Model->scale(vec3(0.1, 0.1, 0.1));
			Model->rotate(0.9, vec3(0, 0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 8);
			setModel(prog, Model);
			for(int i = 0; i < cocoonMesh.size(); i++){
				cocoonMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

   	/* code to draw waving hierarchical model */
   	void drawHierModel(shared_ptr<MatrixStack> Model) {
   		// draw mesh 
		Model->pushMatrix();
			SetMaterial(prog, 0);

			Model->loadIdentity();
			//rotate whole entity around camera
			Model->rotate(1.57, vec3(0, 1, 0));
			Model->translate(vec3(gTrans - 3, 0, 0));
			
			/* draw top cube - aka head */
			Model->pushMatrix();
			
				Model->translate(vec3(0, 1.4, 0));
				Model->scale(vec3(0.5, 0.5, 0.5));
				setModel(prog, Model);
				sphere->draw(prog);
			Model->popMatrix();
			//draw the torso with these transforms
			Model->pushMatrix();
			  Model->scale(vec3(1.25, 1.35, 1.25));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
			
			
			//RIGHT ARM
			
			// draw the upper 'arm' - relative 
			//note you must change this to include 3 components!
			Model->pushMatrix();
			  Model->rotate(1.57, vec3(0, 1, 0));

			  //place at shoulder
			  Model->translate(vec3(0.8, 0.8, 0));
			  //rotate shoulder joint
			  Model->rotate(sTheta, vec3(0, 0, 1));
			  //move to shoulder joint
			  Model->translate(vec3(0.8, 0, 0));
	
			    //now draw lower arm - this is INCOMPLETE and you will add a 3rd component
			  	//right now this is in the SAME place as the upper arm
			  	Model->pushMatrix();
				  Model->translate(vec3(0.5, 0.0, 0));
					//rotate elbow joint
			      Model->rotate(-eTheta, vec3(0, 0, 1));
					//move to elbow joint
				  Model->translate(vec3(0.7, 0, 0));

				  //draw hand
				  Model->pushMatrix();
					Model->translate(vec3(0.5, 0.0, 0));
						//rotate wrist joint
					Model->rotate(-hTheta, vec3(0, 0, 1));
						//move to wrist joint
					Model->translate(vec3(0.35, 0, 0));
					Model->scale(vec3(0.4, 0.25, 0.25));
					setModel(prog, Model);
					sphere->draw(prog);
			  	  Model->popMatrix();

				  Model->scale(vec3(0.6, 0.25, 0.25));
			  	  setModel(prog, Model);
			  	  sphere->draw(prog);
			  	Model->popMatrix();

			  //Do final scale ONLY to upper arm then draw
			  //non-uniform scale
			  Model->scale(vec3(0.8, 0.25, 0.25));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();



			//LEFT ARM

			// draw the upper 'arm' - relative 
			//note you must change this to include 3 components!
			Model->pushMatrix();
			  Model->rotate(1.57, vec3(0, 1, 0));

			  //place at shoulder
			  Model->translate(vec3(-0.8, 0.8, 0));
			  //rotate shoulder joint
			  Model->rotate(0.25, vec3(0, 0, 1));

			  //move to shoulder joint
			  Model->translate(vec3(-0.8, 0, 0));
	
			    //now draw lower arm - this is INCOMPLETE and you will add a 3rd component
			  	//right now this is in the SAME place as the upper arm
			  	Model->pushMatrix();
				  Model->translate(vec3(-0.5, 0.0, 0));
					//rotate elbow joint
			      Model->rotate(2, vec3(0, 0, 1));
					//move to elbow joint
				  Model->translate(vec3(-0.7, 0, 0));
				  

				  //draw hand
				  Model->pushMatrix();
					Model->translate(vec3(-0.5, 0.0, 0));
						//rotate wrist joint
					Model->rotate(-0.3, vec3(0.1, 0, 1));
						//move to wist joint
					Model->translate(vec3(-0.35, 0, 0));

					Model->scale(vec3(0.4, 0.25, 0.25));
					setModel(prog, Model);
					sphere->draw(prog);
			  	  Model->popMatrix();

				  Model->scale(vec3(0.6, 0.25, 0.25));
			  	  setModel(prog, Model);
			  	  sphere->draw(prog);
			  	Model->popMatrix();

			  //Do final scale ONLY to upper arm then draw
			  //non-uniform scale
			  Model->scale(vec3(0.8, 0.25, 0.25));
			  setModel(prog, Model);
			  sphere->draw(prog);
			Model->popMatrix();
		
		Model->popMatrix();

		prog->unbind();

		
		//animation update example
		eTheta = sin(glfwGetTime());
		if(eTheta > 0){
			eTheta = 0;
		}
		sTheta = cos(glfwGetTime());
		hTheta = -tan(glfwGetTime()/4);
		   
   	}

	void drawBunniesBaseCode(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();

		float sp = 3.0;
		float off = -3.5;
		  for (int i =0; i < 3; i++) {
		  	for (int j=0; j < 3; j++) {
			  Model->pushMatrix();
				Model->translate(vec3(off+sp*i, -1, off+sp*j));
				Model->scale(vec3(4.85, 4.85, 4.85));
				SetMaterial(prog, (i+j)%3);
				setModel(prog, Model);
				//glUniformMatrix4fv(prog->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
				theBunny->draw(prog);
			  Model->popMatrix();
			}
		  }
		Model->popMatrix();
	}

	void drawDummy(shared_ptr<MatrixStack> Model){
		//DUMMY
		Model->pushMatrix();
	
			SetMaterial(prog, 7);
			//BEFORE SCALING!
			Model->translate(vec3(-2,-0.65,-10));
			
			Model->scale(vec3(0.02, 0.02, 0.02));
			Model->rotate(-0.47, vec3(0, 0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->translate(vec3(-2,0,-95));
			
			//TORSO
			Model->pushMatrix();
				Model->translate(vec3(2,0,95));
				Model->rotate(-0.27, vec3(0, 1, 0));
				Model->translate(vec3(-2,0,-95));

				setModel(prog, Model);
				dummyMesh[12]->draw(prog);//hips
				dummyMesh[13]->draw(prog);//belly
				dummyMesh[14]->draw(prog);//torso
					
				//HEAD AND NECK
				Model->pushMatrix();
					Model->translate(vec3(2,0,148));
					Model->rotate(-0.27, vec3(0, 1, 0));	
					Model->translate(vec3(-2,0,-148));

					setModel(prog, Model);
					dummyMesh[27]->draw(prog);
					dummyMesh[28]->draw(prog);
				Model->popMatrix();


				//RIGHT ARM
				Model->pushMatrix();
					Model->translate(vec3(2,-20,138));
					Model->rotate(0.47, vec3(0, 1, 0));	
					Model->rotate(1.27, vec3(0, 0, 1));	
					Model->translate(vec3(-2,20,-138));

					setModel(prog, Model);
					dummyMesh[15]->draw(prog);//shoulder
					dummyMesh[16]->draw(prog);

					Model->pushMatrix();
						Model->translate(vec3(2,-47,138));
						Model->rotate(1.27, vec3(0, 0, 1));	
						Model->translate(vec3(-2,47,-138));

						setModel(prog, Model);
						dummyMesh[17]->draw(prog);//elbow
						dummyMesh[18]->draw(prog);

						Model->pushMatrix();
							Model->translate(vec3(2,-73,138));
							Model->rotate(-1.27, vec3(0, 1, 0));	
							Model->translate(vec3(-2,73,-138));

							setModel(prog, Model);
							dummyMesh[19]->draw(prog);//wrist
							dummyMesh[20]->draw(prog);

						Model->popMatrix();
					Model->popMatrix();
				Model->popMatrix();
				

				//LEFT ARM
				Model->pushMatrix();
					Model->translate(vec3(2,20,138));
					Model->rotate(0.37, vec3(0, 1, 0));	
					Model->rotate(-1.27, vec3(0, 0, 1));	
					Model->translate(vec3(-2,-20,-138));

					setModel(prog, Model);
					dummyMesh[21]->draw(prog);//shoulder
					dummyMesh[22]->draw(prog);

					Model->pushMatrix();
						Model->translate(vec3(2,47,138));
						Model->rotate(-1.27, vec3(0, 0, 1));	
						Model->translate(vec3(-2,-47,-138));

						setModel(prog, Model);
						dummyMesh[23]->draw(prog);//elbow
						dummyMesh[24]->draw(prog);

						Model->pushMatrix();
							Model->translate(vec3(2,73,138));
							Model->rotate(-1.27, vec3(0, 1, 0));	
							Model->translate(vec3(-2,-73,-138));

							setModel(prog, Model);
							dummyMesh[25]->draw(prog);//wrist
							dummyMesh[26]->draw(prog);

							drawSword(Model);
							SetMaterial(prog, 7);

						Model->popMatrix();
					Model->popMatrix();
				Model->popMatrix();
			Model->popMatrix();

			//RIGHT LEG
			Model->pushMatrix();
				Model->translate(vec3(2,-8,95));
				Model->rotate(-0.47, vec3(0, 0, 1));
				Model->translate(vec3(-2,8,-95));

				setModel(prog, Model);
				dummyMesh[5]->draw(prog);
				dummyMesh[4]->draw(prog);//pelvis

				Model->pushMatrix();
					Model->translate(vec3(2,-8,50));
					Model->rotate(0.27, vec3(0, 1, 0));
					Model->translate(vec3(-2,8,-50));

					setModel(prog, Model);
					dummyMesh[3]->draw(prog);
					dummyMesh[2]->draw(prog);//knee

					Model->pushMatrix();
						Model->translate(vec3(2,-8,10));
						Model->rotate(-0.57, vec3(0, 1, 0));	
						Model->translate(vec3(-2,8,-10));

						setModel(prog, Model);
						dummyMesh[1]->draw(prog);
						dummyMesh[0]->draw(prog);//ankle

					Model->popMatrix();
				Model->popMatrix();
			Model->popMatrix();

			//LEFT LEG
			Model->pushMatrix();
				Model->translate(vec3(2,8,95));
				Model->rotate(0.27, vec3(0, 0, 1));
				Model->rotate(-1.27, vec3(0, 1, 0));
				Model->translate(vec3(-2,-8,-95));

				setModel(prog, Model);
				dummyMesh[11]->draw(prog);
				dummyMesh[10]->draw(prog);//pelvis

				Model->pushMatrix();
					Model->translate(vec3(2,8,50));
					Model->rotate(1.27, vec3(0, 1, 0));
					Model->translate(vec3(-2,-8,-50));

					setModel(prog, Model);
					dummyMesh[9]->draw(prog);
					dummyMesh[8]->draw(prog);//knee

					Model->pushMatrix();
						Model->translate(vec3(2,8,10));
						Model->rotate(-0.47, vec3(0, 1, 0));
						Model->rotate(-0.37, vec3(0, 0, 1));	
						Model->translate(vec3(-2,-8,-10));

						setModel(prog, Model);
						dummyMesh[7]->draw(prog);
						dummyMesh[6]->draw(prog);//ankle

					Model->popMatrix();
				Model->popMatrix();
			Model->popMatrix();
		Model->popMatrix();
	}

	void drawSpider(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(5, -0.5, -9.8));
			Model->scale(vec3(0.8, 0.8, 0.8));
			Model->rotate(-1.9, vec3(0, 1, 0));
			Model->translate(vec3(-10, 1.0, 1));
			Model->rotate(-0.6, vec3(0, 0, 1));
			Model->rotate(0.2, vec3(1, 0, 0));

			SetMaterial(prog, 5);
			setModel(prog, Model);
			spiderMesh[0]->draw(prog);
			spiderMesh[1]->draw(prog);
			spiderMesh[2]->draw(prog);
			spiderMesh[3]->draw(prog);
			spiderMesh[6]->draw(prog);
			Model->pushMatrix();
				Model->translate(vec3(2, 3, 0));
				Model->rotate(sin(glfwGetTime()/4)/2 - 0.4, vec3(0, 1, 0));
				Model->translate(vec3(-2, -3, 0));

				SetMaterial(prog, 2);
				setModel(prog, Model);
				//HEAD AND EYES
				spiderMesh[4]->draw(prog);
				spiderMesh[5]->draw(prog);
				
				SetMaterial(prog, 5);
				setModel(prog, Model);
				spiderMesh[7]->draw(prog);
				spiderMesh[8]->draw(prog);

				//COLLISION SPHERE
				Model->pushMatrix();
					collider.x = Model->getPositionX();
					collider.y = Model->getPositionY();
					collider.z = Model->getPositionZ();
					Model->translate(vec3(11, 8, 0));
					setModel(prog, Model);
				Model->popMatrix();
			Model->popMatrix();
		Model->popMatrix();
	}

	void drawSword(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			if(7.4 < collider.x && collider.x < 8.0){
				swordMaterial = 1;
			}
			else{
				swordMaterial = 6;
			}

			SetMaterial(prog, swordMaterial);
			setModel(prog, Model);
			for(int i = 0; i < swordMesh.size(); i++){
				swordMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawTower(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-50.15, 70.2, -206.39));
			Model->rotate(1.57, vec3(0, 1, 0));		
			Model->scale(vec3(4, 4, 4));

			SetMaterial(prog, 5);
			setModel(prog, Model);
			for(int i = 0; i < towerMesh.size(); i++){
				towerMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawEyeFire(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-50.15, 145.2, -204.39));
			Model->rotate(1.57, vec3(1, 0, 0));
			Model->rotate(1.57, vec3(0, 1, 0));
			Model->scale(vec3(4, 4, 4));

			SetMaterial(prog, 0);
			setModel(prog, Model);
			for(int i = 0; i < eyeFireMesh.size(); i++){
				eyeFireMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawEyePupil(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-50.15, 145.2, -204.39));
			Model->rotate(1.57, vec3(1, 0, 0));
			Model->rotate(1.57, vec3(0, 1, 0));	
			Model->scale(vec3(4, 4, 4));

			SetMaterial(prog, 5);
			setModel(prog, Model);
			for(int i = 0; i < eyePupilMesh.size(); i++){
				eyePupilMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawTerrain(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(0, 0, 0));
			Model->scale(vec3(3, 3, 3));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(3.1415, vec3(0, 1, 0));

			SetMaterial(prog, 2);
			setModel(prog, Model);
			for(int i = 0; i < terrainMesh.size(); i++){
				terrainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawMountainOnPath(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-5, 0, -20));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		
		Model->popMatrix();
		Model->pushMatrix();
			Model->translate(vec3(10, 5, -30));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		
		Model->popMatrix();
		Model->pushMatrix();
			Model->translate(vec3(-5, 5, -30));
			Model->rotate(3.15, vec3(0, 1, 0));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		
		Model->popMatrix();
		Model->pushMatrix();
			Model->translate(vec3(0, 5, -47));
			Model->rotate(1, vec3(0, 1, 0));
			Model->rotate(1.57, vec3(0, 0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->scale(vec3(0.4, 0.4, 0.4));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(15, 0, -5));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->scale(vec3(0.2, 0.2, 0.2));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawMountain(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(70, 25, -200));
			Model->scale(vec3(3, 3, 3));
			Model->rotate(3.15, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 4);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(-50, -35, -200));
			Model->scale(vec3(3, 3, 3));
			Model->rotate(3.15, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 4);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawMountain2(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-40, 0, -50));
			Model->scale(vec3(1, 1, 1));
			Model->rotate(1.57, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 4);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawRightWall(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(20, 0, -18));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->rotate(3.15, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(30, 10, -28));
			Model->scale(vec3(0.7, 0.7, 0.7));
			Model->rotate(2.15, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(10, 10, -58));
			Model->scale(vec3(0.7, 0.7, 0.7));
			Model->rotate(-2.55, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawLeftWall(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-20, 0, -20));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(-70, 0, -30));
			Model->scale(vec3(1, 1, 1));
			Model->rotate(-1.57, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawLeftWall2(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-30, 0, 0));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawBackWall(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(0, 0, 30));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(3.1415, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();

		Model->pushMatrix();
			Model->translate(vec3(-10, 0, 20));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.47, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawBackLeftWall(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(-20, 0, 20));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawBackRightWall(shared_ptr<MatrixStack> Model){
		Model->pushMatrix();
			Model->translate(vec3(20, 0, 15));
			Model->scale(vec3(0.5, 0.5, 0.5));
			Model->rotate(3.15, vec3(0, 1, 0));
			Model->translate(vec3(-1, -1.0, 1));
			Model->rotate(-1.57, vec3(1, 0, 0));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < mountainMesh.size(); i++){
				mountainMesh[i]->draw(prog);
			}
		Model->popMatrix();
	}

	void drawSteps(shared_ptr<MatrixStack> Model){
		float rotVal = 0.0;
		for(int i = 0; i < 30; i++){
			Model->pushMatrix();
			Model->rotate(-rotVal, vec3(0, 1, 0));
			Model->translate(vec3(0, -2 + i, -15 - (2 * i)));
			Model->rotate(1.57, vec3(0, 1, 0));
			Model->rotate(-1.57, vec3(1, 0, 0));
			Model->scale(vec3(0.2, 0.25, 0.1));

			SetMaterial(prog, 3);
			setModel(prog, Model);
			for(int i = 0; i < stepMesh.size(); i++){
				stepMesh[i]->draw(prog);
			}
			rotVal += 0.02;
		Model->popMatrix();
		}
	}

	void moveCamera(){
		if(moveForward == true){
			eyePosition.x += gaze.x * 0.5 * (2 * movementSpeed);
			eyePosition.y += gaze.y * 0.5 * (2 * movementSpeed);
			eyePosition.z += gaze.z * 0.5 * (2 * movementSpeed);
			lookAtPoint.x += gaze.x * (2 * movementSpeed);
			lookAtPoint.y += gaze.y * (2 * movementSpeed);
			lookAtPoint.z += gaze.z * (2 * movementSpeed);
		}

		if(moveBackward == true){
			eyePosition.x -= gaze.x * 0.5 * movementSpeed;
			eyePosition.y -= gaze.y * 0.5 * movementSpeed;
			eyePosition.z -= gaze.z * 0.5 * movementSpeed;
			lookAtPoint.x -= gaze.x * movementSpeed;
			lookAtPoint.y -= gaze.y * movementSpeed;
			lookAtPoint.z -= gaze.z * movementSpeed;
		}
		
		if(moveLeft == true){
			eyePosition.x -= cameraStrafe.x * 0.5 * movementSpeed;
			eyePosition.y -= cameraStrafe.y * 0.5 * movementSpeed;
			eyePosition.z -= cameraStrafe.z * 0.5 * movementSpeed;
			lookAtPoint.x -= cameraStrafe.x * movementSpeed;
			lookAtPoint.y -= cameraStrafe.y * movementSpeed;
			lookAtPoint.z -= cameraStrafe.z * movementSpeed;
		}

		if(moveRight == true){
			eyePosition.x += cameraStrafe.x * 0.5 * movementSpeed;//speed
			eyePosition.y += cameraStrafe.y * 0.5 * movementSpeed;//speed
			eyePosition.z += cameraStrafe.z * 0.5 * movementSpeed;//speed
			lookAtPoint.x += cameraStrafe.x * movementSpeed;
			lookAtPoint.y += cameraStrafe.y * movementSpeed;
			lookAtPoint.z += cameraStrafe.z * movementSpeed;
		}

		if(moveUp == true){
			eyePosition.x += upVector.x * 0.5 * movementSpeed;//speed
			eyePosition.y += upVector.y * 0.5 * movementSpeed;//speed
			eyePosition.z += upVector.z * 0.5 * movementSpeed;//speed
			lookAtPoint.x += upVector.x * movementSpeed;
			lookAtPoint.y += upVector.y * movementSpeed;
			lookAtPoint.z += upVector.z * movementSpeed;
		}

		if(moveDown == true){
			eyePosition.x -= upVector.x * 0.5 * movementSpeed;//speed
			eyePosition.y -= upVector.y * 0.5 * movementSpeed;//speed
			eyePosition.z -= upVector.z * 0.5 * movementSpeed;//speed
			lookAtPoint.x -= upVector.x * movementSpeed;
			lookAtPoint.y -= upVector.y * movementSpeed;
			lookAtPoint.z -= upVector.z * movementSpeed;
		}

	}

	void updateUsingCameraPath(float frametime)  {
   	  if (goCamera) {
       if(!splinepath[0].isDone()){
       		splinepath[0].update(frametime);
            g_eye = splinepath[0].getPosition();
        } else {
            splinepath[1].update(frametime);
            g_eye = splinepath[1].getPosition();
        }
      }
   	}

	void render(float frametime) {
		// Get current frame buffer size.
		glfwGetFramebufferSize(windowManager->getHandle(), &width, &height);
		glViewport(0, 0, width, height);
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		//initial pos for cursor
		lastX = width / 2.0;
		lastY = height / 2.0;

		// Clear framebuffer
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		//Use the matrix stack for Lab 6
		float aspect = width/(float)height;

		// Create the matrix stacks - please leave these alone for now
		auto Projection = make_shared<MatrixStack>();
		auto View = make_shared<MatrixStack>();
		auto Model = make_shared<MatrixStack>();

		//update the camera position
		updateUsingCameraPath(frametime);
		Model->pushMatrix();
		Model->loadIdentity();
		// camera rotate
		thePartSystem->setCamera(View->topMatrix());
		// Apply perspective projection.
		Projection->pushMatrix();
		Projection->perspective(70.0f, aspect, 0.01f, 500.0f);

		// View is global translation along negative z for now
		View->pushMatrix();
		View->loadIdentity();
		//camera up and down
		moveCamera();
		
		upVector = vec3(0, 1, 0);
		double pi = 3.14159265;
		double radius = 1.0;
		
		theta += ((2 * pi) / width) * changeX;
		phi += (pi / height) * changeY;
		
		//so camera doesnt keep spinning after input
		oldX = changeX;
		oldY = changeY;

		if(oldX - changeX == 0 && oldY - changeY == 0){
			changeY = 0;
			changeX = 0;
		}
		
		if(phi > 1.39626){
			phi = 1.39626;
		}
		if(phi < -1.39626){
			phi = -1.39626;
		}

		double x = radius * cos(phi) * cos(theta);
		double y = radius * sin(phi);
		double z = radius * cos(phi) * sin(theta);
		vec3 lookAtPoint = normalize(vec3(x, y, z));
		lookAtPoint += eyePosition;
		gaze = normalize(lookAtPoint - eyePosition);
		
		cameraRight = cross(upVector, gaze);	//U
		cameraRight = normalize(cameraRight);
		
		cameraUp = cross(gaze, cameraRight);	//V
		cameraUp = normalize(cameraUp);
		
		cameraStrafe = cross(gaze, cameraUp);	//V
		cameraStrafe = normalize(cameraStrafe);

		View->lookAt(eyePosition, lookAtPoint, upVector);

		// Draw the scene
		prog->bind();
		glUniformMatrix4fv(prog->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));

		if(!goCamera){
			glUniformMatrix4fv(prog->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		}
		else{
			SetView(prog);
		}

		glUniform3f(prog->getUniform("lightPos"), -2.0 - lightTrans, 2.0, 2.0);

		// draw the array of bunnies
		//drawBunniesBaseCode(Model);

		//centered sphere for heir modeling
		//drawSphere(Model);
		
		//draw all models
		drawDummy(Model);
		drawSpider(Model);
		drawSteps(Model);
		drawTower(Model);
		drawEyeFire(Model);
		drawEyePupil(Model);
		drawBackWall(Model);
		drawBackRightWall(Model);
		drawBackLeftWall(Model);
		drawRightWall(Model);
		drawLeftWall(Model);
		drawLeftWall2(Model);
		drawMountain(Model);
		drawMountain2(Model);
		drawMountainOnPath(Model);
		drawCocoon(Model);

		//draw the waving HM
		SetMaterial(prog, 1);
		//drawHierModel(Model);
		prog->unbind();

		//switch shaders to the texture mapping shader and draw the ground
		texProg->bind();
		glUniformMatrix4fv(texProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix()));
		
		if(!goCamera){
			glUniformMatrix4fv(texProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix()));
		}
		else{
			SetView(texProg);
		}

		glUniformMatrix4fv(texProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix()));
		glUniform3f(texProg->getUniform("lightPos"), -2.0 - lightTrans, 2.0, 2.0);
		drawGround(texProg);
		texProg->unbind();
		
		texProg->bind();
		//draw big background sphere
		glUniform1i(texProg->getUniform("flip"), -1);
		texture1->bind(texProg->getUniform("Texture0"));
		Model->pushMatrix();
			Model->loadIdentity();
			Model->scale(vec3(300.0));
			setModel(texProg, Model);
			sphere->draw(texProg);
		Model->popMatrix();
		texProg->unbind();

		// Draw PARTICLES
		partProg->bind();
		texture->bind(partProg->getUniform("alphaTexture"));
		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("P"), 1, GL_FALSE, value_ptr(Projection->topMatrix())));

		if(!goCamera){
			CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("V"), 1, GL_FALSE, value_ptr(View->topMatrix())));
		}
		else{
			SetView(texProg);
		}

		CHECKED_GL_CALL(glUniformMatrix4fv(partProg->getUniform("M"), 1, GL_FALSE, value_ptr(Model->topMatrix())));
		CHECKED_GL_CALL(glUniform3f(partProg->getUniform("pColor"), 0.9, 0.8, 0.4));
		
		thePartSystem->drawMe(partProg);
		thePartSystem->update();

		partProg->unbind();
	
		// Pop matrix stacks.
		Projection->popMatrix();
		View->popMatrix();

	}
};

int main(int argc, char *argv[])
{
	// Where the resources are loaded from
	std::string resourceDir = "../resources";

	if (argc >= 2)
	{
		resourceDir = argv[1];
	}

	Application *application = new Application();
	application->theta = -1.39626;
	application-> eyePosition = vec3(0, 0, 0);

	// Your main will always include a similar set up to establish your window
	// and GL context, etc.

	WindowManager *windowManager = new WindowManager();
	windowManager->init(960, 480);
	windowManager->setEventCallbacks(application);
	application->windowManager = windowManager;

	// This is the code that will likely change program to program as you
	// may need to initialize or set up different data and state

	application->init(resourceDir);
	application->initTex(resourceDir);
	application->initGeom(resourceDir);

	//LOCK CURSOR
	//glfwSetInputMode(windowManager->getHandle(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);  

	auto lastTime = chrono::high_resolution_clock::now();

	// Loop until the user closes the window.
	while (! glfwWindowShouldClose(windowManager->getHandle()))
	{
		// save current time for next frame
		auto nextLastTime = chrono::high_resolution_clock::now();

		// get time since last frame
		float deltaTime =
			chrono::duration_cast<std::chrono::microseconds>(
				chrono::high_resolution_clock::now() - lastTime)
				.count();
		// convert microseconds (weird) to seconds (less weird)
		deltaTime *= 0.000001;

		// reset lastTime so that we can calculate the deltaTime
		// on the next frame
		lastTime = nextLastTime;

		// Render scene.
		application->render(deltaTime);
		// Swap front and back buffers.
		glfwSwapBuffers(windowManager->getHandle());
		// Poll for and process events.
		glfwPollEvents();
	}

	// Quit program.
	windowManager->shutdown();
	return 0;
}
