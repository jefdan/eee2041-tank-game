// Includes.
#include <GL/glew.h>
#include <GL/glut.h>
#include <Shader.h>
#include <Vector.h>
#include <Matrix.h>
#include <Mesh.h>
#include <Texture.h>
#include <SphericalCameraManipulator.h>
#include <iostream>
#include <math.h>
#include <string>
#include <vector>
#include <fstream>

// Function declarations.
bool initGL(int argc, char** argv);
void initShader();
void initGeometry();					            //Function to init Geometry 
void drawGeometry();					            //Function to draw Geometry
void display(void);
void keyboard(unsigned char key, int x, int y);
void keyUp(unsigned char key, int x, int y);
void handleKeys();
void mouse(int button, int state, int x, int y);
void motion(int x, int y);
void Timer(int value);
void initTexture(std::string filename, GLuint & textureID);
void drawMaze();
void drawTank();
void drawBall();
void fireBall();
void updateCamera();
int printOglError(char *file, int line);
void fireBallTimer(int value);
void loadLevel(const char* filename);

// Global variables.
float game_time = 0.0f; // In game timer.

int screenWidth   	        = 720;
int screenHeight   	        = 720;

bool keyStates[256];

// Tank position tracking
Vector3f tankPosition = Vector3f(0.0f, 0.0f, 0.0f);
float tankRotation = 0.0f;
float turretRotation = 0.0f;
float wheelRotation = 0.0f;

// Tank physics.
float tankVelocity = 0.0f;
float tankAcceleration = 0.005f;
float tankDeceleration = 0.01f;
float tankMaxVelocity = 0.02f;
float tankVelocityDecay = 0.9f;

// Tank rotation physics.
float tankRotationVelocity = 0.0f;
float tankRotationAcceleration = 0.1f;
float tankRotationMaxVelocity = 1.0f;
float tankRotationVelocityDecay = 0.8f;

// Turret rotation physics.
float turretVelocity = 0.0f;
float turretAcceleration = 0.1f;
float turretDeceleration = 0.2f;
float turretMaxVelocity = 1.0f;
float turretVelocityDecay = 0.8f;

// Ball properties
std::vector<Vector3f> ballPositions;
std::vector<Vector3f> ballVelocities;
std::vector<bool> ballActives;
float ballSpeed = 0.5f;
Vector3f gravity = Vector3f(0.0f, -0.01f, 0.0f);

bool canFire = true; // Add a boolean to control firing rate

GLuint shaderProgramID;
GLuint tankShaderProgramID;

// Viewing/Camera.
Matrix4x4 ModelViewMatrix;		// ModelView Matrix
GLuint MVMatrixUniformLocation;		// ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;		// Projection Matrix
GLuint ProjectionUniformLocation;	// Projection Matrix Uniform Location
SphericalCameraManipulator cameraManip;

// Meshes.
Mesh tank_chassis;
Mesh tank_front_wheel;
Mesh tank_back_wheel;
Mesh tank_turret;

Mesh box;
Mesh coin;
Mesh ball;

GLuint textureCoordinateAttribute; // Vertex Texcoord Attribute Location
GLuint textureMapUniformLocation; // Texture Map Location
GLuint vertexPositionAttribute;		// Vertex Position Attribute Location

// OpenGL textures.
GLuint hamvee_texture;
GLuint box_texture;
GLuint coin_texture;
GLuint ball_texture;

GLuint vertexNormalAttribute;	

GLuint LightPositionUniformLocation;
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

Vector3f lightPosition= Vector3f(20.0,20.0,20.0);
Vector3f ambient    = Vector3f(0.1,0.1,0.1);
Vector3f specular   = Vector3f(0.0,1.0,0.0);
float specularPower = 10.0;

int** maze;
int mazeWidth = 10;
int mazeHeight = 10;

//! Main Program Entry
int main(int argc, char** argv)
{	
	//init OpenGL
	if(!initGL(argc, argv))
		return -1;

	initShader();

	//Init Key States to false;
    for(int i = 0 ; i < 256; i++)
        keyStates[i] = false;

    // Load the level
    loadLevel("../levels/home-sweet-home.level");
    
    // Setting up my programme.
	tank_chassis.loadOBJ("../models/chassis.obj");
	tank_front_wheel.loadOBJ("../models/front_wheel.obj");
	tank_back_wheel.loadOBJ("../models/back_wheel.obj");
	tank_turret.loadOBJ("../models/turret.obj");
	initTexture("../models/hamvee.bmp", hamvee_texture);

	box.loadOBJ("../models/cube.obj");
	initTexture("../models/Crate.bmp", box_texture);

	coin.loadOBJ("../models/coin.obj");
	initTexture("../models/coin.bmp", coin_texture);

	ball.loadOBJ("../models/ball.obj");
	initTexture("../models/ball.bmp", ball_texture);

	//Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.f,0.f,2.f);
	cameraManip.setFocus(tank_chassis.getMeshCentroid());

	glClearColor(0.0,0.33,0.67,1.0);

	//Enter main loop
    glutMainLoop();

    //Delete shader program
	glDeleteProgram(shaderProgramID);

    // Deallocate memory for the maze
    for (int i = 0; i < mazeHeight; ++i) {
        delete[] maze[i];
    }
    delete[] maze;

    return 0;
}

//! Function to Initlise OpenGL
bool initGL(int argc, char** argv)
{
	//Init GLUT
    glutInit(&argc, argv);
    
	//Set Display Mode
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);

	//Set Window Size
    glutInitWindowSize(screenWidth, screenHeight);
    
    // Window Position
    glutInitWindowPosition(200, 200);

	//Create Window
    glutCreateWindow("Jeffrey's Tank Assignment");
    
    // Init GLEW
	if (glewInit() != GLEW_OK) 
	{
		std::cout << "Failed to initialize GLEW" << std::endl;
		return false;
	}
	
	//Set Display function
    glutDisplayFunc(display);
	
	//Set Keyboard Interaction Functions
	glutKeyboardFunc(keyboard);
	glutKeyboardUpFunc(keyUp); 

	//Set Mouse Interaction Functions
	glutMouseFunc(mouse);
	glutPassiveMotionFunc(motion);
	glutMotionFunc(motion);

    //Start start timer function after 100 milliseconds
    glutTimerFunc(100,Timer, 0);

	glutInitDisplayMode(GLUT_DOUBLE | GLUT_DEPTH);
	glEnable(GL_DEPTH_TEST);

	return true;
}

//Init Shader
void initShader()
{
	//Create shader
    shaderProgramID = Shader::LoadFromFile("shader.vert","shader.frag");
    
    // Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
	textureCoordinateAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
	textureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
	
	// Get ModelView Matrix uniform location
	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform"); 
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform"); 
	LightPositionUniformLocation    = glGetUniformLocation(shaderProgramID, "LightPosition_uniform"); 
	AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform"); 
	SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform"); 
	SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
}

void initTexture(std::string filename, GLuint & textureID)
{
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_2D, textureID);

	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);

	//Get texture Data
	int width, height;
	char* data;
	Texture::LoadBMP(filename, width, height, data);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
	//Cleanup data as copied to GPU
	delete[] data;
}

//! Display Loop
void display(void)
{
	game_time+= 1.0f; // Increment the in-game timer

    handleKeys();

    updateCamera(); // Update camera to look at tank.

	glViewport(0,0,screenWidth, screenHeight); // Set viewport size.
	
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    //Draw your scene

	glUseProgram(shaderProgramID);

	//Set Colour after program is in use
	glActiveTexture(GL_TEXTURE0);

	//Projection Matrix - Perspective Projection
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 100.0);
   
    //Set Projection Matrix
    glUniformMatrix4fv(	
		ProjectionUniformLocation,
		1,
		false,
		ProjectionMatrix.getPtr());

	drawMaze();
	drawTank();
	drawBall();

	glUseProgram(0);

	//Swap Buffers and post redisplay
	glutSwapBuffers();
	glutPostRedisplay();
}

void drawMaze()
{
	for (int i = 0; i < mazeHeight; i++)
	{
		for (int j = 0; j < mazeWidth; j++)
		{
			if (maze[i][j] >= 1)
			{
				ModelViewMatrix.toIdentity();

				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				m.translate(i*2, 0, j*-2);
				glUniformMatrix4fv(
					MVMatrixUniformLocation,
					1,
					false,
					m.getPtr());

				glUniform3f(LightPositionUniformLocation, lightPosition.x,lightPosition.y,lightPosition.z);
				glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
				glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
				glUniform1f(SpecularPowerUniformLocation, specularPower);
				
				// Bind hamvee texture before drawing the tank chassis
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, box_texture);
				glUniform1i(textureMapUniformLocation, 0);
			
				box.Draw(
					vertexPositionAttribute,
					vertexNormalAttribute, textureCoordinateAttribute
				);
			}
			if (maze[i][j] >= 2)
			{
				ModelViewMatrix.toIdentity();

				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				m.translate(i*2, 2, j*-2);
				m.rotate(game_time, 0, 1, 0); 
				glUniformMatrix4fv(
					MVMatrixUniformLocation,
					1,
					false,
					m.getPtr());

				glUniform3f(LightPositionUniformLocation, lightPosition.x,lightPosition.y,lightPosition.z);
				glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
				glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
				glUniform1f(SpecularPowerUniformLocation, specularPower);
				
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, coin_texture);
				glUniform1i(textureMapUniformLocation, 0);
			
				coin.Draw(
					vertexPositionAttribute,
					vertexNormalAttribute, textureCoordinateAttribute
				);
			}
		}
	}
}

void drawTank()
{
	// Decay velocity.
	tankVelocity *= tankVelocityDecay;
	turretVelocity *= turretVelocityDecay;
	tankRotationVelocity *= tankRotationVelocityDecay;

	// Move tank based on velocity.
	if(tankVelocity != 0.0f) {
		float radians = -tankRotation * (M_PI / 180.0f);
		tankPosition.x -= sin(radians) * tankVelocity;
		tankPosition.z += cos(radians) * tankVelocity;

		wheelRotation += tankVelocity * 50.0f;
		
		updateCamera();
	}

	if(turretVelocity != 0.0f) {
		turretRotation += turretVelocity;
		
		updateCamera();
	}

	if(tankRotationVelocity != 0.0f) {
		tankRotation += tankRotationVelocity;
		
		updateCamera();
	}

	ModelViewMatrix.toIdentity();

	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);

	m.translate(tankPosition.x, 0.75, tankPosition.z);
	m.rotate(tankRotation, 0, 1, 0);
	m.scale(0.5, 0.5, 0.5);

	glUniformMatrix4fv(
		MVMatrixUniformLocation,
		1,
		false,
		m.getPtr());

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hamvee_texture);
	glUniform1i(textureMapUniformLocation, 0);

	tank_chassis.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);

	tank_front_wheel.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);

	tank_back_wheel.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);

	Matrix4x4 o = cameraManip.apply(ModelViewMatrix);

	o.translate(tankPosition.x, 0.75, tankPosition.z);
	o.rotate(turretRotation, 0, 1, 0);
	o.scale(0.5, 0.5, 0.5);

	glUniformMatrix4fv(
		MVMatrixUniformLocation,
		1,
		false,
		o.getPtr());

	tank_turret.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);
}

void drawBall() {
	for (int i = 0; i < ballPositions.size(); i++)
	{
		if (ballActives[i]) {
			ModelViewMatrix.toIdentity();
			Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
			m.translate(ballPositions[i].x, ballPositions[i].y, ballPositions[i].z);
			m.scale(0.2, 0.2, 0.2);

			glUniformMatrix4fv(
				MVMatrixUniformLocation,
				1,
				false,
				m.getPtr());

			glUniform3f(LightPositionUniformLocation, lightPosition.x, lightPosition.y, lightPosition.z);
			glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
			glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
			glUniform1f(SpecularPowerUniformLocation, specularPower);

			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, ball_texture);
			glUniform1i(textureMapUniformLocation, 0);

			ball.Draw(
				vertexPositionAttribute,
				vertexNormalAttribute, textureCoordinateAttribute
			);

			ballVelocities[i] = ballVelocities[i] + gravity;
			ballPositions[i] = ballPositions[i] + ballVelocities[i];

			if (
				ballPositions[i].x > 20 || // If the ball is out of bounds...
				ballPositions[i].x < -20 ||
				ballPositions[i].y < 0 ||
				ballPositions[i].z > 20 ||
				ballPositions[i].z < -20)
			{
				// Get rid of the oldest ball in the array.
				ballActives.erase(ballActives.begin() + i);
				ballPositions.erase(ballPositions.begin() + i);
				ballVelocities.erase(ballVelocities.begin() + i);
				i--;
			}
		}
	}
}

// Function to fire the ball
void fireBall() {
    ballActives.push_back(true);
    float radians = -turretRotation * (M_PI / 180.0f);
	float ballDistance = 3.0f;
    ballPositions.push_back(tankPosition + Vector3f(-sin(radians) * ballDistance, 2.0f, cos(radians) * ballDistance)); // Initial position of the ball

    // Calculate the direction based on turret rotation
    radians = -turretRotation * (M_PI / 180.0f);
    ballVelocities.push_back(Vector3f(-sin(radians), 0.0f, cos(radians)) * ballSpeed);
}

//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	if (key == 27) // Quit when ESC pressed.
	{
		exit(0);
	}

	glutPostRedisplay();

    keyStates[key] = true;
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}


//! Handle Keys
void handleKeys()
{    
    if (fabs(tankVelocity) < 0.001f) {
        tankVelocity = 0.0f;
    }

	if (fabs(turretVelocity) < 0.001f) {
        turretVelocity = 0.0f;
    }
    
    if(keyStates['w']) // Accelerate tank forward.
    {
        tankVelocity += tankAcceleration;

		// Max movement speed.

        if(tankVelocity > tankMaxVelocity) {
            tankVelocity = tankMaxVelocity;
        }
    }
    else if(keyStates['s']) // Accelerate backwards.
    {
        tankVelocity -= tankAcceleration;

        if(tankVelocity < -tankMaxVelocity) {
            tankVelocity = -tankMaxVelocity;
        }
    }
    
    if(keyStates['a']) // Rotate left.
    {
		tankRotationVelocity += tankRotationAcceleration;
		if (tankRotationVelocity > tankRotationMaxVelocity) tankRotationVelocity = tankRotationMaxVelocity;

		// Also need to rotate the turret.

		turretVelocity += turretAcceleration;
		if (turretVelocity > turretMaxVelocity) turretVelocity = turretMaxVelocity;

        updateCamera();
    }
    if(keyStates['d']) // Rotate right.
    {
		tankRotationVelocity -= tankRotationAcceleration;
		if (tankRotationVelocity < -tankRotationMaxVelocity) tankRotationVelocity = -tankRotationMaxVelocity;

		turretVelocity -= turretAcceleration; // Rotate right
		if (turretVelocity < -turretMaxVelocity) turretVelocity = -turretMaxVelocity;
        updateCamera();
    }
    if(keyStates['j']) // Rotate turret left.
    {
        turretVelocity += turretAcceleration;
		if (turretVelocity > turretMaxVelocity) turretVelocity = turretMaxVelocity;
        updateCamera();
    }
    if(keyStates['l']) // Rotate turret right.
    {
        turretVelocity -= turretAcceleration;
		if (turretVelocity < -turretMaxVelocity) turretVelocity = -turretMaxVelocity;
        updateCamera();
    }
	if(keyStates['k'] && canFire)
	{
		fireBall();
		canFire = false;
		glutTimerFunc(200, fireBallTimer, 0);
	}
}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
	cameraManip.handleMouse(button, state,x,y);
	glutPostRedisplay();
}

//! Motion
void motion(int x, int y)
{
	cameraManip.handleMouseMotion(x,y);
	glutPostRedisplay();
}

//! Timer Function
void Timer(int value)
{
    //Call function again after 10 milli seconds
	glutTimerFunc(10,Timer, 0);
}

int printOglError(char *file, int line)
{
	GLenum glErr;
	int retCode = 0;
	glErr = glGetError();
	if (glErr != GL_NO_ERROR)
	{
		printf("glError in file %s @ line %d: %s\n",
		file, line, gluErrorString(glErr));
		retCode = 1;
	}
return retCode;
}

void updateCamera() // Update the camera to focus on the tank.
{
    Vector3f tankWorldPos = Vector3f(tankPosition.x, 0.75, tankPosition.z);
    cameraManip.setFocus(tankWorldPos);
	cameraManip.setPanTiltRadius(turretRotation/(180/M_PI), -1.0f, 5.0f);
}

void fireBallTimer(int value)
{
	canFire = true;
}

void loadLevel(const char* filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open level file: " << filename << std::endl;
        // Handle error, perhaps load a default level or exit
        return;
    }

    std::string line;
    std::vector<std::vector<int>> tempMaze;

    while (std::getline(file, line)) {
        std::vector<int> row;
        for (char c : line) {
            if (isdigit(c)) {
                row.push_back(c - '0');
            }
        }
        tempMaze.push_back(row);
    }

    mazeHeight = tempMaze.size();
    mazeWidth = tempMaze[0].size();

    // Dynamically allocate memory for the maze
    maze = new int*[mazeHeight];
    for (int i = 0; i < mazeHeight; ++i) {
        maze[i] = new int[mazeWidth];
        if (maze[i] == nullptr) {
            std::cerr << "Memory allocation failed!" << std::endl;
            // Handle the error appropriately, e.g., exit the program
            return;
        }
    }

    // Copy the data to the maze array
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            maze[i][j] = tempMaze[i][j];
        }
    }

    file.close();
}
