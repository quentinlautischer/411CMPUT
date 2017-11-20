#include "model.h"

class ModelLoader
{
private:
  FILE * obj_file_path = nullptr;
  std::unique_ptr<Model> model = nullptr;

public:
  ModelLoader()
  {
  }

  Model* load(std::string obj_path)
  {
    model = std::make_unique<Model>();
    obj_file_path = fopen(obj_path.c_str(), "r");

    if (obj_file_path == nullptr)
    {
      std::cout << "Object file path returned null" << std::endl;
      return nullptr;
    }

    while(true)
    {
      char lineHeader[128];

      int res = fscanf(obj_file_path, "%s", lineHeader);
      if (res == EOF)
        break;

      if (strcmp(lineHeader, "v") == 0)
      {
        float x, y, z;
        fscanf(obj_file_path, "%f %f %f\n", &x, &y, &z);
        model->add_vertex(*new Eigen::Vector3f(x, y, z));
      }

      if (strcmp(lineHeader, "f") == 0)
      {
        int f1, t1, n1, f2, t2, n2, f3, t3, n3;
        fscanf(obj_file_path, "%d/%d/%d %d/%d/%d %d/%d/%d \n", &f1, &t1, &n1, &f2, &t2, &n2, &f3, &t3, &n3);
        model->add_face(*new Eigen::Vector3f(f1, f2, f3));
        model->add_face_t(*new Eigen::Vector3f(t1, t2, t3));
        model->add_face_n(*new Eigen::Vector3f(n1, n2, n3));
        std::cout << "#####" << std::endl;
        std::cout << f1 << " " << f2 << " " << f3 << std::endl;
        std::cout << t1 << " " << t2 << " " << t3 << std::endl;
        std::cout << n1 << " " << n2 << " " << n3 << std::endl;
      }

      if (strcmp(lineHeader, "vt") == 0)
      {
        float x, y;
        fscanf(obj_file_path, "%f %f\n", &x, &y);
        model->add_vt(*new Eigen::Vector2f(x, y));
      }

      if (strcmp(lineHeader, "vn") == 0)
      {
        float x, y, z;
        fscanf(obj_file_path, "%f %f %f\n", &x, &y, &z);
        model->add_vn(*new Eigen::Vector3f(x, y, z));
      }

    }

    return model.release();
  }
};