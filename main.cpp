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

// Global variables for textures
GLuint floorTexture;
GLuint dinoTexture;

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
    // This is a simple mapping - you might want to adjust this based on your needs
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

// Draw a floor with a texture
void drawFloor()
{
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
}

// Draw the dino model with a texture
void drawDino()
{
    // Bind the texture to the dino
    glBindTexture(GL_TEXTURE_2D, dinoTexture);

    glBegin(GL_QUADS);
    for (const Face &face : faces)
    {
        // Calculate face normal for lighting
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

        // Draw vertices with texture coordinates
        for (const Vertex &vertex : face.vertices)
        {
            glTexCoord2f(vertex.texU, vertex.texV);
            glVertex3f(vertex.x, vertex.y, vertex.z);
        }
    }
    glEnd();
}

void display(void)
{
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    setLightingAndShading();

    glPushMatrix();
    gluLookAt(0.0, 1.0 + camY, 5.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);

    glTranslatef(sceTX, sceTY, sceTZ);
    glRotatef(sceRY, 0.0, 1.0, 0.0);

    // Draw the floor first
    glPushMatrix();
    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, floorTexture);

    // Set material properties for the floor
    GLfloat floorMaterial[] = {1.0, 1.0, 1.0, 1.0};
    glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, floorMaterial);

    glBegin(GL_QUADS);
    glNormal3f(0.0, 1.0, 0.0); // Normal pointing up for proper lighting
    glTexCoord2f(0.0, 0.0);
    glVertex3f(-20.0, -0.4, 20.0);
    glTexCoord2f(0.0, 1.0);
    glVertex3f(-20.0, -0.4, -20.0);
    glTexCoord2f(1.0, 1.0);
    glVertex3f(20.0, -0.4, -20.0);
    glTexCoord2f(1.0, 0.0);
    glVertex3f(20.0, -0.4, 20.0);
    glEnd();
    glPopMatrix();

    // Draw the dino
    glPushMatrix();
    glTranslatef(objTX, objTY + 1.5, objTZ);
    glRotatef(objRY, 0.0, 1.0, 0.0);
    glRotatef(-90, 1.0, 0.0, 0.0);
    glColor3f(1.0, 1.0, 1.0);
    drawDino();
    glPopMatrix();

    glPopMatrix();
    glutSwapBuffers();
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
    dinoTexture = loadTexture("dino-texture2.jpg");

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