#pragma once
#include "getbmp.h"
#include <iomanip>
#include <iostream>
#include <fstream>

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

float lightAmb[] = {0.0, 0.0, 0.0, 1.0};
float lightDifAndSpec[] = {1.0, 1.0, 1.0, 1.0};
float globAmb[] = {0.2, 0.2, 0.2, 1.0};

float matAmbAndDif1[] = {0.9, 0.9, 0.9, 1.0};
float matAmbAndDif2[] = {0.0, 0.9, 0.0, 1.0};
float matSpec[] = {1.0, 1.0, 1.0, 1.0};
float matShine[] = {50.0};


class Model
{
private:
  float max = 0;
  int sToggle = 0;

  

public:
  std::vector<Eigen::Vector3f> vertices_;
  std::vector<Eigen::Vector2f> vt_;
  std::vector<Eigen::Vector3f> vn_;

  std::vector<Eigen::Vector3f> faces_;
  std::vector<Eigen::Vector3f> textures_;
  std::vector<Eigen::Vector3f> normals_;
  Eigen::Translation<float,3> translation_;
  Eigen::Quaternion<float, 0> orientation_;
  Eigen::Translation<float, 3> origin_;


  Model()
  {
    vertices_ = std::vector<Eigen::Vector3f>();
    vt_ = std::vector<Eigen::Vector2f>();
    vn_ = std::vector<Eigen::Vector3f>();

    faces_ = std::vector<Eigen::Vector3f>();
    textures_ = std::vector<Eigen::Vector3f>();
    normals_ = std::vector<Eigen::Vector3f>();

    reset();
  }

  void printInfo()
  {
    std::cout << std::endl << "Model Info:" << std::endl;
    std::cout << "Translation: X: " << translation_.x() << " Y: " << translation_.y() << " Z: " << translation_.z() << std::endl;
    std::cout << "Orientation: X: " << orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI) << 
    " Y: " << orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI) << 
    " Z: " << orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI) << std::endl;
  }

  void lightUp()
  {
    float lightPos[] = {5.0, 1.5, 5.0, 1.0};
    glLightfv(GL_LIGHT0, GL_AMBIENT, lightAmb);
    glLightfv(GL_LIGHT0, GL_DIFFUSE, lightDifAndSpec);
    glLightfv(GL_LIGHT0, GL_SPECULAR, lightDifAndSpec);
    glLightfv(GL_LIGHT0, GL_POSITION, lightPos);

    glEnable(GL_LIGHT0);
    glEnable(GL_LIGHTING);

    // glLightModelfv(GL_LIGHT_MODEL_AMBIENT, globAmb);

    glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, matAmbAndDif1);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, matSpec);
    glMaterialfv(GL_FRONT_AND_BACK, GL_SHININESS, matShine);
  }

  void doSurface()
  {
    if (sToggle == 0) //wire
    {
      glDisable(GL_LIGHT0);
      glDisable(GL_LIGHTING);
      glDisable(GL_TEXTURE_2D);
      glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    } 
    else if (sToggle == 1) //flat shaded
    {
      lightUp();
      glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
      glVertexPointer(3, GL_FLOAT, 0, vertices_.data());
      glNormalPointer(GL_FLOAT, 0, normals_.data());

      glShadeModel(GL_FLAT);
    }
    else if (sToggle == 2) //smoothly shaded
    {
      lightUp();

      glShadeModel(GL_SMOOTH);
    }
    else // texture mapped object
    {
      glEnable(GL_TEXTURE_2D);
      lightUp();
    }

  }

  void reset()
  {
    translation_ = Eigen::Translation<float, 3>(0, 0, 0);

    orientation_ = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX())
      * Eigen::AngleAxisf(0,  Eigen::Vector3f::UnitY())
      * Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ()); 
  }

  void add_vertex(const Eigen::Vector3f &v)
  {
    vertices_.push_back(v);
  }

  void add_face(const Eigen::Vector3f &f)
  {
    faces_.push_back(f);
  }

  void add_face_t(const Eigen::Vector3f &f)
  {
    textures_.push_back(f);
  }

  void add_face_n(const Eigen::Vector3f &f)
  {
    normals_.push_back(f);
  }

  void add_vt(const Eigen::Vector2f &f)
  {
    vt_.push_back(f);
  }

  void add_vn(const Eigen::Vector3f &f)
  {
    vn_.push_back(f);
  }

  void calcOrigin()
  {
    float x = 0;
    float y = 0;
    float z = 0;
    
    for(int i = 0; i < vertices_.size(); i++)
    {
      x += vertices_[i].data()[0];
      y += vertices_[i].data()[1];
      z += vertices_[i].data()[2];
    }

    origin_ = Eigen::Translation<float, 3>(x/(float)vertices_.size(), y/(float)vertices_.size(), z/(float)vertices_.size());
  }

  void scaleAndCenterModelVertices()
  {
    float x_max = -9999.0;
    float y_max = -9999.0;
    float z_max = -9990.0;
    float x_min = 9999.0;
    float y_min = 9999.0; 
    float z_min = 9999.0;
    int temp = 0;
    for(int i = 0; i < vertices_.size(); i++)
    {
      temp = vertices_[i].data()[0];
      if (temp > x_max)
        x_max = temp;
      if (temp < x_min)
        x_min = temp;
      
      temp = vertices_[i].data()[1];
      if (temp > y_max)
        y_max = temp;
      if (temp < y_min)
        y_min = temp;
      
      temp = vertices_[i].data()[2];
      if (temp > z_max)
        z_max = temp;
      if (temp < z_min)
        z_min = temp;
    }

    float scale_x = x_max - x_min;
    float scale_y = y_max - y_min;
    float scale_z = z_max - z_min;
    
    max = std::max(scale_x, scale_y);
    max = std::max(max, scale_z);
    std::cout << "Scaling: " << max << std::endl;
    for(int i = 0; i < vertices_.size(); i++)
    {
      vertices_[i].data()[0] = (vertices_[i].data()[0] / max) * 1.25;
      vertices_[i].data()[1] = (vertices_[i].data()[1] / max) * 1.25;
      vertices_[i].data()[2] = (vertices_[i].data()[2] / max) * 1.25;
    }

    calcOrigin();
    std::cout << "Original Origin: " << origin_.x() << " " << origin_.y() << " " << origin_.z() << " " << std::endl;
    std::vector<Eigen::Vector3f> newVertices_ = std::vector<Eigen::Vector3f>();
    for(int i = 0; i < vertices_.size(); i++)
    {
      float x = vertices_[i].data()[0];
      float y = vertices_[i].data()[1];
      float z = vertices_[i].data()[2];
      newVertices_.push_back(*new Eigen::Vector3f(x-origin_.x(), y-origin_.y(), z-origin_.z()));
    }
    vertices_ = newVertices_;

    calcOrigin();
    std::cout << "New Origin: " << origin_.x() << " " << origin_.y() << " " << origin_.z() << " " << std::endl;
  }

  void print_info()
  {
    std::cout << "Triangle Count: " << faces_.size() * 2 << std::endl;
    std::cout << "Vertex Count: " << vertices_.size() << std::endl;
  }

  // I think I've misunderstood the scaling part of the assignment.
  // This is a fix for writing out the "Origin" vertices for the 
  // object printing.
  void undoScaling()
  {
    for(int i = 0; i < vertices_.size(); i++)
    {
      vertices_[i].data()[0] = (vertices_[i].data()[0] / 1.25) * max;
      vertices_[i].data()[1] = (vertices_[i].data()[1] / 1.25) * max;
      vertices_[i].data()[2] = (vertices_[i].data()[2] / 1.25) * max;
    }
  }

  void redoScaling()
  {
   for(int i = 0; i < vertices_.size(); i++)
    {
      vertices_[i].data()[0] = (vertices_[i].data()[0] / max) * 1.25;
      vertices_[i].data()[1] = (vertices_[i].data()[1] / max) * 1.25;
      vertices_[i].data()[2] = (vertices_[i].data()[2] / max) * 1.25;
    } 
  }

  void print(std::string outputFileName)
  {
    std::cout << std::endl << "Writing model to: " << outputFileName << std::endl;

    undoScaling();

    std::ofstream outFile;
    outFile.open(outputFileName);
    
    int i;
    for(i = 0; i < vertices_.size(); i++)
    {
      outFile << std::fixed << "v " << vertices_[i][0] << " " << vertices_[i][1] << " " << vertices_[i][2] << std::endl;
    }

    for(i = 0; i < vt_.size(); i++)
    {
      outFile << std::fixed << "vt " << vt_[i][0] << " " << vt_[i][1] << std::endl;
    }

    for(i = 0; i < vn_.size(); i++)
    {
      outFile << std::fixed << "vn " << vn_[i][0] << " " << vn_[i][1] << " " << vn_[i][2] << std::endl;
    }

    for(i = 0; i < faces_.size(); i++)
    {
      outFile << std::fixed << "f " << (int)faces_[i][0] << "/" << (int)textures_[i][0] << "/" <<  (int)normals_[i][0] << " " 
      << (int)faces_[i][1] << "/" << (int)textures_[i][1] << "/" <<  (int)normals_[i][1] << " " 
      << (int)faces_[i][2] << "/" << (int)textures_[i][2] << "/" <<  (int)normals_[i][2] << std::endl;
    }

    outFile.close(); 

    redoScaling();

    std::cout << "File Written." << std::endl;
  }

  void translateLeft()
  {
    translation_.x() = translation_.x() - 0.1;
  }

  void translateRight()
  {
    translation_.x() = translation_.x() + 0.1;
  }

  void translateUp()
  {
    translation_.y() = translation_.y() + 0.1;
  }

  void translateDown()
  {
    translation_.y() = translation_.y() - 0.1;
  }

  void translateIn()
  {
    translation_.z() = translation_.z() + 0.1;
  }

  void translateOut()
  {
    translation_.z() = translation_.z() - 0.1;
  }

  Eigen::Quaternion<float, 0> createQuaternion(float xAngleDegree, float yAngleDegree, float zAngleDegree)
  {
    Eigen::Quaternion<float, 0> q;
    
    q = Eigen::AngleAxisf(xAngleDegree*(M_PI/180), Eigen::Vector3f::UnitX())
    * Eigen::AngleAxisf(yAngleDegree*(M_PI/180),  Eigen::Vector3f::UnitY())
    * Eigen::AngleAxisf(zAngleDegree*(M_PI/180), Eigen::Vector3f::UnitZ());

    return q;
  }

  void rotationLeft()
  {
    orientation_ = createQuaternion(0.0, -10.0, 0.0) * orientation_;
    orientation_.normalize();
  }

  void rotationRight()
  {
    orientation_ = createQuaternion(0.0, 10.0, 0.0) * orientation_;
    orientation_.normalize();
  }

  void rotationUp()
  {
    orientation_ = createQuaternion(0.0, 0.0, -10.0) * orientation_;
    orientation_.normalize();
  }

  void rotationDown()
  {
    orientation_ = createQuaternion(0.0, 0.0, 10.0) * orientation_;
    orientation_.normalize();
  }

  void rotationCounterClockwise()
  {
    orientation_ = createQuaternion(10.0, 0.0, 0.0) * orientation_;
    orientation_.normalize();
  }

  void rotationClockwise()
  {
    orientation_ = createQuaternion(-10.0, 0.0, 0.0) * orientation_;
    orientation_.normalize();
  }

  void keyInput(unsigned char key, int x, int y)
  {
    switch (key) 
   {
      // Translations
      case 'n':
        translateIn();
        break;
      case 'N':
        translateOut();
        break;
      // Rotations
      case 'y':
        rotationLeft();    
        break;   
      case 'Y':
        rotationRight();
        break;
      case 'p':
        rotationUp();
        break;
      case 'P':
        rotationDown();
        break;
      case 'r':
        rotationCounterClockwise();
        break;
      case 'R':
        rotationClockwise();
        break;
      case 's':
        sToggle++;
        sToggle%=4;
        break;
      }
  }

  void specialKeyInput(int key, int x, int y)
  {
    
    switch (key)
    {
      //translations
      case (GLUT_KEY_LEFT):
        translateLeft();    
        break;   
      case (GLUT_KEY_RIGHT):
        translateRight();
        break;
      case (GLUT_KEY_UP):
        translateUp();
        break;
      case (GLUT_KEY_DOWN):
        translateDown();
        break;
      default:
        break;
    }
  }

};