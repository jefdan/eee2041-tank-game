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
int printOglError(char *file, int line);

// Global variables.

// Screen size.
int screenWidth   	        = 720;
int screenHeight   	        = 720;

//! Array of key states
bool keyStates[256];

GLuint shaderProgramID;

// Viewing/Camera.
Matrix4x4 ModelViewMatrix;		// ModelView Matrix
GLuint MVMatrixUniformLocation;		// ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;		// Projection Matrix
GLuint ProjectionUniformLocation;	// Projection Matrix Uniform Location
SphericalCameraManipulator cameraManip;

// Meshes.
Mesh tank_chassis;
Mesh box;

// Hamvee texture.
GLuint hamvee_vertex_texture_coordinate_attribute; // Vertex Texcoord Attribute Location
GLuint hamvee_texture_map_uniform_location; // Texture Map Location
GLuint hamvee_texture; // OpenGL Texture
GLuint hamvee_vertex_position_attribute;		// Vertex Position Attribute Location

// Box texture.
GLuint box_vertex_texture_coordinate_attribute;
GLuint box_texture_map_uniform_location;
GLuint box_texture;
GLuint box_vertex_position_attribute;

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
	initTexture("../models/hamvee.bmp", hamvee_texture);

	box.loadOBJ("../models/cube.obj");
	initTexture("../models/Crate.bmp", box_texture);

	//Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.f,0.f,2.f);
	cameraManip.setFocus(tank_chassis.getMeshCentroid());

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
    glutCreateWindow("Tank Assignment");
    
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
	hamvee_vertex_position_attribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	box_vertex_position_attribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
	hamvee_vertex_texture_coordinate_attribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
	hamvee_texture_map_uniform_location = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
	box_vertex_texture_coordinate_attribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
	box_texture_map_uniform_location = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
	
	// Get ModelView Matrix uniform location
	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform"); 
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform"); 
	LightPositionUniformLocation    = glGetUniformLocation(shaderProgramID, "LightPosition_uniform"); 
	AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform"); 
	SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform"); 
	SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");
    hamvee_texture_map_uniform_location       = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
	box_texture_map_uniform_location       = glGetUniformLocation(shaderProgramID, "TextureMap_uniform"); 
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
    //Handle keys
    handleKeys();

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

	//Apply Camera Manipluator to Set Model View Matrix on GPU
    ModelViewMatrix.toIdentity();

	//Apply Camera Manipluator to Set Model View Matrix on GPU
	Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
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
	glBindTexture(GL_TEXTURE_2D, hamvee_texture);
	glUniform1i(hamvee_texture_map_uniform_location, 0);

	tank_chassis.Draw(
		hamvee_vertex_position_attribute,
		vertexNormalAttribute, hamvee_vertex_texture_coordinate_attribute
	);

	// Bind box texture before drawing the box
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, box_texture);
	glUniform1i(box_texture_map_uniform_location, 0);

	box.Draw(
		box_vertex_position_attribute,
		vertexNormalAttribute, box_vertex_texture_coordinate_attribute
	);

	glUseProgram(0);

	//Swap Buffers and post redisplay
	glutSwapBuffers();
	glutPostRedisplay();
}



//! Keyboard Interaction
void keyboard(unsigned char key, int x, int y)
{
	//Quits program when esc is pressed
	if (key == 27)	//esc key code
	{
		exit(0);
	}
	else if(key == 'a')
	{
		std::cout << "a key pressed" << std::endl;
	}
	else if(key == 'b')
	{
		glClearColor(1.0,1.0,1.0,1.0);
	}
	else if(key == 'B')
	{
		glClearColor(0.0,0.0,0.0,1.0);
	}
    else if(key == 'w')
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    }
    else if(key == 'W')
    {
        glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
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
	if(keyStates['a'])
    {
        
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

