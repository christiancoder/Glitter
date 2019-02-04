//=============================================================================
// VFSRenderingEnginesAndShaders
//=============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <iostream>
#include "teapot.h"
//=============================================================================

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

//=============================================================================

//    "   vec3 wsNormal = itModel * aNormal;\n"
//    "   outColor = max( dot( wsNormal, wsCameraDir ), 0.0 ) * color;\n"

const char* vertexShaderSource ="#version 330 core\n"
    "uniform mat4 model;\n"
    "uniform mat3 itModel;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n"
    "uniform vec3 color;\n"
    "uniform vec3 wsCameraDir;\n"
    "layout (location = 0) in vec3 aPos;\n"
    "layout (location = 1) in vec3 aNormal;\n"
    "out vec3 outColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4( aPos, 1.0 );\n"
    "   outColor = color;\n"
    "}\n"
    "\0";

const char* fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "in vec3 outColor;\n"
    "void main()\n"
    "{\n"
    "   FragColor = vec4(outColor, 1.0f);\n"
    "}\n"
    "\0";


//=============================================================================

static uint32_t const gNumColors = 30;
static glm::vec3 gColors[gNumColors] =
{
    glm::vec3( 0.200f, 0.400f, 0.600f ),
    glm::vec3( 0.600f, 0.800f, 1.000f ),
    glm::vec3( 0.600f, 0.600f, 0.200f ),
    glm::vec3( 0.400f, 0.400f, 0.600f ),
    glm::vec3( 0.800f, 0.600f, 0.200f ),
    glm::vec3( 0.000f, 0.400f, 0.400f ),
    glm::vec3( 0.200f, 0.600f, 1.000f ),
    glm::vec3( 0.600f, 0.200f, 0.000f ),
    glm::vec3( 0.800f, 0.800f, 0.600f ),
    glm::vec3( 0.400f, 0.400f, 0.400f ),
    glm::vec3( 1.000f, 0.800f, 0.400f ),
    glm::vec3( 0.400f, 0.600f, 0.800f ),
    glm::vec3( 0.400f, 0.200f, 0.400f ),
    glm::vec3( 0.600f, 0.600f, 0.800f ),
    glm::vec3( 0.800f, 0.800f, 0.800f ),
    glm::vec3( 0.400f, 0.600f, 0.600f ),
    glm::vec3( 0.800f, 0.800f, 0.400f ),
    glm::vec3( 0.800f, 0.400f, 0.000f ),
    glm::vec3( 0.600f, 0.600f, 1.000f ),
    glm::vec3( 0.000f, 0.400f, 0.800f ),
    glm::vec3( 0.600f, 0.800f, 0.800f ),
    glm::vec3( 0.600f, 0.600f, 0.600f ),
    glm::vec3( 1.000f, 0.800f, 0.000f ),
    glm::vec3( 0.000f, 0.600f, 0.600f ),
    glm::vec3( 0.600f, 0.800f, 0.200f ),
    glm::vec3( 1.000f, 0.600f, 0.000f ),
    glm::vec3( 0.600f, 0.600f, 0.400f ),
    glm::vec3( 0.400f, 0.800f, 0.800f ),
    glm::vec3( 0.200f, 0.600f, 0.400f ),
    glm::vec3( 0.800f, 0.800f, 0.200f ),
};

//=============================================================================

struct ShaderProgram
{
    ShaderProgram( GLuint const program, GLint const modelMatrixLoc, GLint const itModelMatrixLoc, GLint const viewMatrixLoc, GLint const projectionMatrixLoc, GLint const mColorLoc, GLint const mCameraDirLoc );
    virtual ~ShaderProgram();
    void Bind( const glm::mat4& model, const glm::vec3& color );

    GLuint mProgram;
    GLint mModelMatrixLoc;
    GLint mITModelMatrixLoc;
    GLint mViewMatrixLoc;
    GLint mProjectionMatrixLoc;
    GLint mColorLoc;
    GLint mCameraDirLoc;
};

//=============================================================================

struct Mesh
{
    Mesh( const std::shared_ptr<ShaderProgram>& shaderProgram, GLuint const vertexArrayObj, GLuint const vertexBufferObj, GLuint const indexBufferObj, GLenum const primitiveType, GLsizei const numIndices );
    virtual ~Mesh();
    void Render( const glm::mat4& model, const glm::vec3& color );

    std::shared_ptr<ShaderProgram> mShaderProgram;
    GLuint mVertexArrayObj;
    GLuint mVertexBufferObj;
    GLuint mIndexBufferObj;
    GLenum mPrimitiveType;
    GLsizei mNumIndices;
};

//=============================================================================

struct Object
{
    Object() = default;
    virtual ~Object() = default;
    virtual void Update( float const deltaTime ) = 0;
    virtual void Render() = 0;
};

//=============================================================================

struct Prop : public Object
{
    Prop( const std::shared_ptr<Mesh>& mesh );
    virtual ~Prop() = default;
    virtual void Update( float const deltaTime ) override;
    virtual void Render() override;

    std::shared_ptr<Mesh> mMesh;
    glm::mat4 mTransform;
    glm::vec3 mColor;
    glm::vec2 mPosXZ;
    glm::vec2 mVelocityXZ;
};

//=============================================================================

struct Floor : public Object
{
    Floor( const std::shared_ptr<Mesh>& mesh );
    virtual ~Floor() = default;
    virtual void Update( float const deltaTime ) {}
    virtual void Render() override;

    std::shared_ptr<Mesh> mMesh;
};

//=============================================================================

struct Camera : public Object
{
    Camera();
    virtual ~Camera() = default;
    virtual void Update( float const deltaTime ) override;
    virtual void Render() {}

    glm::vec3 mPosition;
    glm::vec2 mPitchYaw;
};

//=============================================================================

struct GameState
{
    enum
    {
        BUTTON_UP = 1 << 0,
        BUTTON_LEFT = 1 << 1,
        BUTTON_DOWN = 1 << 2,
        BUTTON_RIGHT = 1 << 3,
    };

    GLFWwindow* mWindow;
    glm::mat4 mViewMatrix;
    glm::mat4 mCameraMatrix;
    glm::mat4 mProjectionMatrix;
    std::vector<std::shared_ptr<Object>> mObjects;
    uint32_t mButtonMask;
    glm::vec2 mPrevMousePos;
    glm::vec2 mCurMousePos;
};

//=============================================================================

static std::shared_ptr<GameState> gGameState;

//=============================================================================

ShaderProgram::ShaderProgram( GLuint const program, GLint const modelMatrixLoc, GLint const itModelMatrixLoc, GLint const viewMatrixLoc, GLint const projectionMatrixLoc, GLint const colorLoc, GLint const cameraDirLoc ):
    mProgram( program ),
    mModelMatrixLoc( modelMatrixLoc ),
    mITModelMatrixLoc( itModelMatrixLoc ),
    mViewMatrixLoc( viewMatrixLoc ),
    mProjectionMatrixLoc( projectionMatrixLoc ),
    mColorLoc( colorLoc ),
    mCameraDirLoc( cameraDirLoc )
{
}

//=============================================================================

ShaderProgram::~ShaderProgram()
{
    if (mProgram)
    {
        glDeleteProgram( mProgram );
    }
}

//=============================================================================

void ShaderProgram::Bind( const glm::mat4& modelMatrix, const glm::vec3& color )
{
    if (mProgram)
    {
        glm::mat3 itModelMatrix( 1.0f );
        itModelMatrix[0] = normalize( glm::vec3( modelMatrix[0] ) );
        itModelMatrix[1] = normalize( glm::vec3( modelMatrix[1] ) );
        itModelMatrix[2] = normalize( glm::vec3( modelMatrix[2] ) );
        glUseProgram( mProgram );
        glUniformMatrix4fv( mModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( modelMatrix ) );
        glUniformMatrix3fv( mITModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( itModelMatrix ) );
        glUniformMatrix4fv( mViewMatrixLoc, 1, GL_FALSE, glm::value_ptr( gGameState->mViewMatrix ) );
        glUniformMatrix4fv( mProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr( gGameState->mProjectionMatrix ) );
        glUniform3f( mColorLoc, color.x, color.y, color.z );
        glUniform3f( mCameraDirLoc, gGameState->mCameraMatrix[2].x, gGameState->mCameraMatrix[2].y, gGameState->mCameraMatrix[2].z );
    }
}

//=============================================================================

Mesh::Mesh( const std::shared_ptr<ShaderProgram>& shaderProgram, GLuint const vertexArrayObj, GLuint const vertexBufferObj, GLuint const indexBufferObj, GLenum const primitiveType, GLsizei const numIndices ):
    mShaderProgram( shaderProgram ),
    mVertexArrayObj( vertexArrayObj ),
    mVertexBufferObj( vertexBufferObj ),
    mIndexBufferObj( indexBufferObj ),
    mPrimitiveType( primitiveType ),
    mNumIndices( numIndices )
{
}

//=============================================================================

Mesh::~Mesh()
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
    if (mIndexBufferObj)
    {
        glDeleteBuffers( 1, &mIndexBufferObj );
        mIndexBufferObj = 0;
    }
}

//=============================================================================

void Mesh::Render( const glm::mat4& modelMatrix, const glm::vec3& color )
{
    if (mVertexArrayObj && mNumIndices > 0)
    {
        mShaderProgram->Bind( modelMatrix, color );
        glBindVertexArray( mVertexArrayObj );
        glDrawElements( mPrimitiveType, mNumIndices, GL_UNSIGNED_SHORT, nullptr );
    }
}

//=============================================================================

Prop::Prop( const std::shared_ptr<Mesh>& mesh ):
    mMesh( mesh )
{
    mPosXZ.x = -10.0f + ((float)(rand() % 101) / 100.0f * 20.0f);
    mPosXZ.y = -10.0f + ((float)(rand() % 101) / 100.0f * 20.0f);
    mVelocityXZ.x = -1.0f + ((float)(rand() % 101) / 100.0f * 2.0f);
    mVelocityXZ.y = -1.0f + ((float)(rand() % 101) / 100.0f * 2.0f);
    mVelocityXZ = glm::normalize( mVelocityXZ );
    mColor = gColors[rand() % gNumColors];
}

//=============================================================================

void Prop::Update( float const deltaTime )
{
    float const speed = 2.5f;  // meters per second
    mPosXZ += mVelocityXZ * deltaTime * speed;
    if (mPosXZ.x < -10.0f || mPosXZ.x > 10.0f ||
        mPosXZ.y < -10.0f || mPosXZ.y > 10.0f)
    {
        mVelocityXZ.x = -1.0f + ((float)(rand() % 101) / 100.0f * 2.0f);
        mVelocityXZ.y = -1.0f + ((float)(rand() % 101) / 100.0f * 2.0f);
        mVelocityXZ = glm::normalize( mVelocityXZ );
        mPosXZ = glm::clamp( mPosXZ, -10.0f, 10.0f );
    }

    glm::mat4 rot = glm::lookAt( glm::vec3( 0.0f, 0.0f, 0.0f ), glm::vec3( mVelocityXZ.x, 0.0f, mVelocityXZ.y ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    rot = glm::inverse( rot );

    mTransform = glm::mat4( 1.0f );
    mTransform = glm::translate( mTransform, glm::vec3( mPosXZ.x, 0.5f, mPosXZ.y ) );
    mTransform *= rot;
    mTransform = glm::scale( mTransform, glm::vec3( 0.01f, 0.01f, 0.01f ) );
}

//=============================================================================

void Prop::Render()
{
    if (mMesh != nullptr)
    {
        mMesh->Render( mTransform, mColor );
    }
}

//=============================================================================

Floor::Floor( const std::shared_ptr<Mesh>& mesh ):
    mMesh( mesh )
{
}

//=============================================================================

void Floor::Render()
{
    if (mMesh != nullptr)
    {
        mMesh->Render( glm::mat4( 1.0f ), glm::vec3( 0.5f, 0.5f, 0.5f ) );
    }
}

//=============================================================================

Camera::Camera():
    mPosition( 0.0f, 13.0f, 23.0f ),
    mPitchYaw( 0.0f, -28.0f )
{
}

//=============================================================================

void Camera::Update( float const deltaTime )
{
    // Get window size.
    int wd;
    int ht;
    glfwGetWindowSize( gGameState->mWindow, &wd, &ht );
    glm::vec2 const windowSize = glm::vec2( (float)wd, (float)ht );
    float const aspectRatio = windowSize.x / windowSize.y;

    // Increment pitch yaw.
    glm::vec2 const rateOfRotation = glm::vec2( 90.0f * aspectRatio, 90.0f ); // degrees per normalized mouse movement
    glm::vec2 const normalizedMouseDelta = (gGameState->mCurMousePos - gGameState->mPrevMousePos) / windowSize;
    glm::vec2 const rotationDelta = -normalizedMouseDelta * rateOfRotation;
    mPitchYaw += rotationDelta;
    mPitchYaw.x = glm::mod( mPitchYaw.x, 360.0f );
    mPitchYaw.y = glm::clamp( mPitchYaw.y, -90.0f, 90.0f );

    // Calculate orientation.
    glm::mat4 transform( 1.0f );
    transform = glm::rotate( transform, glm::radians( mPitchYaw.x ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
    transform = glm::rotate( transform, glm::radians( mPitchYaw.y ), glm::vec3( 1.0f, 0.0f, 0.0f ) );

    // Update translation.
    float const speed = 5.0f;  // meters per second
    mPosition += (gGameState->mButtonMask & GameState::BUTTON_UP) ? -((speed * deltaTime) * glm::vec3( transform[2] )) : glm::vec3( 0.0f );
    mPosition += (gGameState->mButtonMask & GameState::BUTTON_DOWN) ? ((speed * deltaTime) * glm::vec3( transform[2] )) : glm::vec3( 0.0f );
    mPosition += (gGameState->mButtonMask & GameState::BUTTON_LEFT) ? -((speed * deltaTime) * glm::vec3( transform[0] )) : glm::vec3( 0.0f );
    mPosition += (gGameState->mButtonMask & GameState::BUTTON_RIGHT) ? ((speed * deltaTime) * glm::vec3( transform[0] )) : glm::vec3( 0.0f );
    transform[3] = glm::vec4( mPosition, 1.0f );

    gGameState->mCameraMatrix = transform;
    gGameState->mViewMatrix = glm::inverse( transform );

    // build projection matrix wd / ht aspect ratio with 45 degree field of view
    gGameState->mProjectionMatrix = glm::perspective( glm::radians( 45.0f ), windowSize.x / windowSize.y, 0.1f, 100.0f );
    //gGameState->mProjectionMatrix = glm::ortho( -10 * aspectRatio, 10.0f * aspectRatio, -10.0f, 10.0f, 0.1f, 100.0f );
}

//=============================================================================

void ProcessInput()
{
    if (glfwGetKey( gGameState->mWindow, GLFW_KEY_ESCAPE ) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose( gGameState->mWindow, true );
    }

    gGameState->mButtonMask = 0;
    gGameState->mButtonMask |= (glfwGetKey( gGameState->mWindow, GLFW_KEY_UP ) == GLFW_PRESS || glfwGetKey( gGameState->mWindow, GLFW_KEY_W ) == GLFW_PRESS) ? GameState::BUTTON_UP : 0;
    gGameState->mButtonMask |= (glfwGetKey( gGameState->mWindow, GLFW_KEY_LEFT ) == GLFW_PRESS || glfwGetKey( gGameState->mWindow, GLFW_KEY_A ) == GLFW_PRESS) ? GameState::BUTTON_LEFT : 0;
    gGameState->mButtonMask |= (glfwGetKey( gGameState->mWindow, GLFW_KEY_DOWN ) == GLFW_PRESS || glfwGetKey( gGameState->mWindow, GLFW_KEY_S ) == GLFW_PRESS) ? GameState::BUTTON_DOWN : 0;
    gGameState->mButtonMask |= (glfwGetKey( gGameState->mWindow, GLFW_KEY_RIGHT ) == GLFW_PRESS || glfwGetKey( gGameState->mWindow, GLFW_KEY_D ) == GLFW_PRESS) ? GameState::BUTTON_RIGHT : 0;

    double xpos, ypos;
    glfwGetCursorPos( gGameState->mWindow, &xpos, &ypos );
    gGameState->mPrevMousePos = gGameState->mCurMousePos;
    gGameState->mCurMousePos.x = (float)xpos;
    gGameState->mCurMousePos.y = (float)ypos;
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

bool Init()
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
    gGameState->mWindow = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", nullptr, nullptr);
    if (gGameState->mWindow == nullptr)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }
    glfwMakeContextCurrent(gGameState->mWindow);
    glfwSetFramebufferSizeCallback(gGameState->mWindow, FramebufferSizeCallback);

    // glad: load all OpenGL function pointers (extensions)
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwSetInputMode( gGameState->mWindow, GLFW_CURSOR, GLFW_CURSOR_DISABLED );

    double xpos, ypos;
    glfwGetCursorPos( gGameState->mWindow, &xpos, &ypos );
    gGameState->mCurMousePos.x = (float)xpos;
    gGameState->mCurMousePos.y = (float)ypos;

    return true;
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
    GLint const itModelMatrixLoc = glGetUniformLocation( program, "itModel" );
    GLint const viewMatrixLoc = glGetUniformLocation( program, "view" );
    GLint const projectionMatrixLoc = glGetUniformLocation( program, "projection" );
    GLint const colorLoc = glGetUniformLocation( program, "color" );
    GLint const cameraDirLoc = glGetUniformLocation( program, "wsCameraDir" );

    return std::shared_ptr<ShaderProgram>( new ShaderProgram( program, modelMatrixLoc, itModelMatrixLoc, viewMatrixLoc, projectionMatrixLoc, colorLoc, cameraDirLoc ) );
}

//=============================================================================

std::shared_ptr<Mesh> BuildPropMesh( const std::shared_ptr<ShaderProgram>& shaderProgram )
{
    uint32_t const numVertices = sizeof( teapotVertices ) / sizeof( glm::vec3 );
    uint32_t const numNormals = sizeof( teapotNormals ) / sizeof( glm::vec3 );
    uint32_t const numIndices = sizeof( teapotIndices ) / sizeof( uint16_t );
    uint32_t const numTris = numIndices / 3;

    // generate vertex buffer and vertex array objects
    GLuint vertexArrayObj = 0;
    GLuint vertexBufferObj = 0;
    GLuint indexBufferObj = 0;
    glGenVertexArrays(1, &vertexArrayObj);
    glGenBuffers(1, &vertexBufferObj);
    glGenBuffers(1, &indexBufferObj);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vertexArrayObj);

    // Alloc vertex buffer.
    GLsizeiptr const vertexBufferSize = sizeof( teapotVertices ) + sizeof( teapotNormals );
    glBindBuffer( GL_ARRAY_BUFFER, vertexBufferObj );
    glBufferData( GL_ARRAY_BUFFER, vertexBufferSize, nullptr, GL_STATIC_DRAW );
    glBufferSubData( GL_ARRAY_BUFFER, 0, sizeof( teapotVertices ), teapotVertices );
    glBufferSubData( GL_ARRAY_BUFFER, sizeof( teapotVertices ), sizeof( teapotNormals ), teapotNormals );

    // Alloc index buffer.
    GLsizeiptr const indexBufferSize = sizeof( teapotIndices );
    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, indexBufferObj );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, teapotIndices, GL_STATIC_DRAW );

    // position attribute
    GLsizei const stride = 3 * sizeof(float);
    GLvoid* offset = (GLvoid*)0;
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(0);
    
    // color attribute
    //offset = (GLvoid*)(3 * sizeof(float));
    offset = (GLvoid*)sizeof( teapotVertices );
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, offset);
    glEnableVertexAttribArray(1);

    glBindVertexArray(0);

    return std::shared_ptr<Mesh>( new Mesh( shaderProgram, vertexArrayObj, vertexBufferObj, indexBufferObj, GL_TRIANGLES, numIndices ) );
}

//=============================================================================

std::shared_ptr<Mesh> BuildFloorMesh( const std::shared_ptr<ShaderProgram>& shaderProgram )
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    GLenum const primitiveType = GL_TRIANGLES;
    GLsizei const numVertices = 4;
    GLsizei const numIndices = 6;
    float const vertices[] = 
    {
        // positions              // normals
        -11.0f, 0.0f, -11.0f,     0.0f, 1.0f, 0.0f,
        -11.0f, 0.0f,  11.0f,     0.0f, 1.0f, 0.0f,
         11.0f, 0.0f, -11.0f,     0.0f, 1.0f, 0.0f,
         11.0f, 0.0f,  11.0f,     0.0f, 1.0f, 0.0f,
    };
    uint16_t const indices[] =
    {
        0, 1, 2,
        2, 3, 1,
    };

    // generate vertex buffer and vertex array objects
    GLuint vertexArrayObj = 0;
    GLuint vertexBufferObj = 0;
    GLuint indexBufferObj = 0;
    glGenVertexArrays(1, &vertexArrayObj);
    glGenBuffers(1, &vertexBufferObj);
    glGenBuffers(1, &indexBufferObj);

    // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
    glBindVertexArray(vertexArrayObj);

    // Alloc vertex buffer.
    GLsizeiptr const vertexBufferSize = sizeof(vertices);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferObj);
    glBufferData(GL_ARRAY_BUFFER, vertexBufferSize, vertices, GL_STATIC_DRAW);

    // Alloc index buffer.
    GLsizeiptr const indexBufferSize = sizeof(indices);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferObj);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferSize, indices, GL_STATIC_DRAW);

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

    return std::shared_ptr<Mesh>( new Mesh( shaderProgram, vertexArrayObj, vertexBufferObj, indexBufferObj, primitiveType, numIndices ) );
}

//=============================================================================

void Update( float const deltaTime )
{
    // process Input, AI, Physics, Collision Detection / Resolution, etc.
    
    // pump events
    glfwPollEvents();

    // process input
    ProcessInput();

    // update objects
    for (const auto& obj : gGameState->mObjects)
    {
        obj->Update( deltaTime );
    }
}

//=============================================================================

void Render()
{
    //glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
    glEnable( GL_DEPTH_TEST );

    // render objects
    for (const auto& obj : gGameState->mObjects)
    {
        obj->Render();
    }

    // glfw: swap buffers
    // -------------------------------------------------------------------------------
    glfwSwapBuffers( gGameState->mWindow );
}

//=============================================================================

int main()
{
    // initialize OpenGL (3.3 Core Profile)
    gGameState = std::shared_ptr<GameState>( new GameState );
    Init();
    if (gGameState->mWindow == nullptr)
    {
        return -1;
    }

    // create shader program
    std::shared_ptr<ShaderProgram> shaderProgram = BuildShaderProgram();
    if (shaderProgram == nullptr)
    {
        return -1;
    }

    // create floor mesh
    std::shared_ptr<Mesh> floorMesh = BuildFloorMesh( shaderProgram );
    if (floorMesh == nullptr)
    {
        return -1;
    }

    // create prop mesh (Triangle)
    std::shared_ptr<Mesh> propMesh = BuildPropMesh( shaderProgram );
    if (propMesh == nullptr)
    {
        return -1;
    }

    // create camera object
    gGameState->mObjects.push_back( std::shared_ptr<Object>( new Camera() ) );

    // create floor object
    gGameState->mObjects.push_back( std::shared_ptr<Object>( new Floor( floorMesh ) ) );

    // create prop object
    uint32_t const numProps = 100;
    for (uint32_t i = 0; i < numProps; i++)
    {
        gGameState->mObjects.push_back( std::shared_ptr<Object>( new Prop( propMesh ) ) );
    }

    // game loop
    // -----------
    double t0 = glfwGetTime();
    while (!glfwWindowShouldClose(gGameState->mWindow))
    {
        // update
        double const t1 = glfwGetTime();
        Update( (float)(t1 - t0) );
        t0 = t1;

        // render objects (View Frustum Culling, Occlusion Culling, Draw Order Sorting, etc)
        Render();
    }

    // glfw: terminate, clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

//=============================================================================
