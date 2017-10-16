#pragma once

#include "animation.h"

class BVHLoader
{
public:
  BVHLoader() {}

  std::unique_ptr<Animation> load(std::string obj_path)
  {
    FILE* obj_file_path = nullptr;

    std::unique_ptr<Animation> animation = std::make_unique<Animation>();
    BVHTreeNode* currentNode = new BVHTreeNode();

    obj_file_path = fopen(obj_path.c_str(), "r");

    if (obj_file_path == nullptr)
    {
      std::cout << "Object file path returned null" << std::endl;
      return nullptr;
    }

    while(true)
    {
      char lineHeader[256];
      char name[256];
      int res = fscanf(obj_file_path, "%s", lineHeader);
      if (res == EOF)
        break;
      else if (strcmp(lineHeader, "MOTION") == 0)
      {
        int channelTotal = 0;
        int frames = 0;
        float frameTime = 0.0;

        animation->tree_->enumerate<std::function<void(BVHTreeNode*)>>([&channelTotal](BVHTreeNode *node)
        {
          channelTotal += node->channels_.size();
        });

        fscanf(obj_file_path, "%s %d", name, &animation->frames_);
        fscanf(obj_file_path, "%s %s %f", name, name, &animation->frameTime_);

        float rot = 0;
        std::vector<float> rotationList = std::vector<float>(); 
        while (fscanf(obj_file_path, "%f", &rot) != EOF)
        {
          rotationList.push_back(rot);

          if (rotationList.size() == channelTotal)
          {
            animation->motion_.push_back(rotationList);
            rotationList = std::vector<float>();
          }
        }
      }
      if (strcmp(lineHeader, "ROOT") == 0)
      {
        fscanf(obj_file_path, "%s\n", name);
        currentNode->setName(name);
        currentNode->type_ = "ROOT"; 
        animation->setTree(currentNode);
        currentNode = animation->tree_.get();
      }
      else if (strcmp(lineHeader, "JOINT") == 0)
      {
        currentNode = currentNode->addChild(new BVHTreeNode());
        fscanf(obj_file_path, "%s\n", name);
        currentNode->setName(name);
        currentNode->type_ = "JOINT"; 

        std::cout << "Added child: " << currentNode->getName() << " to Parent: " << currentNode->parent_->getName() << std::endl;
      }
      else if (strcmp(lineHeader, "End") == 0)
      {
        fscanf(obj_file_path, "%s\n", name);
        currentNode = currentNode->addChild(new BVHTreeNode());
        currentNode->setName("");
        currentNode->type_ = "End Site"; 
      }
      else if (strcmp(lineHeader, "{") == 0)
      {
        continue;
      }
      else if (strcmp(lineHeader, "}") == 0)
      {
        currentNode = currentNode->parent_;
      }
      else if (strcmp(lineHeader, "OFFSET") == 0)
      {
        std::cout << "CurrentNode Offset: " << currentNode->offset_[0] << ":" << currentNode->offset_[1] << ":" << currentNode->offset_[2] << std::endl;
        fscanf(obj_file_path, "%f %f %f", &currentNode->offset_[0], &currentNode->offset_[1], &currentNode->offset_[2]);
        std::cout << "CurrentNode Offset Updated: " << currentNode->offset_[0] << ":" << currentNode->offset_[1] << ":" << currentNode->offset_[2] << std::endl;
      }
      else if (strcmp(lineHeader, "CHANNELS") == 0)
      {
        int num = 0;
        char channelName[256];
        fscanf(obj_file_path, "%d", &num);
        for(int i = 0; i < num; i++)
        {
          fscanf(obj_file_path, "%s", channelName);
          currentNode->channels_.push_back(std::string(channelName));
        }
      }
    }
    
    animation->printHierarchy(std::cout);

    return animation;
  }
};