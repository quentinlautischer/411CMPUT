////////////////////////////////////////////////////////////////////////////////      
// mationViewer.cpp - Assignment 2 - CMPUT 411 - University of Alberta
//
//
// Quentin Lautischer
//////////////////////////////////////////////////////////////////////////////// 

#include <iostream>
#include <string>
#include <memory>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <math.h>

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
static int virtualSpeed = 120;
static bool isPaused = false;
static double currentFrame = 0.0;
std::thread* t;

std::unique_ptr<BVHLoader> animationLoader = nullptr;
std::unique_ptr<Animation> animation = nullptr;
std::unique_ptr<Camera> camera = nullptr;

double zoom = 0.0;

void setInitialJointLocations()
{
  animation->tree_->enumerate<std::function<void(BVHTreeNode*)>>
    ([&](BVHTreeNode *node)
    {
      if (node->parent_ != nullptr)
      {
        node->location_ = node->offset_ + node->parent_->location_;
        node->parentLocation_ = node->parent_->location_;
      }
      else
      {
        node->location_ = node->offset_;
      }
    });
}

void drawScene()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();

  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
    
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Camera
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI), 1.0f, 0.0f, 0.0f);  
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI), 0.0f, 0.0f, 1.0f);

  glPopMatrix();


  glColor3f(1.0, 1.0, 1.0);
  glLineWidth(1.5);

  glTranslatef(-animation->motion_[0][0]-camera->translation_.x(), -animation->motion_[0][1]-camera->translation_.y(), -animation->motion_[0][1]-camera->translation_.z()-50+zoom);
  // setInitialJointLocations();
  
  animation->tree_->enumerate<std::function<void(BVHTreeNode*)>,std::function<void(BVHTreeNode*)>>
  ([&](BVHTreeNode *node)
  {
    glPushMatrix();
    
    if (node->parent_ != nullptr) 
    {
      // Translate to Join pivot point
      glTranslatef(node->parent_->location_.data()[0], node->parent_->location_.data()[1], node->parent_->location_.data()[2]);
      
      for(std::string channel : node->channels_)
      {
        if (channel == "Zrotation")
          glRotatef(node->parent_->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180.0/M_PI), 0.0f, 0.0f, 1.0f);
      }
      for(std::string channel : node->channels_)
      {
        if (channel == "Yrotation")
          glRotatef(node->parent_->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180.0/M_PI), 0.0f, 1.0f, 0.0f);
      }
      for(std::string channel : node->channels_)
      {
        if (channel == "Xrotation")
          glRotatef(node->parent_->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180.0/M_PI), 1.0f, 0.0f, 0.0f);
      }
      glTranslatef(-node->parent_->location_.data()[0], -node->parent_->location_.data()[1], -node->parent_->location_.data()[2]);
    }

    for(std::string channel : node->channels_)
    {
      if (channel == "Xposition")
        glTranslatef(node->channelValues_[channel], 0.0, 0.0);
      else if (channel == "Yposition")
        glTranslatef(0.0, node->channelValues_[channel], 0.0);
      else if (channel == "Zposition")
        glTranslatef(0.0, 0.0, node->channelValues_[channel]);
    }
    

    if (node->parent_ != nullptr) 
    {
      glBegin(GL_LINES);
        glVertex3fv(node->parentLocation_.data());
        glVertex3fv(node->location_.data());  
      glEnd();
    }
  },
  [&](BVHTreeNode *node)
  {
    glPopMatrix();
  });
    
  glutSwapBuffers();
}

void setup(void)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  glClearColor(0.0, 0.0, 0.0, 0.0);

  glutPostRedisplay();
}

Eigen::Quaternion<float, 0> createQuaternion(float xAngleDegree, float yAngleDegree, float zAngleDegree)
{
  Eigen::Quaternion<float, 0> q;
  
  q = Eigen::AngleAxisf(xAngleDegree*(M_PI/180), Eigen::Vector3f::UnitX())
  * Eigen::AngleAxisf(yAngleDegree*(M_PI/180),  Eigen::Vector3f::UnitY())
  * Eigen::AngleAxisf(zAngleDegree*(M_PI/180), Eigen::Vector3f::UnitZ());

  return q;
}

Eigen::Quaternion<float, 0> createQuaternion2(float xAngleDegree, float yAngleDegree, float zAngleDegree)
{
  Eigen::Quaternion<float, 0> q;
  
  q = Eigen::AngleAxisf(xAngleDegree*(M_PI/180), Eigen::Vector3f::UnitX())
  * Eigen::AngleAxisf(yAngleDegree*(M_PI/180),  Eigen::Vector3f::UnitY())
  * Eigen::AngleAxisf(zAngleDegree*(M_PI/180), Eigen::Vector3f::UnitZ());

  return q;
}

void processNextFrame()
{
  // std::cout << "Processing Next Frame" << std::endl;

  int currentFrameData = 0;
  double floorFrame = 0.0;
  double ceilFrame = 0.0;

  if (virtualSpeed > 0)
  {
    floorFrame = floor(currentFrame);
    ceilFrame = ceil(currentFrame);
  }
  else
  {
    floorFrame = ceil(currentFrame);
    ceilFrame = floor(currentFrame);
  }
 
  double t = currentFrame - floorFrame;
  

  animation->tree_->enumerate<std::function<void(BVHTreeNode*)>>
    ([&](BVHTreeNode *node)
    {
        Eigen::Quaternion<float, 0> q1 = createQuaternion(0.0, 0.0, 0.0);
        Eigen::Quaternion<float, 0> q2 = createQuaternion2(0.0, 0.0, 0.0);


        Eigen::Quaternion<float, 0> qz1 = createQuaternion(0.0, 0.0, 0.0);
        Eigen::Quaternion<float, 0> qy1 = createQuaternion(0.0, 0.0, 0.0);
        Eigen::Quaternion<float, 0> qx1 = createQuaternion(0.0, 0.0, 0.0);
        Eigen::Quaternion<float, 0> qz2 = createQuaternion2(0.0, 0.0, 0.0);
        Eigen::Quaternion<float, 0> qy2 = createQuaternion2(0.0, 0.0, 0.0);
        Eigen::Quaternion<float, 0> qx2 = createQuaternion2(0.0, 0.0, 0.0);
        
        for(std::string channel : node->channels_)
        {
          //LERP
          node->channelValues_[channel] = (1-t)*animation->motion_[floorFrame][currentFrameData] + t*animation->motion_[ceilFrame][currentFrameData];
          // std::cout << " " << node->channelValues_[channel];
          currentFrameData++;

          if (channel == "Xrotation")
          {
            qx1 = createQuaternion(node->channelValues_[channel], 0.0, 0.0);
            qx2 = createQuaternion(node->channelValues_[channel], 0.0, 0.0);
          }
          else if (channel == "Yrotation")
          {
            qy1 = createQuaternion(0.0, node->channelValues_[channel], 0.0);
            qy2 = createQuaternion(0.0, node->channelValues_[channel], 0.0);
          }
          else if (channel == "Zrotation")
          {
            qz1 = createQuaternion(0.0, 0.0, node->channelValues_[channel]);
            qz2 = createQuaternion(0.0, 0.0, node->channelValues_[channel]);   
          }
          else
            continue;
        }

        q1 = qx1*q1;
        q1 = qy1*q1;
        q1 = qz1*q1;
        q1.normalize();

        q2 = qx2*q2;
        q2 = qy2*q2;
        q2 = qz2*q2;
        q2.normalize();  

        //SLERP ORIENTATION
        
        node->orientation_ = q1.slerp(t, q2);

        




    });

  currentFrame = (currentFrame + 1.0*(virtualSpeed/120.0));
  if (currentFrame <= 0.00 && virtualSpeed < 0)
    currentFrame = (double) animation->frames_-1;
  else
    currentFrame = fmod(currentFrame, (double) animation->frames_-1);


  // std::cout << "Current Frame #: " <<currentFrame << std::endl;
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
        animation->printToBVH("output.bvh");
        break;
      case 'q':
         isPaused = true;
         std::terminate();
         exit(0);
         break;
      case 'x':
        //Reset
        isPaused = true;
        camera->reset();
        currentFrame = 0.0;
        virtualSpeed = 120;
        processNextFrame();
        glutPostRedisplay();
        break;
      case '+':
        virtualSpeed += 10;
        std::cout << "Virtual Speed: " << virtualSpeed << std::endl;
        break;
      case '-':
        virtualSpeed -= 10;
        std::cout << "Virtual Speed: " << virtualSpeed << std::endl;
        break;
      case 'P':
        isPaused = true;
        break;
      case 'p':
        isPaused = false;
        break;
      case ' ':    
        processNextFrame();
        glutPostRedisplay();
        break;   
      default:
        camera->keyInput(key, x, y);
        break;
   }
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
  glutCreateWindow("Project 2 - ModelViewer - Quentin Lautischer");

  camera.reset(new Camera());
  animationLoader.reset(new BVHLoader());
  animation = animationLoader->load(argv[1]);

  t = new std::thread([](){
    while(1)
    {
      if (!isPaused)
      {
        processNextFrame();
      }
      glutPostRedisplay();
      std::this_thread::sleep_for(std::chrono::microseconds(8333));
    }
  });

  setInitialJointLocations();

  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);
  glutDisplayFunc(drawScene);

  glewInit();

  setup(); 

  glutMainLoop();
}