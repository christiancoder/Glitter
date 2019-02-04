//=============================================================================
// VFSRenderingEnginesAndShaders
//=============================================================================

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
//#include <glm/mat4x4.hpp>
//#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <memory>
#include <vector>
#include <iostream>

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
    "out vec3 ourColor;\n"
    "void main()\n"
    "{\n"
    "   gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
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
    ShaderProgram( GLuint const program, GLint const modelMatrixLoc, GLint const viewMatrixLoc, GLint const projectionMatrixLoc );
    virtual ~ShaderProgram();
    void Bind( const glm::mat4& model );

    GLuint mProgram;
    GLint mModelMatrixLoc;
    GLint mViewMatrixLoc;
    GLint mProjectionMatrixLoc;
};

//=============================================================================

struct Mesh
{
    Mesh( const std::shared_ptr<ShaderProgram>& shaderProgram, GLuint const vertexArrayObj, GLuint const vertexBufferObj, GLenum const primitiveType, GLsizei const numVertices );
    virtual ~Mesh();
    void Render( const glm::mat4& model );

    std::shared_ptr<ShaderProgram> mShaderProgram;
    GLuint mVertexArrayObj;
    GLuint mVertexBufferObj;
    GLenum mPrimitiveType;
    GLsizei mNumVertices;
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
    glm::vec2 mPosXZ;
    glm::vec2 mVelocityXZ;
};

//=============================================================================

struct Floor : public Object
{
    Floor( const std::shared_ptr<Mesh>& mesh );
    virtual ~Floor() = default;
    virtual void Update( float const deltaTime ) override;
    virtual void Render() override;

    std::shared_ptr<Mesh> mMesh;
    glm::mat4 mTransform;
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
    glm::mat4 mProjectionMatrix;
    std::vector<std::shared_ptr<Object>> mObjects;
    uint32_t mButtonMask;
    glm::vec2 mPrevMousePos;
    glm::vec2 mCurMousePos;
};

//=============================================================================

static std::shared_ptr<GameState> gGameState;

//=============================================================================

ShaderProgram::ShaderProgram( GLuint const program, GLint const modelMatrixLoc, GLint const viewMatrixLoc, GLint const projectionMatrixLoc ):
    mProgram( program ),
    mModelMatrixLoc( modelMatrixLoc ),
    mViewMatrixLoc( viewMatrixLoc ),
    mProjectionMatrixLoc( projectionMatrixLoc )
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

void ShaderProgram::Bind( const glm::mat4& modelMatrix )
{
    if (mProgram)
    {
        glUseProgram( mProgram );
        glUniformMatrix4fv( mModelMatrixLoc, 1, GL_FALSE, glm::value_ptr( modelMatrix ) );
        glUniformMatrix4fv( mViewMatrixLoc, 1, GL_FALSE, glm::value_ptr( gGameState->mViewMatrix ) );
        glUniformMatrix4fv( mProjectionMatrixLoc, 1, GL_FALSE, glm::value_ptr( gGameState->mProjectionMatrix ) );
    }
}

//=============================================================================

Mesh::Mesh( const std::shared_ptr<ShaderProgram>& shaderProgram, GLuint const vertexArrayObj, GLuint const vertexBufferObj, GLenum const primitiveType, GLsizei const numVertices ):
    mShaderProgram( shaderProgram ),
    mVertexArrayObj( vertexArrayObj ),
    mVertexBufferObj( vertexBufferObj ),
    mPrimitiveType( primitiveType ),
    mNumVertices( numVertices )
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
}

//=============================================================================

void Mesh::Render( const glm::mat4& modelMatrix )
{
    if (mVertexArrayObj && mNumVertices > 0)
    {
        mShaderProgram->Bind( modelMatrix );
        glBindVertexArray( mVertexArrayObj );
        glDrawArrays( mPrimitiveType, 0, mNumVertices );
    }
}

//=============================================================================

Prop::Prop( const std::shared_ptr<Mesh>& mesh ):
    mMesh( mesh )
{
    ///mRot = (float)(rand() % 359);
    mPosXZ.x = -10.0f + ((float)(rand() % 101) / 100.0f * 20.0f);
    mPosXZ.y = -10.0f + ((float)(rand() % 101) / 100.0f * 20.0f);
    mVelocityXZ.x = -1.0f + ((float)(rand() % 101) / 100.0f * 2.0f);
    mVelocityXZ.y = -1.0f + ((float)(rand() % 101) / 100.0f * 2.0f);
    mVelocityXZ = glm::normalize( mVelocityXZ );
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
    mTransform = glm::translate( mTransform, glm::vec3( mPosXZ.x, 0.0f, mPosXZ.y ) );
    mTransform *= rot;

    //mTransform = glm::rotate( mTransform, glm::radians( mRot ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
}

//=============================================================================

void Prop::Render()
{
    if (mMesh != nullptr)
    {
        mMesh->Render( mTransform );
    }
}

//=============================================================================

Floor::Floor( const std::shared_ptr<Mesh>& mesh ):
    mMesh( mesh )
{
}

//=============================================================================

void Floor::Update( float const deltaTime )
{
    mTransform = glm::mat4( 1.0f );
}

//=============================================================================

void Floor::Render()
{
    if (mMesh != nullptr)
    {
        mMesh->Render( mTransform );
    }
}

//=============================================================================

Camera::Camera():
    mPosition( 0.0f, 0.0f, 20.0f ),
    mPitchYaw( 0.0f, 0.0f )
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

    gGameState->mViewMatrix = glm::inverse( transform );

    // build projection matrix wd / ht aspect ratio with 45 degree field of view
    gGameState->mProjectionMatrix = glm::perspective( glm::radians( 45.0f ), windowSize.x / windowSize.y, 0.1f, 100.0f );
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
    gGameState->mCurMousePos.x = xpos;
    gGameState->mCurMousePos.y = ypos;
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
    gGameState->mCurMousePos.x = xpos;
    gGameState->mCurMousePos.y = ypos;

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
    GLint const viewMatrixLoc = glGetUniformLocation( program, "view" );
    GLint const projectionMatrixLoc = glGetUniformLocation( program, "projection" );

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
        // positions         // colors
         0.5f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  // bottom right
        -0.5f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f,  // bottom left
         0.0f, 1.0f, 0.0f,  0.0f, 0.0f, 1.0f   // top 
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

std::shared_ptr<Mesh> BuildFloorMesh( const std::shared_ptr<ShaderProgram>& shaderProgram )
{
    // set up vertex data (and buffer(s)) and configure vertex attributes
    // ------------------------------------------------------------------
    GLenum const primitiveType = GL_TRIANGLES;
    GLsizei const numVertices = 6;
    float vertices[] = 
    {
        // positions         // colors
        -11.0f, 0.0f, -11.0f,  0.5f, 0.5f, 0.5f,  // bottom right
        -11.0f, 0.0f,  11.0f,  0.5f, 0.5f, 0.5f,  // bottom left
         11.0f, 0.0f, -11.0f,  0.5f, 0.5f, 0.5f,   // top 
         
         11.0f, 0.0f, -11.0f,  0.5f, 0.5f, 0.5f,  // bottom right
         11.0f, 0.0f,  11.0f,  0.5f, 0.5f, 0.5f,  // bottom left
        -11.0f, 0.0f,  11.0f,  0.5f, 0.5f, 0.5f   // top 

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