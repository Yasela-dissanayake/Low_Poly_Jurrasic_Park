#include <GL/glut.h>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>
#include <SOIL/SOIL.h>
#include <cmath>

GLfloat camX = 0.0;
GLfloat camY = 0.0;
GLfloat camZ = 0.0;

GLfloat sceRX = 0.0;
GLfloat sceRY = 0.0;
GLfloat sceRZ = 0.0;
GLfloat sceTX = 0.0;
GLfloat sceTY = 0.0;
GLfloat sceTZ = 0.0;

////variables to move the objects
GLfloat objRX = 0.0;
GLfloat objRY = 0.0;
GLfloat objRZ = 0.0;
GLfloat objTX = 0.0;
GLfloat objTY = 0.0;
GLfloat objTZ = 0.0;

// Lighjting
GLfloat light1_pos[] = {-10.0, 1.0, -0.5, 1.0}; // First light position
GLfloat light2_pos[] = {10.0, 1.0, -0.5, 1.0};  // Second light position

float tailAngle = 0.0f;
float headAngle = 0.0f;
float animationSpeed = 2.0f; // Controls how fast the animation moves

// Global variables for textures
GLuint floorTexture;
GLuint dinoTexture;

// Structure to hold vertex data
struct Vertex
{
    float x, y, z;
    float texU, texV; // Added texture coordinates
    Vertex(float _x = 0, float _y = 0, float _z = 0, float _u = 0, float _v = 0)
        : x(_x), y(_y), z(_z), texU(_u), texV(_v) {}
};

// Structure to hold face data
struct Face
{
    std::vector<Vertex> vertices;
};

// Vector to store all faces
std::vector<Face> faces;

// Vertex data for the leaf faces
GLfloat face1[4][3] = {
    {-0.512071, -1.0, 0.254202},
    {0.512071, -1.0, 0.254202},
    {1.0, -0.009334, 0.487656},
    {-1.0, -0.009334, 0.487656}};

GLfloat face2[4][3] = {
    {-1.0, -0.009334, 0.487656},
    {1.0, -0.009334, 0.487656},
    {0.748522, 1.011341, 0.242636},
    {-0.748522, 1.011341, 0.242636}};

GLfloat face3[4][3] = {
    {-0.748522, 1.011341, 0.242636},
    {0.748522, 1.011341, 0.242636},
    {0.16307, 1.725813, 0.0},
    {-0.185751, 1.725813, 0.0}};

GLfloat face4[4][3] = {
    {0.512071, -1.0, 0.254202},
    {-0.512071, -1.0, 0.254202},
    {-0.058127, -1.601064, 0.0},
    {0.058127, -1.601064, 0.0}};

GLfloat face5[4][3] = {
    {0.058127, -1.601064, 0.0},
    {-0.058127, -1.601064, 0.0},
    {-0.114407, -2.417604, 0.0},
    {0.114407, -2.417604, 0.0}};

void setLightingAndShading()
{
    glEnable(GL_LIGHTING);

    // First Light Source (GL_LIGHT0) - White light
    GLfloat l0amb[] = {0.2, 0.2, 0.2, 1.0};
    GLfloat l0diff[] = {0.8, 0.8, 0.8, 1.0};
    GLfloat l0spec[] = {0.2, 0.2, 0.2, 1.0};

    glLightfv(GL_LIGHT0, GL_AMBIENT, l0amb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, l0diff);
    glLightfv(GL_LIGHT0, GL_SPECULAR, l0spec);
    glLightfv(GL_LIGHT0, GL_POSITION, light1_pos);

    // Second Light Source (GL_LIGHT1) - Yellow tinted light
    GLfloat l1amb[] = {0.2, 0.2, 0.0, 1.0};  // Yellow ambient
    GLfloat l1diff[] = {1.0, 1.0, 0.0, 1.0}; // Yellow diffuse
    GLfloat l1spec[] = {0.2, 0.2, 0.0, 1.0}; // Yellow specular

    glLightfv(GL_LIGHT1, GL_AMBIENT, l1amb);
    glLightfv(GL_LIGHT1, GL_DIFFUSE, l1diff);
    glLightfv(GL_LIGHT1, GL_SPECULAR, l1spec);
    glLightfv(GL_LIGHT1, GL_POSITION, light2_pos);

    // Material properties
    glEnable(GL_COLOR_MATERIAL);
    glShadeModel(GL_SMOOTH);
    glColorMaterial(GL_FRONT, GL_AMBIENT_AND_DIFFUSE);

    GLfloat specRef[] = {0.7, 0.7, 0.7, 1.0};
    glMaterialfv(GL_FRONT, GL_SPECULAR, specRef);
    glMateriali(GL_FRONT, GL_SHININESS, 128);
}

GLuint loadTexture(const char *filename)
{
    GLuint texture = SOIL_load_OGL_texture(
        filename,
        SOIL_LOAD_AUTO,
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

// Function to load faces from file
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

// Draw  floor with a texture
void drawFloor()
{
    glEnable(GL_TEXTURE_2D);
    // Reset color to white before drawing the textured floor
    glColor3f(1.0f, 1.0f, 1.0f);

    // Bind the texture to the floor
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    glBegin(GL_QUADS);
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-20.0, -0.4, 20.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-20.0, -0.4, -20.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(20.0, -0.4, -20.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(20.0, -0.4, 20.0);
    glEnd();

    glDisable(GL_TEXTURE_2D);
}

//  animation angles update
void updateAnimation()
{
    // body swing
    tailAngle = 15.0f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.001f * animationSpeed);

    // body swing 2
    headAngle = 10.0f * sin(glutGet(GLUT_ELAPSED_TIME) * 0.0007f * animationSpeed);
}

// Modified drawDino function to include animations
void drawDino()
{
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, dinoTexture);

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

void drawFencePost()
{
    glPushMatrix();
    glColor3f(0.5f, 0.35f, 0.05f);  // Brown color
    glTranslatef(0.0f, 2.1f, 0.0f); // Adjust to align with the floor level
    glScalef(0.1f, 5.0f, 0.1f);     // Scale to make it a thin vertical post, height increased 5 times
    glutSolidCube(1.0f);
    glPopMatrix();
}

// Draw a single horizontal plank
void drawPlank()
{
    glPushMatrix();
    glColor3f(0.65f, 0.5f, 0.35f);  // Light brown color
    glTranslatef(0.0f, 2.1f, 0.0f); // Adjust to align with the floor level
    glScalef(1.0f, 0.1f, 0.1f);     // Scale to make it a horizontal plank
    glutSolidCube(1.0f);
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

void drawTreeType1()
{
    // Draw the trunk
    glColor3f(0.5f, 0.35f, 0.05f);
    glPushMatrix();
    glTranslatef(0.0f, -0.42f, 5.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.5, 0.3, 3.0, 5, 20);
    glPopMatrix();

    // Draw the tree top
    glColor3f(0, 1, 0);
    glPushMatrix();
    glTranslatef(0.0f, 3.0f, 5.0f);
    glScalef(1.0f, 1.5f, 1.0f);
    glutSolidIcosahedron();
    glPopMatrix();

    // Reset color to white after drawing the tree
    glColor3f(1.0f, 1.0f, 1.0f);
}

void drawLeaf()
{
    glEnable(GL_TEXTURE_2D);
    // glBindTexture(GL_TEXTURE_2D, leafTexture);
    glColor3f(0.0f, 1.0f, 0.0f);
    for (int i = 0; i < 20; i++)
    { // Changed from 10 to 20
        glPushMatrix();
        // Distribute leaves in a more spherical pattern
        float angle1 = (float)i * -18.0f;             // Adjusted angle to spread 20 leaves more evenly
        float angle2 = (i % 2 == 0) ? 15.0f : -15.0f; // Vertical variation
        glTranslatef(0.0f, 2.0f, 3.0f);               // Base position
        // Rotate to distribute leaves
        glRotatef(angle1, 0, 1, 0); // Horizontal rotation
        // glRotatef(angle2, 1, 0, 0); // Vertical tilt
        // Move leaf outward
        glTranslatef(0.0f, 0.0f, 0.5f);
        // Scale and orient leaf
        glScalef(0.3f, 0.3f, 0.3f);
        glRotatef(-270, 1, 0, 0);
        // Draw the leaf
        glBegin(GL_QUADS);
        // Face 1
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(face1[0]);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(face1[1]);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(face1[2]);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(face1[3]);
        // Face 2
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(face2[0]);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(face2[1]);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(face2[2]);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(face2[3]);
        // Face 3
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(face3[0]);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(face3[1]);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(face3[2]);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(face3[3]);
        // Face 4
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(face4[0]);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(face4[1]);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(face4[2]);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(face4[3]);
        // Face 5
        glTexCoord2f(0.0f, 0.0f);
        glVertex3fv(face5[0]);
        glTexCoord2f(1.0f, 0.0f);
        glVertex3fv(face5[1]);
        glTexCoord2f(1.0f, 1.0f);
        glVertex3fv(face5[2]);
        glTexCoord2f(0.0f, 1.0f);
        glVertex3fv(face5[3]);
        glEnd();
        glPopMatrix();
    }
    glDisable(GL_TEXTURE_2D);
}

// Function to draw the complete tree
void drawTreeType2()
{
    // Draw the trunk
    glColor3f(0.5f, 0.35f, 0.05f);
    glPushMatrix();
    glTranslatef(5.0f, -0.42f, 5.0f);
    glRotatef(-90, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.7, 0.5, 1.5, 5, 20);
    glTranslatef(0, 0, 1.5);
    gluCylinder(gluNewQuadric(), 0.5, 0.3, 1.5, 5, 20);
    glTranslatef(0, 0, 1.5);
    gluCylinder(gluNewQuadric(), 0.3, 0.01, 1.5, 5, 20);
    glTranslatef(-0.1, -0.1, 0);
    glRotatef(30, 1.0f, 0.0f, 0.0f);
    gluCylinder(gluNewQuadric(), 0.2, 0.01, 1, 5, 20);
    glRotatef(30, 0.0f, 1.0f, 0.0f);
    glTranslatef(0, 0.4, 0.3);
    gluCylinder(gluNewQuadric(), 0.1, 0.01, 1, 5, 20);

    glPushMatrix();
    glRotatef(90, 1.0f, 0.0f, 0.0f);
    glTranslatef(0, -1.1, -2.8);
    drawLeaf();
    glPopMatrix();

    glPushMatrix();
    glRotatef(20, 1.0f, 0.0f, 0.0f);
    glTranslatef(-0.5, -1.0, -2);
    glScalef(0.8, 0.8, 0.8);
    drawLeaf();
    glPopMatrix();

    glPopMatrix();
}

void drawFence()
{
    float fenceLength = 40.0f; // Length of one side of the floor
    int numSections = 40;      // Number of fence sections per side
    float sectionSpacing = fenceLength / numSections;

    // Draw front side
    for (int i = 0; i <= numSections; i++)
    {
        glPushMatrix();
        glTranslatef(-20.0f + i * sectionSpacing, -0.4f, 20.0f); // Adjust to align with the floor level
        drawFenceSection();
        glPopMatrix();
    }

    // Draw back side
    for (int i = 0; i <= numSections; i++)
    {
        glPushMatrix();
        glTranslatef(-20.0f + i * sectionSpacing, -0.4f, -20.0f); // Adjust to align with the floor level
        drawFenceSection();
        glPopMatrix();
    }

    // Draw left side
    for (int i = 0; i <= numSections; i++)
    {
        glPushMatrix();
        glTranslatef(-20.0f, -0.4f, -20.0f + i * sectionSpacing); // Adjust to align with the floor level
        glRotatef(90, 0.0f, 1.0f, 0.0f);
        drawFenceSection();
        glPopMatrix();
    }

    // Draw right side
    for (int i = 0; i <= numSections; i++)
    {
        glPushMatrix();
        glTranslatef(20.0f, -0.4f, -20.0f + i * sectionSpacing); // Adjust to align with the floor level
        glRotatef(90, 0.0f, 1.0f, 0.0f);
        drawFenceSection();
        glPopMatrix();
    }
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setLightingAndShading();

    glPushMatrix();
    gluLookAt(0.0, 1.0 + camY, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glTranslatef(sceTX, sceTY, sceTZ);
    glRotatef(sceRY, 0.0, 1.0, 0.0);

    drawFloor();

    drawFence();

    drawTreeType1();

    drawTreeType2();

    // Draw the dino
    glPushMatrix();
    glTranslatef(objTX, objTY + 1.5, objTZ);
    glRotatef(objRY, 0.0, 1.0, 0.0);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);
    drawDino();
    glPopMatrix();

    glPushMatrix();
    glTranslatef(10.0, -1.0, 10.0);
    glTranslatef(objTX, objTY + 1.5, objTZ);
    glRotatef(objRY, 0.0, 1.0, 0.0);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glRotatef(180, 0.0, 0.0, 1.0);
    glScalef(0.5, 0.5, 0.5);
    glColor3f(1.0, 1.0, 1.0);
    drawDino();
    glPopMatrix();

    // draw other objects( tree and fence)
    glPopMatrix();
    glutSwapBuffers();

    glutPostRedisplay();
}

void init(void)
{
    glClearColor(0.0, 0.0, 0.0, 1.0);
    glClearDepth(1.0);

    // Enable necessary features
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_LIGHTING);
    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHT1);
    glEnable(GL_TEXTURE_2D);
    glEnable(GL_COLOR_MATERIAL);

    // Load textures
    floorTexture = loadTexture("ground.png");
    dinoTexture = loadTexture("dino-skin3.jpg");

    // Verify texture loading
    if (!floorTexture || !dinoTexture)
    {
        printf("Error loading textures!\n");
    }

    // Load the faces from file
    loadFacesFromFile("dino1.txt");
}

void keyboard(unsigned char key, int x, int y)
{
    // ifs can be replaced with switch...case

    if (key == 'l')
        objRY += 1;

    if (key == 'r')
        objRY -= 1;

    if (key == 'Z')
        sceTZ += 0.2;

    if (key == 'z')
        sceTZ -= 0.2;

    if (key == 'w')
        sceTX += 1;

    if (key == 's')
        sceTX -= 1;

    if (key == 'y')
        sceRY += 1;

    if (key == 'Y')
        sceRY -= 1;

    if (key == 'k')
        objRX += 1;

    if (key == 'K')
        objRX -= 1;

    if (key == '1') // toggle the light 1
        if (glIsEnabled(GL_LIGHT0))
            glDisable(GL_LIGHT0);
        else
            glEnable(GL_LIGHT0);
    if (key == '2') // toggle light 2
        if (glIsEnabled(GL_LIGHT1))
            glDisable(GL_LIGHT1);
        else
            glEnable(GL_LIGHT1);

    // control light position
    if (key == 'q')
        light1_pos[0] -= 1.0;
    if (key == 'Q')
        light1_pos[0] += 1.0;
    if (key == 'e')
        light2_pos[0] -= 1.0;
    if (key == 'E')
        light2_pos[0] += 1.0;

    glutPostRedisplay();
}

void keyboardSpecial(int key, int x, int y)
{
    if (key == GLUT_KEY_UP)
        camY += 1;

    if (key == GLUT_KEY_DOWN)
        camY -= 1;

    if (key == GLUT_KEY_RIGHT)
        sceTX += 1;

    if (key == GLUT_KEY_LEFT)
        sceTX -= 1;

    glutPostRedisplay();
}

void reshape(GLsizei w, GLsizei h)
{
    glViewport(0, 0, w, h);
    GLfloat aspect_ratio = h == 0 ? w / 1 : (GLfloat)w / (GLfloat)h;

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // Define the Perspective projection frustum
    //  (FOV_in_vertical, aspect_ratio, z-distance to the near plane from the camera position, z-distance to far plane from the camera position)
    gluPerspective(120.0, aspect_ratio, 1.0, 100.0);
    glMatrixMode(GL_MODELVIEW);
}

int main(int argc, char **argv)
{
    glutInit(&argc, argv); // Initialize GLUT
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
    glutInitWindowSize(400, 400);
    glutInitWindowPosition(0, 0);
    glutCreateWindow("Low poly jurrasic park");
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(keyboardSpecial);
    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    init();
    glutMainLoop();
    return 0;
}
