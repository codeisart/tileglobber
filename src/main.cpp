#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <regex>

#include "lodepng.h"

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

struct Png
{
	std::vector<uint8_t> pixels;
	unsigned width = 0;
       	unsigned height = 0;

	bool load(const std::string filename)
	{
		if(auto error = lodepng::decode(pixels, width, height, filename.c_str()))
		{
			std::cout << "decoder error " << error << ": " << lodepng_error_text(error) << std::endl;
			return false;
		}
		return true;
	}
};

struct Tile
{
	fs::directory_entry filename;
	Png image;

	bool load() { return image.load(filename.path().string() ); }
};

// ZOOM -> (X,Y) -> (TILE).
using TileMap = std::map<int, std::map<std::pair<int,int>,Tile>>;
TileMap gTiles;

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
			gTiles[zoom][{x,y}] = Tile { p };

		}		 
	}

	cout << "discovered:" << endl;	
	for(const auto& i : gTiles)
	{
		cout << "zoom: "  << i.first << ", tiles: " << i.second.size() << endl;
	}
}

bool load_tileset(int zoom)
{
	std::cout << "loading zoom " << zoom << " ";
	auto& zoomset = gTiles[zoom];
	for(auto& i : zoomset)
	{
		Tile& t = i.second;
		if(!t.load())
			return false;
		//std::cout << t.image.width << ", " << t.image.height << std::endl;
		//std::cout << ".";
	}
	std::cout << " done." << std::endl;
	return true;
}

void write_merged(int zoom, const std::string& filename)
{
	using namespace std;
	
	auto& zoomset = gTiles[zoom];

	// calculate extents.
	int max_x=1;
       	int max_y=1;
	for(auto& i : zoomset)
	{
		max_x = std::max(i.first.first, max_x);
		max_y = std::max(i.first.second, max_y);
	}
	cout << "tilezet zoom " << zoom << " tile extents are X=" << max_x << ", Y=" << max_y << endl;

	unsigned width = 0;
	unsigned height = 0;
	for(int y = 0; y < max_y; ++y)
	{
		Tile& t = zoomset[{0,y}];
		width+=t.image.width;
		height+=t.image.height;
	}

	cout << "merged pixel dimentions WIDTH=" << width << ", HEIGHT=" << height << endl;

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
	load_tileset(2);		// Supply, or calculate top zoom level.
	write_merged(2, "output.png");	// Supply, or default.

	// Success.
	return 0;
}
