#include <stdio.h>
#include <string.h>
#include <cmath>

#include<GL/glew.h>
#include<GLFW/glfw3.h>

#include<glm/glm.hpp>
#include<glm/gtc/matrix_transform.hpp>
#include<glm/gtc/type_ptr.hpp>

// Window dimentions
const GLint WIDTH = 800, HEIGHT = 600;
// We'll use this to convert degree to radian for rotation
const float toRadians = 3.14159265f / 180.0f;

// These will hold the ids of out VAO and VBO. We need this id to get out objects from the graphics memory
GLuint VAO, VBO, shader, uniformModel;

// Vertex Shader
static const char* vShader = "											\n\
#version 330															\n\
																		\n\
// create a layout in location '0' what will take						\n\
// a vector of size 3 as an input what we'll call						\n\
// 'pos'. If we don't define the location, it will						\n\
// be automatically generated and so we have to							\n\
// query it later														\n\
layout(location = 0) in vec3 pos;										\n\
																		\n\
out vec4 vCol;															\n\
																		\n\
uniform mat4 model;														\n\
																		\n\
void main()																\n\
{																		\n\
// 'gl-Position' is a size 4 vector object								\n\
// that already exists. In this case, we're								\n\
// assigning it to a new vector with the								\n\
// x, y, z position we took as input and then passing					\n\
// '1' as the extra variable since out input is of						\n\
// size 3. Also we're scaling the output by 0.4							\n\
	gl_Position = model * vec4(pos, 1.0);								\n\
// 'clamp' will map the negatives values that are outside 				\n\
// '0.0' and '1.0' range and map them into that range					\n\
	vCol = vec4(clamp(pos, 0.0f, 1.0f), 1.0f);							\n\
}";

// Fragment shader
static const char* fShader ="								\n\
#version 330												\n\
															\n\
// Fragment shader only has one output. So in this			\n\
// case we're outputting the color as a 4 size vector		\n\
// that will represent R, G, B, A							\n\
															\n\
in vec4 vCol;												\n\
out vec4 colour;											\n\
															\n\
void main()													\n\
{															\n\
	colour = vCol;											\n\
}";


void CreateTriangle()
{
	// These are the vertices of the triangle. Each row represents (x, y, z)
	GLfloat vertices[] = {
		-1.0f, -1.0f, 0.0f,
		1.0f, -1.0f, 0.0f,
		0.0f, 1.0f, 0.0f
	};
	
	// Now we're creating '1' array or VAO and storing its id to the VAO variable
	glGenVertexArrays(1, &VAO);
	// Now we want any openGL function that we call and buffer to use this particular VAO. We're 'selecting' it
	glBindVertexArray(VAO);

	// Now we'll create '1' Buffer object and store it's id to the VBO variable
	glGenBuffers(1, &VBO);
	// We are binding the VBO to the currently selected array which will be the previously made VAO in this case
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	// Now we're putting our vertices data into the VBO 
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	
	// '0' is simply the id we're giving to the attribute of our vertices. Vertices may ontain many attributes. We only have one which is posiiton.
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	// Now we're enabling the attribute
	glEnableVertexAttribArray(0);

	// After we're done with our VAO and VBO, We will release or 'deselect' them. '0' means we're binding to nothing
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindVertexArray(0);
}

void AddShader(GLuint theProgram, const char* shaderCode, GLenum shaderType)
{
	// Create a shader of the required type
	GLuint theShader = glCreateShader(shaderType);
	
	// Grabbing the pointer to the share code
	const GLchar* theCode[1];
	theCode[0] = shaderCode;
	
	GLint codeLength[1];
	codeLength[0] = strlen(shaderCode);
	
	//preparing the shader source to be processed
	glShaderSource(theShader, 1, theCode, codeLength);
	// Now we'll compile the shader
	glCompileShader(theShader);

	// Error checking
	GLint result = 0;
	GLchar eLog[1024] = { 0 };

	glGetShaderiv(theShader, GL_COMPILE_STATUS, &result);

	if (!result)
	{	
		// This funtion will fetch the error log
		glGetShaderInfoLog(theShader, sizeof(eLog), NULL, eLog);
		printf("Error compiling the %d shader: '%s'\n", shaderType, eLog);
		return;
	}
	

	// If the shader is successfully compiled, we'll attach it to our program
	glAttachShader(theProgram, theShader);
}

void CompileShaders()
{
	// We're telling the gpu to create a new program for us and store its id in the 'shader' variable
	shader = glCreateProgram();

	if (!shader)
	{
		printf("Error creating shader program!\n");
		return;
	}
	
	// Adding our vertex and fragment shaders into the program
	AddShader(shader, vShader, GL_VERTEX_SHADER);
	AddShader(shader, fShader, GL_FRAGMENT_SHADER);
	
	// We are using a flag 'result' to detect errors and 'eLog' char array to store the error message
	GLint result = 0;
	GLchar eLog[1024] = { 0 };
	
	// We will try to link the program and then check for errors
	glLinkProgram(shader);
	// This function will get the result of a certaion operation depending on the enum in the middle
	glGetProgramiv(shader, GL_LINK_STATUS, &result);

	if (!result)
	{	
		// This function will fetch the error log
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("Error linking program: '%s'\n", eLog);
		return;
	}
	
	// Now we'll validate the program and check for errors
	glValidateProgram(shader);
	glGetProgramiv(shader, GL_VALIDATE_STATUS, &result);

	if (!result)
	{
		glGetProgramInfoLog(shader, sizeof(eLog), NULL, eLog);
		printf("Error validating program: '%s'\n", eLog);
		return;
	}
	
	// Grabbing the location of the uniform variable
	uniformModel = glGetUniformLocation(shader, "model");
}

int main()
{
	// Initialize GLFW
	if (!glfwInit())
	{
		printf("GLFW initialization failed");
		glfwTerminate();
		return 1;
	}

	// Setup GLFW window properties
	// OpenGL version (We're choosing version 3.3)
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	// We're setting our OpenGL profile to use just core features to avoid all the old and deprecated features.
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	// We want out program to be forward compatible
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	
	// Creating the window
	GLFWwindow* mainWindow = glfwCreateWindow(WIDTH, HEIGHT, "Test Window", NULL, NULL);
	if (!mainWindow)
	{
		printf("GLFW window creation failed!");
		glfwTerminate();
		return 1;
	}
	
	// Get Buffer size information
	int bufferWidth, bufferHeight;
	glfwGetFramebufferSize(mainWindow, &bufferWidth, &bufferHeight);

	// Set context for GLEW to use
	glfwMakeContextCurrent(mainWindow);

	// Allow mordern extension features (These are experimental features)
	// Optional
	glewExperimental = GL_TRUE;

	if (glewInit() != GLEW_OK)
	{
		printf("GLEW initialization failed!");
		// Destroy the window and then GLFW
		glfwDestroyWindow(mainWindow);
		glfwTerminate();
	}

	// Setup Viewport size
	glViewport(0, 0, bufferWidth, bufferHeight);

	CreateTriangle();
	CompileShaders();

	// Loop until window closed
	while (!glfwWindowShouldClose(mainWindow))
	{
		// Get + Handle user input events. We need this to get access to the user inputs for example the signal of pressing the close 'X' button
		glfwPollEvents();

		// Clear window
		// We will clear the window but with a color, in this case its black. Hence this RGBA combo. the 'glClear' function will use these values
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		// There are other properties of buffers. We just want to clear the 'color' property
		glClear(GL_COLOR_BUFFER_BIT);
		// Start the program
		glUseProgram(shader);
		
		// Passing 1.0f will set it to identity matrix
		glm::mat4 model (1.0f);
		// This will scale
		model = glm::scale(model, glm::vec3(0.4f, 0.4f, 1.0f));

		glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));

		glBindVertexArray(VAO);
		glDrawArrays(GL_TRIANGLES, 0, 3);
		glBindVertexArray(0);
		
		// Close the program
		glUseProgram(0);
		
		// We have 2 windows in the buffer. One is what the user is seeing and the other is the one we're drawing that will be shown later
		// After creating the new buffer, we'll just swap it with the exising one so the user can see that
		glfwSwapBuffers(mainWindow);
	}

	return 0;
}