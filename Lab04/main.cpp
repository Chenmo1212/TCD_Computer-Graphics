// Windows includes (For Time, IO, etc.)
#include <windows.h>
#include <mmsystem.h>
#include <iostream>
#include <string>
#include <stdio.h>
#include <math.h>
#include <vector> // STL dynamic memory.

// OpenGL includes
#include <GL/glew.h>
#include <GL/freeglut.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Assimp includes
#include <assimp/cimport.h> // scene importer
#include <assimp/scene.h> // collects data
#include <assimp/postprocess.h> // various extra operations

// Project includes
#include "maths_funcs.h"

# define PI 3.1415926

GLint WIDTH = 800; // window width 
GLint HEIGHT = 800; // window height


// Variables referenced in header files
float camera_x = 0.0f;
float camera_y = 20.0f;
float camera_z = 14.0f;
float x_pos = 0.0f;
float y_pos = 5.0f;
float z_pos = 0.0f;

float BombermanPosX = 1, BombermanPosZ = 2; // Position initial of Bomberman
float BombPosX = 0.0f, BombPosZ = 0.0f; // Position Bomb
bool isShowBomb = false;
DWORD lastSpacePressTime = 0;

std::vector<vec3> pumpkin_pos;

GLfloat rotate_y = 90.0f;
bool isRotate = false;

int model_location, view_mat_location, proj_mat_location, ortho_mat_location;

class ProjectionMatrices {
public:
	mat4 projection;
	mat4 view;
	mat4 model;
	mat4 ortho;
};

// Viewport 1 Variables
ProjectionMatrices scene;

GLuint cube_vao = 0, cube_vp_vbo = 0, cube_vn_vbo = 0;
GLuint bomb_vao = 0, bomb_vp_vbo = 0, bomb_vn_vbo = 0;
GLuint pumpkin_vao = 0, pumpkin_vp_vbo = 0, pumpkin_vn_vbo = 0;
GLuint bomberman_vao = 0, bomberman_vp_vbo = 0, bomberman_vn_vbo = 0;

/*----------------------------------------------------------------------------
MESH TO LOAD
----------------------------------------------------------------------------*/
// this mesh is a dae file format but you should be able to use any other format too, obj is typically what is used
// put the mesh in your project directory, or provide a filepath for it here
//#define MESH_NAME "monkeyhead_smooth.dae"
//#define MESH_NAME "Bomb.dae"
/*----------------------------------------------------------------------------
----------------------------------------------------------------------------*/

#pragma region SimpleTypes
typedef struct
{
	size_t mPointCount = 0;
	std::vector<vec3> mVertices;
	std::vector<vec3> mNormals;
	std::vector<vec2> mTextureCoords;
} ModelData;
#pragma endregion SimpleTypes

using namespace std;
GLuint shaderProgramID;

ModelData grass, cube, bomb, bomberman, pumpkin;

unsigned int mesh_vao = 0;

vec4 LightPosition = vec4(8.0, 10.0, 6.0, 1.0);
float Light_angle = 0.0;
GLuint Light_rad = 10;
float ld = 1.0;


/*----------------------------------------------------------------------------
GAME FUNCTION
----------------------------------------------------------------------------*/

// Collision Detection
bool collision(float coord1[], float coord2[], float R1, float R2) {

	float D = sqrt(pow(coord1[0] - coord2[0], 2) + pow(coord1[1] - coord2[1], 2) + pow(coord1[2] - coord2[2], 2));
	if (D <= (R1 + R2)) {
		return true;
	}
	return false;
}



#pragma region MESH LOADING
/*----------------------------------------------------------------------------
MESH LOADING FUNCTION
----------------------------------------------------------------------------*/

ModelData load_mesh(const char* file_name) {
	ModelData modelData;

	/* Use assimp to read the model file, forcing it to be read as    */
	/* triangles. The second flag (aiProcess_PreTransformVertices) is */
	/* relevant if there are multiple meshes in the model file that   */
	/* are offset from the origin. This is pre-transform them so      */
	/* they're in the right position.                                 */
	const aiScene* scene = aiImportFile(
		file_name, 
		aiProcess_Triangulate | aiProcess_PreTransformVertices
	); 

	if (!scene) {
		fprintf(stderr, "ERROR: reading mesh %s\n", file_name);
		return modelData;
	}

	printf("  %i materials\n", scene->mNumMaterials);
	printf("  %i meshes\n", scene->mNumMeshes);
	printf("  %i textures\n", scene->mNumTextures);

	for (unsigned int m_i = 0; m_i < scene->mNumMeshes; m_i++) {
		const aiMesh* mesh = scene->mMeshes[m_i];
		printf("    %i vertices in mesh\n", mesh->mNumVertices);
		modelData.mPointCount += mesh->mNumVertices;
		for (unsigned int v_i = 0; v_i < mesh->mNumVertices; v_i++) {
			if (mesh->HasPositions()) {
				const aiVector3D* vp = &(mesh->mVertices[v_i]);
				modelData.mVertices.push_back(vec3(vp->x, vp->y, vp->z));
			}
			if (mesh->HasNormals()) {
				const aiVector3D* vn = &(mesh->mNormals[v_i]);
				modelData.mNormals.push_back(vec3(vn->x, vn->y, vn->z));
			}
			if (mesh->HasTextureCoords(0)) {
				const aiVector3D* vt = &(mesh->mTextureCoords[0][v_i]);
				modelData.mTextureCoords.push_back(vec2(vt->x, vt->y));
			}
			if (mesh->HasTangentsAndBitangents()) {
				/* You can extract tangents and bitangents here              */
				/* Note that you might need to make Assimp generate this     */
				/* data for you. Take a look at the flags that aiImportFile  */
				/* can take.                                                 */
			}
		}
	}

	aiReleaseImport(scene);
	return modelData;
}

#pragma endregion MESH LOADING

// Shader Functions- click on + to expand
#pragma region SHADER_FUNCTIONS
char* readShaderSource(const char* shaderFile) {
	FILE* fp;
	fopen_s(&fp, shaderFile, "rb");

	if (fp == NULL) { return NULL; }

	fseek(fp, 0L, SEEK_END);
	long size = ftell(fp);

	fseek(fp, 0L, SEEK_SET);
	char* buf = new char[size + 1];
	fread(buf, 1, size, fp);
	buf[size] = '\0';

	fclose(fp);

	return buf;
}


static void AddShader(GLuint ShaderProgram, const char* pShaderText, GLenum ShaderType)
{
	// create a shader object
	GLuint ShaderObj = glCreateShader(ShaderType);

	if (ShaderObj == 0) {
		std::cerr << "Error creating shader..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	const char* pShaderSource = readShaderSource(pShaderText);

	// Bind the source code to the shader, this happens before compilation
	glShaderSource(ShaderObj, 1, (const GLchar**)&pShaderSource, NULL);
	// compile the shader and check for errors
	glCompileShader(ShaderObj);
	GLint success;
	// check for shader related errors using glGetShaderiv
	glGetShaderiv(ShaderObj, GL_COMPILE_STATUS, &success);
	if (!success) {
		GLchar InfoLog[1024] = { '\0' };
		glGetShaderInfoLog(ShaderObj, 1024, NULL, InfoLog);
		std::cerr << "Error compiling "
			<< (ShaderType == GL_VERTEX_SHADER ? "vertex" : "fragment")
			<< " shader program: " << InfoLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Attach the compiled shader object to the program object
	glAttachShader(ShaderProgram, ShaderObj);
}

GLuint CompileShaders()
{
	//Start the process of setting up our shaders by creating a program ID
	//Note: we will link all the shaders together into this ID
	shaderProgramID = glCreateProgram();
	if (shaderProgramID == 0) {
		std::cerr << "Error creating shader program..." << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// Create two shader objects, one for the vertex, and one for the fragment shader
	AddShader(shaderProgramID, "simpleVertexShader.txt", GL_VERTEX_SHADER);
	AddShader(shaderProgramID, "simpleFragmentShader.txt", GL_FRAGMENT_SHADER);

	GLint Success = 0;
	GLchar ErrorLog[1024] = { '\0' };
	// After compiling all shader objects and attaching them to the program, we can finally link it
	glLinkProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_LINK_STATUS, &Success);
	if (Success == 0) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Error linking shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}

	// program has been successfully linked but needs to be validated to check whether the program can execute given the current pipeline state
	glValidateProgram(shaderProgramID);
	// check for program related errors using glGetProgramiv
	glGetProgramiv(shaderProgramID, GL_VALIDATE_STATUS, &Success);
	if (!Success) {
		glGetProgramInfoLog(shaderProgramID, sizeof(ErrorLog), NULL, ErrorLog);
		std::cerr << "Invalid shader program: " << ErrorLog << std::endl;
		std::cerr << "Press enter/return to exit..." << std::endl;
		std::cin.get();
		exit(1);
	}
	// Finally, use the linked shader program
	// Note: this program will stay in effect for all draw calls until you replace it with another or explicitly disable its use
	glUseProgram(shaderProgramID);
	return shaderProgramID;
}
#pragma endregion SHADER_FUNCTIONS

// VBO Functions - click on + to expand
#pragma region VBO_FUNCTIONS
void generateObjectBufferMesh(ModelData mesh_data, GLuint vao, GLuint vp_vbo, GLuint vn_vbo) {
	/*----------------------------------------------------------------------------
	LOAD MESH HERE AND COPY INTO BUFFERS
	----------------------------------------------------------------------------*/
	GLuint loc1, loc2, loc3;
	//Note: you may get an error "vector subscript out of range" if you are using this code for a mesh that doesnt have positions and normals
	//Might be an idea to do a check for that before generating and binding the buffer.
	//unsigned int vp_vbo = 0;
	loc1 = glGetAttribLocation(shaderProgramID, "vertex_position");
	loc2 = glGetAttribLocation(shaderProgramID, "vertex_normal");
	loc3 = glGetAttribLocation(shaderProgramID, "vertex_texture");

	glGenBuffers(1, &vp_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mVertices[0], GL_STATIC_DRAW);
	//unsigned int vn_vbo = 0;
	glGenBuffers(1, &vn_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glBufferData(GL_ARRAY_BUFFER, mesh_data.mPointCount * sizeof(vec3), &mesh_data.mNormals[0], GL_STATIC_DRAW);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	unsigned int vt_vbo = 0;
	//	glGenBuffers (1, &vt_vbo);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glBufferData (GL_ARRAY_BUFFER, monkey_head_data.mTextureCoords * sizeof (vec2), &monkey_head_data.mTextureCoords[0], GL_STATIC_DRAW);

	//unsigned int vao = 0;
	glBindVertexArray(vao);

	glEnableVertexAttribArray(loc1);
	glBindBuffer(GL_ARRAY_BUFFER, vp_vbo);
	glVertexAttribPointer(loc1, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(loc2);
	glBindBuffer(GL_ARRAY_BUFFER, vn_vbo);
	glVertexAttribPointer(loc2, 3, GL_FLOAT, GL_FALSE, 0, NULL);

	//	This is for texture coordinates which you don't currently need, so I have commented it out
	//	glEnableVertexAttribArray (loc3);
	//	glBindBuffer (GL_ARRAY_BUFFER, vt_vbo);
	//	glVertexAttribPointer (loc3, 2, GL_FLOAT, GL_FALSE, 0, NULL);
}
#pragma endregion VBO_FUNCTIONS


void draw_cube(ModelData mesh, float scale_num = 1., vec3 trans=vec3(0.,0.,0.), float rotate_x = 0., float rotate_y = 0., float rotate_z = 0.) {
	// Main Viewport
	//glViewport(0, 0, WIDTH, HEIGHT);

	mat4 temp = identity_mat4() * scene.model;
	temp = rotate_z_deg(temp, rotate_z);
	temp = rotate_y_deg(temp, rotate_y);
	temp = rotate_x_deg(temp, rotate_x);
	temp = scale(temp, vec3(.1f, .1f, .1f) * scale_num);
	temp = translate(temp, trans);

	glUniformMatrix4fv(proj_mat_location, 1, GL_FALSE, scene.projection.m);
	glUniformMatrix4fv(view_mat_location, 1, GL_FALSE, scene.view.m);
	glUniformMatrix4fv(model_location, 1, GL_FALSE, temp.m);
	glUniformMatrix4fv(ortho_mat_location, 1, GL_FALSE, scene.ortho.m);
	glDrawArrays(GL_TRIANGLES, 0, mesh.mPointCount);
}

std::vector<vec3> rand_pumpkin_pos() {
	std::vector<vec3> positions;
	for (int i = 0; i < 5; i++) {
		positions.push_back(vec3((rand() % 20 - 10) / 2 * 2 + 1, 2, (rand() % 20 - 10) / 2 * 2 + 1));
	}
	return positions;
}

void display() {

	// tell GL to only draw onto a pixel if the shape is closer to the viewer
	glEnable(GL_DEPTH_TEST); // enable depth-testing
	glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
	glClearColor(0.5f, 0.5f, 0.5f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glUseProgram(shaderProgramID);
	// Declare your uniform variables that will be used in your shader
	model_location = glGetUniformLocation(shaderProgramID, "model");
	view_mat_location = glGetUniformLocation(shaderProgramID, "view");
	proj_mat_location = glGetUniformLocation(shaderProgramID, "proj");
	ortho_mat_location = glGetUniformLocation(shaderProgramID, "ortho");

	float lightX, lightZ;
	lightX = Light_rad * cos((Light_angle / 180) * PI);
	lightZ = Light_rad * sin((Light_angle / 180) * PI);
	int vertexColorLocation = glGetUniformLocation(shaderProgramID, "LightPosition");
	glUniform4f(vertexColorLocation, lightX, LightPosition.v[1], lightZ, LightPosition.v[3]);

	float tempLd = ld;
	tempLd = tempLd >= 2 ? 2 : tempLd;
	tempLd = tempLd <= 0 ? 0 : tempLd;
	int Ld_ = glGetUniformLocation(shaderProgramID, "Ld");
	glUniform3f(Ld_, tempLd, tempLd, tempLd);

	int model_color = glGetUniformLocation(shaderProgramID, "color");
	glBindVertexArray(cube_vao);
	for (int i = -6; i <= 6; i++) {
		for (int j = -6; j <= 6; j++) {
			glUniform3f(model_color, 221 / 255.0, 229 / 255.0, 232 / 255.0);
			draw_cube(cube, 5, vec3(i * 2, 0, j * 2));

			glUniform3f(model_color, 139.0 / 255.0, 166 / 255.0, 76 / 255.0);
			if (i == -6 || i == 6 || j == -6 || j == 6) {
				draw_cube(cube, 5, vec3(i * 2, 1.5, j * 2));
			} else if (i % 2 == 0 && j % 2 ==0) {
				draw_cube(cube, 5, vec3(i * 2, 1.5, j * 2));
			}
		}
	}

	if (isShowBomb) {
		DWORD curr_time = timeGetTime();
		glBindVertexArray(bomb_vao);
		glUniform3f(model_color, 143.0/255.0f, 95.0/255.0f, 170.0/255.0f);
		draw_cube(bomb, 20, vec3(BombPosX, 2, BombPosZ));
		if (curr_time - lastSpacePressTime >= 3000) {
			glUniform3f(model_color, 1, 1, 97.0/255.0f);
			draw_cube(bomb, 20, vec3(BombPosX + 1, 2, BombPosZ));
			draw_cube(bomb, 20, vec3(BombPosX + 2, 2, BombPosZ));
			draw_cube(bomb, 20, vec3(BombPosX - 1, 2, BombPosZ));
			draw_cube(bomb, 20, vec3(BombPosX - 2, 2, BombPosZ));
			draw_cube(bomb, 20, vec3(BombPosX, 2, BombPosZ + 1));
			draw_cube(bomb, 20, vec3(BombPosX, 2, BombPosZ + 2));
			draw_cube(bomb, 20, vec3(BombPosX, 2, BombPosZ - 1));
			draw_cube(bomb, 20, vec3(BombPosX, 2, BombPosZ - 2));
		}
	}

	glBindVertexArray(bomberman_vao);
	glUniform3f(model_color, 1, 1, 1);
	draw_cube(bomberman, 6, vec3(BombermanPosX, 2, BombermanPosZ));

	glBindVertexArray(pumpkin_vao);
	glUniform3f(model_color, 215.0 /255.0f, 85.0 /255.0, 40.0 /255.0f);
	for (int i = 0; i < 5; i++) {
		draw_cube(pumpkin, 25, pumpkin_pos[i]);
	}

	glutSwapBuffers();
}


void updateScene() {

	static DWORD last_time = 0;
	DWORD curr_time = timeGetTime();
	if (last_time == 0)
		last_time = curr_time;
	float delta = (curr_time - last_time) * 0.001f;
	last_time = curr_time;

	if (curr_time - lastSpacePressTime >= 3500) {
		isShowBomb = false;
		lastSpacePressTime = 0;
	}

	if (isRotate) {
		// Rotate the model slowly around the y axis at 20 degrees per second
		rotate_y += 20.0f * delta;
		rotate_y = fmodf(rotate_y, 360.0f);
		std::cout << rotate_y << " x:" << 14 * cos(glm::radians(rotate_y)) << " z:" << 14 * sin(glm::radians(rotate_y)) << std::endl;
	}
	int camera_len = sqrt(camera_x * camera_x + camera_z * camera_z);
	scene.view = look_at(vec3(camera_len * cos(glm::radians(rotate_y)), camera_y, camera_len * sin(glm::radians(rotate_y))), vec3(0.0f, 0.0f, 0.0f), vec3(0, 1, 0));
	scene.model = identity_mat4();
	//scene.model = rotate_y_deg(scene.model, rotate_y);
	//scene.model = rotate_y_deg(scene.model, rotate_y);
	//scene.model = translate(scene.model, vec3(x_pos, y_pos, z_pos));

	// Draw the next frame
	glutPostRedisplay();
}


void init()
{
	srand((unsigned)time(0));

	// Set up the shaders
	GLuint shaderProgramID = CompileShaders();
	cube = load_mesh("Cargo.dae"); // box
	bomb = load_mesh("bomb.dae"); // box
	pumpkin = load_mesh("pumpkin.dae"); // box
	bomberman = load_mesh("bomberman.dae"); // box


	glGenVertexArrays(1, &cube_vao);
	glBindVertexArray(cube_vao);
	glGenBuffers(1, &cube_vp_vbo);
	glGenBuffers(1, &cube_vn_vbo);
	generateObjectBufferMesh(cube, cube_vao, cube_vp_vbo, cube_vn_vbo);

	glGenVertexArrays(1, &bomb_vao);
	glBindVertexArray(bomb_vao);
	glGenBuffers(1, &bomb_vp_vbo);
	glGenBuffers(1, &bomb_vn_vbo);
	generateObjectBufferMesh(bomb, bomb_vao, bomb_vp_vbo, bomb_vn_vbo);

	glGenVertexArrays(1, &pumpkin_vao);
	glBindVertexArray(pumpkin_vao);
	glGenBuffers(1, &pumpkin_vp_vbo);
	glGenBuffers(1, &pumpkin_vn_vbo);
	generateObjectBufferMesh(pumpkin, pumpkin_vao, pumpkin_vp_vbo, pumpkin_vn_vbo);

	glGenVertexArrays(1, &bomberman_vao);
	glBindVertexArray(bomberman_vao);
	glGenBuffers(1, &bomberman_vp_vbo);
	glGenBuffers(1, &bomberman_vn_vbo);
	generateObjectBufferMesh(bomberman, bomberman_vao, bomberman_vp_vbo, bomberman_vn_vbo);

	pumpkin_pos = rand_pumpkin_pos();
}



void keyPress(unsigned char key, int xmouse, int ymouse) {
	DWORD currentTime = 0;
	//std::cout << "Keypress: " << key << " X:" << x_pos << " Z:" << z_pos << std::endl;
	std::cout << "Keypress: " << key << " X:" << BombermanPosX << " Z:" << BombermanPosZ << std::endl;
	//std::cout << "Keypress: " << key << " X:" << camera_x << " Y:" << camera_y << " Z:" << camera_z << std::endl;
	switch (key) {
		// Main object movement
		case('w'):
			BombermanPosZ -= 1.0f;
			break;
		case('a'):
			BombermanPosX -= 1.0f;
			break;
		case('s'):
			BombermanPosZ += 1.0f;
			break;
		case('d'):
			BombermanPosX += 1.0f;
			break;
		case(' '):
			currentTime = timeGetTime();
			if (currentTime - lastSpacePressTime >= 3000) {
				BombPosX = BombermanPosX;
				BombPosZ = BombermanPosZ;
				isShowBomb = true;
				cout << " space !!!   " << BombPosX << ", " << BombPosZ << endl;
				lastSpacePressTime = currentTime;
			}
			break;
		case('r'):
			isRotate = !isRotate;
			break;
		case('t'):
			rotate_y = 90.0f;
			break;
		case('q'):
			exit(0);
	}
};
void mouseMove(int x, int y) {
	// Fix x_mouse coordinates calculation to only take the first viewport
	camera_x = (float)-(x - WIDTH / 2) / (WIDTH / 2);
	camera_y = (float)-(y - HEIGHT / 2) / (HEIGHT / 2);
};

void mouseWheel(int key, int wheeldir, int x, int y) {
	if (wheeldir == 1)
	{
		camera_y -= 1.f;
	}
	else if (wheeldir == -1) {
		camera_y += 1.f;
	}
}

void specialKeypress(int key, int x, int y) {
	switch (key) {
		// Specular Lighting settings, up for up, down for down
		case(GLUT_KEY_UP):
			ld += 0.1;
			break;
		case(GLUT_KEY_DOWN):
			ld -= 0.1;
			break;
		case(GLUT_KEY_LEFT):
			if (isRotate) {

			} else {
				Light_angle += 10.0;
			}
			break;
		case(GLUT_KEY_RIGHT):
			Light_angle -= 10.0;
			break;
	}
};

void instructions()
{
	cout
		<< "##########################################\n"
		<< "###########      COMMANDS      ###########\n"
		<< "##########################################\n"
		<< "#              Map Transfer              #\n"
		<< "##########################################\n"
		<< "#   'w' --> move the map up              #\n"
		<< "#   's' --> move the map down            #\n"
		<< "#   'a' --> move the map left            #\n"
		<< "#   'd' --> move the map right           #\n"
		<< "##########################################\n"
		<< "#        Arrow keys modify the light     #\n"
		<< "##########################################\n"
		<< "#  'up'    --> modify the density        #\n"
		<< "#  'down'  --> modify the density        #\n"
		<< "#  'left'  --> modify the angle of light #\n"
		<< "#  'right' --> modify the angle of light #\n"
		<< "##########################################\n"
		<< "#          'q' --> quit the game         #\n"
		<< "# 'mouse wheel' --> adjust camera angle  #\n"
		<< "##########################################\n"
		<< "##########################################\n";
}

void reshape(int width, int height) {
	WIDTH = width;
	HEIGHT = height;
	glViewport(0, 0, (GLint)width, (GLint)height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	//gluPerspective(70.0, width / (float)height, 0.1, 30.0);
	glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char** argv) {
	srand(time(NULL)); // Initialize the random number generator

	instructions();
	// Creating model initial values
	// Model 1 View
	scene.projection = perspective(80, (float)(WIDTH) / (float)HEIGHT, 0.1, 100.0);
	scene.view = identity_mat4();
	scene.model = identity_mat4();
	scene.ortho = identity_mat4();

	// Set up the window
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutCreateWindow("Bomberman");

	// Tell glut where the display function is
	glutDisplayFunc(display);
	glutReshapeFunc(reshape);
	glutIdleFunc(updateScene);
	glutKeyboardFunc(keyPress);
	glutSpecialFunc(specialKeypress);
	//glutPassiveMotionFunc(mouseMove);
	glutMouseWheelFunc(mouseWheel);

	// A call to glewInit() must be done after glut is initialized!
	GLenum res = glewInit();
	// Check for any errors
	if (res != GLEW_OK) {
		fprintf(stderr, "Error: '%s'\n", glewGetErrorString(res));
		return 1;
	}
	// Set up your objects and shaders
	init();
	// Begin infinite event loop
	glutMainLoop();
	return 0;
}
