////////////////////////////////////////////////////////////////////////////////      
// modelViewer.cpp - Assignment 1 - CMPUT 411 - University of Alberta
//
// This program loads any .obj mesh outputs the resulting transformed mesh.
// This program allows for viewing and manipulation of the model with 
// 12 degrees of freedom.
//
// Quentin Lautischer
//////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <sstream>

#include <Eigen/Dense>
#include "camera.h"
#include "model.h"
#include "modelLoader.h"

#ifdef __APPLE__
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <OpenGL/glext.h>
#else
#  include <GL/glew.h>
#  include <GL/freeglut.h>
#  include <GL/glext.h>
#pragma comment(lib, "glew32.lib") 
#endif

int modelList;
static int isWire = 1;
static int isOrtho = 1;
static int isFog = 0;

std::unique_ptr<ModelLoader> modelLoader = nullptr;
std::unique_ptr<Model> model = nullptr;
std::unique_ptr<Camera> camera = nullptr;

void drawScene()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  if (isWire) 
    glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  if (isOrtho) {
    glOrtho(-1.0, 1.0, -1.0, 1.0, 8, 100);
  } else {
    glFrustum(-1.0, 1.0, -1.0, 1.0, 8, 100);
  }
    
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  
  glMatrixMode (GL_MODELVIEW);

  // Camera
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI), 1.0f, 0.0f, 0.0f);  
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI), 0.0f, 0.0f, 1.0f);

  glPopMatrix();
   
  glMatrixMode (GL_MODELVIEW);

  glTranslatef(model->translation_.x(), model->translation_.y(), model->translation_.z());

  glTranslatef(-camera->translation_.x(), -camera->translation_.y(), -camera->translation_.z());
  
  // Model  
  glRotatef(model->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI), 1.0f, 0.0f, 0.0f);  
  glRotatef(model->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(model->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI), 0.0f, 0.0f, 1.0f);
  
  glColor3f(1.0, 1.0, 1.0);
  glCallList(modelList);

  model->printInfo();
  camera->printInfo();

  glutSwapBuffers();
}

void setup(void)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  glClearColor(0.0, 0.0, 0.0, 0.0);

  model->scaleAndCenterModelVertices();

  modelList = glGenLists(1);
  glNewList(modelList, GL_COMPILE);
  for (int i = 0; i < model->faces_.size(); ++i)
  {
    glBegin(GL_TRIANGLE_STRIP);
      glVertex3fv(model->vertices_[model->faces_[i].data()[0]-1].data());
      glVertex3fv(model->vertices_[model->faces_[i].data()[1]-1].data());
      glVertex3fv(model->vertices_[model->faces_[i].data()[2]-1].data());
    glEnd();
    if (model->faces_[i].data()[3] != -1)
    {
      glBegin(GL_TRIANGLE_STRIP);
        glVertex3fv(model->vertices_[model->faces_[i].data()[0]-1].data());
        glVertex3fv(model->vertices_[model->faces_[i].data()[2]-1].data());
        glVertex3fv(model->vertices_[model->faces_[i].data()[3]-1].data());
      glEnd();
    }
  }
  glEndList();

  model->translation_.z() = -10.0;

  if (isFog)
    glEnable(GL_FOG);
  glFogi(GL_FOG_MODE, GL_LINEAR);
  glFogf(GL_FOG_START, 10.0);
  glFogf(GL_FOG_END, 11.0);
  glHint(GL_FOG_HINT, GL_NICEST);

  glutPostRedisplay();
}

// OpenGL window reshape routine.
void resize (int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  glOrtho(-1.0, 1.0, -1.0, 1.0, 8, 100);
  glMatrixMode(GL_MODELVIEW);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch (key) 
   {
      case 'w':
        model->print("out.obj");
        break;
      case 'q':
         exit(0);
         break;
      case 'v':
        //Orthographic
        std::cout << std::endl << "Orthographic Mode" << std::endl;
        isOrtho = 1;
        glutPostRedisplay();
        break;
      case 'V':
        //Perspective
        std::cout << std::endl << "Perspective Mode" << std::endl;
        isOrtho = 0;
        glutPostRedisplay();
        break;
      case 'f':
        //Fog Effect Off
        std::cout << std::endl << "Fog Disabled" << std::endl;
        glDisable(GL_FOG);
        break;
      case 'F':
        std::cout << std::endl << "Fog Enabled" << std::endl;
        glEnable(GL_FOG);
        //Fog Effect On
        break;
      case 'x':
        //Reset
        model->reset();
        camera->reset();
        model->translation_.z() = -10;
        glutPostRedisplay();
        break;
      case ' ':    
        if (isWire == 0) 
          isWire = 1;
        else
          isWire = 0;
        glutPostRedisplay();
        break;   
      default:
        model->keyInput(key, x, y);
        camera->keyInput(key, x, y);
        break;
   }
   glutPostRedisplay();
}

void specialKeyInput(int key, int x, int y)
{
  model->specialKeyInput(key,x,y);
  glutPostRedisplay();
}

// Main routine.
int main(int argc, char **argv)
{
  glutInit(&argc, argv);
  glutInitContextVersion(4, 3); 
  glutInitContextProfile(GLUT_COMPATIBILITY_PROFILE);

  glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGBA | GLUT_DEPTH); 
  glutInitWindowSize(500, 500);
  glutInitWindowPosition(100, 100);
  glutCreateWindow("Project 1 - ModelViewer - Quentin Lautischer");

  modelLoader.reset(new ModelLoader());
  camera.reset(new Camera());
  model.reset(modelLoader->load(argv[1]));

  model->print_info();

  
  glutReshapeFunc(resize);
  glutSpecialFunc(specialKeyInput);
  glutKeyboardFunc(keyInput);
  glutDisplayFunc(drawScene);

  glewExperimental = GL_TRUE; 
  glewInit();

  setup(); 

  glutMainLoop();
}