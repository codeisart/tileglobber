#include <map>
#include <string>
#include <iostream>
#include <fstream>
#include <experimental/filesystem>
#include <regex>
#include <optional>

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
		out_x 		= stoi(sm[3]);
		out_y 		= stoi(sm[2]);

		return true;
	}

	// fail.
	return false;
}

void discover_files(const fs::path& tilesetPath)
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
		std::cout << ".";
	}
	std::cout << " done." << std::endl;
	return true;
}

bool write_merged(int zoom, const std::string& filename)
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

	unsigned height = 0;
	for(int y = 0; y < max_y; ++y)
	{
		Tile& t = zoomset[{0,y}];
		height+=t.image.height;
	}
	
	unsigned width = 0;
	for(int x = 0; x < max_x; ++x)
	{
		Tile& t = zoomset[{x,0}];
		width+=t.image.width;
	}
	
	cout << "merged pixel dimentions WIDTH=" << width << ", HEIGHT=" << height << endl;
	
	// make new image.
	std::vector<uint8_t> pixels(width*height*4);
	uint8_t* p = pixels.data();

	int pix_y = 0;
	for(int y=0; y<max_y; ++y)
	{
		int pix_x = 0;
		int last_tile_height = 0;
		for(int x=0; x<max_x; ++x)
		{
			Tile& t = zoomset[{x,y}];
			uint8_t* dst = &p[(pix_y*width*4)+(pix_x*4)];
			uint8_t* src = t.image.pixels.data();

			// render tile, 1 scanline at a time.
			for(int v = 0; v < t.image.height; ++v)
			{
				memcpy(dst, src, t.image.width*4);
				dst += width*4;
				src += t.image.width*4;
			}

			pix_x+= t.image.width;
			last_tile_height = t.image.height;
		}
		pix_y+= last_tile_height;
	}
	
        cout << "writing " << filename << "... ";	
	if(auto error = lodepng::encode(filename.c_str(), pixels, width, height))
	{
		cout << "encoder error " << error << ": "<< lodepng_error_text(error) << endl;
		return false;
	}
	cout << "done" << endl;

	// success.
	return true;
}

int main(int argc, const char** argv)
{
	using namespace std;
	if( argc < 3)
	{
		cout << "Args: InputFolder OutputFolder (output-file-prefix) (zoomset)" << endl;
		return -1;
	}			
	
	fs::path inputpath(argv[1]);
	fs::path outputpath(argv[2]);
	string outputprefix = "tile";
	std::optional<int> zoomset;

	if( argc >=3 )
		 outputprefix = argv[3];
	
	if( argc >=4 )
		zoomset = stoi(argv[4]);	

	if(!is_directory(inputpath))
	{
		cout << inputpath << " is not a directory." << endl;
		return -1;
	}
	if(!is_directory(outputpath))
	{
		cout << outputpath << " is not a directory." << endl;
		return -1;
	}

	// find all sets.
	discover_files(inputpath);

	// process each zoomset.
	for(auto& i : gTiles)
	{	
		int zoom = i.first;	
		if(zoomset && *zoomset != zoom )
			continue;

		load_tileset(zoom);

		stringstream ss;
		ss << outputprefix << zoom << ".png";

		fs::path tileoutput = outputpath;		
		tileoutput /= ss.str();
		write_merged(zoom,  tileoutput);			// Supply, or default.
	}

	// Success.
	return 0;
}
