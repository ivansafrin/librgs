#define OUT_VER 0x10

typedef struct 
{
	int version;				// module version (OUT_VER)
	char *description;			// description of module, with version string
	int id;						// module id. each input module gets its own. non-nullsoft modules should
								// be >= 65536. 

	HWND hMainWindow;			// winamp's main window (filled in by winamp)
	HINSTANCE hDllInstance;		// DLL instance handle (filled in by winamp)

	void (*Config)(HWND hwndParent); // configuration dialog 
	void (*About)(HWND hwndParent);  // about dialog

	void (*Init)();				// called when loaded
	void (*Quit)();				// called when unloaded

	int (*Open)(int samplerate, int numchannels, int bitspersamp, int bufferlenms, int prebufferms); 
					// returns >=0 on success, <0 on failure
					// NOTENOTENOTE: bufferlenms and prebufferms are ignored in most if not all output plug-ins. 
					//    ... so don't expect the max latency returned to be what you asked for.
					// returns max latency in ms (0 for diskwriters, etc)
					// bufferlenms and prebufferms must be in ms. 0 to use defaults. 
					// prebufferms must be <= bufferlenms

	void (*Close)();	// close the ol' output device.

	int (*Write)(char *buf, int len);	
					// 0 on success. Len == bytes to write (<= 8192 always). buf is straight audio data. 
					// 1 returns not able to write (yet). Non-blocking, always.

	int (*CanWrite)();	// returns number of bytes possible to write at a given time. 
						// Never will decrease unless you call Write (or Close, heh)

	int (*IsPlaying)(); // non0 if output is still going or if data in buffers waiting to be
						// written (i.e. closing while IsPlaying() returns 1 would truncate the song

	int (*Pause)(int pause); // returns previous pause state

	void (*SetVolume)(int volume); // volume is 0-255
	void (*SetPan)(int pan); // pan is -128 to 128

	void (*Flush)(int t);	// flushes buffers and restarts output at time t (in ms) 
							// (used for seeking)

	int (*GetOutputTime)(); // returns played time in MS
	int (*GetWrittenTime)(); // returns time written in MS (used for synching up vis stuff)

} Out_Module;



// note: exported symbol is now winampGetInModule2.

#define IN_VER 0x100

typedef struct 
{
	int version;				// module type (IN_VER)
	char *description;			// description of module, with version string

	HWND hMainWindow;			// winamp's main window (filled in by winamp)
	HINSTANCE hDllInstance;		// DLL instance handle (Also filled in by winamp)

	char *FileExtensions;		// "mp3\0Layer 3 MPEG\0mp2\0Layer 2 MPEG\0mpg\0Layer 1 MPEG\0"
								// May be altered from Config, so the user can select what they want
	
	int is_seekable;			// is this stream seekable? 
	int UsesOutputPlug;			// does this plug-in use the output plug-ins? (musn't ever change, ever :)

	void (*Config)(HWND hwndParent); // configuration dialog
	void (*About)(HWND hwndParent);  // about dialog

	void (*Init)();				// called at program init
	void (*Quit)();				// called at program quit

	void (*GetFileInfo)(char *file, char *title, int *length_in_ms); // if file == NULL, current playing is used
	int (*InfoBox)(char *file, HWND hwndParent);
	
	int (*IsOurFile)(char *fn);	// called before extension checks, to allow detection of mms://, etc
	// playback stuff
	int (*Play)(char *fn);		// return zero on success, -1 on file-not-found, some other value on other (stopping winamp) error
	void (*Pause)();			// pause stream
	void (*UnPause)();			// unpause stream
	int (*IsPaused)();			// ispaused? return 1 if paused, 0 if not
	void (*Stop)();				// stop (unload) stream

	// time stuff
	int (*GetLength)();			// get length in ms
	int (*GetOutputTime)();		// returns current output time in ms. (usually returns outMod->GetOutputTime()
	void (*SetOutputTime)(int time_in_ms);	// seeks to point in stream (in ms). Usually you signal yoru thread to seek, which seeks and calls outMod->Flush()..

	// volume stuff
	void (*SetVolume)(int volume);	// from 0 to 255.. usually just call outMod->SetVolume
	void (*SetPan)(int pan);	// from -127 to 127.. usually just call outMod->SetPan
	
	// in-window builtin vis stuff

	void (*SAVSAInit)(int maxlatency_in_ms, int srate);		// call once in Play(). maxlatency_in_ms should be the value returned from outMod->Open()
	// call after opening audio device with max latency in ms and samplerate
	void (*SAVSADeInit)();	// call in Stop()


	// simple vis supplying mode
	void (*SAAddPCMData)(void *PCMData, int nch, int bps, int timestamp); 
											// sets the spec data directly from PCM data
											// quick and easy way to get vis working :)
											// needs at least 576 samples :)

	// advanced vis supplying mode, only use if you're cool. Use SAAddPCMData for most stuff.
	int (*SAGetMode)();		// gets csa (the current type (4=ws,2=osc,1=spec))
							// use when calling SAAdd()
	void (*SAAdd)(void *data, int timestamp, int csa); // sets the spec data, filled in by winamp


	// vis stuff (plug-in)
	// simple vis supplying mode
	void (*VSAAddPCMData)(void *PCMData, int nch, int bps, int timestamp); // sets the vis data directly from PCM data
											// quick and easy way to get vis working :)
											// needs at least 576 samples :)

	// advanced vis supplying mode, only use if you're cool. Use VSAAddPCMData for most stuff.
	int (*VSAGetMode)(int *specNch, int *waveNch); // use to figure out what to give to VSAAdd
	void (*VSAAdd)(void *data, int timestamp); // filled in by winamp, called by plug-in


	// call this in Play() to tell the vis plug-ins the current output params. 
	void (*VSASetInfo)(int nch, int srate);


	// dsp plug-in processing: 
	// (filled in by winamp, called by input plug)

	// returns 1 if active (which means that the number of samples returned by dsp_dosamples
	// could be greater than went in.. Use it to estimate if you'll have enough room in the
	// output buffer
	int (*dsp_isactive)(); 

	// returns number of samples to output. This can be as much as twice numsamples. 
	// be sure to allocate enough buffer for samples, then.
	int (*dsp_dosamples)(short int *samples, int numsamples, int bps, int nch, int srate);


	// eq stuff
	void (*EQSet)(int on, char data[10], int preamp); // 0-64 each, 31 is +0, 0 is +12, 63 is -12. Do nothing to ignore.

	// info setting (filled in by winamp)
	void (*SetInfo)(int bitrate, int srate, int stereo, int synched); // if -1, changes ignored? :)

	Out_Module *outMod; // filled in by winamp, optionally used :)
} In_Module;


#define WINAMP_FILE_QUIT                40001
#define WINAMP_OPTIONS_PREFS            40012
#define WINAMP_OPTIONS_AOT              40019
#define WINAMP_FILE_REPEAT              40022
#define WINAMP_FILE_SHUFFLE             40023
#define WINAMP_HIGH_PRIORITY            40025
#define WINAMP_FILE_PLAY                40029
#define WINAMP_OPTIONS_EQ               40036
#define WINAMP_OPTIONS_ELAPSED          40037
#define WINAMP_OPTIONS_REMAINING        40038
#define WINAMP_OPTIONS_PLEDIT           40040
#define WINAMP_HELP_ABOUT               40041
#define WINAMP_MAINMENU                 40043
#define WINAMP_BUTTON1                  40044
#define WINAMP_BUTTON2                  40045
#define WINAMP_BUTTON3                  40046
#define WINAMP_BUTTON4                  40047
#define WINAMP_BUTTON5                  40048
#define WINAMP_VOLUMEUP                 40058
#define WINAMP_VOLUMEDOWN               40059
#define WINAMP_FFWD5S                   40060
#define WINAMP_REW5S                    40061
#define WINAMP_NEXT_WINDOW              40063
#define WINAMP_OPTIONS_WINDOWSHADE      40064
#define WINAMP_BUTTON1_SHIFT            40144
#define WINAMP_BUTTON2_SHIFT            40145
#define WINAMP_BUTTON3_SHIFT            40146
#define WINAMP_BUTTON4_SHIFT            40147
#define WINAMP_BUTTON5_SHIFT            40148
#define WINAMP_BUTTON1_CTRL             40154
#define WINAMP_BUTTON2_CTRL             40155
#define WINAMP_BUTTON3_CTRL             40156
#define WINAMP_BUTTON4_CTRL             40157
#define WINAMP_BUTTON5_CTRL             40158
#define WINAMP_OPTIONS_DSIZE            40165
#define IDC_SORT_FILENAME               40166
#define IDC_SORT_FILETITLE              40167
#define IDC_SORT_ENTIREFILENAME         40168
#define IDC_SELECTALL                   40169
#define IDC_SELECTNONE                  40170
#define IDC_SELECTINV                   40171
#define IDM_EQ_LOADPRE                  40172
#define IDM_EQ_LOADMP3                  40173
#define IDM_EQ_LOADDEFAULT              40174
#define IDM_EQ_SAVEPRE                  40175
#define IDM_EQ_SAVEMP3                  40176
#define IDM_EQ_SAVEDEFAULT              40177
#define IDM_EQ_DELPRE                   40178
#define IDM_EQ_DELMP3                   40180
#define IDC_PLAYLIST_PLAY               40184
#define WINAMP_FILE_LOC                 40185
#define WINAMP_OPTIONS_EASYMOVE         40186
#define WINAMP_FILE_DIR                 40187
#define WINAMP_EDIT_ID3                 40188
#define WINAMP_TOGGLE_AUTOSCROLL        40189
#define WINAMP_VISSETUP                 40190
#define WINAMP_PLGSETUP                 40191
#define WINAMP_VISPLUGIN                40192
#define WINAMP_JUMP                     40193
#define WINAMP_JUMPFILE                 40194
#define WINAMP_JUMP10FWD                40195
#define WINAMP_JUMP10BACK               40197
