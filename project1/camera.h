#include <Eigen/Geometry>

class Camera
{
public:
  Eigen::Translation<float,3> origin_;
  Eigen::Translation<float,3> translation_;
  Eigen::Quaternion<float, 0> orientation_;
  Camera()
  {
    reset();
  }

  void printInfo()
  {
    std::cout << std::endl << "Camera Info:" << std::endl;
    std::cout << "Origin: X: " << origin_.x() << " Y: " << origin_.y() << " Z: " << origin_.z() << std::endl;
    std::cout << "Translation: X: " << translation_.x() << " Y: " << translation_.y() << " Z: " << translation_.z() << std::endl;
    std::cout << "Orientation: X: " << orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[0] * (180/M_PI) << 
    " Y: " << orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[1] * (180/M_PI) << 
    " Z: " << orientation_.toRotationMatrix().eulerAngles(2, 1, 0)[2] * (180/M_PI) << std::endl;
  }

  void reset()
  {
    origin_ = Eigen::Translation<float, 3>(0,0,0);
    translation_ = Eigen::Translation<float, 3>(0, 0, 0);
    orientation_ = Eigen::AngleAxisf(0, Eigen::Vector3f::UnitX())
    * Eigen::AngleAxisf(0,  Eigen::Vector3f::UnitY())
    * Eigen::AngleAxisf(0, Eigen::Vector3f::UnitZ());
  }

  void translateLeft()
  {
    translation_.x() = translation_.x() - 0.1;
    origin_.x() = origin_.x() - 0.1;
  }

  void translateRight()
  {
    translation_.x() = translation_.x() + 0.1;
    origin_.x() = origin_.x() + 0.1;
  }

  void translateUp()
  {
    translation_.y() = translation_.y() + 0.1;
    origin_.y() = origin_.y() + 0.1;
  }

  void translateDown()
  {
    translation_.y() = translation_.y() - 0.1;
    origin_.y() = origin_.y() - 0.1;
  }

  void translateIn()
  {
    translation_.z() = translation_.z() - 0.1;
    origin_.z() = origin_.z() - 0.1;
  }

  void translateOut()
  {
    translation_.z() = translation_.z() + 0.1;
    origin_.z() = origin_.z() + 0.1;
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
    orientation_ = createQuaternion(0.0, 1.0, 0.0) * orientation_;
    orientation_.normalize();
  }

  void rotationRight()
  {
    orientation_ = createQuaternion(0.0, -1.0, 0.0) * orientation_;
    orientation_.normalize();
  }

  void rotationUp()
  {
    orientation_ = createQuaternion(0.0, 0.0, -1.0) * orientation_;
    orientation_.normalize();
  }

  void rotationDown()
  {
    orientation_ = createQuaternion(0.0, 0.0, 1.0) * orientation_;
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
      case 'd':
        translateLeft();    
        break;   
      case 'D':
        translateRight();
        break;
      case 'c':
        translateUp();
        break;
      case 'C':
        translateDown();
        break;
      case 'z':
        translateIn();
        break;
      case 'Z':
        translateOut();
        break;
      // Rotations
      case 'a':
        rotationLeft();    
        break;   
      case 'A':
        rotationRight();
        break;
      case 't':
        rotationUp();
        break;
      case 'T':
        rotationDown();
        break;
      case 'l':
        rotationCounterClockwise();
        break;
      case 'L':
        rotationClockwise();
        break;
      }
  }
};