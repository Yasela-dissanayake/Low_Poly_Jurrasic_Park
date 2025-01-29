#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <SOIL/SOIL.h>
#include <cstdlib>
#include <ctime>
#include <tuple>
#include <vector>
#include <fstream>
#include <string>
#include <sstream>

GLfloat trainPosition = 0.0;
GLfloat camXInit = 0.0;
GLfloat camYInit = 0.5;
GLfloat camZInit = 0.0;
GLfloat sceRXInit = 0.0;
GLfloat sceRYInit = 75.0;
GLfloat sceRZInit = 0.0;
GLfloat sceTXInit = 0.0;
GLfloat sceTYInit = 0.0;
GLfloat sceTZInit = 0.0;

// variables to move the camera
GLfloat camX = camXInit;
GLfloat camY = camYInit;
GLfloat camZ = camZInit;

// variables to move the look at position
GLfloat lookX = 0.0;
GLfloat lookY = 0.0;
GLfloat lookZ = 0.0;

// variables to move the scene
GLfloat sceRX = sceRXInit;
GLfloat sceRY = sceRYInit;
GLfloat sceRZ = sceRZInit;
GLfloat sceTX = sceTXInit;
GLfloat sceTY = sceTYInit;
GLfloat sceTZ = sceTZInit;

////variables to move the objects
GLfloat objRX = 0.0;
GLfloat objRY = 0.0;
GLfloat objRZ = 0.0;
GLfloat objTX = 0.0;
GLfloat objTY = 0.0;
GLfloat objTZ = 0.0;

constexpr float PI = 3.14159265358979323846;

// Variables to control the visibility of the grid and axes
bool showGrid = true;
bool showAxes = true;

// Global color variables
GLfloat wheelColor[] = {0.7529f, 0.7529f, 0.7529f};
GLfloat diskColor[] = {0.8529f, 0.7929f, 0.6529f};
GLfloat bodyColor[] = {0.5176f, 0.5176f, 0.5176f};
GLfloat decorativeElementColor[] = {0.3804f, 0.3804f, 0.3804f};
GLfloat decorativeTopElementColor[] = {0.5098f, 0.5098f, 0.5059f};
GLfloat windshieldColor[] = {0.1255f, 0.1333f, 0.1686f};
GLfloat windowColor[] = {0.4745f, 0.5412f, 0.6784f};
GLfloat frameColor[] = {0.6039f, 0.6157f, 0.6588f};
GLfloat primaryLightColor[] = {0.9647f, 0.9647f, 0.9725f};
GLfloat doorColor[] = {0.4784f, 0.4588f, 0.4431f};
GLfloat grilleColor[] = {0.3686f, 0.4f, 0.4667f};
GLfloat wheelCoverColor[] = {0.4f, 0.4f, 0.4f};

void mouseMotion(int x, int y);
void mouseButton(int button, int state, int x, int y);

int lastMouseX, lastMouseY;
bool leftButtonDown = false;
bool rightButtonDown = false;
bool middleButtonDown = false;

GLfloat light1_pos[] = {-10.0, 1.0, -0.5, 1.0}; // First light position
GLfloat light2_pos[] = {10.0, 1.0, -0.5, 1.0};  // Second light position

GLuint floorTexture;
GLuint doorTexture;
GLuint doorrTexture;
GLuint logoTexture;
GLuint woodTexture;
GLuint dinoTexture;
GLuint dino2Texture;
GLuint rhinoTexture;

float gateAngle = 0.0f;
float tailAngle = 0.0f;
float headAngle = 0.0f;
float animationSpeed = 2.0f; // Controls how fast the animation moves

struct Vertex
{
    float x, y, z;
    float texU, texV; // Added texture coordinates
    Vertex(float _x = 0, float _y = 0, float _z = 0, float _u = 0, float _v = 0)
        : x(_x), y(_y), z(_z), texU(_u), texV(_v) {}
};

struct Face
{
    std::vector<Vertex> vertices;
};

std::vector<Face> faces;

// save normals
std::tuple<float, float, float> normal;

// function for calculate normals
std::tuple<float, float, float> calculateNormal(
    const std::tuple<float, float, float> &v1,
    const std::tuple<float, float, float> &v2,
    const std::tuple<float, float, float> &v3)
{

    float x1 = std::get<0>(v1), y1 = std::get<1>(v1), z1 = std::get<2>(v1);
    float x2 = std::get<0>(v2), y2 = std::get<1>(v2), z2 = std::get<2>(v2);
    float x3 = std::get<0>(v3), y3 = std::get<1>(v3), z3 = std::get<2>(v3);

    float ux = x2 - x1, uy = y2 - y1, uz = z2 - z1;
    float vx = x3 - x1, vy = y3 - y1, vz = z3 - z1;

    float nx = uy * vz - uz * vy;
    float ny = uz * vx - ux * vz;
    float nz = ux * vy - uy * vx;

    float length = std::sqrt(nx * nx + ny * ny + nz * nz);
    if (length != 0)
    {
        nx /= length;
        ny /= length;
        nz /= length;
    }

    return std::make_tuple(nx, ny, nz);
}

// Function to trim whitespace from string
std::string trim(const std::string &str)
{
    size_t first = str.find_first_not_of(" \t");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t");
    return str.substr(first, (last - first + 1));
}

// Function to parse vertex coordinates from string
Vertex parseVertex(const std::string &line)
{
    std::string coords = line.substr(line.find("[") + 1);
    coords = coords.substr(0, coords.find("]"));

    std::stringstream ss(coords);
    std::string temp;
    std::vector<float> values;

    while (std::getline(ss, temp, ','))
    {
        values.push_back(std::stof(trim(temp)));
    }

    // Calculate texture coordinates based on vertex position
    float texU = (values[0] + 5.0f) / 10.0f; // Map [-5,5] to [0,1]
    float texV = (values[2] + 5.0f) / 10.0f; // Map [-5,5] to [0,1]

    return Vertex(values[0], values[1], values[2], texU, texV);
}

//-------------------Environment-------------------

GLuint loadTexture(const char *filename)
{
    GLuint texture = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_RGBA,
        SOIL_CREATE_NEW_ID,
        SOIL_FLAG_MIPMAPS | SOIL_FLAG_INVERT_Y);

    if (!texture)
    {
        printf("Texture loading failed: %s\n", SOIL_last_result());
    }
    else
    {
        glBindTexture(GL_TEXTURE_2D, texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    }

    return texture;
}

void loadFacesFromFile(const char *filename)
{
    std::ifstream file(filename);
    if (!file.is_open())
    {
        printf("Failed to open file: %s\n", filename);
        return;
    }

    std::string line;
    Face currentFace;

    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.find("Face") == 0)
        {
            if (!currentFace.vertices.empty())
            {
                faces.push_back(currentFace);
                currentFace.vertices.clear();
            }
        }
        else if (line.find("Vertex") == 0)
        {
            currentFace.vertices.push_back(parseVertex(line));
        }
    }

    if (!currentFace.vertices.empty())
    {
        faces.push_back(currentFace);
    }

    file.close();
    printf("Loaded %zu faces from file\n", faces.size());
}

void drawGrid()
{
    GLfloat step = 1.0f;
    GLint line;

    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f); // Translate to the origin

    glColor3f(0.7f, 0.7f, 0.7f); // Light gray color for the grid lines
    glBegin(GL_LINES);
    for (line = -20; line <= 20; line += step)
    {
        glVertex3f(line, 0.0, 20);
        glVertex3f(line, 0.0, -20);

        glVertex3f(20, 0.0, line);
        glVertex3f(-20, 0.0, line);
    }
    glEnd();

    // Draw measurements
    glColor3f(1.0f, 1.0f, 1.0f); // White color for the measurements
    for (line = -20; line <= 20; line += step)
    {
        glRasterPos3f(line, 0.0, 0.0);
        char buffer[12];
        snprintf(buffer, sizeof(buffer), "%d", line);
        for (char *c = buffer; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }

        glRasterPos3f(0.0, 0.0, line);
        snprintf(buffer, sizeof(buffer), "%d", line);
        for (char *c = buffer; *c != '\0'; c++)
        {
            glutBitmapCharacter(GLUT_BITMAP_HELVETICA_10, *c);
        }
    }

    glPopMatrix();
}

void drawAxes()
{
    glPushMatrix();
    glTranslatef(0.0f, 0.0f, 0.0f);

    glBegin(GL_LINES);

    glLineWidth(1.5);

    glColor3f(1.0, 0.0, 0.0);
    glVertex3f(-200, 0, 0);
    glVertex3f(200, 0, 0);

    glColor3f(0.0, 1.0, 0.0);
    glVertex3f(0, -200, 0);
    glVertex3f(0, 200, 0);

    glColor3f(0.0, 0.0, 1.0);
    glVertex3f(0, 0, -200);
    glVertex3f(0, 0, 200);

    glEnd();

    glPopMatrix();
}

void init(void)
{
    glClearColor(0, 0, 0, 1.0);
    glClearDepth(1.0);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_DEPTH_TEST);

    // Enable lighting
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);

    // Set up light parameters for the first light
    GLfloat light0Ambient[] = {0.2f, 0.2f, 0.2f, 1.0f};
    GLfloat light0Diffuse[] = {0.8f, 0.8f, 0.8f, 1.0f};
    GLfloat light0Specular[] = {1.0f, 0.5f, 1.0f, 1.0f};
    GLfloat light0Position[] = {0.0f, 10.0f, 10.0f, 10.0f};

    glLightfv(GL_LIGHT0, GL_AMBIENT, light0Ambient);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, light0Diffuse);
    glLightfv(GL_LIGHT0, GL_SPECULAR, light0Specular);
    glLightfv(GL_LIGHT0, GL_POSITION, light0Position);

    // Enable color tracking
    glEnable(GL_COLOR_MATERIAL);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    // Enable smooth shading
    glShadeModel(GL_SMOOTH);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    floorTexture = loadTexture("ground.png");
    dino2Texture = loadTexture("dino-texture2.jpg");

    // Load the faces from file
    loadFacesFromFile("dino1.txt");
}

// animation angles update
void updateAnimation()
{
    // body swing
    tailAngle = 15.0f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f * animationSpeed);

    // body swing 2
    headAngle = 10.0f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.0007f * animationSpeed);
}

//-------------------Basic Shapes-------------------
void drawClosedCylinder()
{
    const int slices = 20;
    const float radius = 0.4f;
    const float height = 1.0f;

    // Draw the cylinder body
    glPushMatrix();
    glTranslatef(-1, 0, 0.0f);
    glRotatef(90, 0.0, 1.0, 0.0);
    glScalef(1, 1, 2.0);

    glBegin(GL_QUAD_STRIP);
    for (int i = 0; i <= slices; ++i)
    {
        float angle = 2.0f * PI * i / slices;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glNormal3f(x, y, 0.0f);
        glVertex3f(x, y, 0.0f);
        glVertex3f(x, y, height);
    }
    glEnd();

    // Draw the front disk
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, -1.0f);
    glVertex3f(0.0f, 0.0f, 0.0f);
    for (int i = 0; i <= slices; ++i)
    {
        float angle = 2.0f * PI * i / slices;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, 0.0f);
    }
    glEnd();

    // Draw the back disk
    glBegin(GL_TRIANGLE_FAN);
    glNormal3f(0.0f, 0.0f, 1.0f);
    glVertex3f(0.0f, 0.0f, height);
    for (int i = 0; i <= slices; ++i)
    {
        float angle = 2.0f * PI * i / slices;
        float x = radius * cos(angle);
        float y = radius * sin(angle);
        glVertex3f(x, y, height);
    }
    glEnd();

    glPopMatrix();
}

void drawCubeWithTexture(GLuint texture, GLuint texture_front, float height, float width)
{
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glPushMatrix();

    float halfHeight = height / 2.0f;
    float halfWidth = width / 2.0f;

    // Front Face
    glBindTexture(GL_TEXTURE_2D, texture_front);
    glBegin(GL_QUADS);
    {
        auto normal = calculateNormal(
            {-halfWidth, -halfHeight, halfWidth},
            {halfWidth, -halfHeight, halfWidth},
            {halfWidth, halfHeight, halfWidth});
        glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-halfWidth, -halfHeight, halfWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(halfWidth, -halfHeight, halfWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(halfWidth, halfHeight, halfWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-halfWidth, halfHeight, halfWidth);
    }
    glEnd();

    // Back Face
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    {
        auto normal = calculateNormal(
            {-halfWidth, -halfHeight, -halfWidth},
            {halfWidth, -halfHeight, -halfWidth},
            {halfWidth, halfHeight, -halfWidth});
        glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-halfWidth, -halfHeight, -halfWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-halfWidth, halfHeight, -halfWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(halfWidth, halfHeight, -halfWidth);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(halfWidth, -halfHeight, -halfWidth);
    }
    glEnd();

    // Top Face
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    {
        auto normal = calculateNormal(
            {-halfWidth, halfHeight, -halfWidth},
            {-halfWidth, halfHeight, halfWidth},
            {halfWidth, halfHeight, halfWidth});
        glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-halfWidth, halfHeight, -halfWidth);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-halfWidth, halfHeight, halfWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(halfWidth, halfHeight, halfWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(halfWidth, halfHeight, -halfWidth);
    }
    glEnd();

    // Bottom Face
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    {
        auto normal = calculateNormal(
            {-halfWidth, -halfHeight, -halfWidth},
            {halfWidth, -halfHeight, -halfWidth},
            {halfWidth, -halfHeight, halfWidth});
        glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-halfWidth, -halfHeight, -halfWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(halfWidth, -halfHeight, -halfWidth);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(halfWidth, -halfHeight, halfWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-halfWidth, -halfHeight, halfWidth);
    }
    glEnd();

    // Right Face
    glBindTexture(GL_TEXTURE_2D, texture);
    glBegin(GL_QUADS);
    {
        auto normal = calculateNormal(
            {halfWidth, -halfHeight, -halfWidth},
            {halfWidth, -halfHeight, halfWidth},
            {halfWidth, halfHeight, halfWidth});
        glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(halfWidth, -halfHeight, -halfWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(halfWidth, halfHeight, -halfWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(halfWidth, halfHeight, halfWidth);
        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(halfWidth, -halfHeight, halfWidth);
    }
    glEnd();

    // Left Face
    glBindTexture(GL_TEXTURE_2D, texture_front);
    glBegin(GL_QUADS);
    {
        auto normal = calculateNormal(
            {-halfWidth, -halfHeight, -halfWidth},
            {-halfWidth, -halfHeight, halfWidth},
            {-halfWidth, halfHeight, halfWidth});
        glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

        glTexCoord2f(0.0f, 0.0f);
        glVertex3f(-halfWidth, -halfHeight, -halfWidth);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3f(-halfWidth, -halfHeight, halfWidth);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3f(-halfWidth, halfHeight, halfWidth);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3f(-halfWidth, halfHeight, -halfWidth);
    }
    glEnd();

    glPopMatrix();
    glDisable(GL_TEXTURE_2D);
}

void drawDisk()
{
    glPushMatrix();
    glScalef(0.05, 0.4, 0.4);
    drawClosedCylinder();
    glPopMatrix();
}

void drawPyramid(float baseSize, float height)
{
    float halfBase = baseSize / 2.0f;
    glPushMatrix();
    glBegin(GL_TRIANGLES);
    // Front face
    glNormal3f(0.0f, 0.5f, 0.5f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(-halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, halfBase);

    // Right face
    glNormal3f(0.5f, 0.5f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, -halfBase);

    // Back face
    glNormal3f(0.0f, 0.5f, -0.5f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(halfBase, 0.0f, -halfBase);
    glVertex3f(-halfBase, 0.0f, -halfBase);

    // Left face
    glNormal3f(-0.5f, 0.5f, 0.0f);
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(-halfBase, 0.0f, -halfBase);
    glVertex3f(-halfBase, 0.0f, halfBase);
    glEnd();

    // Base of the pyramid
    glBegin(GL_QUADS);
    glNormal3f(0.0f, -1.0f, 0.0f);
    glVertex3f(-halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, -halfBase);
    glVertex3f(-halfBase, 0.0f, -halfBase);
    glEnd();

    glPopMatrix();
}

void pileWithBevels(float height, float width, float bevel)
{

    float half_Width = width / 2;
    float half_Width_n_bevel = half_Width + bevel;

    glPushMatrix();

    // bottom face
    glBegin(GL_POLYGON);
    normal = calculateNormal(
        {-half_Width, 0, half_Width_n_bevel},
        {-half_Width_n_bevel, 0, half_Width},
        {-half_Width_n_bevel, 0, -half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0, half_Width_n_bevel);
    glVertex3f(half_Width, 0, half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, 0, half_Width);
    glVertex3f(half_Width_n_bevel, 0, -half_Width);
    glVertex3f(half_Width, 0, -half_Width_n_bevel);
    glVertex3f(-half_Width, 0, -half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, 0, -half_Width);
    glVertex3f(-half_Width_n_bevel, 0, half_Width);
    glEnd();

    // top face
    glBegin(GL_POLYGON);
    normal = calculateNormal(
        {-half_Width, height, half_Width_n_bevel},
        {half_Width, height, half_Width_n_bevel},
        {half_Width_n_bevel, height, half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, height, half_Width_n_bevel);
    glVertex3f(half_Width, height, half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, height, half_Width);
    glVertex3f(half_Width_n_bevel, height, -half_Width);
    glVertex3f(half_Width, height, -half_Width_n_bevel);
    glVertex3f(-half_Width, height, -half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, height, -half_Width);
    glVertex3f(-half_Width_n_bevel, height, half_Width);
    glEnd();

    glBegin(GL_QUADS);

    // front face
    normal = calculateNormal(
        {-half_Width, 0.0f, -half_Width_n_bevel},
        {half_Width, 0, half_Width_n_bevel},
        {half_Width, height, half_Width_n_bevel});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, half_Width_n_bevel);
    glVertex3f(half_Width, 0, half_Width_n_bevel);
    glVertex3f(half_Width, height, half_Width_n_bevel);
    glVertex3f(-half_Width, height, half_Width_n_bevel);

    // front mini face right
    normal = calculateNormal(
        {half_Width, 0.0f, half_Width_n_bevel},
        {half_Width_n_bevel, 0.0f, half_Width},
        {half_Width_n_bevel, height, half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_Width, 0.0f, half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(half_Width_n_bevel, height, half_Width);
    glVertex3f(half_Width, height, half_Width_n_bevel);

    // front mini face left
    normal = calculateNormal(
        {-half_Width, 0.0f, half_Width_n_bevel},
        {-half_Width, height, half_Width_n_bevel},
        {-half_Width_n_bevel, height, half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(-half_Width_n_bevel, height, half_Width);
    glVertex3f(-half_Width, height, half_Width_n_bevel);

    // left face
    normal = calculateNormal(
        {half_Width_n_bevel, 0.0f, half_Width},
        {half_Width_n_bevel, 0, -half_Width},
        {half_Width_n_bevel, height, half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(half_Width_n_bevel, 0, -half_Width);
    glVertex3f(half_Width_n_bevel, height, -half_Width);
    glVertex3f(half_Width_n_bevel, height, half_Width);

    // right face
    normal = calculateNormal(
        {-half_Width_n_bevel, 0.0f, half_Width},
        {-half_Width_n_bevel, height, half_Width},
        {-half_Width_n_bevel, height, -half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width_n_bevel, 0.0f, half_Width);
    glVertex3f(-half_Width_n_bevel, height, half_Width);
    glVertex3f(-half_Width_n_bevel, height, -half_Width);
    glVertex3f(-half_Width_n_bevel, 0, -half_Width);

    // back face
    normal = calculateNormal(
        {-half_Width, 0.0f, -half_Width_n_bevel},
        {-half_Width, height, -half_Width_n_bevel},
        {half_Width, height, -half_Width_n_bevel});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, -half_Width_n_bevel);
    glVertex3f(-half_Width, height, -half_Width_n_bevel);
    glVertex3f(half_Width, height, -half_Width_n_bevel);
    glVertex3f(half_Width, 0, -half_Width_n_bevel);

    // back mini face right
    normal = calculateNormal(
        {half_Width, 0.0f, -half_Width_n_bevel},
        {half_Width, height, -half_Width_n_bevel},
        {half_Width_n_bevel, height, -half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_Width, 0.0f, -half_Width_n_bevel);
    glVertex3f(half_Width, height, -half_Width_n_bevel);
    glVertex3f(half_Width_n_bevel, height, -half_Width);
    glVertex3f(half_Width_n_bevel, 0.0f, -half_Width);

    normal = calculateNormal(
        {-half_Width, 0.0f, -half_Width_n_bevel},
        {-half_Width_n_bevel, 0.0f, -half_Width},
        {-half_Width_n_bevel, height, -half_Width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(-half_Width, 0.0f, -half_Width_n_bevel);
    glVertex3f(-half_Width_n_bevel, 0.0f, -half_Width);
    glVertex3f(-half_Width_n_bevel, height, -half_Width);
    glVertex3f(-half_Width, height, -half_Width_n_bevel);

    glEnd();

    glPopMatrix();
}

// prism
void drawPrism(float base, float height, float depth)
{
    glPushMatrix();
    glBegin(GL_TRIANGLES);

    // Front face (triangle)
    glVertex3f(-base / 2, 0.0f, depth / 2);
    glVertex3f(base / 2, 0.0f, depth / 2);
    glVertex3f(0.0f, height, depth / 2);

    // Back face (triangle)
    glVertex3f(-base / 2, 0.0f, -depth / 2);
    glVertex3f(base / 2, 0.0f, -depth / 2);
    glVertex3f(0.0f, height, -depth / 2);

    glEnd();
    glBegin(GL_QUADS);

    // Bottom face (rectangle)
    glVertex3f(-base / 2, 0.0f, depth / 2);
    glVertex3f(base / 2, 0.0f, depth / 2);
    glVertex3f(base / 2, 0.0f, -depth / 2);
    glVertex3f(-base / 2, 0.0f, -depth / 2);

    // Left face (rectangle)
    glVertex3f(-base / 2, 0.0f, depth / 2);
    glVertex3f(0.0f, height, depth / 2);
    glVertex3f(0.0f, height, -depth / 2);
    glVertex3f(-base / 2, 0.0f, -depth / 2);

    // Right face (rectangle)
    glVertex3f(base / 2, 0.0f, depth / 2);
    glVertex3f(0.0f, height, depth / 2);
    glVertex3f(0.0f, height, -depth / 2);
    glVertex3f(base / 2, 0.0f, -depth / 2);

    glEnd();
    glPopMatrix();
}

//-------------------Floor with Texture-------------------
void drawFloor()
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    // Reset color to white before drawing the textured floor
    glColor3f(1.0f, 1.0f, 1.0f);

    // Bind the texture to the floor
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-20.0, 0, 20.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-20.0, 0, -20.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(20.0, 0, -20.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(20.0, 0, 20.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
    glPopMatrix();
}

//-------------------Draw Tree Trunk-------------------
void drawTreeTrunk()
{

    // glColor3f(0.49019f, 0.36078f, 0.23921f);
    // glColor3f(0.5f, 0.35f, 0.05f);
    glPushMatrix();

    glScalef(1, 1.5, 1);
    pileWithBevels(1, 0.125, 0.075);
    glTranslatef(0, 0.5, 0);
    glRotatef(-60, 0, 0, 1);
    glScalef(0.5, 0.5, 0.5);
    pileWithBevels(1, 0.125, 0.075);

    glPushMatrix();
    glTranslatef(0.2, -0.3, 0);
    glRotatef(180, 1, 0, 0);

    glRotatef(60, 0, 0, 1);
    pileWithBevels(1, 0.125, 0.075);
    glPopMatrix();

    glPopMatrix();
}

//-----------------  Pine Tree -------------------
void drawPineTree(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)
{
    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    // Draw the trunk
    glColor3f(0.5f, 0.35f, 0.05f);
    pileWithBevels(1, 0.125, 0.075);

    // Draw the tree top
    glColor3f(0.0f, 0.5f, 0.0f);
    glPushMatrix();
    glTranslatef(0.0f, 0.5f, 0.0f);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glutSolidCone(0.5f, 1.0f, 10, 10);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.0f, 0.8f, 0.0f);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glutSolidCone(0.5f, 1.0f, 10, 10);
    glPopMatrix();

    glPopMatrix();
}

//-------------------Draw regular tree-------------------
void drawRegularTree(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)
{

    glPushMatrix();

    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    glPushMatrix();
    glColor3f(0.5f, 0.35f, 0.05f);
    drawTreeTrunk();
    glPopMatrix();

    glPushMatrix();
    glColor3f(0, 1, 0);
    glTranslatef(0.0f, 2.0f, 0.0f);
    glScalef(0.7f, 0.7f, 0.7f);
    glutSolidIcosahedron();
    glTranslatef(0.9f, -1.1f, 0.0f);
    glScalef(0.5f, 0.5f, 0.5f);
    glutSolidIcosahedron();

    glTranslatef(-3.5f, -1.0f, 0.0f);
    glutSolidIcosahedron();

    glPopMatrix();

    glPopMatrix();
}

//-------------------Floor with multiple tree types-------------------
void drawForest()
{
    srand(40);
    // srand(static_cast<unsigned int>(time(0))); // Use current time as seed for randomness

    for (int i = 0; i < 60; ++i)
    {
        float x = static_cast<float>(rand() % 41 - 20); // Random x position between -20 and 20
        float z = static_cast<float>(rand() % 41 - 20); // Random z position between -20 and 20
        float scales[] = {1.0f, 1.2f, 1.4f, 1.6f, 1.8f, 2.0f};
        float s = scales[rand() % 6]; // Randomly select one of the predefined scales

        glPushMatrix();
        if (rand() % 2 == 0)
        {
            drawPineTree(x, 0, z, s);
        }
        else
        {
            drawRegularTree(x, 0, z, s);
        }
        glPopMatrix();
    }
}

//-------------------Door pile Pyramid-------------------
void doorPilePyramid(float height, float half_top_width, float half_bottom_width, GLuint texture, GLuint texture_f)
{
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    // Enable 2D texturing
    // glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture

    glBegin(GL_QUADS);

    glBindTexture(GL_TEXTURE_2D, texture_f); // Bind the texture
    // Front Face
    normal = calculateNormal(
        {-half_bottom_width, 0.0f, half_bottom_width},
        {half_bottom_width, 0.0f, half_bottom_width},
        {half_top_width, height, half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-half_bottom_width, 0.0f, half_bottom_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(half_top_width, height, half_top_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-half_top_width, height, half_top_width);

    // Back Face
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture
    normal = calculateNormal(
        {-half_bottom_width, 0.0f, -half_bottom_width},
        {half_bottom_width, 0.0f, -half_bottom_width},
        {half_top_width, height, -half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(half_top_width, height, -half_top_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-half_top_width, height, -half_top_width);

    // Top Face
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture
    normal = calculateNormal(
        {-half_top_width, height, -half_top_width},
        {-half_top_width, height, half_top_width},
        {half_top_width, height, half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-half_top_width, height, -half_top_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-half_top_width, height, half_top_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(half_top_width, height, half_top_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(half_top_width, height, -half_top_width);

    // Bottom Face
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture
    normal = calculateNormal(
        {-half_bottom_width, 0.0f, -half_bottom_width},
        {half_bottom_width, 0.0f, -half_bottom_width},
        {half_bottom_width, 0.0f, half_bottom_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-half_bottom_width, 0.0f, half_bottom_width);

    // Right Face
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture
    normal = calculateNormal(
        {half_bottom_width, 0.0f, -half_bottom_width},
        {half_top_width, height, -half_top_width},
        {half_top_width, height, half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(half_top_width, height, -half_top_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(half_top_width, height, half_top_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);

    // Left Face
    glBindTexture(GL_TEXTURE_2D, texture); // Bind the texture
    normal = calculateNormal(
        {-half_bottom_width, 0.0f, -half_bottom_width},
        {-half_bottom_width, 0.0f, half_bottom_width},
        {-half_top_width, height, half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glTexCoord2f(0.0f, 0.0f);
    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(-half_bottom_width, 0.0f, half_bottom_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(-half_top_width, height, half_top_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-half_top_width, height, -half_top_width);

    glEnd();
    glDisable(GL_TEXTURE_2D); // Disable texturing
    glPopMatrix();
}

//-------------------Door pile-------------------
void drawDoorPile(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)
{
    glPushMatrix();
    woodTexture = loadTexture("wood.png");
    // glColor3f(0.6f, 0.3f, 0.0f); // Brown color
    glScalef(1, scale, 1);
    doorPilePyramid(2, 0.10, 0.4, woodTexture, woodTexture);
    glPopMatrix();
}

// Draw one section of the fence
void drawFenceSection()
{
    // Draw two vertical posts
    glPushMatrix();

    // draw post
    glPushMatrix();
    glColor3f(0.5f, 0.35f, 0.05f);  // Brown color
    glTranslatef(0.0f, 2.1f, 0.0f); // Adjust to align with the floor level
    glScalef(0.1f, 5.0f, 0.1f);     // Scale to make it a thin vertical post, height increased 5 times
    glutSolidCube(1.0f);
    glPopMatrix();

    glTranslatef(1.0f, 0.0f, 0.0f);

    // draw post
    glPushMatrix();
    glColor3f(0.5f, 0.35f, 0.05f);  // Brown color
    glTranslatef(0.0f, 2.1f, 0.0f); // Adjust to align with the floor level
    glScalef(0.1f, 5.0f, 0.1f);     // Scale to make it a thin vertical post, height increased 5 times
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();

    // Draw two horizontal planks
    glPushMatrix();

    // plank
    glPushMatrix();
    glColor3f(0.65f, 0.5f, 0.35f);  // Light brown color
    glTranslatef(0.0f, 2.1f, 0.0f); // Adjust to align with the floor level
    glScalef(1.0f, 0.1f, 0.1f);     // Scale to make it a horizontal plank
    glutSolidCube(1.0f);
    glPopMatrix();

    glTranslatef(0.0f, -2.5f, 0.0f); // Adjust the position of the second plank

    // plank
    glPushMatrix();
    glColor3f(0.65f, 0.5f, 0.35f);  // Light brown color
    glTranslatef(0.0f, 2.1f, 0.0f); // Adjust to align with the floor level
    glScalef(1.0f, 0.1f, 0.1f);     // Scale to make it a horizontal plank
    glutSolidCube(1.0f);
    glPopMatrix();

    glPopMatrix();
}

// Daw fence
void drawFence()
{
    float fenceLength = 40.0f; // Length of one side of the floor
    int numSections = 40;      // Number of fence sections per side
    float sectionSpacing = fenceLength / numSections;

    // Draw right side
    for (int i = 0; i <= numSections - 2; i++)
    {
        glPushMatrix();
        glTranslatef(-20.0f + i * sectionSpacing, -0.4f, 20.0f); // Adjust to align with the floor level
        drawFenceSection();
        glPopMatrix();
    }

    // Draw left side
    for (int i = 0; i <= numSections - 2; i++)
    {
        glPushMatrix();
        glTranslatef(-20.0f + i * sectionSpacing, -0.4f, -20.0f); // Adjust to align with the floor level
        drawFenceSection();
        glPopMatrix();
    }

    // Draw front side1
    for (int i = 0; i <= ((numSections / 2) - 2); i++)
    {
        glPushMatrix();
        glTranslatef(-19.0f, -0.4f, -20.0f + i * sectionSpacing); // Adjust to align with the floor level
        glRotatef(90, 0.0f, 1.0f, 0.0f);
        drawFenceSection();
        glPopMatrix();
    }

    // Draw front side2
    for (int i = ((numSections / 2) + 3); i <= (numSections); i++)
    {
        glPushMatrix();
        glTranslatef(-19.0f, -0.4f, -20.0f + i * sectionSpacing); // Adjust to align with the floor level
        glRotatef(90, 0.0f, 1.0f, 0.0f);
        drawFenceSection();
        glPopMatrix();
    }

    // Draw back side
    for (int i = 0; i <= numSections; i++)
    {
        glPushMatrix();
        glTranslatef(20.0f, -0.4f, -20.0f + i * sectionSpacing); // Adjust to align with the floor level
        glRotatef(90, 0.0f, 1.0f, 0.0f);
        drawFenceSection();
        glPopMatrix();
    }
}

// Draw foot
void drawFoot(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)

{
    glPushMatrix();
    glTranslatef(x, y, z);
    // glTranslatef(-0.2, 0.05, 0);
    glRotatef(90, 0, 0, 1);

    glPushMatrix();
    glTranslatef(0, 0, 0);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.6, 0.115);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, 0.3);
    glRotatef(25, 1, 0, 0);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.6, 0.115);
    // glTranslatef(-0.2, 0.05, 0);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, -0.3);
    glRotatef(-25, 1, 0, 0);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.6, 0.115);
    // glTranslatef(-0.2, 0.05, 0);
    glPopMatrix();

    glPopMatrix();
}

void drawTeeth(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)
{
    glPushMatrix();
    // glTranslatef(x, y, z);
    glScalef(scale, scale, scale);
    for (int i = 0; i < 6; i++)
    {
        drawPrism(0.8, 1, 0.8);
        glTranslatef(0, 0, 1.05);
    }
    glPopMatrix();
}

// Eye
void drawEye()
{
    GLUquadric *quad = gluNewQuadric();
    glPushMatrix();
    gluSphere(quad, 0.06, 32, 32);
    glColor3f(0, 0, 0);
    glTranslatef(0, -0.02, -0.01);
    gluSphere(quad, 0.04, 32, 32);
    glPopMatrix();

    gluDeleteQuadric(quad);
}
// Draw dino 1
void drawDino(float scale = 1.0f)
{
    dinoTexture = loadTexture("dino1-skin1.png");
    glColor3f(1.0f, 1.0f, 1.0f);
    glPushMatrix();
    // glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    // left leg
    glPushMatrix();

    doorPilePyramid(1.5, 0.3, 0.15, dinoTexture, dinoTexture);
    glTranslatef(0, 1.5, 0);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.6, 0.6);
    glPopMatrix();

    // right leg
    glPushMatrix();
    glTranslatef(0, 0, 1);
    doorPilePyramid(1.5, 0.3, 0.15, dinoTexture, dinoTexture);
    glTranslatef(0, 1.5, 0);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.6, 0.6);
    glPopMatrix();

    // middle body1
    glPushMatrix();
    glTranslatef(0, 0, 0.5);
    glRotatef(90, 0, 0, 1);
    glTranslatef(2, -1, 0);
    doorPilePyramid(1.5, 0.5, 0.3, dinoTexture, dinoTexture);
    glPopMatrix();

    // middle body2
    glPushMatrix();
    glTranslatef(-0.7, 2, 0.5);
    drawCubeWithTexture(dinoTexture, dinoTexture, 1, 1);
    glPopMatrix();

    // tail part1
    glPushMatrix();
    glTranslatef(1, 0, 0.5);
    glRotatef(90, 0, 0, 1);
    glTranslatef(1.8, -0.8, 0);
    glRotatef(-20, 0, 0, 1);
    doorPilePyramid(1, 0.2, 0.1, dinoTexture, dinoTexture);
    glPopMatrix();

    // tail part2
    glPushMatrix();
    glTranslatef(2, 0, 0.5);
    glRotatef(90, 0, 0, 1);
    glTranslatef(1.45, -0.7, 0);
    glRotatef(-20, 0, 0, 1);
    doorPilePyramid(1, 0.06, 0.03, dinoTexture, dinoTexture);
    glPopMatrix();

    // neck part1
    glPushMatrix();
    glRotatef(-150, 0, 0, 1);
    glTranslatef(-0.5, -3.5, 0.5);
    doorPilePyramid(1.5, 0.5, 0.3, dinoTexture, dinoTexture);
    glPopMatrix();

    // head
    glPushMatrix();
    glTranslatef(-2, 3.2, 0.5);
    glRotatef(-90, 0, 0, 1);
    doorPilePyramid(1, 0.3, 0.15, dinoTexture, dinoTexture);
    glPopMatrix();

    // hand1
    glPushMatrix();
    glTranslatef(-1, 2, 0);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.25, 0.25);
    glTranslatef(-0.5, 0, 0);
    glRotatef(90, 0, 0, 1);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.8, 0.125);
    glPopMatrix();

    // hand2
    glPushMatrix();
    glTranslatef(-1, 2, 1);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.25, 0.25);
    glTranslatef(-0.5, 0, 0);
    glRotatef(90, 0, 0, 1);
    drawCubeWithTexture(dinoTexture, dinoTexture, 0.8, 0.125);
    glPopMatrix();

    // foot 1
    drawFoot(-0.2, 0.05, 0);

    // foot 2
    drawFoot(-0.2, 0.05, 1);

    // left eye

    glPushMatrix();
    glTranslatef(-1.5, 3.25, 0.3);
    drawEye();
    glPopMatrix();

    // right eye
    glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(-1.5, 3.25, 0.7);
    drawEye();
    glPopMatrix();

    glPopMatrix();
}

// Draw dino 2
void drawDino2()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, dino2Texture);

    // Update animation angles
    updateAnimation();

    glPushMatrix();

    float maxY = -999999.0f;
    float minY = 999999.0f;

    // First find the Y range of the model
    for (const Face &face : faces)
    {
        for (const Vertex &vertex : face.vertices)
        {
            maxY = std::max(maxY, vertex.y);
            minY = std::min(minY, vertex.y);
        }
    }

    float range = maxY - minY;
    float topBodyThreshold = maxY - (range * 0.25f);    // Top 25% body
    float bottomBodyThreshold = minY + (range * 0.25f); // Bottom 25% body

    glBegin(GL_QUADS);
    for (const Face &face : faces)
    {
        // Calculate average Y position for this face
        float avgY = 0.0f;
        for (const Vertex &vertex : face.vertices)
        {
            avgY += vertex.y;
        }
        avgY /= face.vertices.size();

        // Calculate face normal
        if (face.vertices.size() >= 3)
        {
            Vertex v1 = face.vertices[0];
            Vertex v2 = face.vertices[1];
            Vertex v3 = face.vertices[2];

            float ux = v2.x - v1.x;
            float uy = v2.y - v1.y;
            float uz = v2.z - v1.z;

            float vx = v3.x - v1.x;
            float vy = v3.y - v1.y;
            float vz = v3.z - v1.z;

            float nx = uy * vz - uz * vy;
            float ny = uz * vx - ux * vz;
            float nz = ux * vy - uy * vx;

            float length = sqrt(nx * nx + ny * ny + nz * nz);
            if (length > 0)
            {
                nx /= length;
                ny /= length;
                nz /= length;
            }
            glNormal3f(nx, ny, nz);
        }

        // Apply different transformations based on part
        for (const Vertex &vertex : face.vertices)
        {
            float transformedX = vertex.x;
            float transformedY = vertex.y;
            float transformedZ = vertex.z;

            if (vertex.y > topBodyThreshold)
            {
                // Head movement
                float angle = headAngle * (vertex.y - topBodyThreshold) / (maxY - topBodyThreshold);
                float radians = angle * M_PI / 180.0f;
                float newX = transformedX * cos(radians) - transformedZ * sin(radians);
                float newZ = transformedX * sin(radians) + transformedZ * cos(radians);
                transformedX = newX;
                transformedZ = newZ;
            }
            else if (vertex.y < bottomBodyThreshold)
            {
                // Tail movement
                float angle = tailAngle * (bottomBodyThreshold - vertex.y) / (bottomBodyThreshold - minY);
                float radians = angle * M_PI / 180.0f;
                float newX = transformedX * cos(radians) - transformedZ * sin(radians);
                float newZ = transformedX * sin(radians) + transformedZ * cos(radians);
                transformedX = newX;
                transformedZ = newZ;
            }

            glTexCoord2f(vertex.texU, vertex.texV);
            glVertex3f(transformedX, transformedY, transformedZ);
        }
    }
    glEnd();

    glPopMatrix();

    glDisable(GL_TEXTURE_2D);
}

void drawHorn()
{
    glPushMatrix();
    glColor3f(1, 1, 1);
    drawPyramid(0.15, 0.35);
    glPopMatrix();
}

void drawNails()
{

    glPushMatrix();
    glPushMatrix();
    glTranslatef(0.1, 0.02, 0.1);
    glRotatef(90, 1, 0, 0);
    glScalef(0.5, 0.5, 0.5);
    drawHorn();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0.02, 0.1);
    glRotatef(90, 1, 0, 0);
    glScalef(0.5, 0.5, 0.5);
    drawHorn();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.1, 0.02, 0.1);
    glRotatef(90, 1, 0, 0);
    glScalef(0.5, 0.5, 0.5);
    drawHorn();
    glPopMatrix();

    glPopMatrix();
}

// draw dino 3 leg
void drawLeg3()
{
    rhinoTexture = loadTexture("rhino.png");
    glColor3f(0.51, 0.77, 0.45);

    glPushMatrix();
    glTranslatef(0.18, -1, 0.5);
    doorPilePyramid(0.7, 0.13, 0.08, rhinoTexture, rhinoTexture);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.18, -1, 0.5);
    doorPilePyramid(0.3, 0.08, 0.16, rhinoTexture, rhinoTexture);

    glPushMatrix();
    drawNails();
    glPopMatrix();

    glPopMatrix();
}

void drawThorns()
{
    glPushMatrix();
    glScalef(1, 0.3, 1);
    drawHorn();
    glTranslatef(0, 0.09, -0.2);
    drawHorn();
    glTranslatef(0, 0.09, -0.2);
    drawHorn();
    glTranslatef(0, 0.09, -0.2);
    drawHorn();
    glTranslatef(0, 0.09, -0.2);
    drawHorn();
    glTranslatef(0, -0.14, -0.2);
    drawHorn();
    glTranslatef(0, -0.18, -0.2);
    drawHorn();
    glScalef(0.8, 0.8, 0.8);
    glTranslatef(0, -0.18, -0.2);
    drawHorn();
    glTranslatef(0, -0.18, -0.2);
    drawHorn();
    glScalef(0.8, 0.8, 0.8);
    glTranslatef(0, -0.18, -0.2);
    drawHorn();
    glTranslatef(0, -0.35, -0.2);
    drawHorn();
    glTranslatef(0, -0.4, -0.2);
    drawHorn();
    glPopMatrix();
}

void drawDino3()
{
    GLUquadric *quad = gluNewQuadric();
    rhinoTexture = loadTexture("rhino.png");
    glColor3f(0.51, 0.77, 0.45);
    glPushMatrix();

    // head ring
    glPushMatrix();
    glScalef(1.1, 1.1, 1.1);
    glScalef(0.7, 1, 1);
    glTranslatef(0, 0.2, 1);
    glEnable(GL_TEXTURE_2D);
    gluQuadricTexture(quad, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, rhinoTexture);
    gluCylinder(quad, 0.8, 0.8, 0.15, 10, 10);
    gluDisk(quad, 0, 0.8, 10, 10);
    glTranslatef(0, 0, 0.15);
    gluDisk(quad, 0, 0.8, 10, 10);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    // head part 1
    glPushMatrix();
    glTranslatef(0, 0, 1.4);
    glEnable(GL_TEXTURE_2D);
    gluQuadricTexture(quad, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, rhinoTexture);
    gluSphere(quad, 0.5, 5, 5);
    glPopMatrix();

    // head part 2
    glPushMatrix();
    glTranslatef(0, 0, 1.8);
    glEnable(GL_TEXTURE_2D);
    gluQuadricTexture(quad, GL_TRUE);
    glBindTexture(GL_TEXTURE_2D, rhinoTexture);
    gluSphere(quad, 0.25, 5, 5);
    glDisable(GL_TEXTURE_2D);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0.17, 0.25, 1.6);
    glRotatef(-30, 0, 0, 1);
    drawHorn();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.17, 0.25, 1.6);
    glRotatef(30, 0, 0, 1);
    drawHorn();
    glPopMatrix();

    // white beak
    glPushMatrix();
    glTranslatef(0, 0, 2);
    glColor3f(1, 1, 1);
    gluCylinder(quad, 0.15, 0.1, 0.15, 6, 6);
    glTranslatef(0, 0, 0.12);
    gluDisk(quad, 0, 0.1, 6, 6);
    glRotatef(30, 1, 0, 0);
    gluCylinder(quad, 0.1, 0.01, 0.17, 6, 6);
    glPopMatrix();
    gluDeleteQuadric(quad);

    // BODY FROM HERES
    glColor3f(0.51, 0.77, 0.45);
    glScalef(1.2, 1.2, 1.2);

    // body part1
    glPushMatrix();
    glRotatef(90, 1, 0, 0);
    doorPilePyramid(1, 0.25, 0.45, rhinoTexture, rhinoTexture);
    glPopMatrix();

    // body part2
    glPushMatrix();
    glRotatef(-90, 1, 0, 0);
    doorPilePyramid(1, 0.18, 0.45, rhinoTexture, rhinoTexture);
    glPopMatrix();

    // tail part1
    glPushMatrix();
    glTranslatef(0, 0, -0.95);
    glRotatef(-110, 1, 0, 0);
    doorPilePyramid(0.5, 0.1, 0.18, rhinoTexture, rhinoTexture);
    glPopMatrix();

    // tail part2
    glPushMatrix();
    glTranslatef(0, -0.17, -1.4);
    glRotatef(-100, 1, 0, 0);
    doorPilePyramid(0.4, 0.02, 0.1, rhinoTexture, rhinoTexture);
    glPopMatrix();

    // back thorns
    glPushMatrix();
    glTranslatef(0, 0.3, 0.7);
    drawThorns();
    glPopMatrix();

    // legs
    glPushMatrix();
    drawLeg3();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.4, 0, 0);
    drawLeg3();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-0.4, 0, -0.5);
    drawLeg3();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(0, 0, -0.5);
    drawLeg3();
    glPopMatrix();

    // Eyes
    glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(0.2, 0.1, 1.5);
    glRotatef(-60, 0, 1, 0);
    glRotatef(90, 0, 0, 1);
    drawEye();
    glPopMatrix();

    glPushMatrix();
    glColor3f(1, 1, 1);
    glTranslatef(-0.2, 0.1, 1.5);
    glRotatef(60, 0, 1, 0);
    glRotatef(-90, 0, 0, 1);
    drawEye();
    glPopMatrix();

    glPopMatrix();
}

// Draw Gate methods
void banner()
{
    woodTexture = loadTexture("wood.png");
    glPushMatrix();
    glScalef(1, 1, 0.15);
    doorPilePyramid(0.3, 0.8, 0.8, woodTexture, woodTexture);

    glPushMatrix();
    logoTexture = loadTexture("logo.png");
    glTranslatef(0, 0.15, 0.5);
    glScalef(0.8, 0.9, 1.2);
    drawCubeWithTexture(woodTexture, logoTexture, 0.3, 0.8);
    glPopMatrix();

    glPopMatrix();
}

void doorWing()
{
    doorTexture = loadTexture("door.png");
    glPushMatrix();
    glRotatef(gateAngle, 0, 1, 0);
    glTranslatef(0.4, 0, 0);
    glScalef(1, 1, 0.1);
    doorPilePyramid(1.25, 0.4, 0.4, doorTexture, doorTexture);
    glPopMatrix();
}

void gateFrameTower()
{
    woodTexture = loadTexture("wood.png");
    glPushMatrix();
    glTranslatef(-0.2, 0, 0);
    glRotatef(-5.5, 0, 0, 1);
    doorPilePyramid(2.1, 0.125, 0.25, woodTexture, woodTexture);
    glPopMatrix();
}

void gateFrame()
{

    glPushMatrix();
    gateFrameTower();
    glTranslatef(1.6, 0, 0);
    glRotatef(180, 0, 1, 0);
    gateFrameTower();
    glPopMatrix();
}

void gate(float scale = 1.0f)
{

    glPushMatrix();
    glScalef(scale, scale, scale);
    // banner
    glPushMatrix();
    glTranslatef(0.8, 1.7, 0);
    banner();
    glPopMatrix();
    // FRAME
    gateFrame();
    // DOORS
    glPushMatrix();
    doorWing();
    glTranslatef(1.6, 0, 0);
    glRotatef(180, 0, 1, 0);
    glScalef(1, 1, -1);
    doorWing();
    glPopMatrix();

    glPopMatrix();
}

//------------Draw Scene---------------------------------
void drawScene()
{
    glPushMatrix();
    drawFloor();
    glPopMatrix();

    drawForest();
    glPushMatrix();
    drawFence();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-19, 0, -1.5);
    glRotatef(270, 0, 1, 0);
    gate(1.8);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(5, 0, 2);
    drawDino(1);
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-10.0, 1.7, -10.0);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(180, 0.0, 0.0, 1.0);
    glScalef(1, 1, 1);
    glColor3f(1.0, 1.0, 1.0);
    drawDino2();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(-10, 1.2, 2);
    drawDino3();
    glPopMatrix();

    glColor3f(1, 1, 1);
}

void display(void)
{
    float scalef = 4;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // setLightingAndShading();

    glPushMatrix();

    // camera orientation (eyeX, eyeY, eyeZ, centerX, centerY, centerZ, upX, upY, upZ)
    gluLookAt(0.0, 2.0 + camY, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
    // move the scene (all the rendered environment) using keyboard keys
    glTranslatef(sceTX, sceTY, sceTZ);
    glRotatef(sceRY, 0.0, 1.0, 0.0);

    drawScene();

    if (showAxes)
        drawAxes();
    if (showGrid)
        drawGrid();
    glPopMatrix();
    glutSwapBuffers();
}

void reshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Define the Perspective projection frustum
    //  (FOV_in_vertical, aspect_ratio, z-distance to the near plane from the camera position, z-distance to far plane from the camera position)
    gluPerspective(120.0, aspect_ratio, 0.01, 500.0);
}

void mouseMotion(int x, int y)
{
    int dx = x - lastMouseX;
    int dy = y - lastMouseY;

    if (leftButtonDown)
    {
        // Rotate the scene
        sceRY += dx * 0.5f;
        sceRX += dy * 0.5f;
    }
    else if (rightButtonDown)
    {
        // Pan the scene
        sceTX += dx * 0.01f;
        sceTY -= dy * 0.01f;
    }

    lastMouseX = x;
    lastMouseY = y;

    glutPostRedisplay();
}

void mouseButton(int button, int state, int x, int y)
{
    if (button == GLUT_LEFT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            leftButtonDown = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP)
        {
            leftButtonDown = false;
        }
    }
    else if (button == GLUT_RIGHT_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            rightButtonDown = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP)
        {
            rightButtonDown = false;
        }
    }
    else if (button == GLUT_MIDDLE_BUTTON)
    {
        if (state == GLUT_DOWN)
        {
            middleButtonDown = true;
            lastMouseX = x;
            lastMouseY = y;
        }
        else if (state == GLUT_UP)
        {
            middleButtonDown = false;
        }
    }
    else if (button == 3)
    { // Scroll up
        camZ -= 0.5;
        glutPostRedisplay();
    }
    else if (button == 4)
    { // Scroll down
        camZ += 0.5;
        glutPostRedisplay();
    }
}

void keyboardSpecial(int key, int x, int y)
{
    if (key == GLUT_KEY_UP)
        camY += 0.2;

    if (key == GLUT_KEY_DOWN)
        camY -= 0.2;

    if (key == GLUT_KEY_LEFT)
        sceRY += 2;

    if (key == GLUT_KEY_RIGHT)
        sceRY -= 2;

    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 27:     // 27 is the ASCII code for the Esc key
        exit(0); // Exit the program
        break;
    case 'l':
        objRY += 1;
        break;
    case 'r':
        objRY -= 1;
        break;
    case 'Z':
        sceTZ += 1;
        break;
    case 'z':
        sceTZ -= 1;
        break;
    case 'a':
        sceTX += 1;
        objTX -= 1;
        break;
    case 'd':
        sceTX -= 1;
        objTX += 1;
        break;
    case 'w':
        sceTZ += 0.5;
        objTZ -= 0.5;
        break;
    case 's':
        sceTZ -= 1;
        objTZ += 1;
        break;
    case 'q':
        sceTY += 0.5;
        objTY -= 0.5;
        break;
    case 'e':
        sceTY -= 0.5;
        objTY += 0.5;
        break;
    case '.':
        lookX = 0;
        lookY = 0;
        lookZ = 0;
        camX = camXInit;
        camY = camYInit;
        camZ = camZInit;
        sceRX = sceRXInit;
        sceRY = sceRYInit;
        sceRZ = sceRZInit;
        sceTX = sceTXInit;
        sceTY = sceTYInit;
        sceTZ = sceTZInit;
        objRX = 0;
        objRY = 0;
        objRZ = 0;
        objTX = 0;
        objTY = 0;
        objTZ = 0;
        break;
    case '1':
        glEnable(GL_LIGHT0);
        break;
    case '3':
        glDisable(GL_LIGHT0);
        break;
    case '!':
        glDisable(GL_LIGHT0); // Light at -x
        break;
    case '@':
        glDisable(GL_LIGHT1); // Light at +x
        break;
    case '2':
        glEnable(GL_LIGHT1);
        break;
    case 'g':
        showGrid = !showGrid; // Toggle grid visibility
        break;
    case 'x':
        showAxes = !showAxes; // Toggle axes visibility
        break;
    case 'o':
        gateAngle += gateAngle >= 80 ? 0 : 1;
        break;
    case 'O':
        gateAngle -= gateAngle <= 0 ? 0 : 1;
        break;
    }
    glutPostRedisplay();
}

void idle()
{
    glutPostRedisplay();
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(1920, 1080);  // Set the window size to the resolution of the second display
    glutInitWindowPosition(1920, 0); // Set the window position to the second display (assuming the first display is 1920x1080)
    glutCreateWindow("Low poly jurrasic park");
    glutFullScreen(); // Set the window to fullscreen mode
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardSpecial);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutIdleFunc(idle);          // Set the idle function
    glutMotionFunc(mouseMotion); // Set the mouse motion function
    glutMouseFunc(mouseButton);  // Set the mouse button function
    init();
    glutMainLoop();
    return 0;
}