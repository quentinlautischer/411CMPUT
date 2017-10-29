#pragma once

#include "bvhTreeNode.h"

class Animation
{
public:
  std::unique_ptr<BVHTreeNode> tree_ = nullptr;
  int frames_ = 0;
  float frameTime_ = 0.0;
  std::vector<std::vector<float>> motion_;

  Animation(BVHTreeNode *tree)
  {
    tree_.reset(tree);
    motion_ = std::vector<std::vector<float>>();
  }

  Animation()
  {
    motion_ = std::vector<std::vector<float>>();
  }

  void setTree(BVHTreeNode *tree)
  {
    tree_.reset(tree);
  }

  void printToBVH(std::string outputFileName)
  {
    std::ofstream outFile;
    outFile.open(outputFileName);

    printHierarchy(outFile);
    printMotion(outFile);

    outFile.close();
  }

  void printMotion(std::ostream &s)
  {
    s << "MOTION" << std::endl;
    s << "Frames: " << frames_ << std::endl;
    s << "Frame Time: " << frameTime_ << std::endl;
    for (int i = 0; i < motion_.size(); i++)
    {
      for (int j = 0; j < motion_.at(i).size(); j++)
      {
        if (j != 0)
           s << " ";
        s << std::setprecision(4) << std::fixed << motion_.at(i).at(j);
      }
      s << std::endl;
    }

  }

  void printHierarchy(std::ostream &s)
  {
    s << "HIERARCHY" << std::endl;

    int nodeDepth = 0;
    tree_->enumerate<std::function<void(BVHTreeNode*)>,std::function<void(BVHTreeNode*)>>
    ([&](BVHTreeNode *node)
    {
        nodeDepth += 1;
        node->printName(s, nodeDepth);
        printIndentation(s, nodeDepth-1);
        s << "{" << std::endl;
        node->printDetail(s, nodeDepth);
    },
    [&](BVHTreeNode *node)
    {
      nodeDepth -= 1;
      printIndentation(s, nodeDepth);
      s << "}" << std::endl;
    });
  }

};