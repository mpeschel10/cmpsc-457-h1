#include <cstdio>
#include <cmath>
#include <chrono>
#include <vector>
#include <unistd.h>

#include "GL/glut.h"

using namespace std;

void glLine(std::vector<std::vector<float>> points, int p1, int p2)
{
    glBegin(GL_LINES);
    glVertex3f(points[0][p1], points[1][p1], points[2][p1]);
    glVertex3f(points[0][p2], points[1][p2], points[2][p2]);
    glEnd();
}

// Calculated in python, the points are:
// -0.5 -0.4330127018922193 -0.375
//  0.5 -0.4330127018922193 -0.375
//  0.0  0.4330127018922193 -0.375
//  0.0  0.0                 0.375

std::vector<std::vector<float>> points = {
    {-0.5, -0.4330127018922193, -0.375},
    { 0.5, -0.4330127018922193, -0.375},
    { 0.0,  0.4330127018922193, -0.375},
    { 0.0,  0.0,                 0.375},
};

void showMatrix(std::vector<std::vector<float>> matrix)
{
    for (std::vector<float> row : matrix)
    {
        for (float value : row)
        {
            std::printf("% f ", value);
        }
        std::printf("\n");
    }
}

vector<vector<float>> makeRotationX(float xRot)
{
    return {
        { 1,         0,          0},
        { 0, cos(xRot), -sin(xRot)},
        { 0, sin(xRot),  cos(xRot)},
    };
}

vector<vector<float>> makeRotationY(float yRot)
{
    return {
        {cos(yRot), 0, -sin(yRot)},
        {        0, 1,          0},
        {sin(yRot), 0,  cos(yRot)},
    };
}

vector<vector<float>> makeRotationZ(float zRot)
{
    return {
        { cos(zRot), sin(zRot), 0},
        {-sin(zRot), cos(zRot), 0},
        {         0,         0, 1},
    };
}

float matrixPartialDot(std::vector<std::vector<float>> A, int r, std::vector<std::vector<float>> B, int c)
{
    float total = 0;
    for (int i = 0; i < B.size(); i++)
    {
        total += A[r][i] * B[i][c];
    }
    return total;
}

std::vector<std::vector<float>> matrixMultiply(std::vector<std::vector<float>> A, std::vector<std::vector<float>> B)
{
    const int BRows = B.size();
    const int BColumns = B[0].size();
    const int ARows = A.size();
    if (BRows != ARows)
    {
        printf(
            "ERROR: Cannot multiply %ix%i and %ix%i matrices: inner dimension must match.\n",
            A[0].size(), ARows, BRows, BColumns
        );
        throw "ball";
    }
    std::vector<std::vector<float>> result;
    for (int d = 0; d < ARows; d++)
    {
        result.push_back(std::vector<float>(BColumns));
    }

    for (int result_r = 0; result_r < ARows; result_r++)
    {
        for (int result_c = 0; result_c < BColumns; result_c++)
        {
            result[result_r][result_c] = matrixPartialDot(A, result_r, B, result_c);
        }
    }

    return result;
}

std::vector<std::vector<float>> makeRotateTransform3D(float xRot, float yRot, float zRot)
{
    vector<vector<float>> matrix = matrixMultiply(matrixMultiply(
            makeRotationX(xRot), makeRotationY(yRot)), makeRotationZ(zRot)
    );
    // printf("Made rotate transform matrix:\n");
    // showMatrix(matrix);
    return matrix;
}

std::vector<std::vector<float>> transpose(std::vector<std::vector<float>> matrix)
{
    int oldRows = matrix.size(); int oldColumns = matrix[0].size();
    int newRows = oldColumns; int newColumns = oldRows;
    std::vector<std::vector<float>> result = std::vector<std::vector<float>>(newRows);
    for (int r = 0; r < newRows; r++)
    {
        result[r].resize(newColumns);
        for (int c = 0; c < newColumns; c++)
        {
            result[r][c] = matrix[c][r];
        }
    }
    return result;
}


float xRot, yRot, zRot = 0;
void display()
{
    std::vector<std::vector<float>> transform = makeRotateTransform3D(xRot, yRot, zRot);
    std::vector<std::vector<float>> displayPoints = matrixMultiply(transform, points);
    
    glClear(GL_COLOR_BUFFER_BIT);
    int pointc = displayPoints[0].size();
    for (int i = 0; i < pointc; i++)
        for (int j = i + 1; j < pointc; j++)
            glLine(displayPoints, i, j);
    glFlush();
}

bool xRotUpdate, yRotUpdate, zRotUpdate = false;
const float PI = 3.1415926535;
const float spinRate = PI; // radians per second, so half a circle every second
std::chrono::steady_clock::time_point lastFrame;
void onIdle(void)
{
    chrono::steady_clock::time_point currentFrame = chrono::steady_clock::now();
    std::chrono::nanoseconds frameSpanNS = currentFrame - lastFrame;
    float deltaTime = (float) frameSpanNS.count() / 1000000000.0;
    
    // printf("xRotUpdate, yRotUpdate, zRotUpdate: %d %d %n", xRotUpdate, yRotUpdate, zRotUpdate);
    if (xRotUpdate)
    {
        xRot += deltaTime * spinRate;
        if (xRot > 2 * PI) xRot -= 2 * PI;
    }
    if (yRotUpdate)
    {
        yRot += deltaTime * spinRate;
        if (yRot > 2 * PI) yRot -= 2 * PI;
    }
    if (zRotUpdate)
    {
        zRot += deltaTime * spinRate;
        if (zRot > 2 * PI) zRot -= 2 * PI;
    }

    if (xRotUpdate || yRotUpdate || zRotUpdate)
        glutPostRedisplay();
    lastFrame = currentFrame;
    usleep(10000);
}

void onKeyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
        case 'x':
            xRotUpdate = !xRotUpdate;
            break;
        case 'y':
            yRotUpdate = !yRotUpdate;
            break;
        case 'z':
            zRotUpdate = !zRotUpdate;
            break;
        default:
            break;
    }
}

void init(void)
{
    glLineWidth(3);
}

void onReshape(int w, int h)
{
    glViewport(0, 0, w, h);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    float fractionWidth = (float) w / 250.0;
    float fractionHeight = (float) h / 250.0;
    glOrtho(-fractionWidth, fractionWidth, -fractionHeight, fractionHeight, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
}

int main(int argc, char** argv)
{
    points = transpose(points);
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_SINGLE | GLUT_RGBA);
    glutInitWindowSize(250, 250);
    glutCreateWindow("Tetrahedron: Press x, y, z");
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(onKeyboard);
    glutReshapeFunc(onReshape);
    glutIdleFunc(onIdle);
    glutMainLoop();
    return 0;
}