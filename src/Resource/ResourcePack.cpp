/*
 * Copyright 2010-2013 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "ResourcePack.h"
#include "../Engine/Palette.h"
#include "../Engine/Font.h"
#include "../Engine/Surface.h"
#include "../Engine/SurfaceSet.h"
#include "../Engine/Music.h"
#include "../Geoscape/Globe.h"
#include "../Geoscape/Polygon.h"
#include "../Geoscape/Polyline.h"
#include "../Engine/SoundSet.h"
#include "../Engine/Sound.h"
#include "../Engine/RNG.h"
#include "../Engine/Options.h"
#include "../Engine/CrossPlatform.h"
#include "../Engine/GMCat.h"
#include "../Engine/ShaderDraw.h"
#include "../Engine/ShaderMove.h"
#include "../Engine/Logger.h"
#include "../Engine/Exception.h"
#include "../Ruleset/MapDataSet.h"
#include "../Ruleset/ExtraSprites.h"
#include "../Ruleset/ExtraSounds.h"
#include "../Interface/TextButton.h"
#include "../Interface/Window.h"

namespace OpenXcom
{

namespace
{
	
struct HairBleach
{
	static const Uint8 ColorGroup = 15<<4;
	static const Uint8 ColorShade = 15;

	static const Uint8 Hair = 9 << 4;
	static const Uint8 Face = 6 << 4;
	static inline void func(Uint8& src, const Uint8& cutoff, int, int, int)
	{
		if(src > cutoff && src <= Face + 15)
		{
			src = Hair + (src & ColorShade) - 6; //make hair color like male in xcom_0.pck
		}
	}
};

}

/**
 * Initializes a blank resource set pointing to a folder.
 */
ResourcePack::ResourcePack() : _palettes(), _fonts(), _surfaces(), _sets(), _sounds(), _polygonsLand(), _polygonsWater(), _polylines(), _musics()
{
	_muteMusic = new Music();
	_muteSound = new Sound();
}

/**
 * Deletes all the loaded resources.
 */
ResourcePack::~ResourcePack()
{
	delete _muteMusic;
	delete _muteSound;
	for (std::map<std::string, Font*>::iterator i = _fonts.begin(); i != _fonts.end(); ++i)
	{
		delete i->second;
	}
	for (std::map<std::string, Surface*>::iterator i = _surfaces.begin(); i != _surfaces.end(); ++i)
	{
		delete i->second;
	}
	for (std::map<std::string, SurfaceSet*>::iterator i = _sets.begin(); i != _sets.end(); ++i)
	{
		delete i->second;
	}
	for (std::list<Polygon*>::iterator i = _polygonsLand.begin(); i != _polygonsLand.end(); ++i)
	{
		delete *i;
	}
	for (std::list<Polygon*>::iterator i = _polygonsWater.begin(); i != _polygonsWater.end(); ++i)
	{
		delete *i;
	}
	for (std::list<Polyline*>::iterator i = _polylines.begin(); i != _polylines.end(); ++i)
	{
		delete *i;
	}
	for (std::map<std::string, Palette*>::iterator i = _palettes.begin(); i != _palettes.end(); ++i)
	{
		delete i->second;
	}
	for (std::map<std::string, Music*>::iterator i = _musics.begin(); i != _musics.end(); ++i)
	{
		delete i->second;
	}
	for (std::map<std::string, SoundSet*>::iterator i = _sounds.begin(); i != _sounds.end(); ++i)
	{
		delete i->second;
	}
}

/**
 * Returns a specific font from the resource set.
 * @param name Name of the font.
 * @return Pointer to the font.
 */
Font *ResourcePack::getFont(const std::string &name) const
{
	std::map<std::string, Font*>::const_iterator i = _fonts.find(name);
	if (_fonts.end() != i) return i->second; else return 0;
}

/**
 * Returns a specific surface from the resource set.
 * @param name Name of the surface.
 * @return Pointer to the surface.
 */
Surface *ResourcePack::getSurface(const std::string &name) const
{
	std::map<std::string, Surface*>::const_iterator i = _surfaces.find(name);
	if (_surfaces.end() != i) return i->second; else return 0;
}

/**
 * Returns a specific surface set from the resource set.
 * @param name Name of the surface set.
 * @return Pointer to the surface set.
 */
SurfaceSet *ResourcePack::getSurfaceSet(const std::string &name) const
{
	std::map<std::string, SurfaceSet*>::const_iterator i = _sets.find(name);
	if (_sets.end() != i) return i->second; else return 0;
}

/**
 * Returns the list of land terrain polygons in the resource set.
 * @return Pointer to the list of land polygons.
 */
std::list<Polygon*> *ResourcePack::getPolygonsLand()
{
	return &_polygonsLand;
}

/**
 * Returns the list of water terrain polygons in the resource set.
 * @return Pointer to the list of water polygons.
 */
std::list<Polygon*> *ResourcePack::getPolygonsWater()
{
	return &_polygonsWater;
}

/**
 * Returns the list of polylines in the resource set.
 * @return Pointer to the list of polylines.
 */
std::list<Polyline*> *ResourcePack::getPolylines()
{
	return &_polylines;
}

/**
 * Returns a specific music from the resource set.
 * @param name Name of the music.
 * @return Pointer to the music.
 */
Music *ResourcePack::getMusic(const std::string &name) const
{
	if (Options::getBool("mute"))
	{
		return _muteMusic;
	}
	else
	{
		std::map<std::string, Music*>::const_iterator i = _musics.find(name);
		if (_musics.end() != i) return i->second; else return 0;
	}
}

/**
 * Returns a random music from the resource set.
 * @param name Name of the music to pick from.
 * @return Pointer to the music.
 */
Music *ResourcePack::getRandomMusic(const std::string &name) const
{
	if (Options::getBool("mute"))
	{
		return _muteMusic;
	}
	else
	{
		std::vector<Music*> music;
		for (std::map<std::string, Music*>::const_iterator i = _musics.begin(); i != _musics.end(); ++i)
		{
			if (i->first.find(name) != std::string::npos)
			{
				music.push_back(i->second);
			}
		}
		if (_musics.empty())
			return _muteMusic;
		else
			return music[RNG::generate(0, music.size()-1)];
	}
}

/**
 * Returns a specific sound from the resource set.
 * @param set Name of the sound set.
 * @param sound ID of the sound.
 * @return Pointer to the sound.
 */
Sound *ResourcePack::getSound(const std::string &set, unsigned int sound) const
{
	if (Options::getBool("mute"))
	{
		return _muteSound;
	}
	else
	{
		std::map<std::string, SoundSet*>::const_iterator i = _sounds.find(set);
		if (_sounds.end() != i) return i->second->getSound(sound); else return 0;
	}
}

/**
 * Returns a specific palette from the resource set.
 * @param name Name of the palette.
 * @return Pointer to the palette.
 */
Palette *ResourcePack::getPalette(const std::string &name) const
{
	std::map<std::string, Palette*>::const_iterator i = _palettes.find(name);
	if (_palettes.end() != i) return i->second; else return 0;
}

/**
 * Changes the palette of font graphics in the resource set.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void ResourcePack::setPalette(SDL_Color *colors, int firstcolor, int ncolors)
{
	for (std::map<std::string, Font*>::iterator i = _fonts.begin(); i != _fonts.end(); ++i)
	{
		i->second->getSurface()->setPalette(colors, firstcolor, ncolors);
	}
}

/**
 * Changes the palette of all TFTD related graphics in the resource set.
 * @param colors Pointer to the set of colors.
 * @param firstcolor Offset of the first color to replace.
 * @param ncolors Amount of colors to replace.
 */
void ResourcePack::setPaletteTerror(SDL_Color *colors, int firstcolor, int ncolors)
{
	_sets["TFTD_SCANG.DAT"]->setPalette(colors, firstcolor, ncolors);
	for (std::map<std::string, Surface*>::iterator i = _surfaces.begin(); i != _surfaces.end(); ++i)
	{
		if ((i->first.substr(i->first.length()-3, 3) != "LBM") && (i->first.substr(0, 4) == "TFTD"))
			i->second->setPalette(colors, firstcolor, ncolors);
	}
	for (std::map<std::string, SurfaceSet*>::iterator i = _sets.begin(); i != _sets.end(); ++i)
	{
		if (i->first.substr(0, 4) == "TFTD")
			i->second->setPalette(colors, firstcolor, ncolors);
	}
}

/**
 * Returns the list of voxeldata in the resource set.
 * @return Pointer to the list of voxeldata.
 */
std::vector<Uint16> *ResourcePack::getVoxelData()
{
	return &_voxelData;
}

/**
 * Fills the resource pack with all the resources
 * contained in the original game folder.
 * @param extraSprites List of additional sprites.
 * @param ExtraSounds List of additional sounds.
 * @param gameFolder folder currently loaded.
 * @param game Name of the game resources beind loaded.
 */
void ResourcePack::loadGeoscapeResources(const std::string &gameFolder, const std::string &game)
{
	if (game == "xcom1")
	{
		// Load palettes
		for (int i = 0; i < 5; ++i)
		{
			std::stringstream s1, s2;
			s1 << gameFolder << "GEODATA/PALETTES.DAT";
			s2 << "PALETTES.DAT_" << i;
			_palettes[s2.str()] = new Palette();
			_palettes[s2.str()]->loadDat(CrossPlatform::getDataFile(s1.str()), 256, Palette::palOffset(i));
		}

		// Last 16 colors are a greyish gradient
		SDL_Color color[] = {{140, 152, 148, 0},
							 {132, 136, 140, 0},
							 {116, 124, 132, 0},
							 {108, 116, 124, 0},
							 {92, 104, 108, 0},
							 {84, 92, 100, 0},
							 {76, 80, 92, 0},
							 {56, 68, 84, 0},
							 {48, 56, 68, 0},
							 {40, 48, 56, 0},
							 {32, 36, 48, 0},
							 {24, 28, 32, 0},
							 {16, 20, 24, 0},
							 {8, 12, 16, 0},
							 {3, 4, 8, 0},
							 {3, 3, 6, 0}};
		Uint8 firstcolor = Palette::blockOffset(15);
		SDL_Color *tacticalPalette = _palettes["PALETTES.DAT_4"]->getColors();
		for (int i = 0; i < 16; ++i)
		{
			tacticalPalette[i + firstcolor].r = color[i].r;
			tacticalPalette[i + firstcolor].g = color[i].g;
			tacticalPalette[i + firstcolor].b = color[i].b;
		}

		std::stringstream s1, s2;
		s1 << gameFolder << "GEODATA/BACKPALS.DAT";
		s2 << "BACKPALS.DAT";
		_palettes[s2.str()] = new Palette();
		_palettes[s2.str()]->loadDat(CrossPlatform::getDataFile(s1.str()), 128);

		// Load fonts only once
		if (_fonts.empty())
		{
			Font::loadIndex(CrossPlatform::getDataFile("Language/Font.dat"));

			std::string font[] = {"Big.fnt",
								  "Small.fnt"};

			for (int i = 0; i < 2; ++i)
			{
				std::stringstream s;
				s << "Language/" << font[i];
				if (font[i] == "Big.fnt")
					_fonts[font[i]] = new Font(16, 16, 0);
				else if (font[i] == "Small.fnt")
					_fonts[font[i]] = new Font(8, 9, -1);
				_fonts[font[i]]->getSurface()->loadScr(CrossPlatform::getDataFile(s.str()));
				_fonts[font[i]]->load();
			}
		}

		// Load surfaces
		{
			std::stringstream s;
			s << gameFolder << "GEODATA/" << "INTERWIN.DAT";
			_surfaces["INTERWIN.DAT"] = new Surface(160, 556);
			_surfaces["INTERWIN.DAT"]->loadScr(CrossPlatform::getDataFile(s.str()));
			_surfaces["INTERWIN.DAT"]->setPalette(_palettes["PALETTES.DAT_0"]->getColors());
		}

		std::string scrs[] = {"BACK01.SCR",
							  "BACK02.SCR",
							  "BACK03.SCR",
							  "BACK04.SCR",
							  "BACK05.SCR",
							  "BACK06.SCR",
							  "BACK07.SCR",
							  "BACK08.SCR",
							  "BACK09.SCR",
							  "BACK10.SCR",
							  "BACK11.SCR",
							  "BACK12.SCR",
							  "BACK13.SCR",
							  "BACK14.SCR",
							  "BACK15.SCR",
							  "BACK16.SCR",
							  "BACK17.SCR",
							  "GEOBORD.SCR",
							  "UP_BORD2.SCR"};

		for (int i = 0; i < 19; ++i)
		{
			std::stringstream s;
			s << gameFolder << "GEOGRAPH/" << scrs[i];
			_surfaces[scrs[i]] = new Surface(320, 200);
			_surfaces[scrs[i]]->loadScr(CrossPlatform::getDataFile(s.str()));
			switch (i)
			{
				case 0: case 2: case 3: case 11: case 14: case 15: case 17:
				{
					_surfaces[scrs[i]]->setPalette(_palettes["PALETTES.DAT_0"]->getColors());
					break;
				}
				case 1: case 5: case 6: case 12: case 13: case 16:
				{
					_surfaces[scrs[i]]->setPalette(_palettes["PALETTES.DAT_1"]->getColors());
					break;
				}
				case 4: case 7: case 8: case 9: case 10: case 18:
				{
					_surfaces[scrs[i]]->setPalette(_palettes["PALETTES.DAT_3"]->getColors());
					break;
				}
			}
		}

		// here we create an "alternate" background surface for the base info screen.
		_surfaces["ALTBACK07.SCR"] = new Surface(320, 200);
		_surfaces["ALTBACK07.SCR"]->loadScr(CrossPlatform::getDataFile(gameFolder + "GEOGRAPH/BACK07.SCR"));
		for (int y = 172; y >= 152; --y)
			for (int x = 5; x <= 314; ++x)
				_surfaces["ALTBACK07.SCR"]->setPixel(x, y+4, _surfaces["ALTBACK07.SCR"]->getPixel(x,y));
		for (int y = 147; y >= 134; --y)
			for (int x = 5; x <= 314; ++x)
				_surfaces["ALTBACK07.SCR"]->setPixel(x, y+9, _surfaces["ALTBACK07.SCR"]->getPixel(x,y));
		for (int y = 132; y >= 109; --y)
			for (int x = 5; x <= 314; ++x)
				_surfaces["ALTBACK07.SCR"]->setPixel(x, y+10, _surfaces["ALTBACK07.SCR"]->getPixel(x,y));


		std::string spks[] = {"UP001.SPK",
							  "UP002.SPK",
							  "UP003.SPK",
							  "UP004.SPK",
							  "UP005.SPK",
							  "UP006.SPK",
							  "UP007.SPK",
							  "UP008.SPK",
							  "UP009.SPK",
							  "UP010.SPK",
							  "UP011.SPK",
							  "UP012.SPK",
							  "UP013.SPK",
							  "UP014.SPK",
							  "UP015.SPK",
							  "UP016.SPK",
							  "UP017.SPK",
							  "UP018.SPK",
							  "UP019.SPK",
							  "UP020.SPK",
							  "UP021.SPK",
							  "UP022.SPK",
							  "UP023.SPK",
							  "UP024.SPK",
							  "UP025.SPK",
							  "UP026.SPK",
							  "UP027.SPK",
							  "UP028.SPK",
							  "UP029.SPK",
							  "UP030.SPK",
							  "UP031.SPK",
							  "UP032.SPK",
							  "UP033.SPK",
							  "UP034.SPK",
							  "UP035.SPK",
							  "UP036.SPK",
							  "UP037.SPK",
							  "UP038.SPK",
							  "UP039.SPK",
							  "UP040.SPK",
							  "UP041.SPK",
							  "UP042.SPK",
							  "GRAPHS.SPK"};

		for (int i = 0; i < 43; ++i)
		{
			std::stringstream s;
			s << gameFolder << "GEOGRAPH/" << spks[i];
			_surfaces[spks[i]] = new Surface(320, 200);
			_surfaces[spks[i]]->loadSpk(CrossPlatform::getDataFile(s.str()));
			if (i != 43)
				_surfaces[spks[i]]->setPalette(_palettes["PALETTES.DAT_3"]->getColors());
			else
				_surfaces[spks[i]]->setPalette(_palettes["PALETTES.DAT_2"]->getColors());
		}

		std::string lbms[] = {"PICT1.LBM",
							  "PICT2.LBM",
							  "PICT3.LBM",
							  "PICT4.LBM",
							  "PICT5.LBM",
							  "PICT6.LBM",
							  "PICT7.LBM"};

		for (int i = 0; i < 7; ++i)
		{
			std::stringstream s;
			s << gameFolder << "UFOINTRO/" << lbms[i];
			_surfaces[lbms[i]] = new Surface(320, 200);
			_surfaces[lbms[i]]->loadImage(CrossPlatform::getDataFile(s.str()));
		}
		// Load surface sets
		std::string sets[] = {"BASEBITS.PCK",
							  "INTICON.PCK",
							  "TEXTURE.DAT"};

		for (int i = 0; i < 3; ++i)
		{
			std::stringstream s;
			s << gameFolder << "GEOGRAPH/" << sets[i];

			std::string ext = sets[i].substr(sets[i].length()-3, sets[i].length());
			if (ext == "PCK")
			{
				std::string tab = sets[i].substr(0, sets[i].length()-4) + ".TAB";
				std::stringstream s2;
				s2 << gameFolder << "GEOGRAPH/" << tab;
				_sets[sets[i]] = new SurfaceSet(32, 40);
				_sets[sets[i]]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
				_sets[sets[i]]->setPalette(_palettes["PALETTES.DAT_1"]->getColors());
			}
			else
			{
				_sets[sets[i]] = new SurfaceSet(32, 32);
				_sets[sets[i]]->loadDat(CrossPlatform::getDataFile(s.str()));
				_sets[sets[i]]->setPalette(_palettes["PALETTES.DAT_0"]->getColors());
			}
		}
		_sets["SCANG.DAT"] = new SurfaceSet(4, 4);
		std::stringstream scang;
		scang << gameFolder << "GEODATA/" << "SCANG.DAT";
		_sets["SCANG.DAT"]->loadDat (CrossPlatform::getDataFile(scang.str()));
		_sets["SCANG.DAT"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
		// Load polygons
		std::stringstream s;
		s.str("");
		s << gameFolder << "GEODATA/" << "WORLD.DAT";
		Globe::loadDat(CrossPlatform::getDataFile(s.str()), &_polygonsLand);

		// Load polylines (extracted from game)
		// -10 = Start of line
		// -20 = End of data
		double lines[] = {-10, 1.18901, -0.412334, 1.23918, -0.425424, 1.213, -0.471239, 1.22828, -0.490874, 1.23482, -0.482147, 1.30245, -0.541052, 1.29373,
					  -0.608684, 1.35918, -0.61741, 1.38099, -0.53887, 1.41154, -0.530144, 1.39626, -0.503964, 1.53153, -0.460331, 1.54025, -0.488692,
					  1.55116, -0.490874, 1.55334, -0.466876, 1.60352, -0.469057, 1.59916, -0.488692, 1.67552, -0.517054, 1.69515, -0.475602, 1.61661,
					  -0.386154, 1.61225, -0.436332, 1.56861, -0.440696, 1.56425, -0.460331, 1.54243, -0.462512, 1.53589, -0.449422, 1.55552, -0.373064,
					  -10, 6.13047, -0.726493, 6.17628, -0.726493, 6.1501, -0.645772, -10, 6.25264, -0.759218, 0.0109083, -0.73522, 0.0567232, -0.741765,
					  -10, 0.128718, -0.7614, 0.122173, -0.80067, 0.102538, -0.807215, 0.1309, -0.829031, 0.14399, -0.85303, 0.111265, -0.863938, 0.0719948,
					  -0.870483, 0.0501782, -0.885755, -10, 0.122173, -0.80067, 0.148353, -0.811578, 0.159261, -0.80067, 0.211621, -0.820305, 0.239983,
					  -0.811578, 0.239983, -0.794125, -10, 0.111265, -0.863938, 0.102538, -0.907571, 0.11781, -0.90539, 0.122173, -0.938114, -10, 0.139626,
					  -0.959931, 0.181078, -0.953386, -10, 0.248709, -0.942478, 0.261799, -0.887936, 0.213803, -0.877028, 0.242164, -0.85303, 0.229074,
					  -0.829031, 0.1309, -0.829031, -10, 0.0458149, -0.109083, 0.0479966, -0.148353, 0.0654498, -0.185441, 0.0698132, -0.237801, 0.0981748,
					  -0.244346, 0.122173, -0.224711, 0.17017, -0.222529, 0.231256, -0.235619, 0.257436, -0.211621, 0.19635, -0.113446, 0.176715, -0.126536,
					  0.148353, -0.0763582, -10, 0.438514, -0.554142, 0.436332, -0.383972, 0.595594, -0.383972, 0.628319, -0.410152, -10, 0.59123, -0.547597,
					  0.619592, -0.493056, -10, 0.283616, 0.4996, 0.349066, 0.495237, 0.349066, 0.434151, 0.362156, 0.469057, 0.407971, 0.440696, 0.447241,
					  0.449422, 0.510509, 0.386154, 0.545415, 0.390517, 0.558505, 0.469057, 0.575959, 0.464694, -10, 5.36252, 0.580322, 5.27962, 0.523599,
					  5.34071, 0.449422, 5.27089, 0.386154, 5.26653, 0.283616, 5.14436, 0.174533, 5.05491, 0.194168, 4.996, 0.14399, 5.01564, 0.0872665,
					  5.06364, 0.0763582, 5.06582, -0.0305433, 5.18145, -0.0370882, 5.15527, -0.0698132, 5.2229, -0.0938114, 5.2578, -0.019635, 5.35816,
					  -0.0327249, 5.38652, -0.0741765, -10, 4.10152, -0.85303, 4.45059, -0.85303, 4.62512, -0.855211, 4.71893, -0.837758, -10, 5.116, -0.776672,
					  5.08545, -0.824668, 5.03309, -0.785398, 4.97419, -0.785398, 4.95019, -0.770127, -10, 3.82227, -1.21519, 3.82227, -1.05374, 4.01426,
					  -0.977384, 3.95972, -0.949023, -10, 4.23897, -0.569414, 4.42659, -0.554142, 4.48113, -0.503964, 4.51386, -0.519235, 4.55531, -0.460331,
					  4.59022, -0.455967, -10, 4.82584, -0.728675, 4.84983, -0.750492, -10, 4.8062, -0.81376, 4.82802, -0.80067, -10, 0.545415, -1.21955, 0.549779,
					  -1.09738, 0.490874, -1.05156, -10, 0.488692, -1.04283, 0.490874, -0.981748, 0.569414, -0.933751, 0.554142, -0.909753, 0.698132, -0.863938,
					  0.665407, -0.818123, -10, 0.693768, -0.763582, 0.857393, -0.730857, -10, 0.861756, -0.805033, 0.831213, -0.87921, 1.0472, -0.885755, 1.0712,
					  -0.944659, 1.2021, -0.966476, 1.34172, -0.951204, 1.39626, -0.885755, 1.53589, -0.857393, 1.71042, -0.872665, 1.72569, -0.909753, 1.91986,
					  -0.859575, 2.03767, -0.870483, 2.08131, -0.872665, 2.09658, -0.922843, 2.19693, -0.925025, 2.23184, -0.86612, 2.34747, -0.842121, 2.32129,
					  -0.785398, 2.28638, -0.783217, 2.27984, -0.73522, 2.16857, -0.698132, -10, 1.88277, -0.375246, 1.8435, -0.407971, 1.77587, -0.370882,
					  1.73006, -0.386154, 1.72569, -0.423242, 1.7017, -0.418879, 1.72569, -0.477784, 1.69515, -0.475602, -10, 1.59916, -0.488692, 1.55116,
					  -0.490874, -10, 1.54025, -0.488692, 1.41154, -0.530144, -10, 1.35918, -0.61741, 1.28064, -0.687223, 1.40499, -0.737402, 1.39626, -0.785398,
					  1.4399, -0.78758, 1.44644, -0.824668, 1.49662, -0.822486, 1.50753, -0.857393, 1.53589, -0.857393, 1.5817, -0.789761, 1.67988, -0.746128,
					  1.8326, -0.724312, 1.95477, -0.7614, 1.95695, -0.785398, 2.09221, -0.815941, 2.02022, -0.833395, 2.03767, -0.870483, -20};

		Polyline *l = 0;
		int start = 0;
		for (int i = 0; lines[i] > -19.999; ++i)
		{
			if (lines[i] < -9.999 && lines[i] > -10.001)
			{
				if (l != 0)
				{
					_polylines.push_back(l);
				}
				int points = 0;
				for (int j = i + 1; lines[j] > -9.999; ++j)
				{
					points++;
				}
				points /= 2;
				l = new Polyline(points);
				start = i + 1;
			}
			else
			{
				if ((i - start) % 2 == 0)
				{
					l->setLongitude((i - start) / 2, lines[i]);
				}
				else
				{
					l->setLatitude((i - start) / 2, lines[i]);
				}
			}
		}
		_polylines.push_back(l);

		if (!Options::getBool("mute"))
		{
			// Load musics
			std::string mus[] = {"GMDEFEND",
								 "GMENBASE",
								 "GMGEO1",
								 "GMGEO2",
								 "GMGEO3",
								 "GMGEO4",
								 "GMINTER",
								 "GMINTRO1",
								 "GMINTRO2",
								 "GMINTRO3",
								 "GMLOSE",
								 "GMMARS",
								 "GMNEWMAR",
								 "GMSTORY",
								 "GMTACTIC",
								 "GMTACTIC2",
								 "GMWIN"};
			std::string exts[] = {"flac", "ogg", "mp3", "mod"};
			int tracks[] = {3, 6, 0, 18, -1, -1, 2, 19, 20, 21, 10, 9, 8, 12, 17, -1, 11};

			// Check which music version is available
			bool cat = true;
			GMCatFile *gmcat = 0;

			std::string musDos = gameFolder + "SOUND/GM.CAT";
			if (CrossPlatform::fileExists(CrossPlatform::getDataFile(musDos)))
			{
				cat = true;
				gmcat = new GMCatFile(CrossPlatform::getDataFile(musDos).c_str());
			}
			else
			{
				cat = false;
			}

			for (int i = 0; i < 17; ++i)
			{
				bool loaded = false;
				// Try digital tracks
				for (int j = 0; j < 3; ++j)
				{
					std::stringstream s;
					s << gameFolder << "SOUND/" << mus[i] << "." << exts[j];
					if (CrossPlatform::fileExists(CrossPlatform::getDataFile(s.str())))
					{
						_musics[mus[i]] = new Music();
						_musics[mus[i]]->load(CrossPlatform::getDataFile(s.str()));
						loaded = true;
						break;
					}
				}
				if (!loaded)
				{
					// Try Adlib music
					if (cat && tracks[i] != -1)
					{
						_musics[mus[i]] = gmcat->loadMIDI(tracks[i]);
						loaded = true;
					}
					// Try MIDI music
					else
					{
						std::stringstream s;
						s << gameFolder << "SOUND/" << mus[i] << ".mid";
						if (CrossPlatform::fileExists(CrossPlatform::getDataFile(s.str())))
						{
							_musics[mus[i]] = new Music();
							_musics[mus[i]]->load(CrossPlatform::getDataFile(s.str()));
							loaded = true;
						}
					}
				}
				if (!loaded && tracks[i] != -1)
				{
					throw Exception(mus[i] + " not found");
				}
			}
			delete gmcat;

			// Load sounds
			std::string catsId[] = {"GEO.CAT",
									"BATTLE.CAT"};
			std::string catsDos[] = {"SOUND2.CAT",
									"SOUND1.CAT"};
			std::string catsWin[] = {"SAMPLE.CAT",
									"SAMPLE2.CAT"};

			// Check which sound version is available
			std::string *cats = 0;
			bool wav = true;

			std::stringstream win, dos;
			win << gameFolder << "SOUND/" << catsWin[0];
			dos << gameFolder << "SOUND/" << catsDos[0];
			if (CrossPlatform::fileExists(CrossPlatform::getDataFile(win.str())))
			{
				cats = catsWin;
				wav = true;
			}
			else if (CrossPlatform::fileExists(CrossPlatform::getDataFile(dos.str())))
			{
				cats = catsDos;
				wav = false;
			}

			for (int i = 0; i < 2; ++i)
			{
				if (cats == 0)
				{
					std::stringstream ss;
					ss << catsDos[i] << " not found";
					throw Exception(ss.str());
				}
				else
				{
					std::stringstream s;
					s << gameFolder << "SOUND/" << cats[i];
					_sounds[catsId[i]] = new SoundSet();
					_sounds[catsId[i]]->loadCat(CrossPlatform::getDataFile(s.str()), wav);
				}
			}

			if (CrossPlatform::fileExists(CrossPlatform::getDataFile(gameFolder + "SOUND/INTRO.CAT")))
			{
				SoundSet *s = _sounds["INTRO.CAT"] = new SoundSet();
				s->loadCat(CrossPlatform::getDataFile(gameFolder + "SOUND/INTRO.CAT"), false);
			} else
			{
				Log(LOG_WARNING) << "INTRO.CAT is missing! :(";
			}

			if (CrossPlatform::fileExists(CrossPlatform::getDataFile(gameFolder + "SOUND/SAMPLE3.CAT")))
			{
				SoundSet *s = _sounds["SAMPLE3.CAT"] = new SoundSet();
				wav = true;
				s->loadCat(CrossPlatform::getDataFile(gameFolder + "SOUND/SAMPLE3.CAT"), true);
			} else
			{
				Log(LOG_WARNING) << "SAMPLE3.CAT is missing! :(";
			}

		}

		TextButton::soundPress = getSound("GEO.CAT", 0);
		Window::soundPopup[0] = getSound("GEO.CAT", 1);
		Window::soundPopup[1] = getSound("GEO.CAT", 2);
		Window::soundPopup[2] = getSound("GEO.CAT", 3);

		loadBattlescapeResources(gameFolder, game); // TODO load this at battlescape start, unload at battlescape end?

		//"fix" of hair color of male personal armor
		SurfaceSet *xcom_1 = _sets["XCOM_1.PCK"];

		for(int i=0; i< 16; ++i )
		{
			//cheast frame
			Surface *surf = xcom_1->getFrame(4*8 + i);
			ShaderMove<Uint8> head = ShaderMove<Uint8>(surf);
			GraphSubset dim = head.getBaseDomain();
			surf->lock();
			dim.beg_y = 6;
			dim.end_y = 9;
			head.setDomain(dim);
			ShaderDraw<HairBleach>(head, ShaderScalar<Uint8>(HairBleach::Face+5));
			dim.beg_y = 9;
			dim.end_y = 10;
			head.setDomain(dim);
			ShaderDraw<HairBleach>(head, ShaderScalar<Uint8>(HairBleach::Face+6));
			surf->unlock();
		}

		for(int i=0; i< 3; ++i )
		{
			//fall frame
			Surface *surf = xcom_1->getFrame(264 + i);
			ShaderMove<Uint8> head = ShaderMove<Uint8>(surf);
			GraphSubset dim = head.getBaseDomain();
			dim.beg_y = 0;
			dim.end_y = 24;
			dim.beg_x = 11;
			dim.end_x = 20;
			head.setDomain(dim);
			surf->lock();
			ShaderDraw<HairBleach>(head, ShaderScalar<Uint8>(HairBleach::Face+6));
			surf->unlock();
		}

		// copy constructor doesn't like doing this directly, so let's make a second handobs file the old fashioned way.
		// handob2 is used for all the left handed sprites.
		_sets["HANDOB2.PCK"] = new SurfaceSet(_sets["HANDOB.PCK"]->getWidth(), _sets["HANDOB.PCK"]->getHeight());
		std::map<int, Surface*> *handob = _sets["HANDOB.PCK"]->getFrames();
		for (std::map<int, Surface*>::const_iterator i = handob->begin(); i != handob->end(); ++i)
		{
			(i->second)->blit(_sets["HANDOB2.PCK"]->addFrame(i->first));
		}
	}
	else if (game == "xcom2")
	{
		// Load TFTD palettes
		for (int i = 0; i < 3; ++i)
		{
			std::stringstream s1, s2;
			s1 << gameFolder << "GEODATA/PALETTES.DAT";
			s2 << "PALETTES.DAT_" << i;
			_palettes["TFTD_" + s2.str()] = new Palette();
			_palettes["TFTD_" + s2.str()]->loadDat(CrossPlatform::getDataFile(s1.str()), 256, Palette::palOffset(i));
		}
		Surface *helper = new Surface(320, 200);
		for (int i = 3; i < 7; ++i)
		{
			std::stringstream s1, s2;
			s1 << gameFolder << "UFOGRAPH/D" << i - 3 << ".LBM";
			s2 << "PALETTES.DAT_" << i;
			helper->loadImage(CrossPlatform::getDataFile(s1.str()));
			_palettes["TFTD_" + s2.str()] = new Palette();
			_palettes["TFTD_" + s2.str()]->setPalette(helper->getPalette());
		}
		delete helper;

		std::stringstream s1, s2;
		s1 << gameFolder << "GEODATA/BACKPALS.DAT";
		s2 << "BACKPALS.DAT";
		_palettes["TFTD_" + s2.str()] = new Palette();
		_palettes["TFTD_" + s2.str()]->loadDat(CrossPlatform::getDataFile(s1.str()), 128);

		// Load fonts only once
		if (_fonts.empty())
		{
			Font::loadIndex(CrossPlatform::getDataFile("Language/Font.dat"));

			std::string font[] = {"Big.fnt",
								  "Small.fnt"};

			for (int i = 0; i < 2; ++i)
			{
				std::stringstream s;
				s << "Language/" << font[i];
				if (font[i] == "Big.fnt")
					_fonts[font[i]] = new Font(16, 16, 0);
				else if (font[i] == "Small.fnt")
					_fonts[font[i]] = new Font(8, 9, -1);
				_fonts[font[i]]->getSurface()->loadScr(CrossPlatform::getDataFile(s.str()));
				_fonts[font[i]]->load();
			}
		}

		// Load TFTD surfaces
		{
			std::stringstream s;
			s << gameFolder << "GEODATA/" << "INTERWIN.DAT";
			_surfaces["TFTD_INTERWIN.DAT"] = new Surface(160, 556);
			_surfaces["TFTD_INTERWIN.DAT"]->loadScr(CrossPlatform::getDataFile(s.str()));
			_surfaces["TFTD_INTERWIN.DAT"]->setPalette(_palettes["TFTD_PALETTES.DAT_0"]->getColors());
		}

		std::string scrs[] = {"BACK01.SCR",
							  "BACK02.SCR",
							  "BACK03.SCR",
							  "BACK04.SCR",
							  "BACK05.SCR",
							  "BACK06.SCR",
							  "BACK07.SCR",
							  "BACK08.SCR",
							  "BACK12.SCR",
							  "BACK13.SCR",
							  "BACK14.SCR",
							  "BACK15.SCR",
							  "BACK16.SCR",
							  "BACK17.SCR",
							  "GEOBORD.SCR",
							  "UP_BORD2.SCR"};

		for (int i = 0; i < 16; ++i)
		{
			std::stringstream s;
			s << gameFolder << "GEOGRAPH/" << scrs[i];
			_surfaces["TFTD_" + scrs[i]] = new Surface(320, 200);
			_surfaces["TFTD_" + scrs[i]]->loadScr(CrossPlatform::getDataFile(s.str()));
		}

		// here we create an "alternate" background surface for the base info screen.
		_surfaces["ALTBACK07.SCR"] = new Surface(320, 200);
		_surfaces["ALTBACK07.SCR"]->loadScr(CrossPlatform::getDataFile(gameFolder + "GEOGRAPH/BACK07.SCR"));
		for (int y = 172; y >= 152; --y)
			for (int x = 5; x <= 314; ++x)
				_surfaces["ALTBACK07.SCR"]->setPixel(x, y+4, _surfaces["ALTBACK07.SCR"]->getPixel(x,y));
		for (int y = 147; y >= 134; --y)
			for (int x = 5; x <= 314; ++x)
				_surfaces["ALTBACK07.SCR"]->setPixel(x, y+9, _surfaces["ALTBACK07.SCR"]->getPixel(x,y));
		for (int y = 132; y >= 109; --y)
			for (int x = 5; x <= 314; ++x)
				_surfaces["ALTBACK07.SCR"]->setPixel(x, y+10, _surfaces["ALTBACK07.SCR"]->getPixel(x,y));


		std::string spks[] = {"UP001.BDY",
							  "UP002.BDY",
							  "UP003.BDY",
							  "UP004.BDY",
							  "UP005.BDY",
							  "UP006.BDY",
							  "UP007.BDY",
							  "UP008.BDY",
							  "UP009.BDY",
							  "UP010.BDY",
							  "UP011.BDY",
							  "UP012.BDY",
							  "UP013.BDY",
							  "UP014.BDY",
							  "UP015.BDY",
							  "UP016.BDY",
							  "UP017.BDY",
							  "UP018.BDY",
							  "UP019.BDY",
							  "UP020.BDY",
							  "UP021.BDY",
							  "UP022.BDY",
							  "UP023.BDY",
							  "UP024.BDY",
							  "UP025.BDY",
							  "UP026.BDY",
							  "UP027.BDY",
							  "UP028.BDY",
							  "UP029.BDY",
							  "UP030.BDY",
							  "UP031.BDY",
							  "UP032.BDY",
							  "UP033.BDY",
							  "UP034.BDY",
							  "UP035.BDY",
							  "UP036.BDY",
							  "UP037.BDY",
							  "UP038.BDY",
							  "UP039.BDY",
							  "UP040.BDY",
							  "UP041.BDY",
							  "UP042.BDY",
							  "UP043.BDY",
							  "UP044.BDY",
							  "UP045.BDY",
							  "UP046.BDY",
							  "UP047.BDY",
							  "UP048.BDY",
							  "UP049.BDY",
							  "UP050.BDY",
							  "UP051.BDY",
							  "UP052.BDY",
							  "UP053.BDY",
							  "UP054.BDY",
							  "UP055.BDY",
							  "UP056.BDY",
							  "UP057.BDY",
							  "UP058.BDY",
							  "UP059.BDY",
							  "UP060.BDY",
							  "UP061.BDY",
							  "UP062.BDY",
							  "UP063.BDY",
							  "UP064.BDY",
							  "UP065.BDY",
							  "UP066.BDY",
							  "UP067.BDY",
							  "UP068.BDY",
							  "UP069.BDY",
							  "UP070.BDY",
							  "UP071.BDY",
							  "UP072.BDY",
							  "UP073.BDY",
							  "UP074.BDY",
							  "UP075.BDY",
							  "UP076.BDY",
							  "UP077.BDY",
							  "UP078.BDY",
							  "UP079.BDY",
							  "UP080.BDY",
							  "UP081.BDY",
							  "UP082.BDY",
							  "UP083.BDY",
							  "UP084.BDY",
							  "UP085.BDY",
							  "UP086.BDY",
							  "UP087.BDY",
							  "UP088.BDY",
							  "UP089.BDY",
							  "UP090.BDY",
							  "UP091.BDY",
							  "UP092.BDY",
							  "UP093.BDY",
							  "UP094.BDY",
							  "UP095.BDY",
							  "UP096.BDY",
							  "UP097.BDY",
							  "UP098.BDY",
							  "UP099.BDY",
							  "UP100.BDY",
							  "UP101.BDY",
							  "UP102.BDY",
							  "UP103.BDY",
							  "UP104.BDY",
							  "UP105.BDY",
							  "UP106.BDY",
							  "UP107.BDY",
							  "UP108.BDY",
							  "UP109.BDY",
							  "UP110.BDY",
							  "UP111.BDY",
							  "UP112.BDY",
							  "GRAPH.BDY",
							  "GRAPHS.SPK"};

		for (int i = 0; i < 113; ++i)
		{
			std::stringstream s;
			s << gameFolder << "GEOGRAPH/" << spks[i];
			_surfaces["TFTD_" + spks[i]] = new Surface(320, 200);
			_surfaces["TFTD_" + spks[i]]->loadBdy(CrossPlatform::getDataFile(s.str()));
			_surfaces["TFTD_" + spks[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_1"]->getColors());
		}
		std::stringstream s;
		s << gameFolder << "GEOGRAPH/" << spks[113];
		_surfaces["TFTD_" + spks[113]] = new Surface(320, 200);
		_surfaces["TFTD_" + spks[113]]->loadSpk(CrossPlatform::getDataFile(s.str()));
		_surfaces["TFTD_" + spks[113]]->setPalette(_palettes["TFTD_PALETTES.DAT_2"]->getColors());

/*
		std::string lbms[] = {"PICT1.LBM",
							  "PICT2.LBM",
							  "PICT3.LBM",
							  "PICT4.LBM",
							  "PICT5.LBM",
							  "PICT6.LBM",
							  "PICT7.LBM"};

		for (int i = 0; i < 7; ++i)
		{
			std::stringstream s;
			s << gameFolder << "UFOINTRO/" << lbms[i];
			_surfaces[lbms[i]] = new Surface(320, 200);
			_surfaces[lbms[i]]->loadImage(CrossPlatform::getDataFile(s.str()));
		}*/
		// Load surface sets
		std::string sets[] = {"BASEBITS.PCK",
							  "INTICON.PCK",
							  "TEXTURE.DAT"};

		for (int i = 0; i < 3; ++i)
		{
			std::stringstream s;
			s << gameFolder << "GEOGRAPH/" << sets[i];

			std::string ext = sets[i].substr(sets[i].length()-3, sets[i].length());
			if (ext == "PCK")
			{
				std::string tab = sets[i].substr(0, sets[i].length()-4) + ".TAB";
				std::stringstream s2;
				s2 << gameFolder << "GEOGRAPH/" << tab;
				_sets["TFTD_" + sets[i]] = new SurfaceSet(32, 40);
				_sets["TFTD_" + sets[i]]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
				_sets["TFTD_" + sets[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_0"]->getColors());
			}
			else
			{
				_sets["TFTD_" + sets[i]] = new SurfaceSet(32, 32);
				_sets["TFTD_" + sets[i]]->loadDat(CrossPlatform::getDataFile(s.str()));
				_sets["TFTD_" + sets[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_0"]->getColors());
			}
		}
		_sets["TFTD_SCANG.DAT"] = new SurfaceSet(4, 4);
		std::stringstream scang;
		scang << gameFolder << "GEODATA/" << "SCANG.DAT";
		_sets["TFTD_SCANG.DAT"]->loadDat (CrossPlatform::getDataFile(scang.str()));
		_sets["TFTD_SCANG.DAT"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
		// Load polygons
		s.str("");
		s << gameFolder << "GEODATA/" << "WORLD.DAT";
		Globe::loadDat(CrossPlatform::getDataFile(s.str()), &_polygonsWater);

		// Load polylines (extract from game somehow)

		// Load sounds and music

		loadBattlescapeResources(gameFolder, game);

		// tftd_handob2 is used for all the left handed terror item sprites.
		_sets["TFTD_HANDOB2.PCK"] = new SurfaceSet(_sets["TFTD_HANDOB.PCK"]->getWidth(), _sets["TFTD_HANDOB.PCK"]->getHeight());
		std::map<int, Surface*> *handob = _sets["TFTD_HANDOB.PCK"]->getFrames();
		for (std::map<int, Surface*>::const_iterator i = handob->begin(); i != handob->end(); ++i)
		{
			(i->second)->blit(_sets["TFTD_HANDOB2.PCK"]->addFrame(i->first));
		}
	}
}

void ResourcePack::loadBattlescapeResources(const std::string &gameFolder, const std::string &game)
{
	if (game == "xcom1")
	{
		// Load Battlescape ICONS
		std::stringstream s;
		s << gameFolder << "UFOGRAPH/" << "SPICONS.DAT";
		_sets["SPICONS.DAT"] = new SurfaceSet(32, 24);
		_sets["SPICONS.DAT"]->loadDat(CrossPlatform::getDataFile(s.str()));
		_sets["SPICONS.DAT"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		s.str("");
		std::stringstream s2;
		s << gameFolder << "UFOGRAPH/" << "CURSOR.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "CURSOR.TAB";
		_sets["CURSOR.PCK"] = new SurfaceSet(32, 40);
		_sets["CURSOR.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["CURSOR.PCK"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		s.str("");
		s2.str("");
		s << gameFolder << "UFOGRAPH/" << "SMOKE.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "SMOKE.TAB";
		_sets["SMOKE.PCK"] = new SurfaceSet(32, 40);
		_sets["SMOKE.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["SMOKE.PCK"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
	
		s.str("");
		s2.str("");
		s << gameFolder << "UFOGRAPH/" << "HIT.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "HIT.TAB";
		_sets["HIT.PCK"] = new SurfaceSet(32, 40);
		_sets["HIT.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["HIT.PCK"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		s.str("");
		s2.str("");
		s << gameFolder << "UFOGRAPH/" << "X1.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "X1.TAB";
		_sets["X1.PCK"] = new SurfaceSet(128, 64);
		_sets["X1.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["X1.PCK"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		s.str("");
		_sets["MEDIBITS.DAT"] = new SurfaceSet(52, 58);
		s << gameFolder << "UFOGRAPH/" << "MEDIBITS.DAT";
		_sets["MEDIBITS.DAT"]->loadDat (CrossPlatform::getDataFile(s.str()));
		_sets["MEDIBITS.DAT"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		s.str("");
		_sets["DETBLOB.DAT"] = new SurfaceSet(16, 16);
		s << gameFolder << "UFOGRAPH/" << "DETBLOB.DAT";
		_sets["DETBLOB.DAT"]->loadDat (CrossPlatform::getDataFile(s.str()));
		_sets["DETBLOB.DAT"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		// Load Battlescape Terrain (only blacks are loaded, others are loaded just in time)
		std::string bsets[] = {"BLANKS.PCK"};

		if (_sets.find("BLANKS.PCK") == _sets.end())
		{
			for (int i = 0; i < 1; ++i)
			{
				std::stringstream s;
				s << gameFolder << "TERRAIN/" << bsets[i];
				std::string tab = bsets[i].substr(0, bsets[i].length()-4) + ".TAB";
				std::stringstream s2;
				s2 << gameFolder << "TERRAIN/" << tab;
				_sets[bsets[i]] = new SurfaceSet(32, 40);
				_sets[bsets[i]]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
				_sets[bsets[i]]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
			}
		}

		// Load Battlescape units
		std::string usets[] = {"SILACOID.PCK",
								"CELATID.PCK",
								"HANDOB.PCK",
								"CYBER.PCK",
								"FLOOROB.PCK",
								"SECTOID.PCK",
								"CIVF.PCK",
								"CIVM.PCK",
								"XCOM_1.PCK",
								"SNAKEMAN.PCK",
								"XCOM_0.PCK",
								"CHRYS.PCK",
								"TANKS.PCK",
								"FLOATER.PCK",
								"XCOM_2.PCK",
								"ZOMBIE.PCK",
								"MUTON.PCK",
								"X_REAP.PCK",
								"ETHEREAL.PCK",
								"X_ROB.PCK"
						 };

		for (int i = 0; i < 20; ++i)
		{
			std::stringstream s;
			s << gameFolder << "UNITS/" << usets[i];
			std::string tab = usets[i].substr(0, usets[i].length()-4) + ".TAB";
			std::stringstream s2;
			s2 << gameFolder + "UNITS/" << tab;
			_sets[usets[i]] = new SurfaceSet(32, 40);
			_sets[usets[i]]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
			_sets[usets[i]]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
		}

		s.str("");
		s << gameFolder << "UNITS/" << "BIGOBS.PCK";
		s2.str("");
		s2 << gameFolder << "UNITS/" << "BIGOBS.TAB";
		_sets["BIGOBS.PCK"] = new SurfaceSet(32, 48);
		_sets["BIGOBS.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["BIGOBS.PCK"]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());

		s.str("");
		s << gameFolder << "GEODATA/" << "LOFTEMPS.DAT";
		MapDataSet::loadLOFTEMPS(CrossPlatform::getDataFile(s.str()), &_voxelData);

		std::string scrs[] = {"TAC00.SCR"};

		for (int i = 0; i < 1; ++i)
		{
			std::stringstream s;
			s << gameFolder << "UFOGRAPH/" << scrs[i];
			_surfaces[scrs[i]] = new Surface(320, 200);
			_surfaces[scrs[i]]->loadScr(CrossPlatform::getDataFile(s.str()));
			_surfaces[scrs[i]]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
		}

		std::string spks[] = {"TAC01.SCR",
							  "DETBORD.PCK",
							  "DETBORD2.PCK",
							  "ICONS.PCK",
							  "MEDIBORD.PCK",
							  "SCANBORD.PCK",
							  "UNIBORD.PCK"};

		for (int i = 0; i < 7; ++i)
		{
			std::stringstream s;
			s << gameFolder << "UFOGRAPH/" << spks[i];
			_surfaces[spks[i]] = new Surface(320, 200);
			_surfaces[spks[i]]->loadSpk(CrossPlatform::getDataFile(s.str()));
			_surfaces[spks[i]]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
		}

		std::string invs[] = {"MAN_0",
							  "MAN_1",
							  "MAN_2",
							  "MAN_3"};
		std::string sets[] = {"F0",
							  "F1",
							  "F2",
							  "F3",
							  "M0",
							  "M1",
							  "M2",
							  "M3"};

		for (int i = 0; i < 4; ++i)
		{
			std::stringstream s1, s1full, s2, s2full;
			s1 << invs[i] << ".SPK";
			s1full << gameFolder << "UFOGRAPH/" << s1.str();
			s2 << invs[i] << sets[0] << ".SPK";
			s2full << gameFolder << "UFOGRAPH/" << s2.str();
			// Load fixed inventory image
			if (CrossPlatform::fileExists(CrossPlatform::getDataFile(s1full.str())))
			{
				_surfaces[s1.str()] = new Surface(320, 200);
				_surfaces[s1.str()]->loadSpk(CrossPlatform::getDataFile(s1full.str()));
				_surfaces[s1.str()]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
			}
			// Load gender-based inventory image
			if (CrossPlatform::fileExists(CrossPlatform::getDataFile(s2full.str())))
			{
				for (int j = 0; j < 8; j++)
				{
					std::stringstream s3, s3full;
					s3 << invs[i] << sets[j] << ".SPK";
					s3full << gameFolder << "UFOGRAPH/" << s3.str();
					_surfaces[s3.str()] = new Surface(320, 200);
					_surfaces[s3.str()]->loadSpk(CrossPlatform::getDataFile(s3full.str()));
					_surfaces[s3.str()]->setPalette(_palettes["PALETTES.DAT_4"]->getColors());
				}
			}
		}
	}
	else if (game == "xcom2")
	{
		// Load Battlescape ICONS
		std::stringstream s;
		s << gameFolder << "UFOGRAPH/" << "SPICONS.DAT";
		_sets["TFTD_SPICONS.DAT"] = new SurfaceSet(32, 24);
		_sets["TFTD_SPICONS.DAT"]->loadDat(CrossPlatform::getDataFile(s.str()));
		_sets["TFTD_SPICONS.DAT"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		s.str("");
		std::stringstream s2;
		s << gameFolder << "UFOGRAPH/" << "CURSOR.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "CURSOR.TAB";
		_sets["TFTD_CURSOR.PCK"] = new SurfaceSet(32, 40);
		_sets["TFTD_CURSOR.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()), 4);
		_sets["TFTD_CURSOR.PCK"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		s.str("");
		s2.str("");
		s << gameFolder << "UFOGRAPH/" << "SMOKE.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "SMOKE.TAB";
		_sets["TFTD_SMOKE.PCK"] = new SurfaceSet(32, 40);
		_sets["TFTD_SMOKE.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()), 4);
		_sets["TFTD_SMOKE.PCK"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
	
		s.str("");
		s2.str("");
		s << gameFolder << "UFOGRAPH/" << "HIT.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "HIT.TAB";
		_sets["TFTD_HIT.PCK"] = new SurfaceSet(32, 40);
		_sets["TFTD_HIT.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["TFTD_HIT.PCK"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		s.str("");
		s2.str("");
		s << gameFolder << "UFOGRAPH/" << "X1.PCK";
		s2 << gameFolder << "UFOGRAPH/" << "X1.TAB";
		_sets["TFTD_X1.PCK"] = new SurfaceSet(128, 64);
		_sets["TFTD_X1.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["TFTD_X1.PCK"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		s.str("");
		_sets["TFTD_MEDIBITS.DAT"] = new SurfaceSet(52, 58);
		s << gameFolder << "UFOGRAPH/" << "MEDIBITS.DAT";
		_sets["TFTD_MEDIBITS.DAT"]->loadDat (CrossPlatform::getDataFile(s.str()));
		_sets["TFTD_MEDIBITS.DAT"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		s.str("");
		_sets["TFTD_DETBLOB.DAT"] = new SurfaceSet(16, 16);
		s << gameFolder << "UFOGRAPH/" << "DETBLOB.DAT";
		_sets["TFTD_DETBLOB.DAT"]->loadDat (CrossPlatform::getDataFile(s.str()));
		_sets["TFTD_DETBLOB.DAT"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		// Load Battlescape Terrain (only blacks are loaded, others are loaded just in time)
		std::string bsets[] = {"BLANKS.PCK"};

		if (_sets.find("BLANKS.PCK") == _sets.end())
		{
			for (int i = 0; i < 1; ++i)
			{
				s.str("");
				std::stringstream s;
				s << gameFolder << "TERRAIN/" << bsets[i];
				std::string tab = bsets[i].substr(0, bsets[i].length()-4) + ".TAB";
				s2.str("");
				s2 << gameFolder << "TERRAIN/" << tab;
				_sets[bsets[i]] = new SurfaceSet(32, 40);
				_sets[bsets[i]]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
				_sets[bsets[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
			}
		}

		// Load Battlescape units
		std::string usets[] = {"AQUA.PCK",
								"BIODRON.PCK",
								"CALCIN.PCK",
								"CIVIL_1.PCK",
								"CIVIL_2.PCK",
								"DEEPONE.PCK",
								"FLOOROB.PCK",
								"GILLMAN.PCK",
								"HALLUCIN.PCK",
								"HANDOB.PCK",
								"LOBSTER.PCK",
								"TANK01.PCK",
								"TASOTHS.PCK",
								"TDXCOM_0.PCK",
								"TDXCOM_1.PCK",
								"TDXCOM_2.PCK",
								"TENTAC.PCK",
								"TRISCEN.PCK",
								"XARQUID.PCK",
								"ZOMBIE.PCK"
						 };

		for (int i = 0; i < 20; ++i)
		{
			s.str("");
			s << gameFolder << "UNITS/" << usets[i];
			std::string tab = usets[i].substr(0, usets[i].length()-4) + ".TAB";
			s2.str("");
			s2 << gameFolder + "UNITS/" << tab;
			_sets["TFTD_" + usets[i]] = new SurfaceSet(32, 40);
			_sets["TFTD_" + usets[i]]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()), 4);
			_sets["TFTD_" + usets[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
		}

		s.str("");
		s << gameFolder << "UNITS/" << "BIGOBS.PCK";
		s2.str("");
		s2 << gameFolder << "UNITS/" << "BIGOBS.TAB";
		_sets["TFTD_BIGOBS.PCK"] = new SurfaceSet(32, 48);
		_sets["TFTD_BIGOBS.PCK"]->loadPck(CrossPlatform::getDataFile(s.str()), CrossPlatform::getDataFile(s2.str()));
		_sets["TFTD_BIGOBS.PCK"]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		s.str("");
		s << gameFolder << "GEODATA/" << "LOFTEMPS.DAT";
		if (!_voxelData.empty())
		{
			_voxelData.erase(_voxelData.begin(), _voxelData.end());
		}
		MapDataSet::loadLOFTEMPS(CrossPlatform::getDataFile(s.str()), &_voxelData);

		std::string scrs[] = {"TAC00.SCR"};

		for (int i = 0; i < 1; ++i)
		{
			s.str("");
			s << gameFolder << "UFOGRAPH/" << scrs[i];
			_surfaces["TFTD_" + scrs[i]] = new Surface(320, 200);
			_surfaces["TFTD_" + scrs[i]]->loadScr(CrossPlatform::getDataFile(s.str()));
			_surfaces["TFTD_" + scrs[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
		}

		std::string spks[] = {"TAC01.BDY",
							  "DETBORD.BDY",
							  "DETBORD2.BDY",
							  "ICONS.BDY",
							  "MEDIBORD.BDY",
							  "SCANBORD.BDY",
							  "UNIBORD.PCK"};

		for (int i = 0; i < 6; ++i)
		{
			s.str("");
			s << gameFolder << "UFOGRAPH/" << spks[i];
			_surfaces["TFTD_" + spks[i]] = new Surface(320, 200);
			_surfaces["TFTD_" + spks[i]]->loadBdy(CrossPlatform::getDataFile(s.str()));
			_surfaces["TFTD_" + spks[i]]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
		}
		s.str("");
		s << gameFolder << "UFOGRAPH/" << spks[6];
		_surfaces["TFTD_" + spks[6]] = new Surface(320, 200);
		_surfaces["TFTD_" + spks[6]]->loadSpk(CrossPlatform::getDataFile(s.str()));
		_surfaces["TFTD_" + spks[6]]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());

		std::string invs[] = {"MAN_0",
							  "MAN_1",
							  "MAN_2",
							  "MAN_3"};
		std::string sets[] = {"F0",
							  "F1",
							  "F2",
							  "F3",
							  "M0",
							  "M1",
							  "M2",
							  "M3"};

		for (int i = 0; i < 4; ++i)
		{
			// Load gender-based inventory image
			for (int j = 0; j < 8; j++)
			{
				std::stringstream s3, s3full;
				s3 << invs[i] << sets[j] << ".BDY";
				s3full << gameFolder << "UFOGRAPH/" << s3.str();
				_surfaces["TFTD_" + s3.str()] = new Surface(320, 200);
				_surfaces["TFTD_" + s3.str()]->loadBdy(CrossPlatform::getDataFile(s3full.str()));
				_surfaces["TFTD_" + s3.str()]->setPalette(_palettes["TFTD_PALETTES.DAT_3"]->getColors());
			}
		}
	}
}

/**
 * Adds to the resource pack extra sprites and sounds
 * defined in ruleset.
 * @param extraSprites List of additional sprites.
 * @param ExtraSounds List of additional sounds.
 */
void ResourcePack::loadExtraResources(std::vector<std::pair<std::string, ExtraSprites *> > extraSprites, std::vector<std::pair<std::string, ExtraSounds *> > extraSounds)
{
	Log(LOG_INFO) << "Loading extra resources from ruleset...";
	bool debugOutput = Options::getBool("debug");
	std::stringstream s;
		
	for (std::vector<std::pair<std::string, ExtraSprites *> >::const_iterator i = extraSprites.begin(); i != extraSprites.end(); ++i)
	{
		std::string sheetName = i->first;
		ExtraSprites *spritePack = i->second;
		bool subdivision = (spritePack->getSubX() != 0 && spritePack->getSubY() != 0);
		if (spritePack->getSingleImage())
		{
			if (_surfaces.find(sheetName) == _surfaces.end())
			{
				if (debugOutput)
				{
					Log(LOG_INFO) << "Creating new single image: " << sheetName;
				}
				_surfaces[sheetName] = new Surface(spritePack->getWidth(), spritePack->getHeight());
			}
			else
			{
				if (debugOutput)
				{
					Log(LOG_INFO) << "Adding/Replacing single image: " << sheetName;
				}
				delete _surfaces[sheetName];
				_surfaces[sheetName] = new Surface(spritePack->getWidth(), spritePack->getHeight());
			}
			s.str("");
			s << CrossPlatform::getDataFile(spritePack->getFolder() + spritePack->getSprites()->operator[](0));
			_surfaces[sheetName]->loadImage(s.str());
		}
		else
		{
			bool adding = false;
			if (_sets.find(sheetName) == _sets.end())
			{
				if (debugOutput)
				{
					Log(LOG_INFO) << "Creating new surface set: " << sheetName;
				}
				adding = true;
				 if (subdivision)
				 {
					_sets[sheetName] = new SurfaceSet(spritePack->getSubX(), spritePack->getSubY());
				 }
				 else
				 {
					_sets[sheetName] = new SurfaceSet(spritePack->getWidth(), spritePack->getHeight());
				 }
			}
			else if (debugOutput)
			{
				Log(LOG_INFO) << "Adding/Replacing items in surface set: " << sheetName;
			}
			
			if (subdivision && debugOutput)
			{
				int frames = (spritePack->getWidth() / spritePack->getSubX())*(spritePack->getHeight() / spritePack->getSubY());
				Log(LOG_INFO) << "Subdividing into " << frames << " frames.";
			}

			for (std::map<int, std::string>::iterator j = spritePack->getSprites()->begin(); j != spritePack->getSprites()->end(); ++j)
			{
				int startFrame = j->first;
				std:: string fileName = j->second;
				s.str("");
				if (fileName.substr(fileName.length() - 1, 1) == "/")
				{
					if (debugOutput)
					{
						Log(LOG_INFO) << "Loading surface set from folder: " << spritePack->getFolder() + fileName << " starting at frame: " << startFrame;
					}
					int offset = startFrame;
					std::stringstream folder;
					folder << CrossPlatform::getDataFolder(spritePack->getFolder() + fileName);
					std::vector<std::string> contents = CrossPlatform::getFolderContents(folder.str());
					for (std::vector<std::string>::iterator k = contents.begin();
						k != contents.end(); ++k)
					{
						s.str("");
						s << folder.str() << CrossPlatform::getDataFile(*k);
						if (_sets[sheetName]->getFrame(offset))
						{
							if (debugOutput)
							{
								Log(LOG_INFO) << "Replacing frame: " << offset;
							}
							_sets[sheetName]->getFrame(offset)->loadImage(s.str());
						}
						else
						{
							if (adding)
							{
								_sets[sheetName]->addFrame(offset)->loadImage(s.str());
							}
							else
							{
								if (debugOutput)
								{
									Log(LOG_INFO) << "Adding frame: " << offset + spritePack->getModIndex();
								}
								_sets[sheetName]->addFrame(offset + spritePack->getModIndex())->loadImage(s.str());
							}
						}
						offset++;
					}
				}
				else
				{
					if (spritePack->getSubX() == 0 && spritePack->getSubY() == 0)
					{
						s << CrossPlatform::getDataFile(spritePack->getFolder() + fileName);
						if (_sets[sheetName]->getFrame(startFrame))
						{
							if (debugOutput)
							{
								Log(LOG_INFO) << "Replacing frame: " << startFrame;
							}
							_sets[sheetName]->getFrame(startFrame)->loadImage(s.str());
						}
						else
						{
							if (debugOutput)
							{
								Log(LOG_INFO) << "Adding frame: " << startFrame << ", using index: " << startFrame + spritePack->getModIndex();
							}
							_sets[sheetName]->addFrame(startFrame + spritePack->getModIndex())->loadImage(s.str());
						}
					}
					else
					{
						_surfaces["tempSurface"] = new Surface(spritePack->getWidth(), spritePack->getHeight());
						s.str("");
						s << CrossPlatform::getDataFile(spritePack->getFolder() + spritePack->getSprites()->operator[](startFrame));
						_surfaces["tempSurface"]->loadImage(s.str());
						int xDivision = spritePack->getWidth() / spritePack->getSubX();
						int yDivision = spritePack->getHeight() / spritePack->getSubY();
						int offset = startFrame;

						for (int y = 0; y != yDivision; ++y)
						{
							for (int x = 0; x != xDivision; ++x)
							{
								if (_sets[sheetName]->getFrame(offset))
								{
									if (debugOutput)
									{
										Log(LOG_INFO) << "Replacing frame: " << offset;
									}
									_sets[sheetName]->getFrame(offset)->clear();
									// for some reason regular blit() doesn't work here how i want it, so i use this function instead.
									_surfaces["tempSurface"]->blitNShade(_sets[sheetName]->getFrame(offset), 0 - (x * spritePack->getSubX()), 0 - (y * spritePack->getSubY()), 0);
								}
								else
								{
									if (adding)
									{
										// for some reason regular blit() doesn't work here how i want it, so i use this function instead.
										_surfaces["tempSurface"]->blitNShade(_sets[sheetName]->addFrame(offset), 0 - (x * spritePack->getSubX()), 0 - (y * spritePack->getSubY()), 0);
									}
									else
									{
										if (debugOutput)
										{
											Log(LOG_INFO) << "Adding frame: " << offset + spritePack->getModIndex();
										}
										// for some reason regular blit() doesn't work here how i want it, so i use this function instead.
										_surfaces["tempSurface"]->blitNShade(_sets[sheetName]->addFrame(offset + spritePack->getModIndex()), 0 - (x * spritePack->getSubX()), 0 - (y * spritePack->getSubY()), 0);
									}
								}
								++offset;
							}
						}
						delete _surfaces["tempSurface"];
						_surfaces.erase("tempSurface");
					}
				}
			}
		}
	}

	if (!Options::getBool("mute"))
	{
		for (std::vector<std::pair<std::string, ExtraSounds *> >::const_iterator i = extraSounds.begin(); i != extraSounds.end(); ++i)
		{
			std::string setName = i->first;
			ExtraSounds *soundPack = i->second;
			if (_sounds.find(setName) == _sounds.end())
			{
				if (debugOutput)
				{
					Log(LOG_INFO) << "Creating new sound set: " << setName << ", this will likely have no in-game use.";
				}
				_sounds[setName] = new SoundSet();
			}
			else if (debugOutput)
			{
				Log(LOG_INFO) << "Adding/Replacing items in sound set: " << setName;
			}
			for (std::map<int, std::string>::iterator j = soundPack->getSounds()->begin(); j != soundPack->getSounds()->end(); ++j)
			{
				int startSound = j->first;
				std::string fileName = j->second;
				s.str("");
				if (fileName.substr(fileName.length() - 1, 1) == "/")
				{
					if (debugOutput)
					{
						Log(LOG_INFO) << "Loading sound set from folder: " << soundPack->getFolder() + fileName << " starting at index: " << startSound;
					}
					int offset = startSound;
					std::stringstream folder;
					folder << CrossPlatform::getDataFolder(soundPack->getFolder() + fileName);
					std::vector<std::string> contents = CrossPlatform::getFolderContents(folder.str());
					for (std::vector<std::string>::iterator k = contents.begin();
						k != contents.end(); ++k)
					{
						s.str("");
						s << folder.str() << CrossPlatform::getDataFile(*k);
						if (_sounds[setName]->getSound(offset))
						{
							_sounds[setName]->getSound(offset)->load(s.str());
						}
						else
						{
							_sounds[setName]->addSound(offset + soundPack->getModIndex())->load(s.str());
						}
						offset++;
					}
				}
				else
				{
					s << CrossPlatform::getDataFile(soundPack->getFolder() + fileName);
					if (_sounds[setName]->getSound(startSound))
					{
						if (debugOutput)
						{
							Log(LOG_INFO) << "Replacing index: " << startSound;
						}
						_sounds[setName]->getSound(startSound)->load(s.str());
					}
					else
					{
						if (debugOutput)
						{
							Log(LOG_INFO) << "Adding index: " << startSound;
						}
						_sounds[setName]->addSound(startSound + soundPack->getModIndex())->load(s.str());
					}
				}
			}
		}
	}
}

}
