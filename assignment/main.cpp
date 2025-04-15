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
#include <iomanip> // Required for setting precision
#include <sstream> // Required for converting float to string
#include <array>  // For std::array

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
bool initCubemapTexture(const std::vector<std::string>& faces, GLuint& textureID); // Added prototype
void drawMaze();
void drawTank();
void drawBall();
void drawSkybox();
void fireBall();
void updateCamera();
int printOglError(char *file, int line);
void fireBallTimer(int value);
void loadLevel(const char* filename);
void render2dText(std::string text, float r, float g, float b, float x, float y);

// Global variables.
float game_time = 0.0f; // In game timer.
int score = 0;
std::string levelName = "holy-grail.level"; // Default level
std::string reloadBar;

// Level Selection
std::vector<std::string> levelFiles;
std::vector<std::string> levelDisplayNames;
bool showLevelSelection = false;

int screenWidth   	        = 720;
int screenHeight   	        = 720;

float mazeX = 0.0;
float mazeZ = 0.0;
int mazeValue = 0;
bool shouldFall = false;

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

// Ball properties
std::vector<Vector3f> ballPositions;
std::vector<Vector3f> ballVelocities;
std::vector<bool> ballActives;
float ballSpeed = 0.5f;
Vector3f gravity = Vector3f(0.0f, -0.01f, 0.0f);
float tankYVelocity = 0.0f; // Tank's vertical velocity
float tankGravity = -0.001f; // Gravity affecting the tank
float gameOverThreshold = -5.0f; // Y position below which game is over
bool gameOver = false;
bool tankFalling = false;

bool canFire = true; // Add a boolean to control firing rate
bool win = false;
bool help = false;
int initialCoins = 0;
bool cockpitView = false; // Add state for cockpit view

GLuint shaderProgramID;
GLuint shinyShaderProgramID;
GLuint tankShaderProgramID;
GLuint skyboxShaderProgramID; // Added for skybox

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

GLuint shinyTextureCoordinateAttribute; // Vertex Texcoord Attribute Location
GLuint shinyTextureMapUniformLocation; // Texture Map Location
GLuint shinyVertexPositionAttribute;		// Vertex Position Attribute Location

// OpenGL textures.
GLuint hamvee_texture; // Add this line
GLuint box_texture;    // Add this line
GLuint coin_texture;   // Add this line
GLuint ball_texture;   // Add this line
GLuint skybox_texture; // Represents the cubemap texture ID now

GLuint vertexNormalAttribute;	

GLuint shinyVertexNormalAttribute;	

GLuint LightDirectionUniformLocation; // Renamed
GLuint ColourUniformLocation;         // Added
GLuint LightColorUniformLocation;     // Added
GLuint AmbientUniformLocation;
GLuint SpecularUniformLocation;
GLuint SpecularPowerUniformLocation;

GLuint ShinyLightPositionUniformLocation;
GLuint ShinyAmbientUniformLocation;
GLuint ShinySpecularUniformLocation;
GLuint ShinySpecularPowerUniformLocation;

Vector3f lightDirection = Vector3f(1.0,1.0,1.0); // Example direction
Vector3f objectColor    = Vector3f(1.0, 1.0, 1.0);  // Default white color
Vector3f lightColor     = Vector3f(0.7, 0.7, 0.6);  // Default white light
Vector3f ambient    = Vector3f(0.5,0.5,0.5);
Vector3f specular   = Vector3f(1.0,1.0,1.0); // Changed from green to white
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

    // Initialize level list
    levelFiles = {"beautiful-paradise.level", "divine-intervention.level", "holy-grail.level", "home-sweet-home.level", "parallels.level"};
    levelDisplayNames = {"Beautiful Paradise", "Divine Intervention", "Holy Grail", "Home Sweet Home", "Parallels"};

    // Load the default level
    loadLevel(("../levels/" + levelName).c_str());
    
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

	skybox.loadOBJ("../models/cube.obj");
	// Define the faces for the cubemap in the correct order
    std::vector<std::string> skyboxFaces = {
        "../models/skybox/left.bmp",   // Negative X
        "../models/skybox/right.bmp",  // Positive X
        "../models/skybox/top.bmp",    // Positive Y
        "../models/skybox/bottom.bmp", // Negative Y
        "../models/skybox/back.bmp",   // Positive Z
        "../models/skybox/front.bmp"   // Negative Z
    };
	if (!initCubemapTexture(skyboxFaces, skybox_texture)) {
        std::cerr << "Failed to load skybox textures." << std::endl;
        // Handle error appropriately, maybe exit
    }

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
    shaderProgramID = Shader::LoadFromFile("shinyShader.vert","shinyShader.frag");

	shinyShaderProgramID = Shader::LoadFromFile("shinyShader.vert","shinyShader.frag");

    // Load skybox shaders
    skyboxShaderProgramID = Shader::LoadFromFile("skybox.vert", "skybox.frag");

    // Get a handle for our vertex position buffer
	vertexPositionAttribute = glGetAttribLocation(shaderProgramID, "aVertexPosition");
	vertexNormalAttribute = glGetAttribLocation(shaderProgramID,    "aVertexNormal");
	textureCoordinateAttribute = glGetAttribLocation(shaderProgramID, "aVertexTexcoord");
	textureMapUniformLocation = glGetUniformLocation(shaderProgramID, "TextureMap_uniform");

	// Get a handle for our vertex position buffer (shiny)
	shinyVertexPositionAttribute = glGetAttribLocation(shinyShaderProgramID, "aVertexPosition");
	shinyVertexNormalAttribute = glGetAttribLocation(shinyShaderProgramID,    "aVertexNormal");
	shinyTextureCoordinateAttribute = glGetAttribLocation(shinyShaderProgramID, "aVertexTexcoord");
	shinyTextureMapUniformLocation = glGetUniformLocation(shinyShaderProgramID, "TextureMap_uniform");
	
	// Get ModelView Matrix uniform location
	MVMatrixUniformLocation = glGetUniformLocation(shaderProgramID, "MVMatrix_uniform"); 
	ProjectionUniformLocation = glGetUniformLocation(shaderProgramID, "ProjMatrix_uniform"); 
	LightDirectionUniformLocation   = glGetUniformLocation(shaderProgramID, "LightDirection_uniform"); // Renamed
	ColourUniformLocation           = glGetUniformLocation(shaderProgramID, "Colour_uniform");         // Added
	LightColorUniformLocation       = glGetUniformLocation(shaderProgramID, "LightColor_uniform");     // Added
	AmbientUniformLocation          = glGetUniformLocation(shaderProgramID, "Ambient_uniform"); 
	SpecularUniformLocation         = glGetUniformLocation(shaderProgramID, "Specular_uniform"); 
	SpecularPowerUniformLocation    = glGetUniformLocation(shaderProgramID, "SpecularPower_uniform");

    // Get uniform locations for skybox shader
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

// Function to initialize a cubemap texture
bool initCubemapTexture(const std::vector<std::string>& faces, GLuint& textureID)
{
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height;
    char* data = nullptr;
    char* flipped_data = nullptr; // Buffer for flipped data

    // Load the 6 faces
    for (unsigned int i = 0; i < faces.size(); i++)
    {
        if (!Texture::LoadBMP(faces[i], width, height, data)) {
             std::cerr << "Failed to load texture: " << faces[i] << std::endl;
             glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind
             glDeleteTextures(1, &textureID); // Delete texture object
             textureID = 0; // Reset texture ID
             return false;
        }

        // Flip the image data vertically
        int row_pitch = width * 3; // Assuming 3 bytes per pixel (RGB)
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
            memcpy(flipped_data + y * row_pitch,            // Destination: Start of row y in flipped buffer
                   data + (height - 1 - y) * row_pitch, // Source: Start of row (height-1-y) in original buffer
                   row_pitch);                          // Bytes to copy: One full row
        }

        // Note the target parameter for cubemap faces
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
                     0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, flipped_data); // Use flipped data

        delete[] data; // Cleanup original data
        delete[] flipped_data; // Cleanup flipped data buffer
        data = nullptr;
        flipped_data = nullptr;
    }

    // Set texture parameters for cubemap
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE); // WRAP_R for the 3rd dimension

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind texture

    std::cout << "Cubemap texture loaded successfully." << std::endl;
    return true;
}

//! Display Loop
void display(void)
{
	if (!win && !tankFalling && !showLevelSelection) // Pause timer during level select
		game_time+= 1.0f; // Increment the in-game timer

    handleKeys();

    updateCamera(); // Update camera to look at tank.

	glViewport(0,0,screenWidth, screenHeight); // Set viewport size.
	
	// Clear the screen
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

    // --- Draw Skybox First ---
    glDepthMask(GL_FALSE); // Disable depth writing
    glDepthFunc(GL_LEQUAL); // Change depth function for skybox drawing
    glCullFace(GL_FRONT);   // Cull front faces for skybox
    glUseProgram(skyboxShaderProgramID);

    // Projection Matrix - Perspective Projection (same as main view)
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 10000.0);
    glUniformMatrix4fv(skyboxProjectionUniformLocation, 1, false, ProjectionMatrix.getPtr());

    drawSkybox(); // Draw the skybox using its specific shader and uniforms

    // Restore defaults
    glCullFace(GL_BACK);    // Restore back face culling
    glDepthMask(GL_TRUE); // Re-enable depth writing
    // --- End Skybox Draw ---

    // --- Draw Rest of the Scene ---
	glUseProgram(shaderProgramID);

	//Set Colour after program is in use
	glActiveTexture(GL_TEXTURE0); // Ensure texture unit 0 is active for main scene

	//Projection Matrix - Perspective Projection
    ProjectionMatrix.perspective(90, 1.0, 0.0001, 10000.0);
   
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

	// Round game_time to 2 decimal places
	float timeInSeconds = game_time / 60.0f;
	std::stringstream stream;
	stream << std::fixed << std::setprecision(2) << timeInSeconds;
	std::string roundedTime = stream.str();

	render2dText("Time: " + roundedTime, 1.0, 1.0, 0.0, -0.9, 0.9);
	render2dText("Score: " + std::to_string(score) + "/" + std::to_string(initialCoins), 1.0, 1.0, 0.0, -0.9, 0.8);
	render2dText("l[e]vels", 1.0, 1.0, 1.0, -0.9, 0.7);
	render2dText("[h]elp", 1.0, 1.0, 1.0, -0.9, 0.6);

	// reloadBar = "[=====]";

	render2dText(reloadBar.c_str(), 1.0, 1.0, 1.0, 0.0, 0.0);

	// render2dText("Maze Position: (" + std::to_string(mazeX) + ", " + std::to_string(mazeZ) + ")", 1.0, 1.0, 1.0, -0.9, 0.6);
    // render2dText("Maze Value: " + std::to_string(mazeValue), 1.0, 1.0, 1.0, -0.9, 0.5);

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

    // Display level selection menu if active
    if (showLevelSelection) {
        render2dText("Select Level:", 1.0, 1.0, 0.0, -0.2, 0.5);
        float yPos = 0.4f;
        for (size_t i = 0; i < levelDisplayNames.size(); ++i) {
            render2dText("[" + std::to_string(i) + "] " + levelDisplayNames[i], 1.0, 1.0, 1.0, -0.2, yPos);
            yPos -= 0.1f; // Adjust spacing as needed
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

				glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z); // Use direction
				glUniform3f(ColourUniformLocation, 1.0f, 1.0f, 1.0f); // Set object color (e.g., white for crate)
				glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z); // Set light color
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
				m.translate(i*2, 2 + (sin(game_time/100)/5), j*2);
				m.rotate(game_time, 0, 1, 0); 
				glUniformMatrix4fv(
					MVMatrixUniformLocation,
					1,
					false,
					m.getPtr());

				glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z); // Use direction
				glUniform3f(ColourUniformLocation, 1.0f, 0.84f, 0.0f); // Set object color (e.g., gold for coin)
				glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z); // Set light color
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

	// Maze collision detection for falling
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
		mazeValue = 0; // Default to 0 if out of bounds
	}

	// Determine if the tank *should* be falling based on position
	bool onSolidGround = inBounds && (mazeValue >= 1);

	// Apply gravity if the tank is falling (jumped or over an edge)
	if (tankFalling) {
		tankYVelocity += tankGravity;
		tankPosition.y += tankYVelocity;
	}

	// Check for landing or falling off edge
	if (onSolidGround) {
		// Check if the tank has landed after falling/jumping
		if (tankFalling && tankPosition.y <= 0.75f) {
			tankYVelocity = 0.0f;
			tankPosition.y = 0.75f;
			tankFalling = false;
		}
		// If not currently falling but on solid ground, ensure correct height
		else if (!tankFalling) {
			tankPosition.y = 0.75f;
		}

		// Coin collision detection (only when grounded)
		if (maze[roundedMazeX][roundedMazeZ] == 2) {
			score++;
			maze[roundedMazeX][roundedMazeZ] = 1; // Remove the coin
			if (score == initialCoins) {
				win = true;
			}
		}
	} else {
		// If not on solid ground, ensure the tank is marked as falling
		if (!tankFalling) {
			tankFalling = true;
			// Optional: Give a small initial downward velocity if falling off edge
			// tankYVelocity = 0.0f; // Or a small negative value
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

			glUniform3f(LightDirectionUniformLocation, lightDirection.x, lightDirection.y, lightDirection.z); // Use direction
			glUniform3f(ColourUniformLocation, 0.5f, 0.5f, 0.5f); // Set object color (e.g., grey for ball)
			glUniform3f(LightColorUniformLocation, lightColor.x, lightColor.y, lightColor.z); // Set light color
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

			// Collision detection with coins
			int mazeX = (int)(ballPositions[i].x / 2);
			int mazeZ = (int)(ballPositions[i].z / 2);

			if (mazeX >= 0 && mazeX < mazeHeight && mazeZ >= 0 && mazeZ < mazeWidth && maze[mazeX][mazeZ] == 2)
			{
				score++;
				maze[mazeX][mazeZ] = 1; // Remove the coin

				// Remove the ball that hit the coin
				ballActives.erase(ballActives.begin() + i);
				ballPositions.erase(ballPositions.begin() + i);
				ballVelocities.erase(ballVelocities.begin() + i);
				i--; // Decrement i to account for the removed element
				
				if (score == initialCoins) {
					win = true;
				}
				continue; // Skip the rest of the loop for this ball
			}

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

void drawSkybox() {
    // Use the skybox shader program (already set in display)
    // glUseProgram(skyboxShaderProgramID); // Set in display loop before calling

    // Get the view matrix from the camera manipulator by applying it to an identity matrix
    Matrix4x4 viewMatrix; // Starts as identity
    viewMatrix = cameraManip.apply(viewMatrix);

    // Remove the translation part of the view matrix using the new method
    viewMatrix.removeTranslation();

    // The ModelView matrix for the skybox is just the modified view matrix
    // (no model transformation needed as it's centered on the camera)
    glUniformMatrix4fv(skyboxMVMatrixUniformLocation, 1, false, viewMatrix.getPtr());
    // Projection matrix is set in the display loop

    // Bind the cubemap texture
    glActiveTexture(GL_TEXTURE0); // Use texture unit 0
    glBindTexture(GL_TEXTURE_CUBE_MAP, skybox_texture);
    glUniform1i(skyboxCubemapUniformLocation, 0); // Tell shader to use texture unit 0

    // Draw the skybox cube mesh
    // Skybox shader only needs vertex positions
    skybox.Draw(
        glGetAttribLocation(skyboxShaderProgramID, "aVertexPosition"), // Use position attribute from skybox shader
        -1, // No normal needed for basic skybox
        -1  // No texcoord needed (using vertex position)
    );

    glBindTexture(GL_TEXTURE_CUBE_MAP, 0); // Unbind cubemap
}

// Function to fire the ball
void fireBall() {
    ballActives.push_back(true);
    float radians = -turretRotation * (M_PI / 180.0f);
	float ballDistance = 0.0f;
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

    if (showLevelSelection) {
        if (key == 'e') {
            showLevelSelection = false; // Cancel level selection
        } else if (key >= '0' && key < ('0' + levelFiles.size())) {
            int levelIndex = key - '0';
            levelName = levelFiles[levelIndex]; // Update the current level name

            // Reset game state before loading new level
            gameOver = false;
            tankFalling = false;
            tankPosition = Vector3f(0.0f, 0.0f, 0.0f); // Will be set by loadLevel spawn point
            tankRotation = 0.0f;
            turretRotation = 0.0f;
            tankYVelocity = 0.0f;
            tankVelocity = 0.0f; // Reset velocity
            tankRotationVelocity = 0.0f;
            turretVelocity = 0.0f;
            score = 0;
            game_time = 0.0f;
            win = false;
            help = false; // Close help if open
            ballPositions.clear(); // Clear existing balls
            ballVelocities.clear();
            ballActives.clear();
            canFire = true; // Reset firing state
            cockpitView = false; // Reset view on level change

            loadLevel(("../levels/" + levelName).c_str());
            showLevelSelection = false; // Hide selection screen after loading
        }
    } else {
        if ((gameOver || win) && key == 'r') {
            // Reset game state
            gameOver = false;
            tankFalling = false;
            tankPosition = Vector3f(0.0f, 0.0f, 0.0f); // Will be set by loadLevel spawn point
            tankRotation = 0.0f;
            turretRotation = 0.0f;
            tankYVelocity = 0.0f;
            tankVelocity = 0.0f; // Reset velocity
            tankRotationVelocity = 0.0f;
            turretVelocity = 0.0f;
            score = 0;
            game_time = 0.0f;
            win = false;
            help = false; // Close help if open
            ballPositions.clear(); // Clear existing balls
            ballVelocities.clear();
            ballActives.clear();
            canFire = true; // Reset firing state
            cockpitView = false; // Reset view on restart

            // Reload the current level
            loadLevel(("../levels/" + levelName).c_str());
        }

        if(key == 'h') {
            help = !help;
        }

        if(key == 'e') {
            showLevelSelection = true; // Show level selection screen
            help = false; // Hide help when showing levels
        }

        // Toggle cockpit view
        if (key == 'c' && !gameOver && !win) {
            cockpitView = !cockpitView;
            updateCamera(); // Update camera immediately after toggle
        }

        // Jump mechanic: Spacebar (ASCII 32)
        if (key == 32 && !tankFalling && !gameOver && !win) {
            tankYVelocity = 0.03f; // Set initial upward velocity for jump
			tankVelocity = 0.40f;
            tankFalling = true; // Tank is now in the air
        }

        keyStates[key] = true; // Only set key state if not in level selection
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
	if (tankFalling || win || showLevelSelection || gameOver) return; // Prevent input during these states

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
    Vector3f tankWorldPos;

    if (cockpitView) {
        // Raise the base position slightly higher for cockpit view
        tankWorldPos = Vector3f(tankPosition.x, tankPosition.y + 1.0f, tankPosition.z); // Increased Y offset for cockpit

        // Cockpit view: Position camera slightly in front of the tank, looking forward based on turret rotation
        float turretRadians = -turretRotation * (M_PI / 180.0f);
        Vector3f lookDirection = Vector3f(-sin(turretRadians), 0.0f, cos(turretRadians));

        // Set the focus point slightly ahead of the tank's current position along the turret direction
        // This becomes the center point around which the spherical camera operates.
        Vector3f focusPoint = tankWorldPos + lookDirection * 1.3f; // Focus slightly in front

        // Set the manipulator's focus
        cameraManip.setFocus(focusPoint);

        // Set pan to match turret rotation, tilt slightly down, and radius very small
        // The small radius places the camera *at* the focus point we just set.
        // Adjust tilt slightly if needed for better view.
        cameraManip.setPanTiltRadius(turretRotation / (180 / M_PI), -1.5f, 0.1f); // Pan = turret, Tilt slightly down, Radius minimal

    } else {
        // Normal third-person view uses the standard offset
        tankWorldPos = Vector3f(tankPosition.x, tankPosition.y + 0.5f, tankPosition.z); // Standard Y offset
        cameraManip.setFocus(tankWorldPos);
	    cameraManip.setPanTiltRadius(turretRotation/(180/M_PI), -1.0f, 4.0f); // Adjusted radius and tilt for better view
    }
}

void fireBallTimer(int value)
{
	canFire = true;
}

void loadLevel(const char* filename) {
    // Extract just the filename part for levelName update
    std::string fullPath(filename);
    size_t lastSlash = fullPath.find_last_of("/\\");
    if (lastSlash != std::string::npos) {
        levelName = fullPath.substr(lastSlash + 1);
    } else {
        levelName = fullPath; // Use the full string if no slash found
    }

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

	// Count initial coins
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

void render2dText(std::string text, float r, float g, float b, float x, float y)
{
	glColor3f(r,g,b);
	glRasterPos2f(x, y); // window coordinates
	for(unsigned int i = 0; i < text.size(); i++)
	glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, text[i]);
}
