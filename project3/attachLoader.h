#pragma once

#include "attach.h"

class AttachLoader
{
private:
  FILE * attach_file_path = nullptr;
  std::unique_ptr<Attach> attach = nullptr;

public:
  AttachLoader()
  {
  }

  Attach* load(std::string filepath)
  {
  	attach = std::make_unique<Attach>();
    attach_file_path = fopen(filepath.c_str(), "r");

    if (attach_file_path == nullptr)
    {
      std::cout << "Object file path returned null" << std::endl;
      return nullptr;
    }

    while(true)
    {
      std::vector<std::pair<int, double>> attachments = std::vector<std::pair<int, double>>();

	  int vnum;
	  int b[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	  float w[10] = {-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};
	  int res =fscanf(attach_file_path, 
	  	"%d (%d, %f) (%d, %f) (%d, %f) (%d, %f) (%d, %f) (%d, %f) (%d, %f) (%d, %f) (%d, %f) (%d, %f)\n",
	   &vnum, &b[0], &w[0], &b[1], &w[1], &b[2], &w[2], &b[3], &w[3], &b[4], &w[4], &b[5], &w[5], &b[6], 
	   &w[6], &b[7], &w[7], &b[8], &w[8], &b[9], &w[9]);
	  if (res == EOF)
	    break;

	  for(int i=0; i < 10; i++)
	  {
	  	if (b[i] == -1)
	  		break;
	  	attachments.push_back(std::pair<int,double>(b[i], w[i]));
	  	std::cout << "Added bone weight" << std::endl;
	  }
	  attach->add_vertex(vnum, attachments);
	}


    return attach.release();
  }
};