
General Features:

	* C99 (should be usable from C++, but really designed for C)
	* Crossplatform (OSX,Windows and Linux) (DS support hopefully later)
	* API-independent, tho currently reliant on SDL
	* Small and fast

Editor Features:
	
	* Sprite animation/bounding box editing.
	* Visual tilemap editing (place entities, define collision and attribute tiles in layers).
	* Single tile or brush tile editing.
	* Arbitrary game settings: attributes, entity and brush presets can be saved to files.
	* Bitmap to tilemap import: converts a bitmap to unique tiles of arbitrary size, then creates a map file from it.

Graphics Features:

	* All surfaces are paletted 8-bit
	* Load palettes from .pal files (editable in Photoshop)
	* Loads sprites from custom sprite file format editable in the editor
	* Loads static graphics from custom image format (convertable in the editor)
	* Optional NTSC emulator filter.
	* Arbitrary screen size (initialized with two resolutions, one actual and one screen).
	* Automatic rendering queue based on screen layers
	* Easy text rendering with two built-in bitmap fonts

Sound Features:

	* Plays music from NSF (Nintendo Sound Format) files.
	* Plays sounds from the native SFXR format (sfs)

Tilemap Features:
	
	* Automatic entity to tilemap and entity to entity collision detection and gravity
	* Atomatic entity loading (in the code predefine the sprites and update/init functions for each entity type, then just load the .map file)
	* Scrolling support + follow entity.
	* Arbitrary tile size
	* Tile data and tile graphics loaded from separate files.

Basic UI Features:

	* Buttons
	* Scrollbars
	* Text Input
	* Select lists 
	
	
