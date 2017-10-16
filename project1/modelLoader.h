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
        int a, b, c;
        int d = -1;
        fscanf(obj_file_path, "%d %d %d %d\n", &a, &b, &c, &d);
        model->add_face(*new Eigen::Vector4f(a, b, c, d));
      }

    }

    return model.release();
  }
};