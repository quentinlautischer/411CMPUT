#pragma once

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

class Model
{
private:
  float max = 0;
public:
  std::vector<Eigen::Vector3f> vertices_;
  std::vector<Eigen::Vector4f> faces_;
  Eigen::Translation<float,3> translation_;
  Eigen::Quaternion<float, 0> orientation_;
  Eigen::Translation<float, 3> origin_;


  Model()
  {
    vertices_ = std::vector<Eigen::Vector3f>();
    faces_ = std::vector<Eigen::Vector4f>();

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

  void add_face(const Eigen::Vector4f &f)
  {
    faces_.push_back(f);
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

    for(i = 0; i < faces_.size(); i++)
    {
      if (faces_[i][3] != -1)
      {
        outFile << std::fixed << "f " << (int)faces_[i][0] << " " << (int)faces_[i][1] << " " << (int)faces_[i][2] << " " << (int)faces_[i][3] << std::endl;
      }
      else
      {
        outFile << std::fixed << "f " << (int)faces_[i][0] << " " << (int)faces_[i][1] << " " << (int)faces_[i][2] << std::endl;
      }
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