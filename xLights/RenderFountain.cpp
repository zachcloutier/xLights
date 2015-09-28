/***************************************************************
 * Name:      RenderFountain.cpp
 * Purpose:   Implements RGB effects
 * Author:    Logan Moore (loganjmoore@gmail.com)
 * Created:   2015-09-14
 * Copyright: 2015 by Logan Moore
 * License:
     This file is part of xLights.

    xLights is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    xLights is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with xLights.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************/

#include "RgbEffects.h"
#include "iostream"
#include "list"

int numParticles = 10;
double sprayAngle = .003;
double gravityAngle = .15;
double fountainHeight = .20;
int cols = 0;
int rows = 0;
xlColor fountainColor;

int GetRandom(int low, int high)
{
    return (low + (rand() % (int)(high - low + 1)));
}

class Particle
{
    public:Particle()
    {
        index = 1;
        angle = -.06;
        gravityForce = 5;
    }

    double x;
    double y;
    double lastx;
    double lasty;
    double angle;
    double gravityForce;
    int index;
    bool kill = false;
    int cols;
    int rows;
    int velocity;
    int sa;

    void move()
    {
        this->lastx = this->x;
        this->lasty = this->y;

        if (!kill)
        {
            this->y = (this->y + velocity)-gravityForce;
            this->x = this->x + angle;
        }
		else
        {
            gravity();
        }

        this->angle = this->angle + (index * (sa*sprayAngle));
    }

    void gravity()
    {
        this->lasty = this->y;
        this->lastx = this->x;

        this->kill = true;

        this->y = (this->y - velocity)-gravityForce;
        this->x = this->x + angle*gravityAngle;
    }

};

std::vector<Particle *> particles;
std::vector<Particle*>::iterator it;

void GenerateParticles()
{
    for (int p = 1; p <= numParticles; p++)
    {
        Particle* par = new Particle();
        par = new Particle();
        par->x = cols / 2;
        par->y = GetRandom(-2,2);
        par->lastx = 0;
        par->lasty = 0;
        par->index = -(numParticles / 2) + p;

        particles.push_back(par);
    }
}

void RgbEffects::RenderFountain(int ftnHeight, int np, int sprayRate, int spAngle)
{
    int x,y;

    numParticles = np;
    double gravityAngle = .15;
    double fountainHeight = (((double)ftnHeight)*.10);

    cols = BufferWi;
    rows = BufferHt;
    palette.GetColor(0, fountainColor);

    if (curPeriod % 6 == 0 && curPeriod < (curEffStartPer+((curEffEndPer-curEffStartPer))/4))
        GenerateParticles();

    for (it = particles.begin(); it != particles.end(); it++)
    {
        (*it)->cols = cols;
        (*it)->rows = rows;
        (*it)->velocity = sprayRate;
        (*it)->sa = spAngle;

        if((*it)->y < ((rows*fountainHeight)+GetRandom(-5,0)))
            (*it)->move();
        else
            (*it)->gravity();

        SetPixel((*it)->x,(*it)->y,fountainColor);
        SetPixel((*it)->lastx,(*it)->lasty,xlBLACK);
    }
}
