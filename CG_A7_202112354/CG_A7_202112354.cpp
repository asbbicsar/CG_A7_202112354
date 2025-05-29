#include <GL/glew.h>
#include <GL/GL.h>
#include <GL/freeglut.h>

#define GLFW_INCLUDE_GLU
#define GLFW_DLL
#include <GLFW/glfw3.h>
#include <vector>

#define GLM_SWIZZLE
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <fstream>
#include <sstream>
#include <string>
#include <iostream>

#define M_PI 3.1415926535

using namespace glm;

int     gNumVertices = 0;    // Number of 3D vertices.
int     gNumTriangles = 0;    // Number of triangles.
int*    gIndexBuffer = NULL; // Vertex indices for the triangles.
vec3*   gVertexArray = NULL;

std::vector<glm::vec3> Positions = { };
std::vector<glm::vec3> Normals = { };
std::vector<glm::vec3> Colors = { };
std::vector<unsigned int> Indices = { };

mat4 modelingMatrix = mat4{
    2.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 2.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 2.0f, 0.0f,
    0.0f, 0.0f, -7.0f, 1.0f,
};
mat4 modeling_inv_tr = transpose(inverse(modelingMatrix));
mat4 cameraMatrix = glm::lookAt(
    glm::vec3(0.0f, 0.0f, 0.0f),  // eye
    glm::vec3(0.0f, 0.0f, -1.0f), // center
    glm::vec3(0.0f, 1.0f, 0.0f)   // up
);
mat4 projectionMatrix = frustum(-0.1f, 0.1f, -0.1f, 0.1f, 0.1f, 1000.0f);


vec4 lightPos = vec4(-4.0f, 4.0f, -3.0f, 1.0f);
vec4 lightColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
vec4 ambientColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);
vec4 diffuseColor = vec4(0.0f, 0.5f, 0.0f, 1.0f);
vec4 specularColor = vec4(0.5f, 0.5f, 0.5f, 1.0f);
float intensity = 0.2f;
float gamma = 2.2f;


void create_scene()
{
    int width = 32;
    int height = 16;

    float theta, phi;
    int t;

    gNumVertices = (height - 2) * width + 2;
    gNumTriangles = (height - 2) * (width - 1) * 2;

    // Allocate an array for gNumVertices vertices.
    gIndexBuffer = new int[3 * gNumTriangles];

    t = 0;
    for (int j = 1; j < height - 1; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            theta = (float)j / (height - 1) * M_PI;
            phi = (float)i / (width - 1) * M_PI * 2;

            float   x = sinf(theta) * cosf(phi);
            float   y = cosf(theta);
            float   z = -sinf(theta) * sinf(phi);

            // TODO: Set vertex t in the vertex array to {x, y, z}.
            Normals.push_back(normalize(vec3(x, y, z)));
            Positions.push_back(vec3(x, y, z));
            Colors.push_back(vec3(ambientColor));
            t++;
        }
    }

    // Set vertex t in the vertex array to {0, 1, 0}.
    Normals.push_back(vec3(0.0f, 1.0f, 0.0f));
    Positions.push_back(vec3(0.0f, 1.0f, 0.0f));
    Colors.push_back(vec3(ambientColor));
    t++;

    // Set vertex t in the vertex array to {0, -1, 0}.
    Normals.push_back(vec3(0.0f, -1.0f, 0.0f));
    Positions.push_back(vec3(0.0f, -1.0f, 0.0f));
    Colors.push_back(vec3(ambientColor));
    t++;

    t = 0;
    for (int j = 0; j < height - 3; ++j)
    {
        for (int i = 0; i < width - 1; ++i)
        {
            Indices.push_back(j * width + i);
            Indices.push_back((j + 1) * width + (i + 1));
            Indices.push_back(j * width + (i + 1));
            Indices.push_back(j * width + i);
            Indices.push_back((j + 1) * width + i);
            Indices.push_back((j + 1) * width + (i + 1));
        }
    }
    for (int i = 0; i < width - 1; ++i)
    {
        Indices.push_back((height - 2) * width);
        Indices.push_back(i);
        Indices.push_back(i + 1);
        Indices.push_back((height - 2) * width + 1);
        Indices.push_back((height - 3) * width + (i + 1));
        Indices.push_back((height - 3) * width + i);
    }

    // The index buffer has now been generated. Here's how to use to determine
    // the vertices of a triangle. Suppose you want to determine the vertices
    // of triangle i, with 0 <= i < gNumTriangles. Define:
    //
    // k0 = gIndexBuffer[3*i + 0]
    // k1 = gIndexBuffer[3*i + 1]
    // k2 = gIndexBuffer[3*i + 2]
    //
    // Now, the vertices of triangle i are at positions k0, k1, and k2 (in that
    // order) in the vertex array (which you should allocate yourself at line
    // 27).
    //
    // Note that this assumes 0-based indexing of arrays (as used in C/C++,
    // Java, etc.) If your language uses 1-based indexing, you will have to
    // add 1 to k0, k1, and k2.
}


GLuint LoadShaders(const std::string& vertex_file_path, const std::string& fragment_file_path)
{
    // Create the shaders
    GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
    GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

    // Read the Vertex Shader code from the file
    std::string VertexShaderCode;
    std::ifstream VertexShaderStream(vertex_file_path.c_str(), std::ios::in);
    if (VertexShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << VertexShaderStream.rdbuf();
        VertexShaderCode = sstr.str();
        VertexShaderStream.close();
    }
    else {
        printf("Impossible to open %s. Are you in the right directory? Don't forget to read the FAQ!\n", vertex_file_path.c_str());
        getchar();
        return 0;
    }

    // Read the Fragment Shader code from the file
    std::string FragmentShaderCode;
    std::ifstream FragmentShaderStream(fragment_file_path.c_str(), std::ios::in);
    if (FragmentShaderStream.is_open()) {
        std::stringstream sstr;
        sstr << FragmentShaderStream.rdbuf();
        FragmentShaderCode = sstr.str();
        FragmentShaderStream.close();
    }

    GLint Result = GL_FALSE;
    int InfoLogLength;

    // Compile Vertex Shader
    printf("Compiling shader: %s\n", vertex_file_path.c_str());
    char const* VertexSourcePointer = VertexShaderCode.c_str();
    glShaderSource(VertexShaderID, 1, &VertexSourcePointer, NULL);
    glCompileShader(VertexShaderID);

    // Check Vertex Shader
    glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> VertexShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
        printf("%s\n", &VertexShaderErrorMessage[0]);
    }

    // Compile Fragment Shader
    printf("Compiling shader : %s\n", fragment_file_path.c_str());
    char const* FragmentSourcePointer = FragmentShaderCode.c_str();
    glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer, NULL);
    glCompileShader(FragmentShaderID);

    // Check Fragment Shader
    glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
    glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> FragmentShaderErrorMessage(InfoLogLength + 1);
        glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
        printf("%s\n", &FragmentShaderErrorMessage[0]);
    }

    // Link the program
    printf("Linking program\n");
    GLuint ProgramID = glCreateProgram();
    glAttachShader(ProgramID, VertexShaderID);
    glAttachShader(ProgramID, FragmentShaderID);
    glLinkProgram(ProgramID);

    // Check the program
    glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
    glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
    if (InfoLogLength > 0) {
        std::vector<char> ProgramErrorMessage(InfoLogLength + 1);
        glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
        printf("%s\n", &ProgramErrorMessage[0]);
    }

    glDetachShader(ProgramID, VertexShaderID);
    glDetachShader(ProgramID, FragmentShaderID);

    glDeleteShader(VertexShaderID);
    glDeleteShader(FragmentShaderID);

    return ProgramID;
}

void Update_mesh(const GLuint& VAO, const std::vector<GLuint>& GLBuffers,
    std::vector<glm::vec3> Positions,
    std::vector<glm::vec3> Normals,
    std::vector<glm::vec3> Colors,
    std::vector<unsigned int>& Indices)
{
    glBindVertexArray(VAO); // Must be bound before binding buffers

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[0]);
    glBufferData(GL_ARRAY_BUFFER, Positions.size() * sizeof(Positions[0]), &Positions[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[1]);
    glBufferData(GL_ARRAY_BUFFER, Normals.size() * sizeof(Normals[0]), &Normals[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ARRAY_BUFFER, GLBuffers[2]);
    glBufferData(GL_ARRAY_BUFFER, Colors.size() * sizeof(Colors[0]), &Colors[0], GL_STATIC_DRAW);

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, GLBuffers[3]);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, Indices.size() * sizeof(Indices[0]), &Indices[0], GL_STATIC_DRAW);

    //glEnableVertexAttribArray(0);
    //glBindBuffer(GL_ARRAY_BUFFER, 0); // Unbind VBO to avoid accidental modification
    glBindVertexArray(0); // Unbind VAO to avoid accidental modification
}

// --------------------------------------------------
// Global Variables
// --------------------------------------------------
int Width = 512;
int Height = 512;
// --------------------------------------------------

void resize_callback(GLFWwindow*, int nw, int nh)
{
    Width = nw;
    Height = nh;
    // Tell the viewport to use all of our screen estate
    glViewport(0, 0, nw, nh);
}

void processInput(GLFWwindow* window)
{
    // Close when the user hits 'q' or escape
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS
        || glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
}

int main(int argc, char* argv[])
{
    create_scene();
    //
    // Initialize Window
    //
    GLFWwindow* window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;

    glfwWindowHint(GLFW_SAMPLES, 4); // 4x antialiasing
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3); // We want OpenGL 3.3
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // We don't want the old OpenGL

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(Width, Height, "Shader Pipeline", NULL, NULL);
    if (!window) {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, resize_callback);

    // Initialize GLEW
    glewExperimental = true; // Needed for core profile
    if (glewInit() != GLEW_OK)
        return -1;

    // Enable depth test
    glEnable(GL_DEPTH_TEST);
    // Accept fragment if it is closer to the camera than the former one
    glDepthFunc(GL_LESS);

    // Create and compile our GLSL program from the shaders
    GLuint program = LoadShaders("Phong.vert", "Phong.frag");

    glm::mat4 model = glm::mat4(1.0f); // Identity matrix for model transformation

    GLuint VAO;
    glGenVertexArrays(1, &VAO);

    const int numBuffers = 4; // Buffers for Positions, Normals, Colors, Indices
    std::vector<GLuint> GLBuffers;
    GLBuffers.resize(numBuffers, 0);
    glGenBuffers(numBuffers, &GLBuffers[0]);

    Update_mesh(VAO, GLBuffers, Positions, Normals, Colors, Indices);

    /* Loop until the user closes the window */
    while (!glfwWindowShouldClose(window))
    {
        // input
        processInput(window);

        // render
        // Clear the screen
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Use our shader
        glUseProgram(program);

        // update uniform
        GLint modelingUniform = glGetUniformLocation(program, "modeling");
        glUniformMatrix4fv(modelingUniform, 1, GL_FALSE, value_ptr(modelingMatrix));
        GLint modelingInvTrUniform = glGetUniformLocation(program, "modeling_inv_tr");
        glUniformMatrix4fv(modelingInvTrUniform, 1, GL_FALSE, value_ptr(modeling_inv_tr));
        GLint cameraUniform = glGetUniformLocation(program, "camera");
        glUniformMatrix4fv(cameraUniform, 1, GL_FALSE, value_ptr(cameraMatrix));
        GLint projectionUniform = glGetUniformLocation(program, "projection");
        glUniformMatrix4fv(projectionUniform, 1, GL_FALSE, value_ptr(projectionMatrix));


        GLint lightPosUniform = glGetUniformLocation(program, "lightPos");
        glUniform4fv(lightPosUniform, 1, value_ptr(lightPos));
        GLint lightColorUniform = glGetUniformLocation(program, "lightColor");
        glUniform4fv(lightColorUniform, 1, value_ptr(lightColor));
        GLint diffuseColorUniform = glGetUniformLocation(program, "diffuseColor");
        glUniform4fv(diffuseColorUniform, 1, value_ptr(diffuseColor));
        GLint specularColorUniform = glGetUniformLocation(program, "specularColor");
        glUniform4fv(specularColorUniform, 1, value_ptr(specularColor));
        GLint intensityUniform = glGetUniformLocation(program, "intensity");
        glUniform1f(intensityUniform, intensity);
        GLint gammaUniform = glGetUniformLocation(program, "gamma");
        glUniform1f(gammaUniform, gamma);

        glBindVertexArray(VAO);

        // Draw the triangle!
        glDrawElements(GL_TRIANGLES, Indices.size(), GL_UNSIGNED_INT, 0);
        glBindVertexArray(0); // no need to unbind it every time

        // Swap front and back buffers
        glfwSwapBuffers(window);

        // Poll for and process events
        glfwPollEvents();
    }

    // Cleanup VAO
    glDeleteVertexArrays(1, &VAO);
    if (GLBuffers[0] != 0)
        glDeleteBuffers(numBuffers, &GLBuffers[0]);
    glDeleteProgram(program);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}