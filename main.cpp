#define STB_IMAGE_IMPLEMENTATION

#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include<iostream>
#include<Shader.h>

#include <stb_image.h>
#include<glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <filesystem>
#include "Chapters.h"

#include "Camera.h"
void resize_viewport(GLFWwindow* window, int height, int width) {
	glViewport(0, 0, 800, 800);
};

void detect_keypress(GLFWwindow* window) {
	if (glfwGetKey(window,GLFW_KEY_ENTER) == true) {
		glfwSetWindowShouldClose(window, true);
	}
}
void detect_opac(GLFWwindow* window, Shader shader, const char* name, float& value) {
	if (glfwGetKey(window, GLFW_KEY_UP) == true) {
		value += 0.001f;
		if (value >= 1.0) {
			value = 1.0f;
		}

		shader.setFloat(name, value);
	}
	else if (glfwGetKey(window, GLFW_KEY_DOWN) == true) {
		value -= 0.001f;
		if (value <= 0.2) {
			value =0.2f;
		}
		shader.setFloat(name, value);
	}
}
glm::vec3 CameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
glm::vec3 CameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 CameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
cam1 camera(CameraPos);

void CameraControllerKeyboard(GLFWwindow* window, float& deltaTime) {
	if (glfwGetKey(window, GLFW_KEY_W) == true) {
		camera.ProcessKeyboard(FORWARD,deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_S) == true) {
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_A) == true) {
		camera.ProcessKeyboard(RIGHT, deltaTime);
	}
	if (glfwGetKey(window, GLFW_KEY_D) == true) {
		camera.ProcessKeyboard(LEFT, deltaTime);
	}
}
bool firstTime = true;
float lastX = 400.0f, lastY = 300.0f;
void CameraControllerMouse(GLFWwindow* window, double xposi, double yposi) {
	float xpos = static_cast<float>(xposi);
	float ypos = static_cast<float>(yposi);
	if (firstTime) {
		xpos = lastX;
		ypos = lastY;
		firstTime = false;
	}
	//get cursor offset for each frame
	float xOffset = xpos - lastX;
	float yOffset = lastY-ypos;
	lastX = xpos;
	lastY = ypos;
	camera.ProcessMouseMovement(xOffset, yOffset, true);
}
void scroll_utility(GLFWwindow* window, double xOffset, double yOffset) {
	camera.ProcessMouseScroll(yOffset);
};

unsigned int loadTexture(const char* path) {
	//image loading
	int height, width, nrChannels;
	unsigned char* data = stbi_load(path, &width, &height, &nrChannels, 0);

	//initialize a texture ID and generate a texture object
	unsigned int texture;
	glGenTextures(1, &texture);
	//bind it to the 2D texture type(unique throughout the whole program, just like VBO and EBO types)
	glBindTexture(GL_TEXTURE_2D, texture);
	//set texture parameters, mipmaps are enabled so configure them too. We're just dealing with wrapping and filtering mecchanisms here
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//if data is loaded by stbi, link the previously bound texture to the image data(array of pixels) using glTexImage2D
	if (data) {
		GLenum format;
		if (nrChannels == 1) {
			format = GL_RED;
		}
		else if (nrChannels == 3) {
			format = GL_RGB;
		}
		else if (nrChannels == 4) {
			format = GL_RGBA;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cout << "Failed to load image" << endl;
	}
	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(true);
	return texture;
}
//lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);
int main() {
	/*translation code, the matrix formed will be of the form: [1,0,0,Tx],[0,1,0,Ty],[0,0,1,Tz],[0,0,0,1] so that when it gets transformed, it will have this effect on the vector:
	* result=[vec.x+Tx,vec.y+Ty.vec.z+Tz,1]
	glm::vec4 vec(1.0f, 0.0f, 0.0f, 1.0f);
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(1.0f,1.0f,0.0f));
	glm::vec4 result = trans * vec;
	cout << result.x << result.y << result.z << endl;*/

	//Initialize glfw
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


	//create a window and send it to the current context
	GLFWwindow* window = glfwCreateWindow(800, 600, "New window", NULL, NULL);
	if (window == NULL) {
		printf("Failed to create window");
		glfwTerminate();
	}
	glfwMakeContextCurrent(window);
	
	//initialize glad so that you can load in opengl function pointers
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
		printf("Couldn't load GLAD");
		return -1;
	}
	//viewport is a subset of window, can be smaller than the window itself
	glViewport(0, 0, 800, 600);

	//called in case the window size changes during program execution
	glfwSetFramebufferSizeCallback(window, resize_viewport);
	//to hide the cursor and capture it
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	glfwSetCursorPosCallback(window, CameraControllerMouse);
	glfwSetScrollCallback(window, scroll_utility);
	Shader lightCubeshader("C:/Users/Lenovo/Desktop/vertexshadersource.txt", "C:/Users/Lenovo/Desktop/fragshadersource.txt");

	glEnable(GL_DEPTH_TEST);
	//define the vertices and the indices of the shap you're gonna draw
	float vertices1[] = {
		// positions         // colors   
		 0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
		-0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f, // bottom left
		 0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f,     // top 
	};

	float vertices2[]={
	-0.45f, 0.5f, 0.0f,
	-0.9f,-0.5f,0.0f,
	0.0f,-0.5f,0.0f
	};

	unsigned int indices[] = {
		0,1,3,
		1,2,3
	};

	float tex_vertices[] = {
		// positions          // colors           // texture coords
	 0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
	 0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
	-0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
	-0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f   // top left 
	};

	float cube_vertices[] = {
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,

	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 1.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,

	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,

	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f, -0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f, -0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,

	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,
	 0.5f,  0.5f, -0.5f,  1.0f, 1.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	 0.5f,  0.5f,  0.5f,  1.0f, 0.0f,
	-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,
	-0.5f,  0.5f, -0.5f,  0.0f, 1.0f
	};
	/*define the buffer to store and keep the vertices in the GPU during program execution
	VAO->Contains the vertex attribute pointers which point to vertex buffer objects and the element buffer objects
	VBO->Contains the vertices, is in the GPU
	EBO->Contains the indices
	*/
	unsigned int VBO, VAO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	//glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(cube_vertices), cube_vertices, GL_STATIC_DRAW);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	/*arg descriptions for glVertexAttribArray:
	arg0->The vertex attribute you want to start at(index in the VAO, similar to the location defined in the shader)
	arg1->Size of a single vertex attribute
	arg2->Type of the values stored in the vertex attributes
	arg3->Normalized or not
	arg4->stride in bits(the amount of data you have to traverse before getting to an attribute of the same type)
	arg5->initial offset in bits*/
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	/*glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);*/
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5*sizeof(float),(void*)(3*sizeof(float)));
	glEnableVertexAttribArray(1);
	/*glBindVertexArray(VAO[1]);
	glBindBuffer(GL_ARRAY_BUFFER, VBO[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices2),vertices2,GL_STATIC_DRAW);
	
	//we need to tell the GPU how to interpret the data stored in the buffer by using vertex attributes, we use the 0th attribute here
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3*sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);*/

	

	

	/*data = stbi_load("C:/Users/Lenovo/Desktop/awesomeface.png", &width, &height, &nrChannels, 0);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	//if data is loaded by stbi, link the previously bound texture to the image data(array of pixels) using glTexImage2D
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	else {
		cout << "Failed to load image" << endl;
	}
	stbi_image_free(data);*/
	//we use sampler2D uniform in our shader to initialize a texture object, we then use GLSL's built-in texture function to return the texture coordinates
	//here, we are setting the uniform's value using our shader's utility function
	/*shader.setInt("texture", 0);
	shader.setInt("texture1", 1);
	float opacity_value = 0.2;
	shader.setFloat("opacity", opacity_value);*/
	
	glm::vec3 cubePositions[] = {
		glm::vec3(0.0f,  0.0f,  0.0f),
		glm::vec3(2.0f,  5.0f, -15.0f),
		glm::vec3(-1.5f, -2.2f, -2.5f),
		glm::vec3(-3.8f, -2.0f, -12.3f),
		glm::vec3(2.4f, -0.4f, -3.5f),
		glm::vec3(-1.7f,  3.0f, -7.5f),
		glm::vec3(1.3f, -2.0f, -2.5f),
		glm::vec3(1.5f,  2.0f, -2.5f),
		glm::vec3(1.5f,  0.2f, -1.5f),
		glm::vec3(-1.3f,  1.0f, -1.5f)
	};

	float deltaTime = 0.0f;
	float prevTime = 0.0f;

	Shader cubeShader("C:/Users/Lenovo/Desktop/vertexshadersource1.txt", "C:/Users/Lenovo/Desktop/fragshadersource1.txt");
	//Shader lightCubeShader("C:/Users/Lenovo/Desktop/vertexshadersource.txt", "C:/Users/Lenovo/Desktop/fragshadersource.txt");
	float vertices[] = {
		// positions          // normals           // texture coords
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,
		 0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  1.0f, 1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.0f, 0.0f,

		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   1.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f,  0.0f, 1.0f,   0.0f, 0.0f,

		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  0.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f,
		 0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 1.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		 0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  0.0f, 1.0f
	};

	unsigned int cubeVAO, LVBO;
	glGenVertexArrays(1, &cubeVAO);
	glGenBuffers(1, &LVBO);
	glBindVertexArray(cubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, LVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2,2,GL_FLOAT,GL_FALSE, 8*sizeof(float),(void*)(6*sizeof(float)));
	glEnableVertexAttribArray(2);

	unsigned int lightCubeVAO;
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, LVBO);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);

	

	unsigned int diffuseMap = loadTexture("C:/Users/Lenovo/Desktop/container2.png");
	unsigned int specularMap = loadTexture("C:/Users/Lenovo/Desktop/container2_specular.png");

	glm::vec3 pointLightPositions[] = {
	glm::vec3(0.7f,  0.2f,  2.0f),
	glm::vec3(2.3f, -3.3f, -4.0f),
	glm::vec3(-4.0f,  2.0f, -12.0f),
	glm::vec3(0.0f,  0.0f, -3.0f)
	};
	//run a rendering loop to display the window continuously until a certain button is pressed
	/*while (!glfwWindowShouldClose(window)) {
		detect_keypress(window);
	
		glClearColor(0.1f, 0.5f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
		
		//use the shader program and draw the shape
		//activate a texture unit(default location of the texture) before binding it

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texture);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, texture1);
		shader.use();

		//calculating deltaTime for uniform camera speed
		float curTime = glfwGetTime();
		deltaTime =2.5f*( curTime - prevTime);
		prevTime = curTime;
		//defining a camera-like effect with the view matrix
		CameraControllerKeyboard(window, deltaTime);
		glm::mat4 view = camera.GetViewMatrix();
		shader.setMatrix("view", view);


		//defing a transformation and feeding it to the shader
		//a typical projective transformation is: M_clip=M_projection*M_view*M_model*vertices
		//generate as many cubes as you want to in the for loop
		for (int i = 0;i < 10;i++) {
			glm::mat4 model = glm::mat4(1.0f);

			model = glm::translate(model, cubePositions[i]);
			if (i % 3 == 0) {
				model = glm::rotate(model, (float)(glfwGetTime() * glm::radians(20.0f * i + 10.0f)), glm::vec3(1.0f, 0.4f, 0.2f));
			}
			else {
				model = glm::rotate(model, glm::radians(20.0f * i + 10.0f), glm::vec3(1.0f, 0.4f, 0.2f));
			}
			
			shader.setMatrix("model", model);
			
			detect_opac(window, shader, "opacity", opacity_value);
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		glm::mat4 projection = glm::mat4(1.0f);
		projection = glm::perspective(glm::radians(camera.Zoom), 800.0f / 600.0f, 0.1f, 100.0f);
		shader.setMatrix("projection", projection);
		/*shader.use();
		glm::mat4 trans1 = glm::mat4(1.0f);
		trans1 = glm::translate(trans1,glm::vec3(-0.5f,0.5f,0.0f));
		trans1 = glm::scale(trans1, glm::vec3(sin(glfwGetTime()),sin(glfwGetTime()),0.0f));
		shader.setMatrix("transform", trans1);
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);*/
		/*glUseProgram(shader_program1);
		//dynamically uodate the color of the 2nd triangle by linking its uniform to time in the rendering loop
		float time = glfwGetTime();
		float color_val = sin(time)/2.0f + 0.5f;
		float color_val2 = cos(time) / 2.0f + 0.5f;
		//uniforms are global and unique to their shaders unlike vertex attributes, so you gotta get the CPU location of the uniform and set its value to color_val
		int vertexColorLocation = glGetUniformLocation(shader_program1, "dynColor");
		glUniform4f(vertexColorLocation, color_val2,color_val , color_val2, 1.0f);

		glBindVertexArray(VAO[1]);
		glDrawArrays(GL_TRIANGLES, 0, 3);*/
		//glDrawElements(GL_TRIANGLES,6,GL_UNSIGNED_INT,0);*/
	while(!glfwWindowShouldClose(window)){
		detect_keypress(window);
		glClearColor(0.05f,0.05f,0.05f,0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,diffuseMap);
		glActiveTexture(GL_TEXTURE1);
		glBindTexture(GL_TEXTURE_2D, specularMap);
		float curTime = glfwGetTime();
		deltaTime = curTime - prevTime;
		prevTime = curTime;
		CameraControllerKeyboard(window,deltaTime);
		cubeShader.use();
		cubeShader.setInt("material.diffuse", 0);
		cubeShader.setInt("material.specular", 1);
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)800 / (float)600, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		cubeShader.setMatrix("projection", projection);
		cubeShader.setMatrix("view", view);
		cubeShader.setFloat("light.constant", 1.0f);
		cubeShader.setFloat("light.linear", 0.09f);
		cubeShader.setFloat("light.quadratic", 0.032f);
		glm::vec3 lightambience = glm::vec3(0.2f, 0.2f, 0.2f);
		glm::vec3 lightdiffuse = glm::vec3(0.5f, 0.5f, 0.5f);
		glm::vec3 lightspecular = glm::vec3(1.0f, 1.0f, 1.0f);
		// world transformation
		//glm::mat4 model = glm::mat4(1.0f);
		//cubeShader.setMatrix("model", model);
		//setting lighting properties
		cubeShader.setVec3("lightPos", camera.Position);
		cubeShader.setVec3("viewPos", camera.Position);
		cubeShader.setVec3("light.direction", camera.Front);
		cubeShader.setFloat("light.outerCutoff", glm::cos(glm::radians(17.5f)));
		cubeShader.setFloat("light.cutoff", glm::cos(glm::radians(12.5f)));
		//stuff related to materials(object Color
		//cubeShader.setVec3("material.ambient", glm::vec3(1.0f, 0.5f, 0.31f));
		//cubeShader.setVec3("material.diffuse", glm::vec3(1.0f, 0.5f, 0.31f));
		//cubeShader.setVec3("material.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		cubeShader.setFloat("material.shininess", 64.0f);
		//stuff related to Light color
		cubeShader.setVec3("light.ambience", lightambience);
		cubeShader.setVec3("light.diffusion", lightdiffuse); 
		cubeShader.setVec3("light.specularity", lightspecular);
		// directional light
		cubeShader.setVec3("dirLight.direction", glm::vec3( - 0.2f, -1.0f, -0.3f));
		cubeShader.setVec3("dirLight.ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeShader.setVec3("dirLight.diffuse", glm::vec3(0.4f, 0.4f, 0.4f));
		cubeShader.setVec3("dirLight.specular", glm::vec3(0.5f, 0.5f, 0.5f));
		// point light 1
		cubeShader.setVec3("pointLights[0].position", pointLightPositions[0]);
		cubeShader.setVec3("pointLights[0].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeShader.setVec3("pointLights[0].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeShader.setVec3("pointLights[0].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("pointLights[0].constant", 1.0f);
		cubeShader.setFloat("pointLights[0].linear", 0.09f);
		cubeShader.setFloat("pointLights[0].quadratic", 0.032f);
		// point light 2
		cubeShader.setVec3("pointLights[1].position", pointLightPositions[1]);
		cubeShader.setVec3("pointLights[1].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeShader.setVec3("pointLights[1].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeShader.setVec3("pointLights[1].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("pointLights[1].constant", 1.0f);
		cubeShader.setFloat("pointLights[1].linear", 0.09f);
		cubeShader.setFloat("pointLights[1].quadratic", 0.032f);
		// point light 3
		cubeShader.setVec3("pointLights[2].position", pointLightPositions[2]);
		cubeShader.setVec3("pointLights[2].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeShader.setVec3("pointLights[2].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeShader.setVec3("pointLights[2].specular", glm::vec3(1.0f, 1.0f, 1.0f));
	    cubeShader.setFloat("pointLights[2].constant", 1.0f);
		cubeShader.setFloat("pointLights[2].linear", 0.09f);
		cubeShader.setFloat("pointLights[2].quadratic", 0.032f);
		// point light 4
	    cubeShader.setVec3("pointLights[3].position", pointLightPositions[3]);
		cubeShader.setVec3("pointLights[3].ambient", glm::vec3(0.05f, 0.05f, 0.05f));
		cubeShader.setVec3("pointLights[3].diffuse", glm::vec3(0.8f, 0.8f, 0.8f));
		cubeShader.setVec3("pointLights[3].specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("pointLights[3].constant", 1.0f);
		cubeShader.setFloat("pointLights[3].linear", 0.09f);
		cubeShader.setFloat("pointLights[3].quadratic", 0.032f);
		// spot light
		cubeShader.setVec3("spotLight.position", camera.Position);
		cubeShader.setVec3("spotLight.direction", camera.Front);
		cubeShader.setVec3("spotLight.ambient", glm::vec3(0.0f, 0.0f, 0.0f));
		cubeShader.setVec3("spotLight.diffuse", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setVec3("spotLight.specular", glm::vec3(1.0f, 1.0f, 1.0f));
		cubeShader.setFloat("spotLight.constant", 1.0f);
		cubeShader.setFloat("spotLight.linear", 0.09f);
		cubeShader.setFloat("spotLight.quadratic", 0.032f);
		cubeShader.setFloat("spotLight.cutOff", glm::cos(glm::radians(12.5f)));
		cubeShader.setFloat("spotLight.outerCutoff", glm::cos(glm::radians(15.0f)));
		// render the cube
		for (int i = 0; i < 10;i++) {
			glm::vec3 trans = cubePositions[i];
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, trans);
			cubeShader.setMatrix("model", model);
			glBindVertexArray(cubeVAO);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}

		lightCubeshader.use();
		
		glm::mat4 lightmodel = glm::mat4(1.0f);
		lightmodel = glm::translate(lightmodel, lightPos);

		lightCubeshader.setMatrix("projection", projection);
		lightCubeshader.setMatrix("view", view);
		for (unsigned int i = 0; i < 4; i++)
		{
			glm::mat4 model = glm::mat4(1.0f);
			model = glm::translate(model, pointLightPositions[i]);
			model = glm::scale(model, glm::vec3(0.2f)); // Make it a smaller cube
			lightCubeshader.setMatrix("model", model);
			glDrawArrays(GL_TRIANGLES, 0, 36);
		}
		lightCubeshader.setVec3("light.ambience", lightambience);
		lightCubeshader.setVec3("light.diffusion", lightdiffuse);
		lightCubeshader.setVec3("light.specularity", lightspecular);
		glBindVertexArray(lightCubeVAO);
		glDrawArrays(GL_TRIANGLES,0,36);
		glfwPollEvents();
		glfwSwapBuffers(window);
		
	}
	glfwTerminate();

	
	return 0;
}







