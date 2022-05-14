#pragma once

#include "olcPixelGameEngine.h"

namespace util {

	// ==== Sprite asset management ==== //
	
	olc::Renderable loadSprite() { return olc::Renderable(); }

	olc::Renderable loadPartialSprite() { return olc::Renderable(); }

	std::vector<olc::Renderable> loadSpriteSheet() { return std::vector<olc::Renderable>(); }

	// ==== Sound asset management ==== //
	
	// Container for sound asset related information
	class SoundAsset {
	private:
		std::string path;
		int index;

	public:
		SoundAsset(std::string soundPath) : path(soundPath), index(-1) {}

		// Returns PGEX olcSound.h sound index
		int getIndex() { return index; }
		
		// Tells PGEX olcSound.h to play this sound
		void playSound()
		{
#ifdef OLC_PGEX_SOUND_H
			// Stuff
#endif
		}
	};

	// Requires olcSound.h
	void loadSound(SoundAsset* sound)
	{
#ifdef OLC_PGEX_SOUND_H
		// Stuff
#endif
	}

	// Requires olcSound.h
	void loadSounds(std::vector<SoundAsset*>& sounds)
	{
#ifdef OLC_PGEX_SOUND_H
		// Load each sound, enter indexs in order
#endif
	}

	// ==== External file management ==== //

	class TextFile {
		std::string path, name;
		std::fstream stream;
		bool open;

		// void openFile(){}
		// void closeFile(){}
	};

	// ==== General ==== //

	class GridSpace {
		
		olc::vi2d cellSize; // Size of the squares the world is cut into
		olc::vi2d screenSize; // Size of the world viewport
		olc::vi2d cameraPos; // Offset of the center of the screen, (0,0) = center of screen at origin

		bool flipY; // By default (0,0) is in the top left corner, same as mouse's (0,0)

		olc::vi2d screenToWorld(olc::vi2d screenPos) { return cameraPos + (screenPos - screenSize / 2); }

		olc::vi2d worldToScreen(olc::vi2d worldPos) { return ((cameraPos - worldPos) - screenSize / 2); }

		// TODO: bounded to screen
		bool boundedToScreen() { return false;  }
	};
};