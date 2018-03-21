
#define _USE_MATH_DEFINES
#include <cmath>
// GLEW
#include <GL/glew.h>

// GLFW
#include <GLFW/glfw3.h>

// GL includes
#include "shader.h"
#include "camera.h"
#include "model.h"

// GLM Mathemtics
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>

/*
TODO 
	DONE Implement second attempt at each frame 
	Implement sound
	Implement pin rotation 
*/
struct {//Struct associating all information needed by the ball
	Model model;
	vector<glm::vec3> minMaxVector;
	glm::vec4 center; 
	glm::mat4 modelMatrix;
	glm::vec3 movementVector;
	
}typedef BallModel;


struct {//Strruct associating all info needed by the PIN
	Model model;
	glm::mat4 modelMatrix;
	glm::vec3 movementVector;
	vector<glm::vec3> minMaxVector;
	glm::vec4 center;
	GLfloat angle;
	GLfloat collision;
}typedef PinInfo;


/*
TODO create bounded box for all objects.
*/
const glm::vec3 INITIAL_CAMERA_POSITION = glm::vec3(0.0f, 1.25f, 13.0f);
// Active window
GLFWwindow* window;

// Properties
GLuint sWidth = 2560, sHeight = 1440;

// Camera
Camera camera(INITIAL_CAMERA_POSITION);

vector<PinInfo> pins;
// Key codes in use
const GLfloat BALL_Z = 105.0f;//the initial z position of the ball 


const glm::vec3 ballScale(0.10f);//scale of ball


bool keys[1024];

bool flags[10];
/**
0 -> reset stage
1 -> reset pins
2 -> Allow camera movement
3 -> Check for collision between pins
4 -> End of First Attempt
5 -> Second attempt
6 -> End of second attempt
*/

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode);
bool rollBall(glm::mat4 &ballModel, GLboolean moveBall);//performs all the physics for the bowling ball 
bool checkCollision(BallModel &m1, PinInfo &m2);//checks to see if two models are colliding
void init_Resources();//Initializes the window ans sets up all resources
void generatePins();//Initializes the Pins vector
void drawPins(Shader&);//draws the pin
vector<glm::vec3> findBoundedBox(Model m1);//finds the minimum and maximum values for x,y,z
glm::vec4 findCenter(vector<glm::vec3>);//finds the center of the object
bool checkCollision();//checks collision between pins
void mouse_callback(GLFWwindow* window, double xpos, double ypos);//move camera around



int main() {
	init_Resources();

	
	BallModel ballInfo = {
		Model((GLchar*)"Orange Bowling Ball.obj"),
		findBoundedBox(ballInfo.model),//finds the min and max value of x,y,z
		findCenter(ballInfo.minMaxVector),//finds center of the object
		glm::mat4(0),//Model Matrix
		glm::vec3(0,0,SPEED),//translation for object
	};//creation of BALL

	//camera.setMovementSpeed(0.08f);
	//Model ball((GLchar*)"Orange Bowling Ball.obj");//bowling ball model
	Model alley((GLchar*)"bowlingAlley.obj");//bowling alley model
	generatePins();//

//	std::cout << ballInfo.model.meshes.size() << endl;
//	std::cout << pins.at(0).model.meshes.size() << endl;
//	std::cout << alley.meshes.size() << endl;


	Shader ballShader("ballVertex.glsl", "ballFragment.glsl");//Ball Shader
	Shader alleyShader("Bowling_AlleyVertex.glsl","Bowling_AlleyFragment.glsl");//Bowling alley Shader
	Shader pinShader("pinVertex.glsl","pinFragment.glsl");
	glm::mat4 projection = glm::perspective(45.0f, (GLfloat)sWidth / (GLfloat)sHeight, 1.0f, 10000.0f);

	ballShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	
	alleyShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(alleyShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	pinShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(pinShader.Program, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	flags[2] = true;
	GLboolean moveBall = GL_FALSE;
	GLfloat angle = 0.01f;
	while (!glfwWindowShouldClose(window)) {
		// Check and call events
		glfwPollEvents();
		if (keys[GLFW_KEY_SPACE]) {//starts the ball movement
			moveBall = GL_TRUE;

			flags[2] = false;
			camera.setPitch(PITCH);
			camera.setYaw(YAW);
			//camera.setMovementSpeed(1.5f);
		}
		if (keys[GLFW_KEY_END]) {//pause movement of ball
			moveBall = GL_FALSE;
			flags[2] = true;
			camera.setPitch(PITCH);
			camera.setYaw(YAW);
		}
		if (keys[GLFW_KEY_ENTER] && flags[4]) {//End of first Frame
			flags[0] = GL_TRUE;
			flags[3] = GL_TRUE;
			flags[5] = GL_TRUE;
			moveBall = false;
			flags[2] = true;
			camera.setPitch(PITCH);
			camera.setYaw(YAW);
		}
		if (keys[GLFW_KEY_ENTER] && flags[6]) {//Resets stage after second frame
			flags[0] = GL_TRUE;
			flags[1] = GL_TRUE;
			flags[3] = GL_FALSE;
			flags[4] = GL_FALSE;
			flags[5] = GL_FALSE;
			flags[6] = GL_FALSE;
			moveBall = GL_FALSE;
			flags[2] = !true;
			camera.setPitch(PITCH);
			camera.setYaw(YAW);
		}


		if (keys[GLFW_KEY_V] && !moveBall && flags[2]) {//stops the camera movement
			flags[2] = false;
			camera.setPitch(PITCH);
			camera.setYaw(YAW);
		}
		else if (keys[GLFW_KEY_C] && !moveBall) {//allows camera movement
			flags[2] = true;
			camera.setPitch(PITCH);
			camera.setYaw(YAW);
		}
		

		// Clear buffers
		glClearColor(0.8f, 0.8f, 0.8f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		alleyShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(alleyShader.Program, "view"), 1,
			GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
		

		ballShader.Use();
		glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "view"), 1,
			GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));

		
		
		pinShader.Use();
		drawPins(pinShader);
		alleyShader.Use();

		glm::mat4 alleyModel;
		//alleyModel = glm::scale(alleyModel, glm::vec3(2, 0, 0));
		alleyModel = glm::translate(alleyModel, glm::vec3(0.0f, 0.0f, 0.0f));
		glUniformMatrix4fv(glGetUniformLocation(alleyShader.Program, "model"), 1,
			GL_FALSE, glm::value_ptr(alleyModel));
		alley.Draw(alleyShader);
		
		ballShader.Use();
		glm::mat4 ballModel;
		angle+=0.01f;
		ballModel = glm::scale(ballModel, ballScale);
		if (rollBall(ballModel, moveBall) && keys[GLFW_KEY_R]) {//roll ball handles translation of the ball model
			//the play can be reset after the first frame by using R
			flags[0] = GL_TRUE;
			flags[1] = GL_TRUE;
			flags[3] = GL_FALSE;
			flags[4] = GL_FALSE;
			flags[5] = GL_FALSE;
			moveBall = GL_FALSE;
			flags[2] = true;
			moveBall = false;
			
			//std::cout << "Ran" << std::endl;
		}

		ballModel = glm::rotate(ballModel, angle , glm::vec3(0.0f, 1.0f, 0.0f));
		ballInfo.modelMatrix = ballModel;
		glUniformMatrix4fv(glGetUniformLocation(ballShader.Program, "model"), 1, GL_FALSE, glm::value_ptr(ballModel));

		ballInfo.model.Draw(ballShader);
		//std::cout << "Looped" << std::endl;
		//

		for(int i = 0; i<pins.size(); i++)
			flags[3] = checkCollision(ballInfo, pins.at(i)) || flags[3];
		
		if (flags[3])
			checkCollision();

		glfwSwapBuffers(window);
	}
	return 0;
}



void init_Resources()
{
	// Init GLFW
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

	// Define the window
	window = glfwCreateWindow(sWidth, sHeight, "COMP3260 - Bowling", 0, 0);
	glfwMakeContextCurrent(window);


	// Initialize GLEW to setup the OpenGL Function pointers
	glewExperimental = GL_TRUE;
	glewInit();

	// Define the viewport dimensions
	glViewport(0, 0, sWidth, sHeight);

	// Setup OpenGL options
	glEnable(GL_DEPTH_TEST);
	glfwSetKeyCallback(window, key_callback);

	glfwSetCursorPosCallback(window, mouse_callback);

	// Options
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//angles.push_back()
	// Populate the spheres with independent characteristics
}


bool rollBall(glm::mat4 &ballModel, GLboolean moveBall) {
	static const glm::vec3 translationVector(0.0f, 1.0f, BALL_Z);
	static GLfloat xoffsetSpeed = 0.4f;
	static GLfloat xoffset = 0.0f;
	static GLfloat lastFrame = glfwGetTime();
	static GLfloat movement = 0.0f;
	
	GLfloat currentTime = glfwGetTime();
	GLfloat deltaTime = currentTime - lastFrame;
	lastFrame = currentTime;
	
	if (flags[0]) {
		//std::cout << "Ran" << std::endl;
		flags[0] = GL_FALSE;
		xoffset = 0.0f;
		movement = 0.0f;
		camera.Position = INITIAL_CAMERA_POSITION;
	}

	if (moveBall &&  movement >= -210){//after the space bar is launched the ball moves towards the pins
		
	
		
		camera.setPitch(PITCH);
		ballModel = glm::translate(ballModel, translationVector + glm::vec3(xoffset , 0 ,movement));
		movement -= (SPEED * 10 * deltaTime);//velocity tied to the frame time 
		//multiplied by 10 
		camera.ProcessKeyboard(FORWARD, deltaTime);//moves camera forward at same speed as the ball  
		camera.setPitch(-20.0f);//sets the pitch to face down towards the ball
		//std::cout << movement << std::endl;

	
	}	
	else { //moves ball side to side 
		//camera.setPitch(PITCH);
 
		if (movement == 0) {
			ballModel = glm::translate(ballModel, translationVector + glm::vec3(xoffset, 0, 0));
			//side to side moveemnt
			if (xoffset >= 6.0f || xoffset <= -6.0f) xoffsetSpeed *= -1;

			xoffset += xoffsetSpeed;

		}
		else {
			ballModel = glm::translate(ballModel, translationVector + glm::vec3(xoffset, 0, movement));
			//std::cout << movement << std::endl;
			flags[4] = true;//ball stops moving hence the first attempt is over
			flags[6] = flags[4] && flags[5];
		}
		

	}

	return movement <= -210;
}


void key_callback(GLFWwindow* window, int key, int scancode, int action, int mode) {


	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GL_TRUE);

	if (action == GLFW_PRESS)
		keys[key] = true;
	else if (action == GLFW_RELEASE)
		keys[key] = false;
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (!flags[2])//disables camera movement 
		return;
	static GLfloat lastX;
	static GLfloat lastY;
	static bool firstMouse = true;
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	GLfloat xoffset = xpos - lastX;
	GLfloat yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}


void generatePins() {


	for (int i = 0; i < 10; i++) {
		//Model pinModel((GLchar*)"bowling_pin_000.obj");//bowling alley model
		PinInfo temp{ Model((GLchar*)"bowling_pin_000.obj") ,glm::mat4(0), glm::vec3(0), findBoundedBox(temp.model), findCenter(temp.minMaxVector), 0.0f, GL_FALSE };
		pins.push_back(temp);//stores the info for each pin
	}
	
}

void drawPins(Shader& pinShader) {

	static GLfloat lastFrame = glfwGetTime();

	GLfloat currentTime = glfwGetTime();
	GLfloat deltaTime = currentTime - lastFrame;
	lastFrame = currentTime;


	const GLfloat yOffset = 2.0f;
	GLfloat xOffset = 0.0f;
	const GLfloat zPos = -95.0f;
	const GLfloat zOffset = -3.0f;


	glm::vec3 pinTranslation(0.0f, yOffset, zPos);
	pinShader.Use();
	glUniformMatrix4fv(glGetUniformLocation(pinShader.Program, "view"), 1,
		GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
	glm::mat4 model[10];
	for (int i = 0; i <= 9; i++) {
		
		if (flags[1]) {
			pins.at(i).movementVector = glm::vec3(0.0f);
			pins.at(i).collision = GL_FALSE;
		
		}


		pinShader.Use();
		if (i == 1) {
			pinTranslation = glm::vec3(1.0f, yOffset,zPos + zOffset );//arranging pins in initial location
			xOffset = 0;
		} 
		else if (i == 3) {
			pinTranslation = glm::vec3(2.0f, yOffset, zPos + (2*zOffset));
			xOffset = 0;
		}
		else if (i == 6) {
			pinTranslation = glm::vec3(3.0f, yOffset, zPos + (3*zOffset));
			xOffset = 0;
		}

		
		if (!pins.at(i).collision || !flags[5]) {//checks to see if this is the second attempt
			//if the pin has already experienced a collision it will not be displayed during the second attempt
			
			model[i] = glm::scale(model[i], glm::vec3(0.1));
			model[i] = glm::translate(model[i], pinTranslation +  glm::vec3(xOffset,0.0f,0.0f) + pins.at(i).movementVector );

			if (pins.at(i).collision) {
				int xSign = pins.at(i).movementVector.x < 0 ? -1 : 1;
				int ySign = pins.at(i).movementVector.z < 0 ? -1 : 1;
				pins.at(i).movementVector += glm::vec3(SPEED * 4 * deltaTime , 0, -2 * SPEED * deltaTime );
			}
			glUniformMatrix4fv(glGetUniformLocation(pinShader.Program, "model"), 1,
				GL_FALSE, glm::value_ptr(model[i]));

		

			pins.at(i).model.Draw(pinShader);
			pins.at(i).modelMatrix = model[i];
		}
		//std::cout << i << std::endl;
		xOffset -= 2;

	}

	flags[1] = GL_FALSE;


}

bool checkCollision(BallModel &m1, PinInfo &m2) {
	
	//std::cout << glm::distance(camera.GetViewMatrix() * m1.modelMatrix * m1.center, camera.GetViewMatrix()* m2.modelMatrix * m2.center) << std::endl;
	glm::vec4 m1Center = m1.modelMatrix * m1.center;
	glm::vec4 m1Max = m1.modelMatrix * glm::vec4(m1.center.x, m1.center.y, m1.minMaxVector.at(1).z,1);
	glm::vec4 m2Center =  m2.modelMatrix * m2.center;
	glm::vec4 m2Max = m2.modelMatrix * glm::vec4(m1.center.x, m2.center.y, m2.minMaxVector.at(1).z, 1);

	GLfloat minDistance = glm::distance(m1Center, m1Max)+glm::distance(m2Center, m2Max);
	
		//using namespace std; 

		//cout << minDistance << "\t";
	GLfloat distance = glm::distance(camera.GetViewMatrix() * m1.modelMatrix * m1.center, camera.GetViewMatrix()* m2.modelMatrix * m2.center);
		
	if (distance < minDistance) {
		glm::vec3 translationVector =  glm::vec3(m2Center.x, m2Center.y, m2Center.z) + glm::vec3(m1Center.x, m1Center.y, m1Center.z) ;
		//translationVector *= m1.movementVector;
		translationVector = glm::vec3((translationVector.x / distance)*SPEED/3, 0, (translationVector.z / distance)*SPEED/3);
		std::cout<<translationVector.x<<std::endl;
		m2.movementVector += translationVector;
		m2.collision = GL_TRUE;

		return GL_TRUE;
	}
	return GL_FALSE; 
}

bool checkCollision() {
	//std::cout << pins.size() << std::endl;
	//std::cout << "ran" << std::endl;
	
	for (int i = 0; i < pins.size(); i++) {
		PinInfo &m1 = pins.at(i);

		for (int j = i + 1; j < pins.size(); j++) {
			PinInfo &m2 = pins.at(j);

			glm::vec4 m1Center = m1.modelMatrix * m1.center;
			glm::vec4 m1Max = m1.modelMatrix * glm::vec4(m1.center.x, m1.center.y, m1.minMaxVector.at(1).z, 1);
			glm::vec4 m2Center = m2.modelMatrix * m2.center;
			glm::vec4 m2Max = m2.modelMatrix * glm::vec4(m2.center.x, m2.center.y, m2.minMaxVector.at(1).z, 1);

			GLfloat minDistance = glm::distance(m1Center, m1Max) + glm::distance(m2Center, m2Max);
			
				//using namespace std; 

				//cout << minDistance << "\t";
			GLfloat distance = glm::distance(camera.GetViewMatrix() * m1.modelMatrix * m1.center, camera.GetViewMatrix()* m2.modelMatrix * m2.center);

			if (distance < minDistance) {
				glm::vec3 translationVector = glm::vec3(m2Center.x, m2Center.y, m2Center.z) + glm::vec3(m1Center.x, m1Center.y, m1Center.z);
				translationVector *= m1.movementVector;
				m2.movementVector += translationVector;
					
				//std::cout << "collision" << std::endl;
				m1.collision = GL_TRUE;
				m2.collision = GL_TRUE;
			}

			
		}
	}

	return GL_FALSE;

}


vector<glm::vec3> findBoundedBox(Model m1) {

	vector<Vertex> vertices = m1.meshes.at(0).vertices;

	glm::vec3 minVertex = vertices.at(0).Position;
	glm::vec3 maxVertex = vertices.at(0).Position;

	for (int i = 1; i < vertices.size(); i++) {

		glm::vec3 &vertex = vertices.at(i).Position;

		if (vertex.x > maxVertex.x) {// checking for min or max value of x
			maxVertex.x = vertex.x;
		}
		else if (vertex.x < minVertex.x) {
			minVertex.x = vertex.x;
		}

		if (vertex.y > maxVertex.y) {// checking for min or max value of y
			maxVertex.y = vertex.y;
		}
		else if (vertex.y < minVertex.y) {
			minVertex.y = vertex.y;
		}

		if (vertex.z > maxVertex.z) {// checking for min or max value of z
			maxVertex.z = vertex.z;
		}
		else if (vertex.z < minVertex.z) {
			minVertex.z = vertex.z;
		}

	}
	vector<glm::vec3> MinMaxVertex;
	MinMaxVertex.push_back(minVertex);
	MinMaxVertex.push_back(maxVertex);

	return MinMaxVertex;


}


glm::vec4 findCenter(vector<glm::vec3> minMaxVector) {
	glm::vec4 center(1.0f);

	center.x = (minMaxVector.at(0).x + minMaxVector.at(1).x) / 2;
	center.y = (minMaxVector.at(0).y + minMaxVector.at(1).y) / 2;
	center.z = (minMaxVector.at(0).z + minMaxVector.at(1).z) / 2;

	return center;
}