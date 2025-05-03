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
#include <cstring>
#include <vector>
#include <fstream>
#include <iomanip>
#include <sstream>
#include <array>
#include <random>

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
bool initCubemapTexture(const std::vector<std::string>& faces, GLuint& textureID);
void drawMaze();
void drawTank();
void drawBall();
void drawSkybox();
void fireBall();
void updateCamera();
int printOglError(char *file, int line);
void fireBallTimer(int value);
void loadLevel(const char* filename);
void generateRandomLevel();
void render2dText(std::string text, float r, float g, float b, float x, float y);

// Global variables.
float game_time = 0.0f; // In game timer.
int score = 0;
std::string levelName = "holy-grail.level"; // Holy grail is the default level.
std::string reloadBar;

// Level Selection.
std::vector<std::string> levelFiles;
std::vector<std::string> levelDisplayNames;
bool showLevelSelection = false;

int screenWidth   	        = 720;
int screenHeight   	        = 720;

float mazeX = 0.0;
float mazeZ = 0.0;
int mazeValue = 0;
bool shouldFall = false;
int ammo = 15;

bool keyStates[256];

// Tank position tracking.
Vector3f tankPosition = Vector3f(0.0f, 0.0f, 0.0f);
float tankRotation = 0.0f;
float turretRotation = 0.0f;
float wheelRotation = 0.0f;

// Tank physics.
float tankVelocity = 0.0f;
float tankAcceleration = 0.005f;
float tankDeceleration = 0.01f;
float tankMaxVelocity = 0.08f;
float tankVelocityDecay = 0.91f;

// Tank rotation physics.
float tankRotationVelocity = 0.0f;
float tankRotationAcceleration = 0.05f;
float tankRotationMaxVelocity = 5.0f;
float tankRotationVelocityDecay = 0.91f;

// Turret rotation physics.
float turretVelocity = 0.0f;
float turretAcceleration = 0.05f;
float turretDeceleration = 0.2f;
float turretMaxVelocity = 5.0f;
float turretVelocityDecay = 0.91f;

// Mouse control.
int lastMouseX = -1;
float mouseSensitivity = 1.0f;

// Ball properties.
std::vector<Vector3f> ballPositions;
std::vector<Vector3f> ballVelocities;
std::vector<bool> ballActives;
float ballSpeed = 0.5f;
Vector3f gravity = Vector3f(0.0f, -0.01f, 0.0f);
float tankYVelocity = 0.0f;
float tankGravity = -0.001f;
float gameOverThreshold = -5.0f; // If the tank falls below this, the game is over.
bool gameOver = false;
bool tankFalling = false;

bool canFire = true;
bool win = false;
bool help = false;
int initialCoins = 0;
bool cockpitView = false;

GLuint shaderProgramID;
GLuint skyboxShaderProgramID;

// Viewing/Camera.
Matrix4x4 ModelViewMatrix;		// ModelView Matrix
GLuint MVMatrixUniformLocation;		// ModelView Matrix Uniform
Matrix4x4 ProjectionMatrix;		// Projection Matrix
GLuint ProjectionUniformLocation;	// Projection Matrix Uniform Location
SphericalCameraManipulator cameraManip;

// Skybox Shader Uniforms
GLuint skyboxMVMatrixUniformLocation;
GLuint skyboxProjectionUniformLocation;
GLuint skyboxCubemapUniformLocation;

// Meshes.
Mesh tank_chassis;
Mesh tank_front_wheel;
Mesh tank_back_wheel;
Mesh tank_turret;

Mesh box;
Mesh coin;
Mesh ball;
Mesh skybox;

GLuint textureCoordinateAttribute; // Vertex Texcoord Attribute Location
GLuint textureMapUniformLocation; // Texture Map Location
GLuint vertexPositionAttribute;		// Vertex Position Attribute Location

// OpenGL textures.
GLuint hamvee_texture;
GLuint box_texture;
GLuint coin_texture;
GLuint ball_texture;
GLuint skybox_texture;

GLuint vertexNormalAttribute;	

GLuint LightDirectionUniformLocation;
GLuint ColourUniformLocation;
GLuint LightColorUniformLocation;
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

Vector3f lightDirection = Vector3f(1.0,1.0,1.0);
Vector3f objectColor    = Vector3f(1.0, 1.0, 1.0);
Vector3f lightColor     = Vector3f(0.7, 0.7, 0.6);
Vector3f ambient    = Vector3f(0.5,0.5,0.5);
Vector3f specular   = Vector3f(1.0,1.0,1.0);
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
    for (int i = 0 ; i < 256; i++) keyStates[i] = false;

	// Construct the level list.
    levelFiles = {"holy-grail.level", "divine-intervention.level", "home-sweet-home.level", "beautiful-paradise.level", "parallels.level", "random.level"};
    levelDisplayNames = {"[BEGINNER] Holy Grail",  "[EASY] Divine Intervention", "[MEDIUM] Home Sweet Home", "[HARD] Beautiful Paradise", "[EXTREME] Parallels", "[RANDOM] Random"};

    loadLevel(("../levels/" + levelName).c_str());
    
    // Set up other things like textures and models.
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

	skybox.loadOBJ("../models/cube.obj");
    std::vector<std::string> skyboxFaces = {
        "../models/skybox/left.bmp", // -x
        "../models/skybox/right.bmp", // +x
        "../models/skybox/top.bmp", // +y
        "../models/skybox/bottom.bmp", // -y
        "../models/skybox/back.bmp", // +z
        "../models/skybox/front.bmp" // -z
    };
	if (!initCubemapTexture(skyboxFaces, skybox_texture)) {
        std::cerr << "Failed to load skybox textures." << std::endl;
    }

	//Init Camera Manipultor
	cameraManip.setPanTiltRadius(0.f,0.f,2.f);
	cameraManip.setFocus(tank_chassis.getMeshCentroid());

	glClearColor(0.0,0.33,0.67,1.0);

    lastMouseX = -1; // Initialize lastMouseX

	//Enter main loop
    glutMainLoop();

    //Delete shader program
	glDeleteProgram(shaderProgramID);

    // Deallocate memory for the maze.
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
    shaderProgramID = Shader::LoadFromFile("shinyShader.vert","shinyShader.frag");

	// Shaders for skybox.
    skyboxShaderProgramID = Shader::LoadFromFile("skybox.vert", "skybox.frag");

	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
	textureCoordinateAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
	textureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");
	
	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform"); 
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform"); 
	LightDirectionUniformLocation   = glGetUniformLocation(shaderProgramID, "LightDirection_uniform");
	ColourUniformLocation           = glGetUniformLocation(shaderProgramID, "Colour_uniform");
	LightColorUniformLocation       = glGetUniformLocation(shaderProgramID, "LightColor_uniform");
	AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform"); 
	SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform"); 
	SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");

    skyboxMVMatrixUniformLocation = glGetUniformLocation(skyboxShaderProgramID, "MVMatrix_uniform");
    skyboxProjectionUniformLocation = glGetUniformLocation(skyboxShaderProgramID, "ProjMatrix_uniform");
    skyboxCubemapUniformLocation = glGetUniformLocation(skyboxShaderProgramID, "skybox"); // Sampler uniform
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

bool initCubemapTexture(const std::vector<std::string>& faces, GLuint& textureID)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height;
    char* data = nullptr;
    char* flipped_data = nullptr;

    // Load faces.
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        if (!Texture::LoadBMP(faces[i], width, height, data)) {
             std::cerr << "Failed to load texture: " << faces[i] << std::endl;
             glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
             glDeleteTextures(1, &textureID);
             textureID = 0;
             return false;
        }

        // Flip the image data vertically.
        int row_pitch = width * 3;
        int image_size = row_pitch * height;
        flipped_data = new char[image_size];
        if (!flipped_data) {
            std::cerr << "Failed to allocate memory for flipping texture: " << faces[i] << std::endl;
            delete[] data;
            glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
            glDeleteTextures(1, &textureID);
            textureID = 0;
            return false;
        }

        for (int y = 0; y < height; ++y) {
            memcpy(flipped_data + y * row_pitch,
                   data + (height - 1 - y) * row_pitch,
                   row_pitch);
        }

        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, flipped_data);

        delete[] data;
        delete[] flipped_data;
        data = nullptr;
        flipped_data = nullptr;
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    std::cout << "Cubemap texture loaded successfully." << std::endl;
    return true;
}

//! Display Loop
void display(void)
{
	if (!win && !tankFalling && !showLevelSelection) game_time+= 1.0f;

    handleKeys();
    updateCamera();

	glViewport(0,0,screenWidth, screenHeight);
	
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    glDepthMask(GL_FALSE);
    glDepthFunc(GL_LEQUAL);
    glCullFace(GL_FRONT);
    glUseProgram(skyboxShaderProgramID);

    ProjectionMatrix.perspective(90, 1.0, 0.0001, 10000.0);
    glUniformMatrix4fv(skyboxProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());

    drawSkybox();

    glCullFace(GL_BACK);
    glDepthMask(GL_TRUE);

	glUseProgram(shaderProgramID);

	glActiveTexture(GL_TEXTURE0);

    ProjectionMatrix.perspective(90, 1.0, 0.0001, 10000.0);
   
    glUniformMatrix4fv(	
		ProjectionUniformLocation,
		1,
		false,
		ProjectionMatrix.getPtr());

	drawMaze();
	drawTank();
	drawBall();

	glUseProgram(0);

	float timeInSeconds = game_time / 60.0f;
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << timeInSeconds;
	std::string roundedTime = stream.str();

	render2dText("Time: " + roundedTime, 1.0, 1.0, 0.0, -0.9, 0.9);
	render2dText("Score: " + std::to_string(score) + "/" + std::to_string(initialCoins), 1.0, 1.0, 0.0, -0.9, 0.8);
	render2dText("Ammo: " + std::to_string(ammo) + "/15", 1.0, 1.0, 1.0, -0.9, 0.7);
	render2dText("l[e]vels", 1.0, 1.0, 1.0, -0.9, 0.6);
	render2dText("[h]elp", 1.0, 1.0, 1.0, -0.9, 0.5);

	render2dText(reloadBar.c_str(), 1.0, 1.0, 1.0, 0.0, 0.0);

	if (gameOver) {
        render2dText("Game Over!", 1.0, 0.0, 0.0, -0.1, 0.0);
		render2dText("Press 'r' to try again.", 1.0, 1.0, 1.0, -0.2, -0.1);
    }

	if (win) {
        render2dText("You Win!", 0.0, 1.0, 0.0, -0.1, 0.0);
		render2dText("Press 'r' to go again.", 1.0, 1.0, 1.0, -0.2, -0.1);
    }

	if (help) {
		render2dText("[w] move tank forwards", 1.0, 1.0, 1.0, -0.9, 0.3);
		render2dText("[a] rotate tank left", 1.0, 1.0, 1.0, -0.9, 0.2);
		render2dText("[s] move tank backwards", 1.0, 1.0, 1.0, -0.9, 0.1);
		render2dText("[d] rotate tank right", 1.0, 1.0, 1.0, -0.9, 0.0);
		render2dText("[space] jump", 1.0, 1.0, 1.0, -0.9, -0.1);

		render2dText("[j] rotate turret left", 1.0, 1.0, 1.0, -0.9, -0.2);
		render2dText("[k] fire turret", 1.0, 1.0, 1.0, -0.9, -0.3);
		render2dText("[l] rotate turret right", 1.0, 1.0, 1.0, -0.9, -0.4);
		render2dText("[c] toggle cockpit view", 1.0, 1.0, 1.0, -0.9, -0.5);
	}

    if (showLevelSelection) {
        render2dText("Select Level:", 1.0, 1.0, 0.0, -0.2, 0.5);
        float yPos = 0.4f;
        for (size_t i = 0; i < levelDisplayNames.size(); ++i) {
            render2dText("[" + std::to_string(i) + "] " + levelDisplayNames[i], 1.0, 1.0, 1.0, -0.2, yPos);
            yPos -= 0.1f;
        }
        render2dText("Press 'e' again to cancel", 1.0, 1.0, 0.0, -0.2, yPos - 0.1f);
    }

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
			// 3 = spawnpoint
			if (maze[i][j] == 3) {
				tankPosition.x = i*2;
				tankPosition.z = j*2;
				tankPosition.y = 0.75f;
				maze[i][j] = 1;
			}
			if (maze[i][j] >= 1)
			{
				ModelViewMatrix.toIdentity();

				Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
				m.translate(i*2, 0, j*2);
				glUniformMatrix4fv(
					MVMatrixUniformLocation,
					1,
					false,
					m.getPtr());

				glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z);
				glUniform3f(ColourUniformLocation, 1.0f, 1.0f, 1.0f);
				glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z);
				glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
				glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
				glUniform1f(SpecularPowerUniformLocation, specularPower);
				
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
				m.translate(i*2, 2 + (sin(game_time/100)/5), j*2);
				m.rotate(game_time, 0, 1, 0);
				m.scale(0.5, 0.5, 0.5);
				glUniformMatrix4fv(
					MVMatrixUniformLocation,
					1,
					false,
					m.getPtr());

				glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z);
				glUniform3f(ColourUniformLocation, 1.0f, 0.84f, 0.0f);
				glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z);
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

	// Maze collision detection for falling.
    mazeX = ((tankPosition.x / 2));
    mazeZ = ((tankPosition.z / 2));

	int roundedMazeX = (int)std::round(mazeX);
	int roundedMazeZ = (int)std::round(mazeZ);

	bool inBounds = (roundedMazeX >= 0 && roundedMazeX < mazeHeight && roundedMazeZ >= 0 && roundedMazeZ < mazeWidth);

	// Only update mazeValue if the player is in bounds, otherwise
	// we could get a segfault.
	if (inBounds) {
		mazeValue = maze[roundedMazeX][roundedMazeZ];
	} else {
		mazeValue = 0;
	}

	bool onSolidGround = inBounds && (mazeValue >= 1);

	if (tankFalling) {
		tankYVelocity += tankGravity;
		tankPosition.y += tankYVelocity;
	}

	if (onSolidGround) {
		if (tankFalling && tankPosition.y <= 0.75f) {
			tankYVelocity = 0.0f;
			tankPosition.y = 0.75f;
			tankFalling = false;
		}
		else if (!tankFalling) {
			tankPosition.y = 0.75f;

		}
		if (maze[roundedMazeX][roundedMazeZ] == 2) {
			score++;
			maze[roundedMazeX][roundedMazeZ] = 1;
			if (score == initialCoins) {
				win = true;
			}
		}
	} else {
		if (!tankFalling) {
			tankFalling = true;
		}
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

	m.translate(tankPosition.x, tankPosition.y, tankPosition.z);
	m.rotate(tankRotation, 0, 1, 0);
	m.scale(0.5, 0.5, 0.5);

	glUniformMatrix4fv(
		MVMatrixUniformLocation,
		1,
		false,
		m.getPtr());

	glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform3f(ColourUniformLocation, 0.8f, 0.8f, 0.8f); // Base color for tank
	glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z);
	glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
	glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
	glUniform1f(SpecularPowerUniformLocation, specularPower);

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

	o.translate(tankPosition.x, tankPosition.y, tankPosition.z);
	o.rotate(turretRotation, 0, 1, 0);
	o.scale(0.5, 0.5, 0.5);

	glUniformMatrix4fv(
		MVMatrixUniformLocation,
		1,
		false,
		o.getPtr());

	glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z);
	glUniform3f(ColourUniformLocation, 0.8f, 0.8f, 0.8f); // Base color for tank
	glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z);
	glUniform4f(AmbientUniformLocation, ambient.x, ambient.y, ambient.z, 1.0);
	glUniform4f(SpecularUniformLocation, specular.x, specular.y, specular.z, 1.0);
	glUniform1f(SpecularPowerUniformLocation, specularPower);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, hamvee_texture);
	glUniform1i(textureMapUniformLocation, 0);

	tank_turret.Draw(
		vertexPositionAttribute,
		vertexNormalAttribute, textureCoordinateAttribute
	);

	if (tankPosition.y < gameOverThreshold) {
        gameOver = true;
    }
}

void drawBall() {
	for (int i = 0; i < ballPositions.size(); i++)
	{
		if (ballActives[i]) {
			ModelViewMatrix.toIdentity();
			Matrix4x4 m = cameraManip.apply(ModelViewMatrix);
			m.translate(ballPositions[i].x, ballPositions[i].y, ballPositions[i].z);
			m.scale(0.1, 0.1, 0.1);
			m.rotate(game_time*10, 0, 1, 1); 

			glUniformMatrix4fv(
				MVMatrixUniformLocation,
				1,
				false,
				m.getPtr());

			glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z);
			glUniform3f(ColourUniformLocation, 0.5f, 0.5f, 0.5f);
			glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z);
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

			int mazeX = (int)std::round(ballPositions[i].x / 2.0f);
			int mazeZ = (int)std::round(ballPositions[i].z / 2.0f);

			if (mazeX >= 0 && mazeX < mazeHeight && mazeZ >= 0 && mazeZ < mazeWidth && maze[mazeX][mazeZ] == 2)
			{
				score++;
				maze[mazeX][mazeZ] = 1;

				ballActives.erase(ballActives.begin() + i);
				ballPositions.erase(ballPositions.begin() + i);
				ballVelocities.erase(ballVelocities.begin() + i);
				i--;

				if (score == initialCoins) {
					win = true;
				}
				continue;
			}

			if (
				ballPositions[i].x > mazeHeight * 2 ||
				ballPositions[i].x < 0 ||
				ballPositions[i].y < -5.0f ||
				ballPositions[i].z > mazeWidth * 2 ||
				ballPositions[i].z < 0)
			{
				// Get rid of the ball if it's out of bounds or fell too far.
				ballActives.erase(ballActives.begin() + i);
				ballPositions.erase(ballPositions.begin() + i);
				ballVelocities.erase(ballVelocities.begin() + i);
				i--;
			}
		}
	}
}

void drawSkybox() {
    Matrix4x4 viewMatrix;
    viewMatrix = cameraManip.apply(viewMatrix);

    viewMatrix.removeTranslation();

    glUniformMatrix4fv(skyboxMVMatrixUniformLocation, 1, false, viewMatrix.getPtr());

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    glUniform1i(skyboxCubemapUniformLocation, 0);

    skybox.Draw(
        glGetAttribLocation(skyboxShaderProgramID, "aVertexPosition"),
        -1,
        -1
    );

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
}

void fireBall() {
	if (ammo <= 0) return;

	--ammo;

    ballActives.push_back(true);
    float radians = -turretRotation * (M_PI / 180.0f);
	float ballDistance = 0.0f;
    ballPositions.push_back(tankPosition + Vector3f(-sin(radians) * ballDistance, 2.0f, cos(radians) * ballDistance));

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

    if (showLevelSelection) {
        if (key == 'e') {
            showLevelSelection = false;
        } else if (key >= '0' && key < ('0' + levelFiles.size())) {
            int levelIndex = key - '0';
            levelName = levelFiles[levelIndex];

            // Reset everything in the game.
            gameOver = false;
            tankFalling = false;
            tankPosition = Vector3f(0.0f, 0.0f, 0.0f);
            tankRotation = 0.0f;
            turretRotation = 0.0f;
            tankYVelocity = 0.0f;
            tankVelocity = 0.0f;
            tankRotationVelocity = 0.0f;
            turretVelocity = 0.0f;
            score = 0;
            game_time = 0.0f;
            win = false;
            help = false;
            ballPositions.clear();
            ballVelocities.clear();
            ballActives.clear();
            canFire = true;
            cockpitView = false;
			ammo = 15;

            loadLevel(("../levels/" + levelName).c_str());
            showLevelSelection = false;
        }
    } else {
        if (key == 'r') {
            gameOver = false;
            tankFalling = false;
            tankPosition = Vector3f(0.0f, 0.0f, 0.0f);
            tankRotation = 0.0f;
            turretRotation = 0.0f;
            tankYVelocity = 0.0f;
            tankVelocity = 0.0f;
            tankRotationVelocity = 0.0f;
            turretVelocity = 0.0f;
            score = 0;
            game_time = 0.0f;
            win = false;
            help = false;
            ballPositions.clear();
            ballVelocities.clear();
            ballActives.clear();
            canFire = true;
            cockpitView = false;
			ammo = 15;

            loadLevel(("../levels/" + levelName).c_str());
        }

        if(key == 'h') {
            help = !help;
        }

        if(key == 'e' && !gameOver) {
            showLevelSelection = true; 
            help = false; 
        }

        if (key == 'c' && !gameOver && !win) {
            cockpitView = !cockpitView;
            updateCamera();
        }

        // ASCII 32 is spacebar, this is the jump mechanic.
        if (key == 32 && !tankFalling && !gameOver && !win) {
            tankYVelocity = 0.03f; 
			tankVelocity = 0.40f;
            tankFalling = true;
        }

        keyStates[key] = true;
    }

	glutPostRedisplay();
}

//! Handle key up situation
void keyUp(unsigned char key, int x, int y)
{
    keyStates[key] = false;
}


//! Handle Keys
void handleKeys()
{
	if (tankFalling || win || showLevelSelection || gameOver) return; // Prevent input during these states.

	if (fabs(tankVelocity) < 0.001f)
	{
		tankVelocity = 0.0f;
	}

	if (fabs(turretVelocity) < 0.001f) {
        turretVelocity = 0.0f;
    }
    
    if(keyStates['w']) // Accelerate tank forward.
    {
        tankVelocity += tankAcceleration;

        if(tankVelocity > tankMaxVelocity) { // Max movement speed.
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

		turretVelocity -= turretAcceleration;
		if (turretVelocity < -turretMaxVelocity) turretVelocity = turretMaxVelocity;
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
		if (turretVelocity < -turretMaxVelocity) turretVelocity = turretMaxVelocity;
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
    // Stop mouse control when a menu is showing.
    if (showLevelSelection || gameOver || win) {
        lastMouseX = -1;
        return;
    }

    if (lastMouseX == -1) {
        lastMouseX = x;
        return;
    }

    int deltaX = x - lastMouseX;
    turretRotation -= (float)deltaX * mouseSensitivity;
    lastMouseX = x;

    updateCamera();
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
    Vector3f tankWorldPos;

    if (cockpitView) {
        tankWorldPos = Vector3f(tankPosition.x, tankPosition.y + 1.0f, tankPosition.z); 

        float turretRadians = -turretRotation * (M_PI / 180.0f);
        Vector3f lookDirection = Vector3f(-sin(turretRadians), 0.0f, cos(turretRadians));

        Vector3f focusPoint = tankWorldPos + lookDirection * 1.3f;

        cameraManip.setFocus(focusPoint);

        cameraManip.setPanTiltRadius(turretRotation / (180 / M_PI), -1.5f, 0.1f);

    } else { // Normal view.
        tankWorldPos = Vector3f(tankPosition.x, 0.75f + 0.5f, tankPosition.z);
        cameraManip.setFocus(tankWorldPos);
	    cameraManip.setPanTiltRadius(turretRotation/(180/M_PI), -1.0f, 4.0f);
    }
}

void fireBallTimer(int value)
{
	canFire = true;
}

void loadLevel(const char* filename) {
    std::string fullPath(filename);
    size_t lastSlash = fullPath.find_last_of("/\\");
    std::string currentLevelFilename;
    if (lastSlash != std::string::npos) {
        currentLevelFilename = fullPath.substr(lastSlash + 1);
    } else {
        currentLevelFilename = fullPath;
    }
    levelName = currentLevelFilename;

    if (currentLevelFilename == "random.level") {
        generateRandomLevel();
        return;
    }

    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Could not open level file: " << filename << std::endl;
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

    maze = new int*[mazeHeight];
    for (int i = 0; i < mazeHeight; ++i) {
        maze[i] = new int[mazeWidth];
        if (maze[i] == nullptr) {
            std::cerr << "Memory allocation failed!" << std::endl;
            return;
        }
    }

    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            maze[i][j] = tempMaze[i][j];
        }
    }

	initialCoins = 0;
	for (int i = 0; i < mazeHeight; ++i) {
		for (int j = 0; j < mazeWidth; ++j) {
			if (maze[i][j] == 2) {
				initialCoins++;
			}
		}
	}

    file.close();
}

void generateRandomLevel() {
    if (maze != nullptr) {
        for (int i = 0; i < mazeHeight; ++i) {
            delete[] maze[i];
        }
        delete[] maze;
        maze = nullptr;
    }

    std::random_device rd;
    std::default_random_engine generator(rd());
    std::uniform_int_distribution<int> sizeDistribution(10, 30);
    std::uniform_int_distribution<int> coinDistribution(0, 100);

    mazeHeight = sizeDistribution(generator);
    mazeWidth = sizeDistribution(generator);

    maze = new int*[mazeHeight];
    for (int i = 0; i < mazeHeight; ++i) {
        maze[i] = new int[mazeWidth];
        if (maze[i] == nullptr) {
            std::cerr << "Memory allocation failed for random maze!" << std::endl;
            return;
        }
    }

    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            maze[i][j] = 1;
        }
    }

    int centerZ = mazeHeight / 2;
    int centerX = mazeWidth / 2;

    if (centerX > 0 && centerX < mazeHeight - 1 && centerZ > 0 && centerZ < mazeWidth - 1) {
        maze[centerX][centerZ] = 3;
        tankPosition.x = centerX * 2;
        tankPosition.z = centerZ * 2;
        tankPosition.y = 0.75f;
    } else {
        centerX = std::min(std::max(1, centerX), mazeHeight - 2);
        centerZ = std::min(std::max(1, centerZ), mazeWidth - 2);
		maze[centerX][centerZ] = 3;
        tankPosition.x = centerX * 2;
        tankPosition.z = centerZ * 2;
        tankPosition.y = 0.75f;
    }

    std::uniform_int_distribution<int> rowOrColDistribution(0, 1);
    std::uniform_int_distribution<int> lineDistribution;

    int numLinesToRemove = sizeDistribution(generator) / 2;
    for (int k = 0; k < numLinesToRemove; ++k) {
        if (rowOrColDistribution(generator) == 0) {
            lineDistribution = std::uniform_int_distribution<int>(1, mazeHeight - 2);
            int rowToRemove = lineDistribution(generator);
            for (int j = 1; j < mazeWidth - 1; ++j) {
                maze[rowToRemove][j] = 0;
            }
        } else {
            lineDistribution = std::uniform_int_distribution<int>(1, mazeWidth - 2);
            int colToRemove = lineDistribution(generator);
            for (int i = 1; i < mazeHeight - 1; ++i) {
                maze[i][colToRemove] = 0;
            }
        }
    }

    std::uniform_int_distribution<int> cellDistributionW(1, mazeWidth - 2);
    std::uniform_int_distribution<int> cellDistributionH(1, mazeHeight - 2);
    int numCratesToRemove = sizeDistribution(generator) * 2;
    for (int k = 0; k < numCratesToRemove; ++k) {
        int rowToRemove = cellDistributionH(generator);
        int colToRemove = cellDistributionW(generator);
        maze[rowToRemove][colToRemove] = 0;
    }

    initialCoins = 0;
    for (int i = 0; i < mazeHeight; ++i) {
        for (int j = 0; j < mazeWidth; ++j) {
            if (maze[i][j] == 1 && coinDistribution(generator) < 10) {
                maze[i][j] = 2;
                initialCoins++;
            }
            if (maze[i][j] == 3) {
                tankPosition.x = i * 2;
                tankPosition.z = j * 2;
                tankPosition.y = 0.75f;
            }
        }
    }

	maze[centerX][centerZ] = 3;
}

void render2dText(std::string text, float r, float g, float b, float x, float y)
{
	glColor3f(r,g,b);
	glRasterPos2f(x, y); // window coordinates
	for(unsigned int i = 0; i < text.size(); i++)
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}
