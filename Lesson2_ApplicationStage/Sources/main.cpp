//=============================================================================
// VFSRenderingEnginesAndShaders
//=============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <iostream>
#include <memory>
#include <vector>

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
    ShaderProgram( GLuint const program ):
        mProgram( program )
    {
    }

    virtual ~ShaderProgram()
    {
        if (mProgram)
        {
            glDeleteProgram( mProgram );
        }
    }

    virtual void Bind()
    {
        if (mProgram)
        {
            glUseProgram( mProgram );
        }
    }

    GLuint mProgram;
};

//=============================================================================

struct Mesh
{
    Mesh( const std::shared_ptr<ShaderProgram>& shaderProgram, GLuint const vertexArrayObj, GLuint const vertexBufferObj, GLenum const primitiveType, GLsizei const numVertices ):
        mShaderProgram( shaderProgram ),
        mVertexArrayObj( vertexArrayObj ),
        mVertexBufferObj( vertexBufferObj ),
        mPrimitiveType( primitiveType ),
        mNumVertices( numVertices )
    {
    }

    virtual ~Mesh()
    {
        if (mVertexArrayObj)
        {
            glDeleteVertexArrays( 1, &mVertexArrayObj );
            mVertexArrayObj = 0;
        }
        if (mVertexBufferObj)
        {
            glDeleteBuffers( 1, &mVertexBufferObj );
            mVertexBufferObj = 0;
        }
    }

    virtual void Render( const glm::mat4& transform )
    {
        mShaderProgram->Bind();
        glBindVertexArray( mVertexArrayObj );
        glDrawArrays( mPrimitiveType, 0, mNumVertices );
    }

    std::shared_ptr<ShaderProgram> mShaderProgram;
    GLuint mVertexArrayObj;
    GLuint mVertexBufferObj;
    GLenum mPrimitiveType;
    GLsizei mNumVertices;
};

//=============================================================================

struct Object
{
    Object( const std::shared_ptr<Mesh>& mesh, const glm::mat4& transform ):
        mMesh( mesh ),
        mTransform( transform )
    {
    }
    
    virtual void Update( float const deltaTime )
    {
        // Do something here.
    }

    virtual void Render()
    {
        if (mMesh != nullptr)
        {
            mMesh->Render( mTransform );
        }
    }

    std::shared_ptr<Mesh> mMesh;
    glm::mat4 mTransform;
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

std::shared_ptr<ShaderProgram> BuildShaderProgram()
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
        return nullptr;
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
        return nullptr;
    }

    // link shaders
    GLuint const program = glCreateProgram();
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    glLinkProgram(program);
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    // check for linking errors
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success)
    {
        glGetProgramInfoLog(program, 512, nullptr, infoLog);
        std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
        return nullptr;
    }

    return std::shared_ptr<ShaderProgram>( new ShaderProgram( program ) );
}

//=============================================================================

std::shared_ptr<Mesh> BuildPropMesh( const std::shared_ptr<ShaderProgram>& shaderProgram )
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    GLenum const primitiveType = GL_TRIANGLES;
    GLsizei const numVertices = 3;
    float vertices[] = 
    {
        // positions         // colors
         0.5f, -0.5f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, -0.5f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f,  0.5f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
    };

    GLuint vertexArrayObj = 0;
    GLuint vertexBufferObj = 0;
    glGenVertexArrays(1, &vertexArrayObj);
    glGenBuffers(1, &vertexBufferObj);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vertexArrayObj);

    // Alloc vertex buffer.
    GLsizeiptr const bufferSize = sizeof(vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj);
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

    return std::shared_ptr<Mesh>( new Mesh( shaderProgram, vertexArrayObj, vertexBufferObj, primitiveType, numVertices ) );
}

//=============================================================================

void Update( std::vector<std::shared_ptr<Object>>& objects, float const deltaTime, GLFWwindow* const window )
{
    // Process Input, AI, Physics, Collision Detection / Resolution, etc.
    // pump events
    glfwPollEvents();

    // process input
    ProcessInput(window);

    // update objects ()
}

//=============================================================================

void Render( std::vector<std::shared_ptr<Object>>& objects, GLFWwindow* const window )
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    for (const auto& obj : objects)
    {
        obj->Render();
    }

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
    std::shared_ptr<ShaderProgram> shaderProgram = BuildShaderProgram();
    if (shaderProgram == nullptr)
    {
        return -1;
    }

    // Create prop mesh (Triangle).
    std::shared_ptr<Mesh> mesh = BuildPropMesh( shaderProgram );
    if (mesh == nullptr)
    {
        return -1;
    }

    // Create prop object.
    std::vector<std::shared_ptr<Object>> objects;
    objects.push_back( std::shared_ptr<Object>( new Object( mesh, glm::mat4(1.0f) ) ) );

    // render loop
    // -----------
    while (!glfwWindowShouldClose(window))
    {
        // input
        // -----

        // update
        Update(objects, 0.0f, window);

        // Render objects (View Frustum Culling, Occlusion Culling, Draw Order Sorting, etc)
        Render(objects, window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

//=============================================================================
