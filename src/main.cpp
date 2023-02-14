// Header Files
#include<windows.h>
#include<stdio.h>
#include<stdlib.h>

#include<string>
#include<fstream>
#include<sstream>
#include<map>
#include<string>
#include<vector>

// OpenGL Header Files
#include<GL/glew.h> // this must be above gl.h
#include<GL/gl.h>

#include"vmath.h"
using namespace vmath;

// Image Loading Library
#define STB_IMAGE_IMPLEMENTATION
#include"stb_image.h"

// Assimp
#include<assimp/Importer.hpp>
#include<assimp/scene.h>
#include<assimp/postprocess.h>

#include"OGL.h"

#include"Bone.h"
#include"AnimMesh.h"
#include"AnimModel.h"
#include"Animation.h"
#include"Animator.h"

#define WIN_WIDTH 640
#define WIN_HEIGHT 360


// OpenGL Libraries

#pragma comment(lib,"glew32.lib")
#pragma comment(lib,"OpenGL32.lib")
#pragma comment(lib,"glu32.lib")
#pragma comment(lib,"assimp.lib")

// Global Variable
HWND ghwnd = NULL;
HDC ghdc = NULL;
HGLRC ghrc = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;
TCHAR str[125];

FILE* glLog;

// Program Struct

struct Program
{
    GLuint program;
    struct {
        GLuint projection;
        GLuint view;
        GLuint model;
        GLuint diffuse;
        GLuint glitchSpeed;
        GLuint glitchIntensity;
        GLuint mainColor;
        GLuint Time;
        GLuint BarSpeed;
        GLuint BarDistance;
        GLuint alpha;
        GLuint flickerSpeed;
        GLuint rimColor;
        GLuint rimPower;
        GLuint glowSpeed;
        GLuint glowDistance;
    }uniforms;
};

Program myprog,modelProgram;

GLuint vao;
GLuint texture;
GLuint boneindex;
GLuint vao_sphere, vao_cylinder, vao_cone, vao_torus,vbo_elements, vbo_vertices;
mat4 projectionMatrix;

Model* backpack;

std::vector<Vertex> pcntCube;
GLuint cube_vao;

float g_time = 0.0f;

std::vector<float> sphere_vertex;
std::vector<unsigned int>sphere_index;

// Cylinder Vertices
std::vector<float> cylinder_vertex;
std::vector<unsigned int>cylinder_index;

// Cylinder Vertices
std::vector<float> cone_vertex;
std::vector<unsigned int> cone_index;

// Cylinder Vertices
std::vector<float> torus_vertex;

int keyPress = 0;

void Sphere(float radius, int sector, int stack, std::vector<float>& sphere, std::vector<unsigned int>& index);
void Cylinder(float baseRadius, float topRadius, float height, int sectors, int stack, std::vector<float>& cylinder, std::vector<unsigned int>& indices);
void Torus(float radius, float tubeRadius, std::vector<float>& torus);

// Global Function Declarations
LRESULT CALLBACK WndProc(HWND,UINT,WPARAM,LPARAM);
void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum saverity, GLsizei length, const GLchar* message, const void* userParam);

// Main Function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,LPSTR lpszCmdLine,int iCmdShow)
{
    // Function Declaration
    int initialize(void);
    void display(void);
    void update(void);
    void uninitialize(void);
    // Variable Declaration
    WNDCLASSEX wndclass;
    HWND hwnd;
    MSG msg;
    TCHAR szAppName[] = TEXT("MyWindow");
    BOOL bDone = FALSE;
    int iRetVal = 0;

    // Code

    if(fopen_s(&gpFile,"Log.txt","w") != 0)
    {
        MessageBox(NULL,TEXT("Creation Of Log File Failed.\nExitting ..."),TEXT("File I/O Error"),MB_ICONERROR);
        exit(0);
    }
    else
    {
        fprintf(gpFile,"Log File Created Sucessfully.\n");
        fclose(gpFile);
    }

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    int x = (screenWidth -  WIN_WIDTH) / 2;
    int y = (screenHeight - WIN_HEIGHT) / 2;

    // Initialization Of WNDCLASSEX Structure
    wndclass.cbSize         = sizeof(WNDCLASSEX);
    wndclass.style          = CS_HREDRAW|CS_VREDRAW|CS_OWNDC;
    wndclass.cbClsExtra     = 0;
    wndclass.cbWndExtra     = 0;
    wndclass.lpfnWndProc    = WndProc;
    wndclass.hInstance      = hInstance;
    wndclass.hbrBackground  = (HBRUSH)GetStockObject(BLACK_BRUSH); 
    wndclass.hIcon          = LoadIcon(hInstance,MAKEINTRESOURCE(BAT_ICON));
    wndclass.hCursor        = LoadCursor(NULL,IDC_ARROW);
    wndclass.lpszClassName  = szAppName;
    wndclass.lpszMenuName   = NULL;
    wndclass.hIconSm        = LoadIcon(hInstance,MAKEINTRESOURCE(BAT_ICON));

    // Registering Above WndClass
    RegisterClassEx(&wndclass);

    // Create Window
    hwnd = CreateWindowEx(  
                            WS_EX_APPWINDOW,
                            szAppName,
                            TEXT("Valentine's Day"),
                            WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
                            x,
                            y,
                            WIN_WIDTH,
                            WIN_HEIGHT,
                            NULL,
                            NULL,
                            hInstance,
                            NULL
                        );

    ghwnd = hwnd;

    iRetVal = initialize();

    if(iRetVal == -1)
    {
        Log("Error", "choose pixel format failed");
        uninitialize();
    }

    if(iRetVal == -2)
    {
        Log("Error", "set pixel format failed");
        uninitialize();        
    }

    if(iRetVal == -3)
    {
        Log("Error", "Create wgl context failed");
        uninitialize();        
    }

    if(iRetVal == -4)
    {
        Log("Error", "make current context failed");
        uninitialize();        
    }

    if(iRetVal == -5)
    {
        Log("Error", "glewinit failed");
        uninitialize();        
    }

    // Show window
    ShowWindow(hwnd,iCmdShow);

    // Foregrounding and Focusing The Window
    // ghwnd or hwnd will work but hwnd is for local functions.
    SetForegroundWindow(hwnd);

    SetFocus(hwnd);

    // Special loop
    while(!bDone)
    {
        if(PeekMessage(&msg,NULL,0,0,PM_REMOVE))
        {
            if(msg.message == WM_QUIT)
                bDone = TRUE;
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            if(gbActiveWindow)
            {
                // Render The Scene
                display();
                // Update the Scene
                update();
            }
        }
    }
    uninitialize();
    return (int)msg.wParam;
}

// Callback Function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
    // Local Function Declaration
    void ToogleFullScreen(void);
    void resize(int,int);
    void uninitialize(void);
    // Local Variable 
    // Code
    int oldMouseX, oldMouseY;
    int offsetX, offsetY;
    vec3 dir;
    switch(iMsg)
    {
        case WM_CREATE:
            //fprintf(gpFile,"In WM_CREATE Message.\n");
            //sprintf(str,"!!! Press F To Enter FullScreen !!!");
        break;

        case WM_CHAR:
            switch(wParam)
            {
                case 'F':
                    case 'f':
                        //fprintf(gpFile,"In ToogleFullscreen.\n");
                        ToogleFullScreen();
                break;
                case 'A':
                case 'a':
                    dir = vmath::cross(DebugCam.cameraFront, DebugCam.cameraUp);
                    dir = vmath::normalize(dir);
                    dir *= vec3(0.3f, 0.3f, 0.3f);
                    DebugCam.cameraPosition -= dir;
                break;
                case 'W':
                case 'w':
                    dir = vec3(0.0f);
                    dir = DebugCam.cameraFront * vec3(0.3f, 0.3f, 0.3f);
                    DebugCam.cameraPosition += dir;
                    break;
                case 'S':
                case 's':
                    dir = vec3(0.0f);
                    dir = DebugCam.cameraFront * vec3(0.3f, 0.3f, 0.3f);
                    DebugCam.cameraPosition -= dir;
                    break;

                case 'D':
                case 'd':
                    dir = vmath::cross(DebugCam.cameraFront, DebugCam.cameraUp);
                    dir = vmath::normalize(dir);
                    dir *= vec3(0.3f, 0.3f, 0.3f);
                    DebugCam.cameraPosition += dir;
                    break;
                case 'L':
                case 'l':
                break;
                default:
                break;
            }
        break;

        case WM_LBUTTONDOWN:
            DebugCam.lastMouseX = LOWORD(lParam);
            DebugCam.lastMouseY = HIWORD(lParam);
        break;
        
        case WM_LBUTTONUP:
            DebugCam.lastMouseX = -1;
            DebugCam.lastMouseY = -1;
        break;
        
        case WM_MOUSEMOVE:
            if (DebugCam.lastMouseX != -1 && DebugCam.lastMouseY != -1)
            {
                offsetX = LOWORD(lParam) - DebugCam.lastMouseX;
                offsetY = DebugCam.lastMouseY - HIWORD(lParam);

                DebugCam.lastMouseX = LOWORD(lParam);
                DebugCam.lastMouseY = HIWORD(lParam);

                offsetX *= 0.1f;
                offsetY *= 0.1f;

                DebugCam.cameraYaw += offsetX;
                DebugCam.cameraPitch += offsetY;

                if (DebugCam.cameraPitch > 89.0f)
                {
                    DebugCam.cameraPitch = 89.0f;
                }
                else if (DebugCam.cameraPitch < -89.0f)
                {
                    DebugCam.cameraPitch = -89.0f;
                }

                dir = vec3(cos(vmath::radians(DebugCam.cameraYaw)) * cos(vmath::radians(DebugCam.cameraPitch)), sin(vmath::radians(DebugCam.cameraPitch)), sin(vmath::radians(DebugCam.cameraYaw)) * cos(vmath::radians(DebugCam.cameraPitch)));
                DebugCam.cameraFront = vmath::normalize(dir);
            }
        break;
        case WM_SETFOCUS:
            //fprintf(gpFile,"Set Focus True.\n");
            gbActiveWindow = TRUE;
        break;

        case WM_KILLFOCUS:
            //fprintf(gpFile,"Set Focus False.\n");
            //gbActiveWindow = FALSE;
        break;
        
        case WM_ERASEBKGND:
            return 0;
        break;

        case WM_KEYDOWN:
            if(wParam == VK_ESCAPE)
            {
                //fprintf(gpFile,"Sending WM_CLOSE.\n");
                DestroyWindow(hwnd);
            }

            if (wParam == VK_SPACE)
            {
                if (keyPress > 3)
                    keyPress = 0;

                keyPress++;
            }

        break;

        case WM_SIZE:
            //fprintf(gpFile,"In WM SIZE message.\n");
            resize(LOWORD(lParam),HIWORD(lParam));
        break; 

        case WM_CLOSE:
            Log("LOG", "In WM_CLOSE Message.");
            DestroyWindow(hwnd);
        break;

        case WM_DESTROY:
            //uninitialize();
            Log("LOG", "In WM_DESTROY Message.");
            PostQuitMessage(0);
        break;

        default:
            break;
    }
    return DefWindowProc(hwnd,iMsg,wParam,lParam);
}

void ToogleFullScreen(void)
{
    // Varriable Declarations
    static DWORD dwStyle;
    static WINDOWPLACEMENT wp;
    MONITORINFO mi;

    // Code
    wp.length = sizeof(WINDOWPLACEMENT);
    if(gbFullScreen == FALSE)
    {
        dwStyle = GetWindowLong(ghwnd,GWL_STYLE);
        if(dwStyle & WS_OVERLAPPEDWINDOW)
        {
            mi.cbSize = sizeof(MONITORINFO);
            if(GetWindowPlacement(ghwnd,&wp) && GetMonitorInfo(MonitorFromWindow(ghwnd,MONITORINFOF_PRIMARY),&mi))
            {
                SetWindowLong(ghwnd,GWL_STYLE,(dwStyle & (~WS_OVERLAPPEDWINDOW)));
                SetWindowPos(   ghwnd,HWND_TOPMOST,mi.rcMonitor.left,mi.rcMonitor.top,
                                mi.rcMonitor.right - mi.rcMonitor.left,
                                mi.rcMonitor.bottom - mi.rcMonitor.top,
                                SWP_NOZORDER|SWP_FRAMECHANGED);
            }

            ShowCursor(TRUE);
            gbFullScreen = TRUE;
        }
    }
    else
    {
        SetWindowLong(ghwnd,GWL_STYLE,dwStyle|WS_OVERLAPPEDWINDOW);
        SetWindowPlacement(ghwnd,&wp);
        SetWindowPos(ghwnd,HWND_TOP,0,0,0,0,SWP_NOMOVE|SWP_NOSIZE|SWP_NOOWNERZORDER|SWP_NOZORDER|SWP_FRAMECHANGED);
        ShowCursor(TRUE);
        gbFullScreen = FALSE;
    }
}

void Sphere(float radius, int sector, int stack, std::vector<float>& sphere, std::vector<unsigned int>& index)
{
    float x, y, z, xy;
    float nx, ny, nz, lengthInv = 1.0f / radius;
    float s, t;

    float sectorStep = 2 * M_PI / sector;
    float stackStep = M_PI / stack;
    float sectorAngle, stackAngle;

    for (int i = 0; i <= stack; ++i)
    {
        stackAngle = M_PI / 2 - i * stackStep;
        xy = radius * cosf(stackAngle);
        z = radius * sinf(stackAngle);

        for (int j = 0; j <= sector; ++j)
        {
            sectorAngle = j * sectorStep;

            // vertex position
            x = xy * cosf(sectorAngle);
            y = xy * sinf(sectorAngle);

            sphere.push_back(x);
            sphere.push_back(y);
            sphere.push_back(z);

            // normals

            nx = x * lengthInv;
            ny = y * lengthInv;
            nz = z * lengthInv;

            sphere.push_back(nx);
            sphere.push_back(ny);
            sphere.push_back(nz);

            // Texcoord UV
            s = (float)j / sector;
            t = (float)i / stack;

            sphere.push_back(s);
            sphere.push_back(t);
        }

    }

    // Indicess
    /*
    k1 --- k1+1
    |   /   |
    |   /   |
    k2 --- k2+1
    */
    unsigned int k1, k2;
    for (int i = 0; i < stack; ++i)
    {
        k1 = i * (sector + 1);
        k2 = k1 + sector + 1;

        for (int j = 0; j < sector; ++j, ++k1, ++k2)
        {
            if (i != 0)
            {
                index.push_back(k1);
                index.push_back(k2);
                index.push_back(k1 + 1);
            }

            if (i != (stack - 1))
            {
                index.push_back(k1 + 1);
                index.push_back(k2);
                index.push_back(k2 + 1);
            }
        }
    }
}

void Cylinder(float baseRadius, float topRadius, float height, int sectors, int stack, std::vector<float>& cylinder, std::vector<unsigned int>& indices)
{
    float x, y, z;
    float radius;

    std::vector<float>unitCircle;
    std::vector<float>sideNormals;
    float sectorStep = 2 * M_PI / sectors;
    float sectorAngle;
    // build unit circle first

    for (int i = 0; i <= sectors; ++i)
    {
        sectorAngle = i * sectorStep;
        unitCircle.push_back(cosf(sectorAngle));
        unitCircle.push_back(sinf(sectorAngle));
        unitCircle.push_back(0);
    }

    // calculate side normals next
    float zAngle = atan2f(baseRadius - topRadius, height);
    float x0 = cosf(zAngle);
    float y0 = 0;
    float z0 = sin(zAngle);

    // rotate (x0,y0,z0) per sector angle

    for (int i = 0; i <= sectors; ++i)
    {
        sectorAngle = i * sectorStep;
        sideNormals.push_back(cosf(sectorAngle) * x0 - sinf(sectorAngle) * y0);
        sideNormals.push_back(sinf(sectorAngle) * x0 + cosf(sectorAngle) * y0);
        sideNormals.push_back(z0);
    }

    // put vertices of side cylinder to array by scaling unit circle

    for (int i = 0; i <= stack; ++i)
    {
        z = -(height * 0.5f) + (float)i / stack * height;
        radius = baseRadius + (float)i / stack * (topRadius - baseRadius);
        float t = 1.0f - (float)i / stack;

        for (int j = 0, k = 0; j <= sectors; ++j, k += 3)
        {
            x = unitCircle[k];
            y = unitCircle[k + 1];
            cylinder.push_back(x * radius);
            cylinder.push_back(y * radius);
            cylinder.push_back(z);
            cylinder.push_back(sideNormals[k]);
            cylinder.push_back(sideNormals[k + 1]);
            cylinder.push_back(sideNormals[k + 2]);
            cylinder.push_back((float)j / sectors);
            cylinder.push_back(t);
        }
    }

    // remember where the base vertices start
    unsigned int baseVertexIndex = (unsigned int)cylinder.size() / 8;

    z = -height * 0.5f;
    cylinder.push_back(0);
    cylinder.push_back(0);
    cylinder.push_back(z);
    cylinder.push_back(0);
    cylinder.push_back(0);
    cylinder.push_back(-1);
    cylinder.push_back(0.5f);
    cylinder.push_back(0.5f);

    for (int i = 0, j = 0; i < sectors; ++i, j += 3)
    {
        x = unitCircle[j];
        y = unitCircle[j + 1];
        cylinder.push_back(x * baseRadius);
        cylinder.push_back(y * baseRadius);
        cylinder.push_back(z);
        cylinder.push_back(0);
        cylinder.push_back(0);
        cylinder.push_back(-1);
        cylinder.push_back(-x * 0.5f + 0.5f);
        cylinder.push_back(-y * 0.5f + 0.5f);
    }

    // remember where the top vertices start
    unsigned int topVertexIndex = (unsigned int)cylinder.size() / 8;

    z = height * 0.5f;
    cylinder.push_back(0);
    cylinder.push_back(0);
    cylinder.push_back(z);
    cylinder.push_back(0);
    cylinder.push_back(0);
    cylinder.push_back(1);
    cylinder.push_back(0.5f);
    cylinder.push_back(0.5f);

    for (int i = 0, j = 0; i < sectors; ++i, j += 3)
    {
        x = unitCircle[j];
        y = unitCircle[j + 1];
        cylinder.push_back(x * topRadius);
        cylinder.push_back(y * topRadius);
        cylinder.push_back(z);
        cylinder.push_back(0);
        cylinder.push_back(0);
        cylinder.push_back(1);
        cylinder.push_back(x * 0.5f + 0.5f);
        cylinder.push_back(-y * 0.5f + 0.5f);
    }

    // put indices for sides
    unsigned int k1, k2;

    for (int i = 0; i < stack; ++i)
    {
        k1 = i * (sectors + 1);
        k2 = k1 + sectors + 1;

        for (int j = 0; j < sectors; ++j, ++k1, ++k2)
        {
            indices.push_back(k1);
            indices.push_back(k1 + 1);
            indices.push_back(k2);

            indices.push_back(k2);
            indices.push_back(k1 + 1);
            indices.push_back(k2 + 1);
        }
    }

    // remember where the base indices start
    unsigned int baseIndex = (unsigned int)indices.size();

    // put indices for base
    for (int i = 0, k = baseVertexIndex + 1; i < sectors; ++i, ++k)
    {
        if (i < (sectors - 1))
        {
            indices.push_back(baseVertexIndex);
            indices.push_back(k + 1);
            indices.push_back(k);
        }
        else
        {
            indices.push_back(baseVertexIndex);
            indices.push_back(baseVertexIndex + 1);
            indices.push_back(k);
        }
    }

    // remember where the base indices start
    unsigned int topIndex = (unsigned int)indices.size();

    for (int i = 0, k = topVertexIndex + 1; i < sectors; ++i, ++k)
    {
        if (i < sectors - 1)
        {
            indices.push_back(topVertexIndex);
            indices.push_back(k);
            indices.push_back(k + 1);
        }
        else
        {
            indices.push_back(topVertexIndex);
            indices.push_back(k);
            indices.push_back(topVertexIndex + 1);
        }
    }
}

void Torus(float tubeRadius, float radius, int numTR, int numR, std::vector<float>& torus)
{
    float du = 2 * M_PI / numR;
    float dv = 2 * M_PI / numTR;

    for (int i = 0; i < numR; i++)
    {
        float u = i * du;

        for (int j = 0; j <= numTR; j++)
        {
            float v = (j % numTR) * dv;

            for (int k = 0; k < 2; k++)
            {
                float uu = u + k * du;

                // push vertices
                torus.push_back((radius + tubeRadius * cosf(v)) * cosf(uu)); // x
                torus.push_back((radius + tubeRadius * cosf(v)) * sinf(uu)); // y
                torus.push_back(tubeRadius * sinf(v)); // z

                // normals
                torus.push_back(cosf(v) * cosf(uu));
                torus.push_back(cosf(v) * sinf(uu));
                torus.push_back(sinf(v));

                // texcoords
                torus.push_back(uu / (2 * M_PI));
                torus.push_back(v / (2 * M_PI));
            }
            v += dv;
        }
    }
}


int initialize(void)
{
    // Function Declarations
    void resize(int,int);
    void printGLInfo(void);
    void uninitialize(void);

    /*
        OpenGL Context Setup Start
    */

    // Variable Declarations
    PIXELFORMATDESCRIPTOR pfd;
    int iPixelFormatIndex = 0;

    // Code
    ZeroMemory(&pfd,sizeof(PIXELFORMATDESCRIPTOR));

    pfd.nSize = sizeof(PIXELFORMATDESCRIPTOR);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 32;
    pfd.cRedBits = 8;
    pfd.cGreenBits = 8;
    pfd.cBlueBits = 8;
    pfd.cAlphaBits = 8;
    pfd.cDepthBits = 32; // 24 is also allowed

    // Get DC
    ghdc = GetDC(ghwnd);

    // Choose Pixel Format
    iPixelFormatIndex = ChoosePixelFormat(ghdc,&pfd);

    if(iPixelFormatIndex == 0)
        return -1;

    // Set The Choosen Pixel Format
    if(SetPixelFormat(ghdc,iPixelFormatIndex,&pfd) == FALSE)
        return -2;

    // Create OpenGL Rendering Index

    ghrc = wglCreateContext(ghdc);

    if(ghrc == NULL)
        return -3;

    // Make Rendering as current context and rendering context
    if(wglMakeCurrent(ghdc,ghrc) == FALSE)
        return -4;

    // Here Starts OpenGL Code :

    //glew initialization
    if(glewInit() != GLEW_OK)
        return -5;

    // print opengl info
    //printGLInfo(); 
    
    Sphere(1.0f, 36, 18, sphere_vertex, sphere_index);
    Cylinder(1, 1, 2, 36, 8, cylinder_vertex, cylinder_index);
    Cylinder(0, 1, 2, 36, 8, cone_vertex, cone_index);
    Torus(0.5f, 1.0f, 36, 36, torus_vertex);

    /*
        OpenGL Context Setup End
    */

    /*
        OpenGL Default State Setup Start
    */

    GLuint vsh = loadShader("resource\\shaders\\basic.vert", GL_VERTEX_SHADER);
    GLuint fsh = loadShader("resource\\shaders\\basic.frag", GL_FRAGMENT_SHADER);

    myprog.program = glCreateProgram();
    glAttachShader(myprog.program, vsh);
    glAttachShader(myprog.program, fsh);
    glBindAttribLocation(myprog.program,0,"vPos");
    glBindAttribLocation(myprog.program, 1, "vCol");
    glLinkProgram(myprog.program);

    checkError(myprog.program,false);

    myprog.uniforms.projection = glGetUniformLocation(myprog.program, "pMat");
    myprog.uniforms.view = glGetUniformLocation(myprog.program, "vMat");
    myprog.uniforms.model = glGetUniformLocation(myprog.program, "mMat");
    myprog.uniforms.diffuse = glGetUniformLocation(myprog.program, "diffuse");

    loadTexture("resource\\textures\\box.png", &texture);

    const GLfloat squareData[] = 
    {
        +1.0f,-1.0f,0.0f,1.0f,0.0f,1.0f,0.0f,1.0f,1.0f,0.0f,
        -1.0f,-1.0f,0.0f,1.0f,1.0f,0.0f,0.0f,1.0f,0.0f,0.0,
        +1.0f,+1.0f,0.0f,1.0f,0.0f,0.0f,1.0f,1.0f,1.0f,1.0f,
        -1.0f,+1.0f,0.0f,1.0f,1.0f,0.0f,1.0f,1.0f,0.0f,1.0f
    };

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);
    {
        GLuint buffer;
        glGenBuffers(1, &buffer);
        glBindBuffer(GL_ARRAY_BUFFER, buffer);
        glBufferData(GL_ARRAY_BUFFER, sizeof(squareData), squareData, GL_STATIC_DRAW);
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 40, (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 40, (void*)(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 40, (void*)(8 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    //
    vsh = loadShader("resource\\shaders\\hologram.vert",GL_VERTEX_SHADER);
    fsh = loadShader("resource\\shaders\\hologram.frag", GL_FRAGMENT_SHADER);

    modelProgram.program = glCreateProgram();
    glAttachShader(modelProgram.program, vsh);
    glAttachShader(modelProgram.program, fsh);
    glBindAttribLocation(modelProgram.program, 0, "a_position");
    glBindAttribLocation(modelProgram.program, 1, "a_normal");
    glBindAttribLocation(modelProgram.program, 2, "a_texcoord");
    glBindAttribLocation(modelProgram.program, 3, "a_tangent");
    glBindAttribLocation(modelProgram.program, 4, "a_bitangent");
    glBindAttribLocation(modelProgram.program, 5, "a_boneIds");
    glBindAttribLocation(modelProgram.program, 6, "a_weights");
    glLinkProgram(modelProgram.program);

    checkError(modelProgram.program,false);

    modelProgram.uniforms.projection = glGetUniformLocation(modelProgram.program,"u_Projection");
    modelProgram.uniforms.view = glGetUniformLocation(modelProgram.program, "u_View");
    modelProgram.uniforms.model = glGetUniformLocation(modelProgram.program, "u_Model");
    modelProgram.uniforms.glitchSpeed = glGetUniformLocation(modelProgram.program, "m_GlitchSpeed");
    modelProgram.uniforms.glitchIntensity = glGetUniformLocation(modelProgram.program, "m_GlitchIntensity");
    modelProgram.uniforms.mainColor = glGetUniformLocation(modelProgram.program, "m_MainColor");
    modelProgram.uniforms.Time = glGetUniformLocation(modelProgram.program, "g_Time");
    modelProgram.uniforms.BarSpeed = glGetUniformLocation(modelProgram.program, "m_BarSpeed");
    modelProgram.uniforms.BarDistance = glGetUniformLocation(modelProgram.program, "m_BarDistance");
    modelProgram.uniforms.alpha = glGetUniformLocation(modelProgram.program, "m_alpha");
    modelProgram.uniforms.flickerSpeed = glGetUniformLocation(modelProgram.program, "m_FlickerSpeed");
    modelProgram.uniforms.rimColor = glGetUniformLocation(modelProgram.program, "m_RimColor");
    modelProgram.uniforms.rimPower = glGetUniformLocation(modelProgram.program, "m_RimPower");
    modelProgram.uniforms.glowSpeed = glGetUniformLocation(modelProgram.program, "m_GlowSpeed");
    modelProgram.uniforms.glowDistance = glGetUniformLocation(modelProgram.program, "m_GlowDistance");


    glGenVertexArrays(1, &vao_sphere);
    glBindVertexArray(vao_sphere);
    {
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, (unsigned int)sphere_vertex.size() * sizeof(float), sphere_vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_elements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)sphere_index.size() * sizeof(unsigned int), sphere_index.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao_cylinder);
    glBindVertexArray(vao_cylinder);
    {
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, (unsigned int)cylinder_vertex.size() * sizeof(float), cylinder_vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_elements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)cylinder_index.size() * sizeof(unsigned int), cylinder_index.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao_cone);
    glBindVertexArray(vao_cone);
    {
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, (unsigned int)cone_vertex.size() * sizeof(float), cone_vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glGenBuffers(1, &vbo_elements);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, (unsigned int)cone_index.size() * sizeof(unsigned int), cone_index.data(), GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);

    glGenVertexArrays(1, &vao_torus);
    glBindVertexArray(vao_torus);
    {
        glGenBuffers(1, &vbo_vertices);
        glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
        glBufferData(GL_ARRAY_BUFFER, (unsigned int)torus_vertex.size() * sizeof(float), torus_vertex.data(), GL_STATIC_DRAW);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 32, 0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 32, (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 32, (void*)(6 * sizeof(float)));
        glEnableVertexAttribArray(2);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }
    glBindVertexArray(0);



    //clear screen using blue color:
    glClearColor(0.1f,0.1f,0.1f,1.0f);

    // Depth Related Changes
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEBUG_OUTPUT);
    glDebugMessageCallback(ErrorCallback,0);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_ALPHA_TEST);
    //glAlphaFunc(GL_GREATER, 0);
    /*
        OpenGL Default State Setup End
    */

    //D:\Programming\OpenGL\SimpleOpenGLFrameWork\resource\models\vampire
    //backpack = new Model("resource\\models\\backpack\\backpack.obj");
    backpack = new Model("resource\\models\\heart_lp.obj");
    backpack->printMeshVertex();

    // Debug Cam Setup !!!
    DebugCam.lastMouseX = -1;
    DebugCam.lastMouseY = -1;
    DebugCam.cameraYaw = -90.0f;
    DebugCam.cameraPitch = 0.0f;
    DebugCam.cameraFront = vec3(0.0f, 0.0f, -1.0f);
    DebugCam.cameraPosition = vec3(0.0f,0.0f,0.0f);
    DebugCam.cameraUp = vec3(0.0f,1.0f,0.0f);

    //warmup resize call
    resize(WIN_WIDTH,WIN_HEIGHT);
    return 0;
}

void printGLInfo(void)
{
    // Local variable declarations
    GLint numExtentions = 0;

    // code
    FILE* glFile = NULL;
    fopen_s(&glFile, "GLInfo.txt", "w");
    fprintf(glFile,"OpenGL Vendor : %s\n",glGetString(GL_VENDOR));
    fprintf(glFile,"OpenGL Renderer : %s\n",glGetString(GL_RENDERER));
    fprintf(glFile,"OpenGL Version : %s\n",glGetString(GL_VERSION));
    fprintf(glFile,"OpenGL GLSL Version : %s\n",glGetString(GL_SHADING_LANGUAGE_VERSION));
    glGetIntegerv(GL_NUM_EXTENSIONS,&numExtentions);
    fprintf(glFile,"Number Of Supported Extensions : %d\n",numExtentions);
    //fprintf(gpFile,"OpenGL  : %s\n",glGetString(GL_VERSION));    
    for (int i = 0; i < numExtentions; i++)
    {
        /* code */
        fprintf(glFile,"%s\n",glGetStringi(GL_EXTENSIONS,i));
    }
    fclose(glFile);
    glFile = NULL;
}

void resize(int width,int height)
{
    // Code

    GLfloat aspectRatio = 0.0f;

    // to avoid divide by 0 error later in codebase.
    if(height == 0)
        height = 1;
    
    glViewport(0,0,(GLsizei)width,(GLsizei)height);
    projectionMatrix = vmath::perspective(45.0f, (GLfloat)width / (GLfloat)height, 0.1f, 100.0f);
}

void display(void)
{
    // Code
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);


    mat4 modelMatrix = mat4::identity();
    mat4 viewMatrix = mat4::identity();

    modelMatrix *= translate(0.0f, 0.0f, -4.0f);
    viewMatrix = vmath::lookat(DebugCam.cameraPosition, DebugCam.cameraFront, DebugCam.cameraUp);

    switch (keyPress)
    {
        case 0:
            glUseProgram(modelProgram.program);
            glUniformMatrix4fv(modelProgram.uniforms.model, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.view, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.projection, 1, GL_FALSE, projectionMatrix);

            glUniform1f(modelProgram.uniforms.glitchSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glitchIntensity, 1.0f);
            glUniform4fv(modelProgram.uniforms.mainColor, 1, vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.Time, g_time);
            glUniform1f(modelProgram.uniforms.BarSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.BarDistance, 15.0f);
            glUniform1f(modelProgram.uniforms.alpha, 1.0f);
            glUniform1f(modelProgram.uniforms.flickerSpeed, 10.0f);
            glUniform4fv(modelProgram.uniforms.rimColor, 1, vmath::vec4(0.0f, 1.0f, 1.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.rimPower, 5.0f);
            glUniform1f(modelProgram.uniforms.glowSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glowDistance, 1.0f);
            glBindVertexArray(vao_sphere);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
            glDrawElements(GL_TRIANGLES, sphere_index.size(), GL_UNSIGNED_INT, (void*)0);
            glBindVertexArray(0);
            glUseProgram(0);
        break;
        case 1:
            modelMatrix *= rotate(g_time * 20.0f, 0.0f, 1.0f, 0.0f);
            glUseProgram(modelProgram.program);
            glUniformMatrix4fv(modelProgram.uniforms.model, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.view, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.projection, 1, GL_FALSE, projectionMatrix);

            glUniform1f(modelProgram.uniforms.glitchSpeed, 50.0f);
            glUniform1f(modelProgram.uniforms.glitchIntensity, 100.0f);
            glUniform4fv(modelProgram.uniforms.mainColor, 1, vmath::vec4(1.0f, 0.0f, 0.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.Time, g_time);
            glUniform1f(modelProgram.uniforms.BarSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.BarDistance, 20.0f);
            glUniform1f(modelProgram.uniforms.alpha, 1.0f);
            glUniform1f(modelProgram.uniforms.flickerSpeed, 10.0f);
            glUniform4fv(modelProgram.uniforms.rimColor, 1, vmath::vec4(0.0f, 1.0f, 0.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.rimPower, 1.0f);
            glUniform1f(modelProgram.uniforms.glowSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glowDistance, 0.0f);
            glBindVertexArray(vao_cylinder);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
            glDrawElements(GL_TRIANGLES, cylinder_index.size(), GL_UNSIGNED_INT, (void*)0);
            //glDrawArrays(GL_TRIANGLE_STRIP, 0, torus_vertex.size() / 8);
            glBindVertexArray(0);
            glUseProgram(0);
        break;
        case 2:

            modelMatrix *=  rotate(90.0f, 1.0f, 0.0f, 0.0f);
            glUseProgram(modelProgram.program);
            glUniformMatrix4fv(modelProgram.uniforms.model, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.view, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.projection, 1, GL_FALSE, projectionMatrix);

            glUniform1f(modelProgram.uniforms.glitchSpeed, 50.0f);
            glUniform1f(modelProgram.uniforms.glitchIntensity, 100.0f);
            glUniform4fv(modelProgram.uniforms.mainColor, 1, vmath::vec4(1.0f, 1.0f, 1.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.Time, g_time);
            glUniform1f(modelProgram.uniforms.BarSpeed, 2.0f);
            glUniform1f(modelProgram.uniforms.BarDistance, 5.0f);
            glUniform1f(modelProgram.uniforms.alpha, 1.0f);
            glUniform1f(modelProgram.uniforms.flickerSpeed, 10.0f);
            glUniform4fv(modelProgram.uniforms.rimColor, 1, vmath::vec4(1.0f, 1.0f, 0.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.rimPower, 10.0f);
            glUniform1f(modelProgram.uniforms.glowSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glowDistance, 0.0f);
            glBindVertexArray(vao_cone);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
            glDrawElements(GL_TRIANGLES, cone_index.size(), GL_UNSIGNED_INT, (void*)0);
            //glDrawArrays(GL_TRIANGLE_STRIP, 0, torus_vertex.size() / 8);
            glBindVertexArray(0);
            glUseProgram(0);
        break;
        case 3:
            modelMatrix *= rotate(g_time * 20.0f, 0.0f, 1.0f, 0.0f);
            glUseProgram(modelProgram.program);
            glUniformMatrix4fv(modelProgram.uniforms.model, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.view, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.projection, 1, GL_FALSE, projectionMatrix);

            glUniform1f(modelProgram.uniforms.glitchSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glitchIntensity, 10.0f);
            glUniform4fv(modelProgram.uniforms.mainColor, 1, vmath::vec4(0.62f, 0.12f, 0.94f, 1.0f));
            glUniform1f(modelProgram.uniforms.Time, g_time);
            glUniform1f(modelProgram.uniforms.BarSpeed, 0.5f);
            glUniform1f(modelProgram.uniforms.BarDistance, 5.0f);
            glUniform1f(modelProgram.uniforms.alpha, 1.0f);
            glUniform1f(modelProgram.uniforms.flickerSpeed, 1.0f);
            glUniform4fv(modelProgram.uniforms.rimColor, 1, vmath::vec4(0.0f, 0.0f, 1.0f, 1.0f));
            glUniform1f(modelProgram.uniforms.rimPower, 1.0f);
            glUniform1f(modelProgram.uniforms.glowSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glowDistance, 0.0f);
            glBindVertexArray(vao_torus);
            glBindBuffer(GL_ARRAY_BUFFER, vbo_vertices);
            //glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, vbo_elements);
            //glDrawElements(GL_TRIANGLES, cone_index.size(), GL_UNSIGNED_INT, (void*)0);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, torus_vertex.size() / 8);
            glBindVertexArray(0);
            glUseProgram(0);
        break;
        case 4:
            modelMatrix *= translate(0.0f, -2.0f, -2.0f);
            modelMatrix *= rotate(g_time * 10.0f, 0.0f, 1.0f, 0.0f) * scale(0.1f, 0.1f, 0.1f);
            //modelMatrix *= scale(0.01f, 0.01f, 0.01f);
            glUseProgram(modelProgram.program);
            glUniformMatrix4fv(modelProgram.uniforms.model, 1, GL_FALSE, modelMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.view, 1, GL_FALSE, viewMatrix);
            glUniformMatrix4fv(modelProgram.uniforms.projection, 1, GL_FALSE, projectionMatrix);

            glUniform1f(modelProgram.uniforms.glitchSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glitchIntensity, 1.0f);
            glUniform4fv(modelProgram.uniforms.mainColor, 1, vmath::vec4(0.6f, 0.1f, 0.1f, 1.0f));
            glUniform1f(modelProgram.uniforms.Time, g_time);
            glUniform1f(modelProgram.uniforms.BarSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.BarDistance, 10.0f);
            glUniform1f(modelProgram.uniforms.alpha, 1.0f);
            glUniform1f(modelProgram.uniforms.flickerSpeed, 5.0f);
            glUniform4fv(modelProgram.uniforms.rimColor, 1, vmath::vec4(0.6f, 0.1f, 0.1f, 1.0f));
            glUniform1f(modelProgram.uniforms.rimPower, 1.0f);
            glUniform1f(modelProgram.uniforms.glowSpeed, 1.0f);
            glUniform1f(modelProgram.uniforms.glowDistance, 0.0f);
            backpack->Draw(modelProgram.program);
            glUseProgram(0);
        break;
    default:
        break;
    }
    SwapBuffers(ghdc);
}

void update(void)
{
    // Code

    static float t = 0.0f;
    static ULONGLONG timeStart = 0;
    ULONGLONG timeCur = GetTickCount64();
    if (timeStart == 0)
        timeStart = timeCur;
    t = (timeCur - timeStart) / 1000.0f;
    timeStart = timeCur;

    g_time += 0.005f;

    if (g_time >= 360.0f)
        g_time = 0.0f;

}

void uninitialize(void)
{
    // Function Declarations

    void ToogleFullScreen(void);

    // Code

    if (backpack)
    {
        backpack->ModelCleanup();
        delete backpack;
        backpack = NULL;
    }

    if (vao)
    {
        glDeleteVertexArrays(1, &vao);
        vao = 0;
    }

    if (myprog.program)
    {
        glUseProgram(myprog.program);

        GLsizei num_attached_shaders;
        glGetProgramiv(myprog.program, GL_ATTACHED_SHADERS, &num_attached_shaders);
        GLuint* shader_objects = NULL;

        shader_objects = (GLuint*)malloc(num_attached_shaders);

        glGetAttachedShaders(myprog.program, num_attached_shaders, &num_attached_shaders, shader_objects);

        for (GLsizei i = 0; i < num_attached_shaders; i++)
        {
            glDetachShader(myprog.program, shader_objects[i]);
            glDeleteShader(shader_objects[i]);
            shader_objects[i] = 0;
        }

        free(shader_objects);

        glUseProgram(0);

        glDeleteProgram(myprog.program);
    }

    if(gbFullScreen)
    {
        ToogleFullScreen();
    }

    if(wglGetCurrentContext() == ghrc)
    {
        wglMakeCurrent(NULL,NULL);
    }

    if(ghrc)
    {
        wglDeleteContext(ghrc);
        ghrc = NULL;
    }

    if(ghdc)
    {
        ReleaseDC(ghwnd,ghdc);
    }

    if(ghwnd)
    {
        DestroyWindow(ghwnd);
    }

    if(gpFile)
    {
        Log("LOG", "Closing Log File.");
        gpFile = NULL;
    }
}

void GLAPIENTRY ErrorCallback(GLenum src, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
    fopen_s(&glLog, "GLLOG.txt", "a");
    fprintf(glLog,"GL CALLBACK: %s type = 0x%x, serverity = 0x%x, message = %s\n", (type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""), type, severity, message);
    fclose(glLog);
}
