#include "tamtypes.h"
#include "kernel.h"
#include "sifrpc.h"
#include "fileio.h"
#include "iopcontrol.h"
#include "iopheap.h"
#include "string.h"
#include "loadfile.h"
#include "stdio.h"
#include "sbv_patches.h"
#include <libcdvd.h>
#include <libpad.h>
#include <libmc.h>
#include <debug.h>
#include "gsKit.h"
#include "dmaKit.h"
#include "gsToolkit.h"
#include "malloc.h"
#include "math.h"

#include <debug.h>

#define USBLD_VERSION "0.41"

typedef
struct TGame
{
	char           Name[33];
	char           Image[16];
	unsigned char  parts;
	unsigned char  media;
} TGame;

typedef
struct TGameList
{
	void *prev;
	TGame Game;
	void *next;
} TGameList;

TGameList *firstgame;
TGameList *actualgame;
TGameList *eth_firstgame;
TGameList *eth_actualgame;

/// Forward decls.
struct TMenuItem;
struct TMenuList;

/// a single submenu item
struct TSubMenuItem {
	/// Icon used for rendering of this item
	GSTEXTURE *icon;
	
	/// item description
	char *text;
	
	/// item description in localised form (used if value is not negative)
	int text_id;
	
	/// item id
	int id;
};

struct TSubMenuList {
	struct TSubMenuItem item;
	
	struct TSubMenuList *prev, *next;
};

/// a single menu item. Linked list impl (for the ease of rendering)
struct TMenuItem {
	/// Icon used for rendering of this item
	GSTEXTURE *icon;
	
	/// item description
	char *text;
	
	/// item description in localised form (used if value is not negative)
	int text_id;
	
	void *userdata;
	
	/// submenu
	struct TSubMenuList *submenu, *current;
	
	/// execute a menu/submenu item (id is the submenu id as specified in AppendSubMenu)
	void (*execute)(struct TMenuItem *self, int id);
	
	void (*nextItem)(struct TMenuItem *self);
	
	void (*prevItem)(struct TMenuItem *self);
	
	int (*resetOrder)(struct TMenuItem *self);
	
	void (*refresh)(struct TMenuItem *self);
	
	/// Alternate execution (can be used for item configuration for example)
	void (*altExecute)(struct TMenuItem *self, int id);
};

struct TMenuList {
	struct TMenuItem *item;
	
	struct TMenuList *prev, *next;
};

/// menu item list.
struct TMenuList *menu;
struct TMenuList *selected_item;

// Configuration handling
// single config value (linked list item)
struct TConfigValue {
        char key[32];
        char val[255];

        struct TConfigValue *next;
};

struct TConfigSet {
	struct TConfigValue *head;
	struct TConfigValue *tail;
};

// global config
struct TConfigSet gConfig;
// global config file path
const char *gConfigPath;


// UI dialog item definition
typedef enum {
	// terminates the definition of dialog. Mandatory
	UI_TERMINATOR,
	// A string label
	UI_LABEL,
	// Break to next line
	UI_BREAK,
	// Break to next line, draw line splitter
	UI_SPLITTER,
	// A spacer of a few pixels
	UI_SPACER,
	// Ok button
	UI_OK, // Just a shortcut for BUTTON with OK label and id 1!
	// Universal button (display's label, returns id on X)
	UI_BUTTON,
	// draw integer input box
	UI_INT,
	// draw string input box
	UI_STRING,
	// bool value (On/Off). (Uses int value for storage)
	UI_BOOL,
	// enumeration (multiple strings)
	UI_ENUM
}  UIItemType;

// UI item definition (for dialogues)
struct UIItem {
	UIItemType type;
	int id; // id of this item
	
	union {
		struct {
			const char *text;
			int stringId; // if zero, the text is used
		} label;
		
		struct { // integer value
			// default value
			int def;
			int current;
			int min;
			int max;
			// if UI_ENUM is used, this contains enumeration values
			const char **enumvalues; // last one has to be NULL
		} intvalue;
		
		struct { // fixed 32 character string
			// default value
			char def[32];
			char text[32];
		} stringvalue;
	};
};

char *infotxt;

float waveframe;
int frame;
int h_anim;
int v_anim;
int direc;
int max_settings;
int max_games;
int eth_max_games;
int gLanguageID;

int background_image;

// Bool - either true or false (1/0)
int dynamic_menu;

// 0,1,2 scrolling speed
unsigned int scroll_speed;

// IP config

int ps2_ip[4];
int ps2_netmask[4];
int ps2_gateway[4];
int pc_ip[4];
int gPCPort;

char gPCShareName[32];

// describes what is happening in the network startup thread (>0 means loading, <0 means error)...
int gNetworkStartup;
// config parameter ("net_auto") - nonzero means net will autoload
int gNetAutostart;
// true if the ip config should be saved as well
int gIPConfigChanged;

// Theme config

char theme[32];
int bg_color[3];
int text_color[3];
char *theme_dir[255];
int max_theme_dir;

GSTEXTURE disc_icon;
GSTEXTURE games_icon;
GSTEXTURE config_icon;
GSTEXTURE exit_icon;
GSTEXTURE theme_icon;
GSTEXTURE language_icon;
GSTEXTURE apps_icon;
GSTEXTURE menu_icon;
GSTEXTURE scroll_icon;
GSTEXTURE usb_icon;
GSTEXTURE save_icon;
GSTEXTURE netconfig_icon;
GSTEXTURE network_icon;

void LoadGameList();

//SYSTEM

int usbdelay;
int ethdelay;

void Reset();
void delay(int count);
void Start_LoadNetworkModules_Thread(void);
void LoadUSBD();
void LaunchGame(TGame *game, int mode, int compatmask);
void SendIrxKernelRAM(void);

#define USB_MODE	0
#define ETH_MODE	1


//PAD

#define KEY_LEFT 1
#define KEY_DOWN 2
#define KEY_RIGHT 3
#define KEY_UP 4
#define KEY_START 5
#define KEY_R3 6
#define KEY_L3 7
#define KEY_SELECT 8
#define KEY_SQUARE 9
#define KEY_CROSS 10
#define KEY_CIRCLE 11
#define KEY_TRIANGLE 12
#define KEY_R1 13
#define KEY_L1 14
#define KEY_R2 15
#define KEY_L2 16

int StartPad();
int ReadPad();
int GetKey(int num);

int GetKeyOn(int num);
int GetKeyOff(int num);
int GetKeyPressed(int num);

void SetButtonDelay(int button, int btndelay);
void UnloadPad();

//GFX

void InitGFX();
void InitMenu();

void AppendMenuItem(struct TMenuItem* item);

struct TSubMenuList* AppendSubMenu(struct TSubMenuList** submenu, GSTEXTURE *icon, char *text, int id, int text_id);
void DestroySubMenu(struct TSubMenuList** submenu);
void UpdateScrollSpeed();

void Flip();
void MsgBox(char* text);

// restores the on-line running settings from the global config var.
void gfxStoreConfig();
// stores the graphics related settings into the global config var.
void gfxRestoreConfig();

void DrawWave(int y, int xoffset);
void DrawBackground();
void SetColor(int r, int g, int b);
void TextColor(int r, int g, int b, int a);
void DrawText(int x, int y, const char *texto, float scale, int centered);
void DrawConfig();
void DrawIPConfig();
void UploadTexture(GSTEXTURE* txt);
void Aviso();
void Intro();
void LoadFont();
void UpdateFont();
int LoadRAW(char *path, GSTEXTURE *Texture);

void LoadTheme(int themeid);
int LoadBackground();
void LoadIcons();
void UpdateIcons();

void DrawIcon(GSTEXTURE *img, int x, int y, float scale);
void DrawIcons();
void DrawInfo();
// on-screen hint with optional action button icon
void DrawHint(const char* hint, int key);
void DrawSubMenu();
int ShowKeyb(char* text, size_t maxLen);

void MenuNextH();
void MenuPrevH();
void MenuNextV();
void MenuPrevV();
struct TMenuItem* MenuGetCurrent();
void MenuItemExecute();
void MenuItemAltExecute();
// Sets the selected item if it is found in the menu list
void MenuSetSelectedItem(struct TMenuItem* item);
void RefreshSubMenu();

// Static render
void DrawScreenStatic();

// swap static/normal render
void SwapMenu();

// Dynamic/Static render setter
void SetMenuDynamic(int dynamic);

// Renders everything
void DrawScreen();

/// Dialog display
int diaExecuteDialog(struct UIItem *ui);
int diaGetInt(struct UIItem* ui, int id, int *value);
int diaSetInt(struct UIItem* ui, int id, int value);
int diaGetString(struct UIItem* ui, int id, char *value);
int diaSetString(struct UIItem* ui, int id, const char *text);
// set label pointer into the label's text (must be valid while rendering dialog)
int diaSetLabel(struct UIItem* ui, int id, const char *text);
// sets the current enum value list for given control
int diaSetEnum(struct UIItem* ui, int id, const char **enumvals);

// main.c config handling
int storeConfig();
int restoreConfig();

//CONFIG
// returns nonzero for valid config key name
int configKeyValidate(const char* key);
int setConfigStr(struct TConfigSet* config, const char* key, const char* value);
int getConfigStr(struct TConfigSet* config, const char* key, char** value);
int setConfigInt(struct TConfigSet* config, const char* key, const int value);
int getConfigInt(struct TConfigSet* config, char* key, int* value);
int setConfigColor(struct TConfigSet* config, const char* key, int* color);
int getConfigColor(struct TConfigSet* config, const char* key, int* color);
int configRemoveKey(struct TConfigSet* config, const char* key);

void readIPConfig();
void writeIPConfig();

int readConfig(struct TConfigSet* config, const char *fname);
int writeConfig(struct TConfigSet* config, const char *fname);
void clearConfig(struct TConfigSet* config);

void ListDir(char* directory);