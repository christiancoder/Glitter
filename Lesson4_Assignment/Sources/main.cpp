//=============================================================================
// VFSRenderingEnginesAndShaders
//=============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <iostream>
#include <string>
#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

//=============================================================================

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//=============================================================================

const char* vertexShaderSource ="#version 330 core\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aColor;\n"
    "layout (location = 2) in vec2 aUV;\n"
    "out vec4 outColor;\n"
    "out vec2 outUV;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "   outColor = vec4( aColor, 1.0 );\n"
    "   outUV = aUV;\n"
    "}\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec4 outColor;\n"
    "in vec2 outUV;\n"
    "uniform sampler2D texture_diffuse1;\n"
    "void main()\n"
    "{\n"
    "   FragColor = texture( texture_diffuse1, outUV ) * outColor;\n"
    "}\n\0";

//=============================================================================

struct ShaderProgram
{
    ShaderProgram( GLuint const program, GLint const modelMatrixLoc, GLint const viewMatrixLoc, GLint const projectionMatrixLoc ):
        mProgram( program ),
        mModelMatrixLoc( modelMatrixLoc ),
        mViewMatrixLoc( viewMatrixLoc ),
        mProjectionMatrixLoc( projectionMatrixLoc )
    {
    }

    virtual ~ShaderProgram()
    {
        if (mProgram)
        {
            glDeleteProgram( mProgram );
        }
    }

    virtual void Bind( const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLuint const textureObj )
    {
        if (mProgram)
        {
            glUseProgram( mProgram );
            glUniformMatrix4fv( mModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( model ) );
            glUniformMatrix4fv( mViewMatrixLoc, 1, GL_FALSE, glm::value_ptr( view ) );
            glUniformMatrix4fv( mProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr( projection ) );
            glActiveTexture( GL_TEXTURE0 );
            glBindTexture( GL_TEXTURE_2D, textureObj );
        }
    }

    GLuint mProgram;
    GLint mModelMatrixLoc;
    GLint mViewMatrixLoc;
    GLint mProjectionMatrixLoc;
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

    virtual void Render( const glm::mat4& model, const glm::mat4& view, const glm::mat4& projection, GLuint const textureObj )
    {
        mShaderProgram->Bind( model, view, projection, textureObj );
        if (mVertexArrayObj && mNumVertices > 0)
        {
            glBindVertexArray( mVertexArrayObj );
            glDrawArrays( mPrimitiveType, 0, mNumVertices );
        }
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
    Object( const std::shared_ptr<Mesh>& mesh, GLuint const textureObj ):
        mMesh( mesh ),
        mTextureObj( textureObj ),
		mRot( 0.0f )
    {
    }
    
    virtual void Update( float const deltaTime )
    {
        // Rotate object.
        float const rotationsPerSecond = 0.5f;
        mRot += ((360.0f * rotationsPerSecond) * deltaTime);
        mRot = fmodf( mRot, 360.0f );
        mTransform = glm::rotate( glm::mat4( 1.0f ), glm::radians( mRot ), glm::vec3( 0.0f, 0.0f, 1.0f ) );
    }

    virtual void Render( const glm::mat4& view, const glm::mat4& projection )
    {
        if (mMesh != nullptr)
        {
            mMesh->Render( mTransform, view, projection, mTextureObj );
        }
    }

    std::shared_ptr<Mesh> mMesh;
    GLuint mTextureObj;
    glm::mat4 mTransform;
    float mRot;
};

//=============================================================================

void ProcessInput(GLFWwindow* const window)
{
    // process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
    }
}

//=============================================================================

void FramebufferSizeCallback(GLFWwindow* window, int width, int height)
{
    // glfw: whenever the window size changed (by OS or user resize) this callback function executes
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

    // glad: load all OpenGL function pointers (extensions)
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

    // get uniform parameter locations
    GLint const modelMatrixLoc = glGetUniformLocation( program, "model" );
    GLint const viewMatrixLoc = glGetUniformLocation( program, "view" );
    GLint const projectionMatrixLoc = glGetUniformLocation( program, "projection" );
    
    // Use texture unit 0
    GLint const textureLoc = glGetUniformLocation( program, "texture_diffuse1" );
    glUniform1i( textureLoc, 0 );

    return std::shared_ptr<ShaderProgram>( new ShaderProgram( program, modelMatrixLoc, viewMatrixLoc, projectionMatrixLoc ) );
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
        // positions         // colors          // uvs
         0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  1.0, 1.0, // bottom right
        -0.5f, -0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.0, 1.0, // bottom left
         0.0f,  0.5f, 0.0f,  1.0f, 1.0f, 1.0f,  0.5, 0.0, // top 
    };

    // generate vertex buffer and vertex array objects
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
    GLsizei const stride = 8 * sizeof(float);
    GLvoid* offset = (GLvoid*)0;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(0);
    
    // color attribute
    offset = (GLvoid*)(3 * sizeof(float));
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(1);

    // uv attribute
    offset = (GLvoid*)(6 * sizeof(float));
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(2);

    glBindVertexArray(0);

    return std::shared_ptr<Mesh>( new Mesh( shaderProgram, vertexArrayObj, vertexBufferObj, primitiveType, numVertices ) );
}

//=============================================================================

unsigned int TextureFromFile(const std::string& filename)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);

    int width, height, nrComponents;
    unsigned char *data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);
    if (data)
    {
        GLenum format;
        if (nrComponents == 1)
            format = GL_RED;
        else if (nrComponents == 3)
            format = GL_RGB;
        else if (nrComponents == 4)
            format = GL_RGBA;

        glActiveTexture( GL_TEXTURE1 );
        glBindTexture(GL_TEXTURE_2D, textureID);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        stbi_image_free(data);
    }
    else
    {
        std::cout << "Texture failed to load at path: " << filename << std::endl;
        stbi_image_free(data);
    }

    return textureID;
}

//=============================================================================

void Update( std::vector<std::shared_ptr<Object>>& objects, float const deltaTime, GLFWwindow* const window )
{
    // process Input, AI, Physics, Collision Detection / Resolution, etc.
    
    // pump events
    glfwPollEvents();

    // process input
    ProcessInput( window );

    // update objects
    for (const auto& obj : objects)
    {
        obj->Update( deltaTime );
    }
}

//=============================================================================

void Render( std::vector<std::shared_ptr<Object>>& objects, GLFWwindow* const window )
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    //glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);

    // place camera back 2 units from 0,0,0 along z-axis, we are using OpenGL's
    // default coordinate system where -z is into the screen
    glm::mat4 view = glm::mat4( 1.0f );
    view = glm::translate( view, glm::vec3( 0.0f, 0.0f, 2.0f ) ); 
    // use inverse of camera matrix to move objects from worldspace into viewspace.
    view = glm::inverse( view );

    // get window size for projection matrix
    int wd;
    int ht;
    glfwGetWindowSize( window, &wd, &ht );

    // build projection matrix wd / ht aspect ratio with 45 degree field of view
    glm::mat4 projection( 1.0f );
    projection = glm::perspective( glm::radians( 45.0f ), (float)wd / (float)ht, 0.1f, 100.0f );

    // render objects
    for (const auto& obj : objects)
    {
        obj->Render( view, projection );
    }

    // glfw: swap buffers
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
}

//=============================================================================

int main()
{
    // initialize OpenGL (3.3 Core Profile)
    GLFWwindow* const window = InitGL();
    if (window == nullptr)
    {
        return -1;
    }

    GLuint const textureObj = TextureFromFile( "awesomeface.png" );

    // create shader program
    std::shared_ptr<ShaderProgram> shaderProgram = BuildShaderProgram();
    if (shaderProgram == nullptr)
    {
        return -1;
    }

    // create prop mesh (Triangle)
    std::shared_ptr<Mesh> mesh = BuildPropMesh( shaderProgram );
    if (mesh == nullptr)
    {
        return -1;
    }

    // create prop object
    std::vector<std::shared_ptr<Object>> objects;
    objects.push_back( std::shared_ptr<Object>( new Object( mesh, textureObj ) ) );

    // game loop
    // -----------
    double t0 = glfwGetTime();
    while (!glfwWindowShouldClose(window))
    {
        // update
        double const t1 = glfwGetTime();
        Update(objects, (float)(t1 - t0), window);
        t0 = t1;

        // render objects (View Frustum Culling, Occlusion Culling, Draw Order Sorting, etc)
        Render(objects, window);
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

//=============================================================================
