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
#include "modelLoader.h"
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

std::unique_ptr<ModelLoader> modelLoader = nullptr;
std::unique_ptr<Model> model = nullptr;

std::unique_ptr<BVHLoader> animationLoader = nullptr;
std::unique_ptr<Animation> animation = nullptr;


std::unique_ptr<Camera> camera = nullptr;

unsigned int texture[1];
int id = 0;


struct BitMapFile
{
   int sizeX;
   int sizeY;
   unsigned char *data;
};

BitMapFile *image[1];

BitMapFile *getbmp(std::string filename)
{
    int offset, headerSize;
  
  // Initialize bitmap files for RGB (input) and RGBA (output).
  BitMapFile *bmpRGB = new BitMapFile;
  BitMapFile *bmpRGBA = new BitMapFile;

  // Read input bmp file name.
  std::ifstream infile(filename.c_str(), std::ios::binary);

  // Get starting point of image data in bmp file.
  infile.seekg(10);
  infile.read((char *)&offset, 4); 

  // Get header size of bmp file.
  infile.read((char *)&headerSize,4);

  // Get image width and height values from bmp file header.
  infile.seekg(18);
  infile.read((char *)&bmpRGB->sizeX, 4);
  infile.read((char *)&bmpRGB->sizeY, 4);

  // Determine the length of zero-byte padding of the scanlines 
  // (each scanline of a bmp file is 4-byte aligned by padding with zeros).
  int padding = (3 * bmpRGB->sizeX) % 4 ? 4 - (3 * bmpRGB->sizeX) % 4 : 0;

  // Add the padding to determine size of each scanline.
  int sizeScanline = 3 * bmpRGB->sizeX + padding;

  // Allocate storage for image in input bitmap file.
  int sizeStorage = sizeScanline * bmpRGB->sizeY;
  bmpRGB->data = new unsigned char[sizeStorage];

  // Read bmp file image data into input bitmap file.
  infile.seekg(offset);
  infile.read((char *) bmpRGB->data , sizeStorage);
 
  // Reverse color values from BGR (bmp storage format) to RGB.
  int startScanline, endScanlineImageData, temp;
  for (int y = 0; y < bmpRGB->sizeY; y++)
  {
       startScanline = y * sizeScanline; // Start position of y'th scanline.
     endScanlineImageData = startScanline + 3 * bmpRGB->sizeX; // Image data excludes padding.
       for (int x = startScanline; x < endScanlineImageData; x += 3)
     {
        temp = bmpRGB->data[x];
      bmpRGB->data[x] = bmpRGB->data[x+2];
      bmpRGB->data[x+2] = temp;     
     }
  }
    // Set image width and height values and allocate storage for image in output bitmap file.
  bmpRGBA->sizeX = bmpRGB->sizeX;
  bmpRGBA->sizeY = bmpRGB->sizeY;
  bmpRGBA->data = new unsigned char[4*bmpRGB->sizeX*bmpRGB->sizeY];

  // Copy RGB data from input to output bitmap files, set output A to 1.
  for(int j = 0; j < 4*bmpRGB->sizeY * bmpRGB->sizeX; j+=4)
  {
    bmpRGBA->data[j] = bmpRGB->data[(j/4)*3];
    bmpRGBA->data[j+1] = bmpRGB->data[(j/4)*3+1];
    bmpRGBA->data[j+2] = bmpRGB->data[(j/4)*3+2];
    bmpRGBA->data[j+3] = 0xFF;
  }

  return bmpRGBA;
}


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

  model->doSurface();
  glBindTexture(GL_TEXTURE_2D, texture[0]);

  glFrustum(-1.0, 1.0, -1.0, 1.0, 1.0, 100.0);
    
  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();

  // Camera
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI), 1.0f, 0.0f, 0.0f);  
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI), 0.0f, 1.0f, 0.0f);
  glRotatef(camera->orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI), 0.0f, 0.0f, 1.0f);

  glPopMatrix();

  glColor3f(1.0, 0.0, 0.0);
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

  glColor3f(1.0, 1.0, 1.0);
  glCallList(modelList);
    
  glutSwapBuffers();
}

void setup(void)
{
  glEnable(GL_DEPTH_TEST);
  glEnable(GL_NORMALIZE);

  glClearColor(0.0, 0.0, 0.0, 0.0);

  model->scaleAndCenterModelVertices();

  image[0] = getbmp("a3files/mesh/skin.bmp");
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image[0]->sizeX, image[0]->sizeY, 0,
  GL_RGBA, GL_UNSIGNED_BYTE, image[0]->data);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

  modelList = glGenLists(1);
  glNewList(modelList, GL_COMPILE);
  for (int i = 0; i < model->faces_.size(); i++)
  {
    glBegin(GL_TRIANGLE_STRIP);
      glTexCoord2fv(model->vt_[model->textures_[i].data()[0]-1].data());
      glNormal3fv(model->vn_[model->normals_[i].data()[0]-1].data());
      glVertex3fv(model->vertices_[model->faces_[i].data()[0]-1].data());

      glTexCoord2fv(model->vt_[model->textures_[i].data()[1]-1].data());
      glNormal3fv(model->vn_[model->normals_[i].data()[1]-1].data());
      glVertex3fv(model->vertices_[model->faces_[i].data()[1]-1].data());

      glTexCoord2fv(model->vt_[model->textures_[i].data()[2]-1].data());
      glNormal3fv(model->vn_[model->normals_[i].data()[2]-1].data());
      glVertex3fv(model->vertices_[model->faces_[i].data()[2]-1].data());

    glEnd();
  }
  glEndList();

  model->translation_.z() = -20.0;

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

  glutPostRedisplay();

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
        model->keyInput(key, x, y);
        break;
   }
}

void specialKeyInput(int key, int x, int y)
{
  model->specialKeyInput(key,x,y);
  glutPostRedisplay();
}

void fps(int val)
{
  if (!isPaused)
    processNextFrame();
  glutTimerFunc(8, fps, 0);
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
  
  modelLoader.reset(new ModelLoader());
  animationLoader.reset(new BVHLoader());
  attachLoader.reset(new AttachLoader());

  model.reset(modelLoader->load(argv[1]));
  animation = animationLoader->load(argv[2]);
  attach.reset()

  //8.333ms is 120 Frames per second 
  glutTimerFunc(8, fps, 0);

  setInitialJointLocations();

  glutReshapeFunc(resize);
  glutKeyboardFunc(keyInput);
  glutSpecialFunc(specialKeyInput);
  glutDisplayFunc(drawScene);

  glewInit();

  setup(); 

  glutMainLoop();
}