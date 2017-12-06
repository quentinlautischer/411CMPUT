#pragma once

class Attach
{
	std::map<int, std::vector<std::pair<int, double>>> attachments;

public:
	Attach()
	{
		attachments = std::map<int, std::vector<std::pair<int, double>>>();
	};

	void add_vertex(int vertex_num, std::vector<std::pair<int, double>> atts)
	{
		attachments.emplace(vertex_num, atts);
	}

	void print(std::string outputFileName)
	{
		std::cout << std::endl << "Writing model to: " << outputFileName << std::endl;

    	std::ofstream outFile;
    	outFile.open(outputFileName);

    	std::cout << "Attach data structure size: " << attachments.size() << std::endl;
    	for(auto it = attachments.begin(); it != attachments.end(); it++)
    	{
    		outFile << it->first;
    		for(auto itt = it->second.begin(); itt != it->second.end(); itt++)
    		{
    			outFile << " (" << itt->first << "," << itt->second << ")";
    		}
    		outFile << " " << std::endl;
    	}
    
    
    	outFile.close(); 

    	std::cout << "File Written." << std::endl;
	}

};