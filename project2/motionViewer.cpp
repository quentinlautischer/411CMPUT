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
#include "animation.h"
#include "bvhLoader.h"

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
static int isOrtho = 0;
static int isFog = 0;

std::unique_ptr<BVHLoader> animationLoader = nullptr;
std::unique_ptr<Animation> animation = nullptr;
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
    glFrustum(-20.0, 20.0, -20.0, 20.0, 8, 100);
  }
    
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();
  
  glMatrixMode (GL_MODELVIEW);

  // Camera
  // glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI), 1.0f, 0.0f, 0.0f);  
  // glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI), 0.0f, 1.0f, 0.0f);
  // glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI), 0.0f, 0.0f, 1.0f);

  // glPopMatrix();
   
  // glMatrixMode (GL_MODELVIEW);

  // glTranslatef(model->translation_.x(), model->translation_.y(), model->translation_.z());

  // glTranslatef(-camera->translation_.x(), -camera->translation_.y(), -camera->translation_.z());
  
  // // Model  
  // glRotatef(model->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI), 1.0f, 0.0f, 0.0f);  
  // glRotatef(model->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI), 0.0f, 1.0f, 0.0f);
  // glRotatef(model->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI), 0.0f, 0.0f, 1.0f);
  // gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0);
  gluLookAt(0.0, 0.0, 20.0, 0.0, 0.0, 0.0, 0.0, 1.0, 0.0);
  glColor3f(1.0, 1.0, 1.0);
  glLineWidth(2.5);
  // glCallList(modelList);
  glBegin(GL_LINES);
    animation->tree_->enumerate<std::function<void(BVHTreeNode*)>>
    ([&](BVHTreeNode *node)
    {
      if (node->parent_ != nullptr) 
      {
        std::cout << "Linking " << node->getName() << " " << node->location_.data()[0]<< " " <<node->location_.data()[1]<<" " << node->location_.data()[2] << " to " << node->parent_->getName() << ":" << node->parent_->location_.data()[0] <<" " << node->parent_->location_.data()[1] <<" " << node->parent_->location_.data()[2]  <<" " << std::endl; 

        glVertex3fv(node->parent_->location_.data());
        glVertex3fv(node->location_.data());  
      }
      else
      {
        std::cout << "No Linking this is a root node" << std::endl;
      }
    });
    
  glEnd();

  glutSwapBuffers();
}

void setup(void)
{
  glEnable(GL_DEPTH_TEST);
  // glEnable(GL_NORMALIZE);

  glClearColor(0.0, 0.0, 0.0, 0.0);

  // model->scaleAndCenterModelVertices();

  modelList = glGenLists(1);
  glNewList(modelList, GL_COMPILE);
  
  glEndList();

  // for (int i = 0; i < model->faces_.size(); ++i)
  // {
  //   glBegin(GL_TRIANGLE_STRIP);
  //     glVertex3fv(model->vertices_[model->faces_[i].data()[0]-1].data());
  //     glVertex3fv(model->vertices_[model->faces_[i].data()[1]-1].data());
  //     glVertex3fv(model->vertices_[model->faces_[i].data()[2]-1].data());
  //   glEnd();
  //   if (model->faces_[i].data()[3] != -1)
  //   {
  //     glBegin(GL_TRIANGLE_STRIP);
  //       glVertex3fv(model->vertices_[model->faces_[i].data()[0]-1].data());
  //       glVertex3fv(model->vertices_[model->faces_[i].data()[2]-1].data());
  //       glVertex3fv(model->vertices_[model->faces_[i].data()[3]-1].data());
  //     glEnd();
  //   }
  // }

  // model->translation_.z() = -10.0;

  glutPostRedisplay();
}

// OpenGL window reshape routine.
void resize (int w, int h)
{
  glViewport (0, 0, w, h);
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  // glOrtho(-1.0, 1.0, -1.0, 1.0, 8, 100);
  glMatrixMode(GL_MODELVIEW);
}

// Keyboard input processing routine.
void keyInput(unsigned char key, int x, int y)
{
   switch (key) 
   {
      case 'w':
        // model->print("out.obj");
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
        // model->reset();
        // camera->reset();
        // model->translation_.z() = -10;
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
        // model->keyInput(key, x, y);
        // camera->keyInput(key, x, y);
        break;
   }
   glutPostRedisplay();
}

void specialKeyInput(int key, int x, int y)
{
  // model->specialKeyInput(key,x,y);
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

  animationLoader.reset(new BVHLoader());
  animation = animationLoader->load(argv[1]);

  animation->printToBVH("output.bvh");

  // std::cout << "TEST: " << animation->tree_->children_.at(0).children_.at(0).parent_->getName() << animation->tree_->children_.at(0).children_.at(0).parent_->type_ << std::endl;
  // std::cout << "TEST: " << animation->tree_->children_[0].children_[0].parent_->parent_->getName() << animation->tree_->children_[0].children_[0].parent_->parent_->type_ << std::endl;
  animation->tree_->enumerate<std::function<void(BVHTreeNode*)>>
    ([&](BVHTreeNode *node)
    {
      if (node->parent_ != nullptr)
      {
        std::cout << "HI" << std::endl;
        std::cout << "CALCULATING " << node->getName() << " " << node->location_.data()[0]<< " " <<node->location_.data()[1]<<" " << node->location_.data()[2] << " to " << node->parent_->getName() << ":" << node->parent_->location_.data()[0] <<" " << node->parent_->location_.data()[1] <<" " << node->parent_->location_.data()[2]  <<" " << std::endl;
        std::cout << "CurrentNode Offset: " << node->offset_[0] << ":" << node->offset_[1] << ":" << node->offset_[2] << std::endl;
        node->location_ = node->offset_ + node->parent_->location_;
        std::cout << "CALCULATING " << node->getName() << " " << node->location_.data()[0]<< " " <<node->location_.data()[1]<<" " << node->location_.data()[2] << " to " << node->parent_->getName() << ":" << node->parent_->location_.data()[0] <<" " << node->parent_->location_.data()[1] <<" " << node->parent_->location_.data()[2]  <<" " << std::endl;
      }
      else
      {
        // node->location_ = node->offset_;
      }
    });


  glutReshapeFunc(resize);
  glutSpecialFunc(specialKeyInput);
  glutKeyboardFunc(keyInput);
  glutDisplayFunc(drawScene);

  glewExperimental = GL_TRUE; 
  glewInit();

  setup(); 

  glutMainLoop();
}