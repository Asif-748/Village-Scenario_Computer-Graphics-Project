#ifdef _WIN32
#include <windows.h>
#endif
#include <GL/glut.h>
#include <cmath>
#include <iostream>
#include <algorithm>
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
using namespace std;

bool isDayMode    = true;
bool isBoatMoving = true;
bool isBoatManual = false;
bool paddleKeyActive = false;
bool isPaused     = false;
float animSpeed   = 1.0f;

float cloudX1 = 100.0f;
float cloudX2 = 550.0f;
float cloudX3 = 950.0f;

float birdX1 = 50.0f;
float birdX2 = -200.0f;
float birdX3 = -400.0f;
float birdWingAngle = 0.0f;
bool  wingUp = true;

float boatX       = 200.0f;
float boatSpeed   = 1.2f;
int   boatDirection = 1;
float paddleAngle = 0.0f;
bool  paddleForward = true;

float riverWaveOffset = 0.0f;

void drawDDALine(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1, dy = y2 - y1;
    float steps =max(abs(dx),abs(dy));
    if (steps == 0) return;
    float xInc = dx / steps, yInc = dy / steps, x = x1, y = y1;
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    for (int i = 0; i <= (int)steps; i++)
    {
        glVertex2f(x, y);
        x += xInc;
        y += yInc;
    }
    glEnd();
}

void drawBresenhamLine(int x1, int y1, int x2, int y2)
{
    int dx = abs(x2 - x1), dy = abs(y2 - y1);
    int sx = (x1 < x2) ? 1 : -1, sy = (y1 < y2) ? 1 : -1;
    int err = dx - dy;
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    while (true)
    {
        glVertex2f((float)x1, (float)y1);
        if (x1 == x2 && y1 == y2) break;
        int e2 = 2 * err;
        if (e2 > -dy)
        {
            err -= dy;
            x1 += sx;
        }
        if (e2 <  dx)
        {
            err += dx;
            y1 += sy;
        }
    }
    glEnd();
}

void plot8CirclePoints(float cx, float cy, float x, float y)
{
    glVertex2f(cx + x, cy + y);
    glVertex2f(cx - x, cy + y);
    glVertex2f(cx + x, cy - y);
    glVertex2f(cx - x, cy - y);
    glVertex2f(cx + y, cy + x);
    glVertex2f(cx - y, cy + x);
    glVertex2f(cx + y, cy - x);
    glVertex2f(cx - y, cy - x);
}

void drawMidpointCircle(float cx, float cy, float r)
{
    float x = 0, y = r, p = 1.0f - r;
    glPointSize(2.0f);
    glBegin(GL_POINTS);
    plot8CirclePoints(cx, cy, x, y);
    while (x < y)
    {
        x++;
        if (p < 0) p += 2 * x + 1;
        else
        {
            y--;
            p += 2 * (x - y) + 1;
        }
        plot8CirclePoints(cx, cy, x, y);
    }
    glEnd();
}

void drawFilledMidpointCircle(float cx, float cy, float r)
{
    for (float i = r; i > 0; i -= 0.5f)
        drawMidpointCircle(cx, cy, i);
}

void drawSolidCircle(float cx, float cy, float r, int segments = 60)
{
    glBegin(GL_TRIANGLE_FAN);
    glVertex2f(cx, cy);
    for (int i = 0; i <= segments; i++)
    {
        float angle = i * (2.0f * M_PI / segments);
        glVertex2f(cx + cos(angle) * r, cy + sin(angle) * r);
    }
    glEnd();
}

//blue in day, dark blue at night with stars
void drawSky()
{
    glBegin(GL_QUADS);
    if (isDayMode)
    {
        glColor3f(0.40f, 0.72f, 0.95f);
        glVertex2f(0, 800);
        glColor3f(0.40f, 0.72f, 0.95f);
        glVertex2f(1200, 800);
        glColor3f(0.75f, 0.90f, 0.98f);
        glVertex2f(1200, 400);
        glColor3f(0.75f, 0.90f, 0.98f);
        glVertex2f(0, 400);
    }
    else
    {
        glColor3f(0.02f, 0.04f, 0.15f);
        glVertex2f(0, 800);
        glColor3f(0.02f, 0.04f, 0.15f);
        glVertex2f(1200, 800);
        glColor3f(0.08f, 0.12f, 0.30f);
        glVertex2f(1200, 400);
        glColor3f(0.08f, 0.12f, 0.30f);
        glVertex2f(0, 400);
    }
    glEnd();

    //Stars appear at night
    if (!isDayMode)
    {
        glColor3f(1.0f, 1.0f, 1.0f);
        glPointSize(2.0f);
        glBegin(GL_POINTS);
        glVertex2f(80,750);
        glVertex2f(220,720);
        glVertex2f(350,770);
        glVertex2f(480,710);
        glVertex2f(620,760);
        glVertex2f(750,730);
        glVertex2f(890,780);
        glVertex2f(1020,740);
        glVertex2f(1130,765);
        glVertex2f(300,650);
        glVertex2f(550,630);
        glVertex2f(820,660);
        glVertex2f(970,620);
        glVertex2f(140,600);
        glEnd();
    }
}

//SUN only in day mode
void drawSun()
{
    if (!isDayMode) return;
    float cx = 1050.0f, cy = 700.0f, r = 45.0f;

    //12 rays around the sun
    glColor3f(1.0f, 0.85f, 0.2f);
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    for (int i = 0; i < 12; i++)
    {
        float angle = i * (2.0f * M_PI / 12.0f);
        glVertex2f(cx + cos(angle) * (r + 5),  cy + sin(angle) * (r + 5));
        glVertex2f(cx + cos(angle) * (r + 22), cy + sin(angle) * (r + 22));
    }
    glEnd();

    //the sun circle
    glColor3f(1.0f, 0.85f, 0.0f);
    drawSolidCircle(cx, cy, r);
}

void drawMoon()
{
    if (isDayMode) return;
    float cx = 180.0f, cy = 700.0f, r = 40.0f;

    glColor3f(0.92f, 0.94f, 0.98f);
    drawSolidCircle(cx, cy, r);

    glColor3f(0.75f, 0.82f, 0.92f);
    drawMidpointCircle(cx, cy, r);

    glColor3f(0.03f, 0.06f, 0.20f);
    drawSolidCircle(cx + 16.0f, cy + 8.0f, r - 4.0f);
}

void drawSingleCloud(float x, float y)
{
    if (isDayMode) glColor3f(0.98f, 0.98f, 1.0f);
    else           glColor3f(0.25f, 0.28f, 0.42f);

    //cloud made of several overlapping circles
    drawSolidCircle(x,      y,    25.0f);
    drawSolidCircle(x + 25, y + 10, 30.0f);
    drawSolidCircle(x + 55, y + 5,  26.0f);
    drawSolidCircle(x + 80, y,    22.0f);
    drawSolidCircle(x + 35, y - 10, 25.0f);
}

void drawCloud()
{
    drawSingleCloud(cloudX1, 680.0f);
    drawSingleCloud(cloudX2, 720.0f);
    drawSingleCloud(cloudX3, 650.0f);
}

//HILLS
void drawDistantHills()
{
    glBegin(GL_TRIANGLES);

    //Back hills
    if (isDayMode) glColor3f(0.14f, 0.40f, 0.18f);
    else glColor3f(0.04f, 0.14f, 0.06f);
    glVertex2f(-50, 360);
    glVertex2f(220, 500);
    glVertex2f(480, 360);

    if (isDayMode) glColor3f(0.10f, 0.35f, 0.15f);
    else glColor3f(0.03f, 0.11f, 0.05f);
    glVertex2f(350, 360);
    glVertex2f(650, 530);
    glVertex2f(950, 360);

    if (isDayMode) glColor3f(0.16f, 0.42f, 0.20f);
    else glColor3f(0.05f, 0.15f, 0.07f);
    glVertex2f(800, 360);
    glVertex2f(1060, 495);
    glVertex2f(1250, 360);

    //Front hills
    if (isDayMode) glColor3f(0.22f, 0.52f, 0.25f);
    else glColor3f(0.07f, 0.18f, 0.09f);
    glVertex2f(80, 360);
    glVertex2f(380, 455);
    glVertex2f(680, 360);

    if (isDayMode) glColor3f(0.25f, 0.55f, 0.28f);
    else glColor3f(0.08f, 0.20f, 0.10f);
    glVertex2f(580, 360);
    glVertex2f(880, 445);
    glVertex2f(1180, 360);

    glEnd();
}

void drawRiceCropTuft(float x, float y, float r, float g, float b)
{
    if (isDayMode) glColor3f(r, g, b);
    else           glColor3f(r * 0.35f, g * 0.35f, b * 0.35f);
    glLineWidth(1.6f);
    glBegin(GL_LINES);
    glVertex2f(x, y);
    glVertex2f(x - 2.5f, y + 14);
    glVertex2f(x, y);
    glVertex2f(x,        y + 16);
    glVertex2f(x, y);
    glVertex2f(x + 2.5f, y + 14);
    glEnd();
}

void drawCropField()
{
    //Brown soil base
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.48f, 0.36f, 0.22f);
    else glColor3f(0.16f, 0.12f, 0.07f);
    glVertex2f(0, 360);
    glVertex2f(1200, 360);
    glVertex2f(1200, 270);
    glVertex2f(0, 270);
    glEnd();

    //Dividing lines between the 4 fields
    if (isDayMode) glColor3f(0.38f, 0.27f, 0.15f);
    else glColor3f(0.12f, 0.09f, 0.05f);
    drawDDALine(290, 355, 290, 275);
    drawDDALine(590, 355, 590, 275);
    drawDDALine(890, 355, 890, 275);

    //Golden rice in each field
    for (float rx = 35;  rx <= 275;  rx += 14)
    {
        drawRiceCropTuft(rx, 280, 0.95f, 0.82f, 0.05f);
        drawRiceCropTuft(rx + 6, 310, 0.92f, 0.78f, 0.04f);
        drawRiceCropTuft(rx + 3, 340, 0.98f, 0.85f, 0.06f);
    }
    for (float rx = 310; rx <= 578;  rx += 14)
    {
        drawRiceCropTuft(rx, 280, 0.90f, 0.75f, 0.05f);
        drawRiceCropTuft(rx + 6, 310, 0.95f, 0.80f, 0.04f);
        drawRiceCropTuft(rx + 3, 340, 0.92f, 0.78f, 0.05f);
    }
    for (float rx = 608; rx <= 878;  rx += 14)
    {
        drawRiceCropTuft(rx, 280, 0.96f, 0.83f, 0.06f);
        drawRiceCropTuft(rx + 6, 310, 0.93f, 0.80f, 0.05f);
        drawRiceCropTuft(rx + 3, 340, 0.90f, 0.76f, 0.04f);
    }
    for (float rx = 910; rx <= 1168; rx += 14)
    {
        drawRiceCropTuft(rx, 280, 0.94f, 0.81f, 0.05f);
        drawRiceCropTuft(rx + 6, 310, 0.98f, 0.85f, 0.06f);
        drawRiceCropTuft(rx + 3, 340, 0.91f, 0.77f, 0.04f);
    }
}

void drawRoad()
{
    //Green grass strip crop field and river
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.52f, 0.72f, 0.24f);
    else glColor3f(0.14f, 0.24f, 0.09f);
    glVertex2f(0, 270);
    glVertex2f(1200, 270);
    glVertex2f(1200, 155);
    glVertex2f(0, 155);
    glEnd();

    //road polygon
    const float rx[] = {0, 250, 500, 850, 1200, 1200, 850, 500, 250, 0};
    const float ry[] = {218, 234, 208, 244, 218,  155, 155, 155, 155, 155};
    glBegin(GL_POLYGON);
    if (isDayMode) glColor3f(0.74f, 0.58f, 0.40f);
    else glColor3f(0.26f, 0.19f, 0.13f);
    for (int i = 0; i < 10; i++) glVertex2f(rx[i], ry[i]);
    glEnd();
}

//RIVER
void drawRiver()
{
    //River rectangle
    glBegin(GL_POLYGON);
    if (isDayMode) glColor3f(0.15f, 0.45f, 0.85f);
    else glColor3f(0.05f, 0.12f, 0.30f);
    glVertex2f(0, 155);
    glVertex2f(1200, 155);
    glVertex2f(1200, 0);
    glVertex2f(0, 0);
    glEnd();

    //Wave using sine curve, offset each frame for animation (Translation)
    if (isDayMode) glColor3f(0.45f, 0.72f, 0.96f);
    else glColor3f(0.12f, 0.25f, 0.48f);
    glLineWidth(2.0f);
    for (float baseY = 20; baseY <= 130; baseY += 28)
    {
        glBegin(GL_LINE_STRIP);
        for (float x = 0; x <= 1200; x += 15)
            glVertex2f(x, baseY + sin((x + riverWaveOffset) * 0.018f) * 7.0f);
        glEnd();
    }
}

//HOUSE
void drawSingleHouse(float roofR, float roofG, float roofB)
{
    //wall
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.82f, 0.70f, 0.55f);
    else glColor3f(0.28f, 0.24f, 0.18f);
    glVertex2f(0, 0);
    glVertex2f(120, 0);
    glVertex2f(120, 70);
    glVertex2f(0, 70);
    glEnd();

    //roof
    glBegin(GL_POLYGON);
    if (isDayMode) glColor3f(roofR, roofG, roofB);
    else glColor3f(roofR * 0.35f, roofG * 0.35f, roofB * 0.35f);
    glVertex2f(-15, 70);
    glVertex2f(135, 70);
    glVertex2f(110, 115);
    glVertex2f(10, 115);
    glEnd();

    //Roof linesDDA Algorithm
    if (isDayMode) glColor3f(roofR * 0.8f, roofG * 0.8f, roofB * 0.8f);
    else glColor3f(0.1f, 0.1f, 0.1f);
    drawDDALine(-15, 70, 135, 70);
    drawDDALine(10, 115, 110, 115);

    //door
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.40f, 0.22f, 0.10f);
    else glColor3f(0.14f, 0.08f, 0.04f);
    glVertex2f(45, 0);
    glVertex2f(75, 0);
    glVertex2f(75, 50);
    glVertex2f(45, 50);
    glEnd();
    drawDDALine(45, 0, 45, 50);
    drawDDALine(75, 0, 75, 50);
    drawDDALine(45, 50, 75, 50);

    // Window
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.35f, 0.20f, 0.08f);
    else glColor3f(0.12f, 0.06f, 0.02f);
    glVertex2f(85, 25);
    glVertex2f(105, 25);
    glVertex2f(105, 45);
    glVertex2f(85, 45);
    glEnd();
    if (isDayMode) glColor3f(0.8f, 0.8f, 0.8f);
    else glColor3f(0.3f, 0.3f, 0.3f);
    drawDDALine(95, 25, 95, 45);
    drawDDALine(85, 35, 105, 35);

    //Bamboo posts and bars using DDA
    if (isDayMode) glColor3f(0.50f, 0.35f, 0.15f);
    else glColor3f(0.18f, 0.12f, 0.05f);
    for (float fx = -50; fx <= -10; fx += 10) drawDDALine(fx, 0, fx, 30);
    drawDDALine(-52, 8,  -8, 8);
    drawDDALine(-52, 22, -8, 22);
}

void drawHouse()
{
    //5 houses Translation (glTranslatef) and Scaling (glScalef)
    glPushMatrix();
    glTranslatef(80,   250, 0);
    glScalef(0.95f, 0.95f, 1);
    drawSingleHouse(0.78f, 0.25f, 0.15f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(290,  255, 0);
    glScalef(0.85f, 0.85f, 1);
    drawSingleHouse(0.88f, 0.68f, 0.18f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(540,  245, 0);
    glScalef(1.05f, 1.05f, 1);
    drawSingleHouse(0.30f, 0.50f, 0.72f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(810,  260, 0);
    glScalef(0.90f, 0.90f, 1);
    drawSingleHouse(0.70f, 0.35f, 0.15f);
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1030, 250, 0);
    glScalef(0.85f, 0.85f, 1);
    drawSingleHouse(0.25f, 0.60f, 0.35f);
    glPopMatrix();
}

//TREE
void drawSingleTree()
{
    //Brown trunk
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.38f, 0.22f, 0.08f);
    else glColor3f(0.12f, 0.07f, 0.03f);
    glVertex2f(-12, 0);
    glVertex2f(12, 0);
    glVertex2f(8, 100);
    glVertex2f(-8, 100);
    glEnd();

    //overlapping solid circles
    if (isDayMode) glColor3f(0.02f, 0.12f, 0.04f
);
    else glColor3f(0.02f, 0.12f, 0.04f);
    drawSolidCircle(0, 130, 45);
    if (isDayMode) glColor3f(0.10f, 0.48f, 0.14f);
    else glColor3f(0.03f, 0.16f, 0.05f);
    drawSolidCircle(-30, 115, 35);
    drawSolidCircle(30, 115, 35);
    if (isDayMode) glColor3f(0.14f, 0.55f, 0.18f);
    else glColor3f(0.05f, 0.18f, 0.06f);
    drawSolidCircle(0, 155, 32);
}

void drawTree()
{
    //behind trees
    float bgData[][2] = {{5, 70}, {195, 265}, {445, 515}, {715, 785}, {1130, 1200}};
    for (auto& d : bgData)
        for (float rx = d[0]; rx <= d[1]; rx += 14)
        {
            drawRiceCropTuft(rx,     280, 0.95f, 0.82f, 0.05f);
            drawRiceCropTuft(rx + 6, 310, 0.92f, 0.78f, 0.04f);
            drawRiceCropTuft(rx + 3, 340, 0.98f, 0.85f, 0.06f);
        }

    //5 trees
    glPushMatrix();
    glTranslatef(40,   260, 0);
    glScalef(1.20f, 1.20f, 1);
    drawSingleTree();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(230,  265, 0);
    glScalef(0.90f, 0.90f, 1);
    drawSingleTree();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(480,  270, 0);
    glScalef(1.05f, 1.05f, 1);
    drawSingleTree();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(750,  265, 0);
    glScalef(0.85f, 0.85f, 1);
    drawSingleTree();
    glPopMatrix();
    glPushMatrix();
    glTranslatef(1170, 255, 0);
    glScalef(1.15f, 1.15f, 1);
    drawSingleTree();
    glPopMatrix();
}

//BOAT
void drawBoat()
{
    glPushMatrix();

    if (!isDayMode)
    {
        //night boat is parked
        glTranslatef(920.0f, 130.0f, 0);
    }
    else
    {
        glTranslatef(boatX, 60.0f, 0);
        if (boatDirection < 0) glScalef(-1, 1, 1);
    }

    //Boat hull
    glBegin(GL_POLYGON);
    if (isDayMode) glColor3f(0.30f, 0.18f, 0.08f);
    else glColor3f(0.10f, 0.06f, 0.02f);
    glVertex2f(-70, 25);
    glVertex2f(-50, 0);
    glVertex2f(50, 0);
    glVertex2f(70, 25);
    glVertex2f(55, 20);
    glVertex2f(-55, 20);
    glEnd();

    //Boat rim (DDA line)
    if (isDayMode) glColor3f(0.50f, 0.30f, 0.12f);
    else glColor3f(0.18f, 0.10f, 0.04f);
    drawDDALine(-70, 25, 70, 25);

    //Bamboo canopy shelter
    glBegin(GL_POLYGON);
    if (isDayMode) glColor3f(0.70f, 0.55f, 0.20f);
    else glColor3f(0.22f, 0.18f, 0.06f);
    glVertex2f(-10, 25);
    glVertex2f(-5, 55);
    glVertex2f(20, 55);
    glVertex2f(25, 25);
    glEnd();
    if (isDayMode) glColor3f(0.40f, 0.28f, 0.10f);
    else glColor3f(0.12f, 0.08f, 0.03f);
    drawDDALine(-5, 55, 20, 55);
    drawDDALine(0,  25, -2, 55);
    drawDDALine(10, 25,  9, 55);
    drawDDALine(20, 25, 19, 55);
    //Boatman is only drawn during day
    if (isDayMode)
    {
        glBegin(GL_QUADS);
        glColor3f(0.85f, 0.25f, 0.20f);
        glVertex2f(-52, 25);
        glVertex2f(-38, 25);
        glVertex2f(-38, 47);
        glVertex2f(-52, 47);
        glEnd();

        //Lungi
        glBegin(GL_QUADS);
        glColor3f(0.10f, 0.48f, 0.35f);
        glVertex2f(-53, 20);
        glVertex2f(-37, 20);
        glVertex2f(-38, 25);
        glVertex2f(-52, 25);
        glEnd();

        //Head
        glColor3f(0.95f, 0.92f, 0.86f);
        drawFilledMidpointCircle(-45, 54, 7);

        //hat
        glBegin(GL_TRIANGLES);
        glColor3f(0.85f, 0.75f, 0.30f);
        glVertex2f(-58, 57);
        glVertex2f(-32, 57);
        glVertex2f(-45, 69);
        glEnd();

        //Paddle
        glPushMatrix();
        glTranslatef(-40, 28, 0);
        glRotatef(paddleAngle, 0, 0, 1);
        glColor3f(0.45f, 0.28f, 0.12f);
        glLineWidth(3.0f);
        glBegin(GL_LINES);
        glVertex2f(0, 15);
        glVertex2f(-8, -25);
        glEnd();
        //lower part
        glBegin(GL_TRIANGLES);
        glVertex2f(-8, -25);
        glVertex2f(-14, -38);
        glVertex2f(-2, -38);
        glEnd();
        glPopMatrix();
    }
    glPopMatrix();
    //STICK
    if (!isDayMode)
    {
        //Verticalstick
        glColor3f(0.12f, 0.07f, 0.03f);
        glLineWidth(4.0f);
        glBegin(GL_LINES);
        glVertex2f(1005.0f, 155.0f);
        glVertex2f(1005.0f, 195.0f);
        glEnd();

        //Straight line rope
        glColor3f(0.40f, 0.28f, 0.12f);
        glLineWidth(2.0f);
        glBegin(GL_LINES);
        glVertex2f(990.0f, 155.0f);
        glVertex2f(1005.0f, 168.0f);
        glEnd();
    }
}

//BIRD
//Translation (position) and Scaling (size)
void drawSingleBird(float x, float y, float scale)
{
    glPushMatrix();
    glTranslatef(x, y, 0);
    glScalef(scale, scale, 1);

    glColor3f(0.08f, 0.08f, 0.12f);
    glLineWidth(1.8f);

    float R = 13.0f;                       // wing span radius (smaller: was 18)
    float flapY = birdWingAngle * 0.14f;   // flap height (smaller: was 0.18)

    // Left wing arc
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 10; i++)
    {
        float t = (float)i / 10.0f;
        glVertex2f(-R * t, (flapY + 6) * sin(M_PI * t));  // amplitude +6 (was +8)
    }
    glEnd();

    // Right wing arc (mirror of left)
    glBegin(GL_LINE_STRIP);
    for (int i = 0; i <= 10; i++)
    {
        float t = (float)i / 10.0f;
        glVertex2f(R * t, (flapY + 6) * sin(M_PI * t));
    }
    glEnd();

    drawFilledMidpointCircle(0, 0, 2);
    glPopMatrix();
}

//BIRDS flying across sky in day mode
void drawBird()
{
    if (!isDayMode) return;

    drawSingleBird(birdX1,       680, 1.00f);
    drawSingleBird(birdX1 + 45,  710, 0.85f);
    drawSingleBird(birdX1 - 50,  660, 0.90f);
    drawSingleBird(birdX2,       620, 1.10f);
    drawSingleBird(birdX2 + 60,  645, 0.80f);
    drawSingleBird(birdX3,       730, 0.95f);
}

//CHILD FIGURE
void drawChildFigure(float shirtR, float shirtG, float shirtB,
                     float shortsR, float shortsG, float shortsB)
{
    float skinR = 0.95f, skinG = 0.92f, skinB = 0.86f;

    // Head
    glColor3f(skinR, skinG, skinB);
    drawFilledMidpointCircle(0.0f, 38.0f, 6.5f);

    // Neck
    glLineWidth(3.5f);
    glColor3f(skinR, skinG, skinB);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 31.0f);
    glVertex2f(0.0f, 28.0f);
    glEnd();

    //Shirt
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(shirtR, shirtG, shirtB);
    else           glColor3f(shirtR * 0.35f, shirtG * 0.35f, shirtB * 0.35f);
    glVertex2f(-8.0f, 14.0f);
    glVertex2f(8.0f, 14.0f);
    glVertex2f(7.0f,  28.0f);
    glVertex2f(-7.0f, 28.0f);
    glEnd();

    //Shorts
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(shortsR, shortsG, shortsB);
    else           glColor3f(shortsR * 0.35f, shortsG * 0.35f, shortsB * 0.35f);
    glVertex2f(-7.0f, 4.0f);
    glVertex2f(7.0f, 4.0f);
    glVertex2f(6.0f,  14.0f);
    glVertex2f(-6.0f, 14.0f);
    glEnd();

    // Legs
    glLineWidth(3.0f);
    glColor3f(skinR, skinG, skinB);
    glBegin(GL_LINES);
    glVertex2f(-3.5f, 4.0f);
    glVertex2f(-4.0f, 0.0f);
    glVertex2f(3.5f,  4.0f);
    glVertex2f(4.0f,  0.0f);
    glEnd();

    // Arms
    glLineWidth(2.5f);
    glColor3f(skinR, skinG, skinB);
    glBegin(GL_LINES);
    glVertex2f(-7.0f, 24.0f);
    glVertex2f(-13.0f, 14.0f);
    glVertex2f(7.0f,  24.0f);
    glVertex2f(13.0f,  14.0f);
    glEnd();
}

//PEOPLE
void drawPeople()
{
    if (!isDayMode) return;  //no people night

    float sR = 0.95f, sG = 0.92f, sB = 0.86f;  //skin color

    //VILLAGER 1
    glPushMatrix();
    glTranslatef(280.0f, 200.0f, 0.0f);

    glColor3f(sR, sG, sB);
    drawFilledMidpointCircle(0.0f, 60.0f, 9.0f);  //head

    glLineWidth(4.0f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 51.0f);
    glVertex2f(0.0f, 46.0f);  //neck
    glEnd();

    //Shirt
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.92f, 0.55f, 0.10f);
    else glColor3f(0.30f, 0.18f, 0.03f);
    glVertex2f(-11.0f, 24.0f);
    glVertex2f(11.0f, 24.0f);
    glVertex2f(10.0f,  46.0f);
    glVertex2f(-10.0f, 46.0f);
    glEnd();

    //Lungi
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.08f, 0.50f, 0.36f);
    else glColor3f(0.02f, 0.16f, 0.12f);
    glVertex2f(-11.0f, 4.0f);
    glVertex2f(11.0f, 4.0f);
    glVertex2f(10.0f,  24.0f);
    glVertex2f(-10.0f, 24.0f);
    glEnd();

    glLineWidth(3.5f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(-5.0f, 4.0f);
    glVertex2f(-5.0f, 0.0f);  //legs
    glVertex2f(5.0f,  4.0f);
    glVertex2f(5.0f,  0.0f);
    glEnd();

    glLineWidth(3.0f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(-10.0f, 42.0f);
    glVertex2f(-16.0f, 26.0f);  //arms
    glVertex2f(10.0f,  42.0f);
    glVertex2f(17.0f,  30.0f);
    glEnd();

    glColor3f(0.40f, 0.25f, 0.10f);
    drawDDALine(17.0f, 0.0f, 17.0f, 72.0f);  //stick
    glPopMatrix();

    //HAWKER
    glPushMatrix();
    glTranslatef(490.0f, 200.0f, 0.0f);

    glColor3f(sR, sG, sB);
    drawFilledMidpointCircle(0.0f, 60.0f, 9.0f);  //head

    //hat
    glBegin(GL_TRIANGLES);
    glColor3f(0.82f, 0.72f, 0.28f);
    glVertex2f(-14.0f, 64.0f);
    glVertex2f(14.0f, 64.0f);
    glVertex2f(0.0f, 76.0f);
    glEnd();
    glColor3f(0.60f, 0.50f, 0.15f);
    drawDDALine(-14.0f, 64.0f, 14.0f, 64.0f);

    glLineWidth(4.0f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 51.0f);
    glVertex2f(0.0f, 46.0f);  //neck
    glEnd();

    //shirt
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.18f, 0.38f, 0.78f);
    else glColor3f(0.06f, 0.12f, 0.25f);
    glVertex2f(-10.0f, 24.0f);
    glVertex2f(10.0f, 24.0f);
    glVertex2f(9.0f,   46.0f);
    glVertex2f(-9.0f, 46.0f);
    glEnd();

    //lungi
    glBegin(GL_QUADS);
    if (isDayMode) glColor3f(0.85f, 0.70f, 0.12f);
    else glColor3f(0.28f, 0.22f, 0.04f);
    glVertex2f(-10.0f, 4.0f);
    glVertex2f(10.0f, 4.0f);
    glVertex2f(9.0f,   24.0f);
    glVertex2f(-9.0f, 24.0f);
    glEnd();

    glLineWidth(3.5f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(-5.0f, 4.0f);
    glVertex2f(-5.0f, 0.0f);  //legs
    glVertex2f(5.0f,  4.0f);
    glVertex2f(5.0f,  0.0f);
    glEnd();

    //lathi
    glColor3f(0.55f, 0.38f, 0.18f);
    glLineWidth(3.5f);
    glBegin(GL_LINES);
    glVertex2f(-36.0f, 44.0f);
    glVertex2f(36.0f, 44.0f);
    glEnd();

    //dori
    glColor3f(0.40f, 0.28f, 0.12f);
    glPointSize(2.0f);
    drawBresenhamLine(-36, 44, -35, 20);
    drawBresenhamLine( 36, 44,  35, 20);

    //patil
    glColor3f(0.65f, 0.22f, 0.08f);
    drawFilledMidpointCircle(-35.0f, 20.0f, 10.0f);
    drawFilledMidpointCircle( 35.0f, 20.0f, 10.0f);

    glLineWidth(3.0f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(-9.0f, 42.0f);
    glVertex2f(-22.0f, 44.0f);  //arms
    glVertex2f(9.0f,  42.0f);
    glVertex2f(22.0f,  44.0f);
    glEnd();

    glPopMatrix();

    //VILLAGER 2
    glPushMatrix();
    glTranslatef(700.0f, 188.0f, 0.0f);

    glColor3f(sR, sG, sB);
    drawFilledMidpointCircle(0.0f, 56.0f, 9.0f);  //head

    glLineWidth(4.0f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(0.0f, 47.0f);
    glVertex2f(0.0f, 44.0f);  //neck
    glEnd();

    //Saree
    glBegin(GL_POLYGON);
    glColor3f(0.90f, 0.10f, 0.16f);
    glVertex2f(-11.0f, 0.0f);
    glVertex2f(11.0f, 0.0f);
    glVertex2f(10.0f,  44.0f);
    glVertex2f(-9.0f, 44.0f);
    glEnd();

    glLineWidth(3.0f);
    glColor3f(sR, sG, sB);
    glBegin(GL_LINES);
    glVertex2f(-10.0f, 40.0f);
    glVertex2f(-16.0f, 28.0f);  //arms
    glVertex2f(10.0f,  40.0f);
    glVertex2f(15.0f,  28.0f);
    glEnd();

    //kolos
    glColor3f(0.72f, 0.24f, 0.06f);
    drawFilledMidpointCircle(14.0f, 26.0f, 7.0f);
    glPopMatrix();

    //CHILDREN
    //Child 1
    glPushMatrix();
    glTranslatef(1145.0f, 245.0f, 0.0f);
    drawChildFigure(0.88f, 0.15f, 0.12f,  0.12f, 0.22f, 0.65f);
    glPopMatrix();

    //Child 2
    glPushMatrix();
    glTranslatef(1178.0f, 245.0f, 0.0f);
    drawChildFigure(0.15f, 0.60f, 0.25f,  0.55f, 0.35f, 0.12f);
    glPopMatrix();
}

void display()
{
    glClear(GL_COLOR_BUFFER_BIT);

    //scene layers back to front
    drawSky();
    drawSun();
    drawMoon();
    drawCloud();
    drawDistantHills();
    drawBird();
    drawCropField();
    drawTree();
    drawRoad();
    drawHouse();
    drawPeople();
    drawRiver();
    drawBoat();

    glutSwapBuffers();
}

void timer(int value)
{
    if (!isPaused)
    {
        float s = animSpeed;

        //Clouds across the sky
        cloudX1 += 0.6f * s;
        if (cloudX1 > 1250.0f) cloudX1 = -150.0f;
        cloudX2 += 0.8f * s;
        if (cloudX2 > 1250.0f) cloudX2 = -150.0f;
        cloudX3 += 0.5f * s;
        if (cloudX3 > 1250.0f) cloudX3 = -150.0f;

        //Birds fly across the sky
        birdX1 += 1.4f * s;
        if (birdX1 > 1300.0f) birdX1 = -100.0f;
        birdX2 += 1.7f * s;
        if (birdX2 > 1300.0f) birdX2 = -200.0f;
        birdX3 += 1.2f * s;
        if (birdX3 > 1300.0f) birdX3 = -300.0f;

        //Wings flap up and down
        if (wingUp)
        {
            birdWingAngle += 1.5f * s;
            if (birdWingAngle >= 25.0f)  wingUp = false;
        }
        else
        {
            birdWingAngle -= 1.5f * s;
            if (birdWingAngle <= -15.0f) wingUp = true;
        }

        //Boat automatic
        if (isDayMode && isBoatMoving && !isBoatManual)
        {
            boatX += boatSpeed * boatDirection * s;
            if (boatX > 1050.0f) boatDirection = -1;
            else if (boatX < 120.0f) boatDirection = 1;
        }

        //Paddle swings
        bool boatIsActive = (isDayMode && isBoatMoving && !isBoatManual) || paddleKeyActive;
        if (boatIsActive)
        {
            if (paddleForward)
            {
                paddleAngle += 2.0f * s;
                if (paddleAngle >= 25.0f)  paddleForward = false;
            }
            else
            {
                paddleAngle -= 2.0f * s;
                if (paddleAngle <= -15.0f) paddleForward = true;
            }
        }
        paddleKeyActive = false;

        // tide
        riverWaveOffset += 0.8f * s;
        if (riverWaveOffset > 120.0f) riverWaveOffset = 0.0f;
    }

    glutPostRedisplay();
    glutTimerFunc(16, timer, 0);
}

void specialKeys(int key, int x, int y)
{
    switch (key)
    {
    case GLUT_KEY_RIGHT:
        isBoatManual = true;
        isBoatMoving = false;
        boatDirection = 1;
        boatX += 8.0f;
        if (boatX > 1050.0f) boatX = 1050.0f;
        paddleKeyActive = true;
        break;
    case GLUT_KEY_LEFT:
        isBoatManual = true;
        isBoatMoving = false;
        boatDirection = -1;
        boatX -= 8.0f;
        if (boatX < 120.0f) boatX = 120.0f;
        paddleKeyActive = true;
        break;
    default:
        break;
    }
    glutPostRedisplay();
}

void keyboard(unsigned char key, int x, int y)
{
    switch (key)
    {
    case 'D':
    case 'd':
        isDayMode = true;
        break;
    case 'N':
    case 'n':
        isDayMode = false;
        break;
    case 'B':
    case 'b':
        isBoatMoving = !isBoatMoving;
        if (isBoatMoving) isBoatManual = false;
        break;
    case 'P':
    case 'p':
        isPaused = !isPaused;
        cout << (isPaused ? "[Paused]" : "[Resumed]") <<endl;
        break;
    case '+':
    case '=':
        animSpeed += 0.2f;
        if (animSpeed > 5.0f) animSpeed = 5.0f;
        cout << "[Speed] " << animSpeed << "x" <<endl;
        break;
    case '-':
    case '_':
        animSpeed -= 0.2f;
        if (animSpeed < 0.2f) animSpeed = 0.2f;
        cout << "[Speed] " << animSpeed << "x" <<endl;
        break;
    case 27:
        exit(0);
        break;  // Esc = quit
    default:
        break;
    }
    glutPostRedisplay();
}

void init()
{
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    gluOrtho2D(0.0, 1200.0, 0.0, 800.0);

    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();

    //blending for smooth graphics
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
}

int main(int argc, char** argv)
{
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB);
    glutInitWindowSize(1200, 800);
    glutInitWindowPosition(100, 50);
    glutCreateWindow("Bangladeshi Village Scene - OpenGL Project");
    cout << "Controls: D=Day  N=Night  B=Boat  P=Pause  +/-=Speed  Arrows=Boat  Esc=Exit" <<endl;
    init();
    glutDisplayFunc(display);
    glutKeyboardFunc(keyboard);
    glutSpecialFunc(specialKeys);
    glutTimerFunc(0, timer, 0);

    glutMainLoop();
    return 0;
}
