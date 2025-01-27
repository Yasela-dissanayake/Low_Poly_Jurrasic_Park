#include <GL/glut.h>
#include <math.h>
#include <stdio.h>
#include <SOIL/SOIL.h>
#include <cstdlib>
#include <ctime>
#include <tuple>

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
    glClearColor(0.4, 0.4, 0.4, 1.0);
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

// void drawCube(float width, float height, float depth)
// {
//     float halfWidth = width / 2.0f;
//     float halfHeight = height / 2.0f;
//     float halfDepth = depth / 2.0f;

//     glBegin(GL_QUADS);

//     // Front face
//     glVertex3f(-halfWidth, -halfHeight, halfDepth);
//     glVertex3f(halfWidth, -halfHeight, halfDepth);
//     glVertex3f(halfWidth, halfHeight, halfDepth);
//     glVertex3f(-halfWidth, halfHeight, halfDepth);

//     // Back face
//     glVertex3f(-halfWidth, -halfHeight, -halfDepth);
//     glVertex3f(-halfWidth, halfHeight, -halfDepth);
//     glVertex3f(halfWidth, halfHeight, -halfDepth);
//     glVertex3f(halfWidth, -halfHeight, -halfDepth);

//     // Left face
//     glVertex3f(-halfWidth, -halfHeight, -halfDepth);
//     glVertex3f(-halfWidth, -halfHeight, halfDepth);
//     glVertex3f(-halfWidth, halfHeight, halfDepth);
//     glVertex3f(-halfWidth, halfHeight, -halfDepth);

//     // Right face
//     glVertex3f(halfWidth, -halfHeight, -halfDepth);
//     glVertex3f(halfWidth, halfHeight, -halfDepth);
//     glVertex3f(halfWidth, halfHeight, halfDepth);
//     glVertex3f(halfWidth, -halfHeight, halfDepth);

//     // Top face
//     glVertex3f(-halfWidth, halfHeight, -halfDepth);
//     glVertex3f(-halfWidth, halfHeight, halfDepth);
//     glVertex3f(halfWidth, halfHeight, halfDepth);
//     glVertex3f(halfWidth, halfHeight, -halfDepth);

//     // Bottom face
//     glVertex3f(-halfWidth, -halfHeight, -halfDepth);
//     glVertex3f(halfWidth, -halfHeight, -halfDepth);
//     glVertex3f(halfWidth, -halfHeight, halfDepth);
//     glVertex3f(-halfWidth, -halfHeight, halfDepth);

//     glEnd();
// }

void drawCubeWithTexture(GLuint texture, GLuint texture_front, float height, float width)
{
    glEnable(GL_TEXTURE_2D);
    glColor3f(1, 1, 1);
    glPushMatrix();

    float halfHeight = height / 2.0f;
    float halfWidth = width / 2.0f;

    // Front Face
    glBindTexture(GL_TEXTURE_2D, texture);
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

    glBegin(GL_TRIANGLES);
    // Front face
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(-halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, halfBase);

    // Right face
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, -halfBase);

    // Back face
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(halfBase, 0.0f, -halfBase);
    glVertex3f(-halfBase, 0.0f, -halfBase);

    // Left face
    glVertex3f(0.0f, height, 0.0f);
    glVertex3f(-halfBase, 0.0f, -halfBase);
    glVertex3f(-halfBase, 0.0f, halfBase);
    glEnd();

    // Base of the pyramid
    glBegin(GL_QUADS);
    glVertex3f(-halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, halfBase);
    glVertex3f(halfBase, 0.0f, -halfBase);
    glVertex3f(-halfBase, 0.0f, -halfBase);
    glEnd();
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

//-------------------Floor with Texture-------------------
void drawFloor()
{

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
}

//-------------------Draw Tree Trunk-------------------
void drawTreeTrunk()
{

    // glColor3f(0.49019f, 0.36078f, 0.23921f);
    glColor3f(0.5f, 0.35f, 0.05f);
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
    srand(42);
    // srand(static_cast<unsigned int>(time(0))); // Use current time as seed for randomness

    for (int i = 0; i < 40; ++i)
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
void doorPilePyramid(GLuint texture, float height, float half_top_width, float half_bottom_width)
{
    glPushMatrix();
    glBegin(GL_QUADS);
    // Front Face
    glBindTexture(GL_TEXTURE_2D, texture);
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
    glBindTexture(GL_TEXTURE_2D, texture);
    normal = calculateNormal(
        {-half_bottom_width, 0.0f, -half_bottom_width},
        {half_bottom_width, 0.0f, -half_bottom_width},
        {half_top_width, height, -half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));

    glVertex3f(-half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glTexCoord2f(1.0f, 1.0f);
    glVertex3f(half_top_width, height, -half_top_width);
    glTexCoord2f(0.0f, 1.0f);
    glVertex3f(-half_top_width, height, -half_top_width);

    // Top Face
    glBindTexture(GL_TEXTURE_2D, texture);
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
    glBindTexture(GL_TEXTURE_2D, texture);
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
    glBindTexture(GL_TEXTURE_2D, texture);
    normal = calculateNormal(
        {half_bottom_width, 0.0f, -half_bottom_width},
        {half_top_width, height, -half_top_width},
        {half_top_width, height, half_top_width});
    glNormal3f(std::get<0>(normal), std::get<1>(normal), std::get<2>(normal));
    glVertex3f(half_bottom_width, 0.0f, -half_bottom_width);
    glVertex3f(half_top_width, height, -half_top_width);
    glVertex3f(half_top_width, height, half_top_width);
    glVertex3f(half_bottom_width, 0.0f, half_bottom_width);

    // Left Face
    glBindTexture(GL_TEXTURE_2D, texture);
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
    glPopMatrix();
}

//-------------------Door pile-------------------
void drawDoorPile(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)
{
    glPushMatrix();
    woodTexture = loadTexture("wood.png");
    glColor3f(0.6f, 0.3f, 0.0f); // Brown color
    glScalef(1, scale, 1);
    doorPilePyramid(woodTexture, 2, 0.10, 0.4);
    glPopMatrix();
}

//-------------------Draw Door-------------------
void drawDoor(float x = 0.0f, float y = 0.0f, float z = 0.0f, float scale = 1.0f)
{
    woodTexture = loadTexture("wood.png");
    logoTexture = loadTexture("logo.png");
    doorTexture = loadTexture("door.png");
    doorrTexture = loadTexture("doorr.png");

    glPushMatrix();
    glTranslatef(x, y, z);
    glScalef(scale, scale, scale);

    // Pile 1
    glPushMatrix();
    glRotatef(15, 1, 0, 0);
    drawDoorPile(0, 0, 0, 1.8);
    glPopMatrix();

    // Pile 2
    glPushMatrix();
    glTranslatef(0, 0, 3.5);
    glRotatef(-15, 1, 0, 0);
    drawDoorPile(0, 0, 0, 1.8);
    glPopMatrix();

    // Logo banner
    glPushMatrix();
    glScalef(1, 0.3, 4.5);
    glTranslatef(0, 10, 0.4);
    drawCubeWithTexture(woodTexture, logoTexture, 2, 0.5);
    glPopMatrix();

    // Left Door
    glPushMatrix();
    glScalef(0.1, 3, 1);
    glTranslatef(0, 0.4, 1.1);
    drawCubeWithTexture(woodTexture, doorTexture, 0.8, 1.25);
    glPopMatrix();

    // Right Door
    glPushMatrix();
    glScalef(0.1, 3, 1);
    glTranslatef(0, 0.4, 2.4);
    drawCubeWithTexture(woodTexture, doorTexture, 0.8, 1.25);
    glPopMatrix();

    glPopMatrix();
}

//------------Draw Scene---------------------------------
void drawScene()
{
    drawFloor();

    drawForest();

    drawDoor(-19, 0, 0, 1);

    glColor3f(1, 1, 1);
    // drawUndergroundStation();
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