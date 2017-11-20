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
static int isOrtho = 0;
static int isFog = 0;

std::unique_ptr<ModelLoader> modelLoader = nullptr;
std::unique_ptr<Model> model = nullptr;
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


void drawScene()
{
  glClear (GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glPopMatrix();
    
  glMatrixMode (GL_PROJECTION);
  glLoadIdentity();
  if (isOrtho) {
    glOrtho(-1.0, 1.0, -1.0, 1.0, 8, 100);
  } else {
    glFrustum(-1.0, 1.0, -1.0, 1.0, 8, 100);
  }

  
  glMatrixMode(GL_MODELVIEW);

  glLoadIdentity();

  model->doSurface();
  glBindTexture(GL_TEXTURE_2D, texture[0]);
  
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
  
  glPopMatrix();

  
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
        model->print("meshout.obj");
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