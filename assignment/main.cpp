//!Includes
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

//!Function Prototypes
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
void updateCamera(); // New function to update the camera focus
int printOglError(char *file, int line);

// Global variables.
float game_time = 0.0f; // In game timer.

// Screen size.
int screenWidth   	        = 720;
int screenHeight   	        = 720;

//! Array of key states
bool keyStates[256];

// Tank position tracking
Vector3f tankPosition = Vector3f(0.0f, 0.0f, 0.0f);
float tankSpeed = 0.1f;
float rotationSpeed = 3.0f;
float tankRotation = 0.0f;
float turretRotation = 0.0f;
float wheelRotation = 0.0f;

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

// Hamvee texture.
GLuint textureCoordinateAttribute; // Vertex Texcoord Attribute Location
GLuint textureMapUniformLocation; // Texture Map Location
GLuint vertexPositionAttribute;		// Vertex Position Attribute Location
GLuint hamvee_texture; // OpenGL Texture
GLuint box_texture; // OpenGL Texture
GLuint coin_texture; // OpenGL Texture

GLuint vertexNormalAttribute;	

//Material Properties
GLuint LightPositionUniformLocation;                // Light Position Uniform   
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

Vector3f lightPosition= Vector3f(20.0,20.0,20.0);   // Light Position 
Vector3f ambient    = Vector3f(0.1,0.1,0.1);
Vector3f specular   = Vector3f(0.0,1.0,0.0);
float specularPower = 10.0;

// Maze
int maze[10][10] = {
	{2,1,1,1,1,1,1,1,1,2},
	{1,0,0,0,0,0,0,0,0,1},
	{1,0,1,1,1,1,1,1,0,1},
	{1,0,1,0,0,0,0,1,0,1},
	{1,0,1,0,1,1,0,1,0,1},
	{1,0,1,0,0,1,0,1,0,1},
	{1,0,1,1,0,1,0,1,0,1},
	{1,0,0,0,0,1,0,0,0,1},
	{1,1,1,1,2,2,1,1,1,1},
};

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

	//Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.f,0.f,2.f);
	cameraManip.setFocus(tank_chassis.getMeshCentroid());

	glClearColor(0.0,0.33,0.67,1.0);

	//Enter main loop
    glutMainLoop();

    //Delete shader program
	glDeleteProgram(shaderProgramID);

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

    //Handle keys
    handleKeys();

    // Update camera to follow tank
    updateCamera();

	//Set Viewport
	glViewport(0,0,screenWidth, screenHeight);
	
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
		ProjectionUniformLocation,  //Uniform location
		1,							//Number of Uniforms
		false,						//Transpose Matrix
		ProjectionMatrix.getPtr());	//Pointer to ModelViewMatrixValues

	drawMaze();
	drawTank();


	glUseProgram(0);

	//Swap Buffers and post redisplay
	glutSwapBuffers();
	glutPostRedisplay();
}

void drawMaze()
{
	for (int i = 0; i < 10; i++)
	{
		for (int j = 0; j < 10; j++)
		{
			if (maze[i][j] >= 1)
			{
				//Apply Camera Manipluator to Set Model View Matrix on GPU
				ModelViewMatrix.toIdentity();

				//Apply Camera Manipluator to Set Model View Matrix on GPU
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				m.translate(i*2, 0, j*-2);
				glUniformMatrix4fv(
					MVMatrixUniformLocation, //Uniform location
					1, //Number of Uniforms
					false, //Transpose Matrix
					m.getPtr()); //Pointer to Matrix Values

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
				//Apply Camera Manipluator to Set Model View Matrix on GPU
				ModelViewMatrix.toIdentity();

				//Apply Camera Manipluator to Set Model View Matrix on GPU
				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				m.translate(i*2, 2, j*-2);
				m.rotate(game_time, 0, 1, 0); 
				glUniformMatrix4fv(
					MVMatrixUniformLocation, //Uniform location
					1, //Number of Uniforms
					false, //Transpose Matrix
					m.getPtr()); //Pointer to Matrix Values

				glUniform3f(LightPositionUniformLocation, lightPosition.x,lightPosition.y,lightPosition.z);
				glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
				glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
				glUniform1f(SpecularPowerUniformLocation, specularPower);
				
				// Bind hamvee texture before drawing the tank chassis
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
	//Apply Camera Manipluator to Set Model View Matrix on GPU
	ModelViewMatrix.toIdentity();

	//Apply Camera Manipluator to Set Model View Matrix on GPU
	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);

	m.translate(tankPosition.x, 0.75, tankPosition.z);
	m.rotate(tankRotation, 0, 1, 0); // Apply tank rotation around Y axis
	m.scale(0.5, 0.5, 0.5);

	glUniformMatrix4fv(
		MVMatrixUniformLocation, //Uniform location
		1, //Number of Uniforms
		false, //Transpose Matrix
		m.getPtr()); //Pointer to Matrix Values

	// Bind box texture before drawing the box
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hamvee_texture);
	glUniform1i(textureMapUniformLocation, 0);

	tank_chassis.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);

	// //Apply Camera Manipluator to Set Model View Matrix on GPU
	// Matrix4x4 n = cameraManip.apply(ModelViewMatrix);

	// n.translate(tankPosition.x, 0.5, tankPosition.z);
	// n.rotate(wheelRotation, cos(tankRotation), 0, sin(tankRotation)); // Apply tank rotation around Y axis

	// glUniformMatrix4fv(
	// 	MVMatrixUniformLocation, //Uniform location
	// 	1, //Number of Uniforms
	// 	false, //Transpose Matrix
	// 	n.getPtr()); //Pointer to Matrix Values


	tank_front_wheel.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);
	tank_back_wheel.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);

	//Apply Camera Manipluator to Set Model View Matrix on GPU
	Matrix4x4 o = cameraManip.apply(ModelViewMatrix);

	o.translate(tankPosition.x, 0.75, tankPosition.z);
	o.rotate(turretRotation, 0, 1, 0); // Apply tank rotation around Y axis
	o.scale(0.5, 0.5, 0.5);

	glUniformMatrix4fv(
		MVMatrixUniformLocation, //Uniform location
		1, //Number of Uniforms
		false, //Transpose Matrix
		o.getPtr()); //Pointer to Matrix Values

	tank_turret.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);
}


//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	//Quits program when esc is pressed
	if (key == 27)	//esc key code
	{
		exit(0);
	}

	glutPostRedisplay();

	//Set key status
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
    //keys should be handled here
	if(keyStates['w'])
    {
        // Move tank forward in the direction it's facing
		float radians = -tankRotation * (M_PI / 180.0f);
		tankPosition.x -= sin(radians) * tankSpeed;
		tankPosition.z += cos(radians) * tankSpeed;

		wheelRotation += 5.0f; // Rotate wheels forward
		
		// Update camera focus to follow tank
		updateCamera();
    }
	if(keyStates['a'])
	{
		tankRotation += rotationSpeed; // Rotate left
		// Update camera focus after rotation
		turretRotation += rotationSpeed; // Rotate turret left
		updateCamera();
	}
	if(keyStates['s'])
	{
		// Move tank backward in the direction it's facing
		float radians = -tankRotation * (M_PI / 180.0f);
		tankPosition.x += sin(radians) * tankSpeed;
		tankPosition.z -= cos(radians) * tankSpeed;

		wheelRotation -= 5.0f; // Rotate wheels backward
		
		// Update camera focus after moving backwards
		updateCamera();
	}
	if(keyStates['d'])
	{
		tankRotation -= rotationSpeed; // Rotate right
		// Update camera focus after rotation
		turretRotation -= rotationSpeed; // Rotate turret right
		updateCamera();
	}
	if(keyStates['j'])
	{
		turretRotation += rotationSpeed; // Rotate right
		// Update camera focus after rotation
		updateCamera();
	}
	if(keyStates['l'])
	{
		turretRotation -= rotationSpeed; // Rotate right
		// Update camera focus after rotation
		updateCamera();
	}
}

//! Mouse Interaction
void mouse(int button, int state, int x, int y)
{
    // glutPostRedisplay(); 
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

// Update camera to follow tank
void updateCamera() 
{
    // Calculate the position where the tank is
    Vector3f tankWorldPos = Vector3f(tankPosition.x, 0.75, tankPosition.z); // Adjust Y value to match approximate tank height
    cameraManip.setFocus(tankWorldPos);
	cameraManip.setPanTiltRadius(turretRotation/(180/M_PI), -1.0f, 5.0f);
	// cameraManip.setFocus(tank_chassis.getMeshCentroid());
}

