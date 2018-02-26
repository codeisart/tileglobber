#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <regex>

namespace fs = std::experimental::filesystem;

// opening:
// discover all tile sets in folder.
// find all files that are png
// open png into memory
// analize tileset and get dimentions

// merge:
// create in memory image of the map to save
// traverse tiles in order and paste contents into main image
// save map


bool parse_filename(const fs::directory_entry& in_path, int& out_x, int& out_y, int& out_zoom)
{
	using namespace std;
	regex r("(\\d+)x(\\d+)x(\\d+)"); 
	string s = in_path.path().filename();
	
	smatch sm;	
	regex_search(s,sm, r);

	if(sm.size() == 4)
	{
		out_zoom 	= stoi(sm[1]);
		out_x 		= stoi(sm[2]);
		out_y 		= stoi(sm[3]);

		//for(int i=0; i < sm.size(); i++)
		//	cout << i << " " << sm[i] << endl;

		// succes.
		return true;
	}

	// fail.
	return false;
}

void discover_files(const std::string& tilesetPath)
{
	using namespace std;
	static const string kPng(".png");
	for(auto& p : fs::directory_iterator(tilesetPath.c_str()))
	{
		if( fs::path(p).extension() == kPng )
		{
			int x,y,zoom;
			parse_filename(p,x,y,zoom);
			cout << "filename=" << p << ", zoom=" << zoom << ", x=" << x << ", y=" << y << endl;
		}		 
	}
}

int main(int argc, const char** argv)
{
	using namespace std;
	if( argc < 2)
	{
		cout << "Please supply a folder containing the tile-set" << endl;;
		return -1;
	}		

	discover_files(argv[1]);

	




	// Success.
	return 0;
}
