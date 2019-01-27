//=============================================================================
// VFSRenderingEnginesAndShaders
//=============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>

//=============================================================================

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//=============================================================================

const char* vertexShaderSource ="#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = vec4(aPos, 1.0);\n"
    "   ourColor = aColor;\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(ourColor, 1.0f);\n"
    "}\n\0";

//=============================================================================

struct ShaderProgram
{
    GLuint program;
};

struct Mesh
{
    ShaderProgram* program;
    GLuint vertexArrayObj;
    GLuint vertexBufferObj;
    GLenum primitiveType;
    GLsizei numVertices;
};

struct Object
{
    Mesh* mesh;
    glm::mat4x3 worldSpaceTransform;
};

//=============================================================================

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void ProcessInput(GLFWwindow* const window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, true);
}

//=============================================================================

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    (void)window;
    glViewport(0, 0, width, height);
}

//=============================================================================

GLFWwindow* InitGL()
{
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* const window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (window == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return nullptr;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, FramebufferSizeCallback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return nullptr;
    }

    return window;
}

//=============================================================================

bool BuildShaderProgram( ShaderProgram& shaderProgram )
{
    // build and compile our shader program
    // ------------------------------------

    // vertex shader
    GLuint const vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, nullptr);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        return false;
    }

    // fragment shader
    GLuint const fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, nullptr);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
        glDeleteShader(vertexShader);
        glDeleteShader(fragmentShader);
        return false;
    }

    // link shaders
    shaderProgram.program = glCreateProgram();
    glAttachShader(shaderProgram.program, vertexShader);
    glAttachShader(shaderProgram.program, fragmentShader);
    glLinkProgram(shaderProgram.program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // check for linking errors
    glGetProgramiv(shaderProgram.program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(shaderProgram.program, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        shaderProgram.program = 0;
        return false;
    }

    return true;
}

//=============================================================================

bool BuildPropMesh( Mesh& mesh )
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    mesh.primitiveType = GL_TRIANGLES;
    mesh.numVertices = 3;
    float vertices[] = 
    {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
    };

    glGenVertexArrays(1, &mesh.vertexArrayObj);
    glGenBuffers(1, &mesh.vertexBufferObj);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(mesh.vertexArrayObj);

    // Alloc vertex buffer.
    GLsizeiptr const bufferSize = sizeof(vertices);
    glBindBuffer(GL_ARRAY_BUFFER, mesh.vertexBufferObj);
    glBufferData(GL_ARRAY_BUFFER, bufferSize, vertices, GL_STATIC_DRAW);

    // position attribute
    GLsizei const stride = 6 * sizeof(float);
    GLvoid* offset = (GLvoid*)0;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(0);
    
    // color attribute
    offset = (GLvoid*)(3 * sizeof(float));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return true;
}

//=============================================================================

void Update( GLFWwindow* const window, float const deltaTime )
{
    // Process Input, AI, Physics, Collision Detection / Resolution, etc.
    // pump events
    glfwPollEvents();

    // process input
    ProcessInput(window);

    // update objects ()
}

//=============================================================================

void Render( GLFWwindow* const window )
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    // render the triangle
    glUseProgram(shaderProgram.program);
    glBindVertexArray(mesh.vertexArrayObj);
    glDrawArrays(mesh.primitiveType, 0, mesh.numVertices);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
}

//=============================================================================

int main()
{
    // Initialize OpenGL (3.3 Core Profile).
    GLFWwindow* const window = InitGL();
    if (window == nullptr)
    {
        return -1;
    }

    // Create shader program.
    ShaderProgram shaderProgram;
    if (!BuildShaderProgram(shaderProgram))
    {
        return -1;
    }

    // Create floor mesh.
    /*GLuint const floor = BuildFloorMesh();
    if (floor == 0)
    {
        return -1;
    }*/

    // Create prop mesh (Triangle).
    Mesh mesh;
    if (!BuildPropMesh(mesh))
    {
        return -1;
    }

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----

        // update
        Update(window,deltaTime);

        // Render objects (View Frustum Culling, Occlusion Culling, Draw Order Sorting, etc)
        Render(window, );
    }

    // de-allocate all resources once they've outlived their purpose:
    // ------------------------------------------------------------------------
    DeleteMesh(mesh)





    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

//=============================================================================
