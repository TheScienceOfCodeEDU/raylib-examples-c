#pragma once

#ifndef UNITY_BUILD
 #define UNITY_BUILD 0
 #include "../common.h"
 #include "setup.h"
#endif

#define GUI_MAX_TRAIL       30
#define GUI_MIN_WIN_SIZE    128
#define GUI_MAX_OPEN_WINS   16
#define GUI_MAX_TEXTBOXES   256
#define GUI_SCROLL_SPEED    16
#define GUI_NO_WIN          (-1)


// > ENUMS
//
typedef enum {
    EGUI_Status_Off,
    EGUI_Status_Ready,
    EGUI_Status_Drawing
} EGUI_Status;

typedef enum {
    EGUI_ControlStatus_Default,
    EGUI_ControlStatus_Collide,
    EGUI_ControlStatus_Focused
} EGUI_ControlStatus;

typedef enum {
    EGUI_InputText,      // editable ASCII text
    EGUI_InputInt,       // integer numeric input
    EGUI_InputFloat      // floating numeric input
} EGUI_InputType;

/*
EXAMPLE: Enum order matters
typedef enum {
    EGUI_Focus_Available,
    EGUI_Focus_CanOverride,
    EGUI_Focus_Granted
} EGUI_Focus;

// NOTE: define a function near the type instead of using it everywhere.
//       Now, we know that the order matters for this Enum.
bool FocusOverridable(EGUI_Focus focus)
{
    return focus <= EGUI_Focus_CanOverride;
}
*/

//
// BEGIN SUBMODULES >>>>>>>
//

// > TYPES
//   SUBMODULE: SETUP
typedef struct {
    Texture2D New;
    Texture2D Open;
    Texture2D Save;
    Texture2D Setup;
    Texture2D Error;
    Texture2D Face;
    Texture2D Dog;
    Texture2D Close;
    Texture2D CloseSmall;
    Texture2D MinimizeSmall;
    Texture2D Layouts;
} GUI_Icons;

typedef struct {
    float       icon_size;
    float       icon_size_sm;
    Vector2     icon_delta;
    GUI_Icons   icons;
} GUI_IconSetup;

typedef struct {
    Color tx_color_0;
    Color tx_color_1;
    Color bg_color_0;
    Color bg_color_1;
    Color bg_color_2;
    Color bg_color_3;
} GUI_ThemeColors;

typedef struct {
    GUI_ThemeColors     gray;
    GUI_ThemeColors     red;
    GUI_ThemeColors     green;
    float               bg_alpha;
    float               color_change;
} GUI_Theme;

typedef enum {
    EGUI_FontType_Default,
    EGUI_FontType_GUI,
    EGUI_FontType_Count
} EGUI_FontType;

typedef struct {
    float           default_height;
    float           border;

    float           font_scale;
    Vector2         font_delta;             // Delta adjustement
    Font            font_custom;
    bool            font_use_custom;        // Indicates if a custom font is used
    float           font_spacing;
    Vector2         blink_size;             // Size of the blinking cursor
    Vector2         blink_delta;            // Blink adjustment
    float           blink_alpha;
} GUI_FontSetup;

typedef enum {
    EGUI_Pointer_None,
    EGUI_Pointer_Default,
    EGUI_Pointer_AGS,
    EGUI_Pointer_Text,
    EGUI_Pointer_Resize,
    EGUI_Pointer_Count,
} EGUI_Pointer;

typedef struct {
    Texture2D       pointer_texture;
    Vector2         pointer_delta_normalized;
    float           pointer_scale;
    float           pointer_alpha;
    Vector2         trail_delta_normalized;
    EGUI_Pointer    additional;
} GUI_PointerSetup;

typedef struct {
    GUI_Theme           theme;
    GUI_IconSetup       icon_setup;
    GUI_FontSetup       font_setups[EGUI_FontType_Count];
    GUI_PointerSetup    pointer_setups[EGUI_Pointer_Count];
} GUI_Setup;
// < END SUBMODULE: SETUP


// > TYPES
//   SUBMODULE: WIN
#define MAX_WINDOW_TITLE 16

typedef enum {
    EGUI_WinActionNone,
    EGUI_WinActionMoving,
    EGUI_WinActionResizing
} EGUI_WinAction;

typedef struct GUI_Window {
    int             id;
    Rectangle       shape;
    GUI_ThemeColors colors;
    const char      *title;
    Texture2D       *icon;
    float           scroll_offset;
    float           content_height; // Automatically stored by GUI_EndWindowContents. Calculated during layout processing.
    bool            focused_face;
    void (*contents) (struct GUI_Window*);
} GUI_Window;
// < END SUBMODULE: WIN


// > TYPES
//   SUBMODULE: LAYOUT
#define RESET_COUNT     0
#define ADD_COUNT       1
#define ONLY_GET_COUNT  2
#define DEFAULT_SIZE    0.0

typedef struct {
    Rectangle       current_workspace;  // Current available (Use only for layouts)
    int             vertical_count;
    float           vertical_size;
    int             horizontal_count;
    float           horizontal_size;
    float           used_height;        // Used to auto calc vertical scroll bar
    float           current_scroll;
    EGUI_FontType   current_font_type;

    // Window that is being processed right now
    // This is NOT the active window focused by the player.
    int             current_window_idx;       // Current window being drawn
    Rectangle       current_window_workspace; // Current window workspace
    bool            force_overflow;
} GUI_LayoutTemp;
// < END SUBMODULE: LAYOUT

//
// <<<<<<< END SUBMODULES
//


// > TYPES
//
typedef struct {
    const char      *id_ptr;            // Control Owner. A unique pointer representing the control owner
    int             window_target_id;   // Window owner of the overlay. (if its inside a window otherwise 0)
    GUI_LayoutTemp  layout;             // Layout state when the overlay draw was queued (used to draw with correct transform the opened menu)
    bool            just_interacted;    // Just interacted ocurred on this frame
    Rectangle       shape_drawed;       // Final shape
    void            (*function)(void);  // Draw function
}  GUI_OverlayDraw;

// TODO@dc: fix naming starting w/section_
typedef struct {
    // Globals
    EGUI_Status      status;

    // Window runtime
    EGUI_WinAction  current_action;
    void            *control_focus_ptr;
    int             window_target_id; // Window currently eligible for interaction (See UPDATE WINDOW TARGET ID)

    // Pointer runtime
    EGUI_Pointer    current_pointer;
    Vector2         mouse_last;
    Vector2         mouse_current;
    bool            pointer_over_gui;   // True if the pointer is over any of the elements in the GUI
    Vector2         pointer_trail[GUI_MAX_TRAIL];

    // Overlay draw
    GUI_OverlayDraw overlay_draw;

    // Layout temporary data
    GUI_LayoutTemp  layout;
} GUI_Temp;

typedef struct {
    RenderTexture2D buffer;
    float           scale;
    int             force_z_index;

    GUI_Window      window_s[GUI_MAX_OPEN_WINS];

    // z_index stores windows indexes or zero as empty.
    int             z_index[GUI_MAX_OPEN_WINS];
} GUI_State;


// > STATE > CONTEXT
//   STABILITY : █████████░  90%
//   NOTES     : Nothing here

static struct {
    GUI_State*  state;
    GUI_Setup*  setup;
    GUI_Temp*    temp;
} GUI_CTX = { 0 };

// > CONTEXT
//   API

void GUI_SetContext(GUI_State* state, GUI_Setup* setup, GUI_Temp* temp);
GUI_State* GUI_GetState();
GUI_Setup* GUI_GetSetup();
void GUI_SetFontType(EGUI_FontType font_type);
GUI_Icons* GUI_GetIcons();
float GUI_GetIconWidth();
float GUI_GetIconWidthForShape(Rectangle shape, float border);
float GUI_GetIconSmallWidth();
