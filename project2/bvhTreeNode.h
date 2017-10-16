#pragma once
#include <list>
void printIndentation(std::ostream &s, int level)
{
  for (int i = 0; i < level; i++)
  {
    s << "\t";
  }
}

class BVHTreeNode
{
private:
  std::string name_;
public:
  std::string type_;
  Eigen::Vector3f offset_;
  Eigen::Vector3f location_;
  std::list<std::string> channels_;

  std::list<BVHTreeNode*> children_;
  BVHTreeNode* parent_ = nullptr;

  BVHTreeNode()
  {
    children_ = std::list<BVHTreeNode*>();
    channels_ = std::list<std::string>();
    offset_ = Eigen::Vector3f(0.0, 0.0, 0.0);
    location_ = Eigen::Vector3f(0.0, 0.0, 0.0);
  }

  BVHTreeNode* addChild(BVHTreeNode *node)
  {
    children_.emplace_back(node);
    children_.back()->parent_ = this;
    return (children_.back());
  }

  void printName(std::ostream &s, int indentationLevel)
  {
    printIndentation(s, indentationLevel-1);

    s << type_;
    if (name_ != "")
      s << " " << name_; 
    s << std::endl;
  }

  void setName(const std::string &name)
  {
    name_ = name;
  }

  std::string getName()
  {
    return name_;
  }

  void printDetail(std::ostream &s, int indentationLevel)
  {
    printIndentation(s, indentationLevel);
    s << std::setprecision(5) << std::fixed << "OFFSET " << offset_[0] << " " << offset_[1] << " " << offset_[2] << std::endl;
    if (channels_.size() > 0)
    {
      printIndentation(s, indentationLevel);
      s << "CHANNELS " << channels_.size();
      // for(int i = 0; i < channels_.size(); i++)
      // {
      //   s << " " << channels_[i];
      // }
      for(std::string str : channels_)
      {
        s << " " << str;
      }
      s << std::endl;
    }
  }

  template<typename Pred>
  void enumerate(Pred pred)
  {
    BVHTreeNode *node = this;
    pred(this);
    // for (int i = 0; i < children_.size(); i++)
    // {
    //   children_.at(i).enumerate(pred);
    // }
    for (BVHTreeNode *node : children_)
    {
      node->enumerate(pred);
    }
  }

  template<typename Pred, typename Post>
  void enumerate(Pred pred, Post post)
  {
    BVHTreeNode *node = this;
    pred(this);
    // for (int i = 0; i < children_.size(); i++)
    // {
    //   children_.at(i).enumerate(pred, post);
    // }
    for (BVHTreeNode *node : children_)
    {
      node->enumerate(pred, post);
    }
    post();
  }

};