#pragma once
#ifndef UNITY_BUILD
#define UNITY_BUILD 0
#include "../main.h"
#endif

#define GUI_MAX_TRAIL       30
#define GUI_MIN_WIN_RECT    (Rectangle){ 0, 0, 320.f, 240.f }
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
    EGUI_Input_Text,      // editable ASCII text
    EGUI_Input_Int,       // integer numeric input
    EGUI_Input_Float      // floating numeric input
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
    GUI_ThemeColors     abstractica;
    float               bg_alpha;
    float               color_change;
} GUI_Theme;

typedef enum {
    EGUI_Font_Default,
    EGUI_Font_GUI,
    EGUI_Font_Count
} EGUI_Font;

typedef struct {
    float           default_height;
    float           border;

    float           scale;
    Vector2         delta;             // Delta adjustement
    Font            custom;
    bool            use_custom;        // Indicates if a custom font is used
    float           spacing;
    Vector2         blink_size;        // Size of the blinking cursor
    Vector2         blink_delta;      // Blink adjustment
    float           blink_alpha;
} GUI_FontSetup;

typedef enum {
    EGUI_Cursor_None,
    EGUI_Cursor_Default,
    EGUI_Cursor_AGS,
    EGUI_Cursor_Text,
    EGUI_Cursor_Resize,
    EGUI_Cursor_Count,
} EGUI_Cursor;

typedef struct {
    Texture2D       texture;
    Vector2         delta_normalized;
    float           scale;
    float           alpha;
    Vector2         trail_delta_normalized;
    EGUI_Cursor     additional_cursor;
} GUI_CursorSetup;

typedef struct {
    GUI_Theme           theme;
    GUI_IconSetup       icons;
    GUI_FontSetup       fonts[EGUI_Font_Count];
    GUI_CursorSetup     cursors[EGUI_Cursor_Count];
} GUI_Setup;
// < END SUBMODULE: SETUP

// > TYPES
//   SUBMODULE: WINDOW
#define MAX_WINDOW_TITLE 16

typedef enum {
    EGUI_WindowAction_None,
    EGUI_WindowAction_Moving,
    EGUI_WindowAction_Resizing
} EGUI_WindowAction;

typedef struct GUI_Window {
    int             id;
    Rectangle       shape;
    GUI_ThemeColors colors;
    const char      *title;
    Texture2D       *icon;
    float           scroll_offset;
    bool            focused_face;
    // Calc fields
    float           content_height; // Automatically stored by GUI_EndWindowContents. Calculated during grid processing.
    // Handlers
    void (*contents) (struct GUI_Window*);
} GUI_Window;

// < END SUBMODULE: WINDOW

// > TYPES
//   SUBMODULE: GRID
#define RESET_COUNT     0
#define ADD_COUNT       1
#define ONLY_GET_COUNT  2
#define DEFAULT_SIZE    0.0 // TODO@dc: remove

typedef struct {
    Rectangle       current_workspace;  // Current available (Use only for grids)
    int             vertical_count;
    float           vertical_size;
    int             horizontal_count;
    float           horizontal_size;
    float           used_height;        // Used to auto calc vertical scroll bar
    float           current_scroll;
    EGUI_Font       current_font;

    // Window that is being processed right now
    // This is NOT the active window focused by the player.
    int             current_window_idx;       // Current window being drawn
    Rectangle       current_window_workspace; // Current window workspace
    bool            force_overflow;
} GUI_GridTemp;
// < END SUBMODULE: GRID

//
// <<<<<<< END SUBMODULES
//


// > TYPES
//


typedef struct {
    const char      *id_ptr;            // Control Owner. A unique pointer representing the control owner
    int             window_target_id;   // Window owner of the overlay. (if its inside a window otherwise 0)
    GUI_GridTemp    grid;               // Grid state when the overlay draw was queued (used to draw with correct transform the opened menu)
    bool            just_enabled;       // TRUE if the overlay was JUST ENABLED on this frame
    Rectangle       final_shape;        // Final shape of the overlay
    void            (*function)(void);  // Draw function
}  GUI_Overlay;

// TODO@dc: fix naming starting w/section_
typedef struct {
    // Globals
    EGUI_Status      status;
    void             *control_focus_ptr;                // Keeps track between frames if there is a focused control

    // Grid temporary data
    GUI_GridTemp  grid;

    // Overlay
    GUI_Overlay     overlay;

    // Cursor
    EGUI_Cursor     cursor;
    Vector2         cursor_last;
    Vector2         cursor_current;
    bool            cursor_over_gui;                    // True if the pointer is over any of the elements in the GUI
    Vector2         cursor_trail[GUI_MAX_TRAIL];

    // Window
    EGUI_WindowAction   window_current_action;
    int                 window_target_id;   // Window currently eligible for interaction (See UPDATE WINDOW TARGET ID)
} GUI_Temp;

typedef struct {
    RenderTexture2D buffer;
    float           scale;
    int             force_z_index;

    GUI_Window      window_s[GUI_MAX_OPEN_WINS];

    // z_index stores windows indexes or zero as empty.
    int             z_index[GUI_MAX_OPEN_WINS];
} GUI_State;


// > STATE CONTEXT
static struct {
    GUI_State*  state;
    GUI_Setup*  setup;
    GUI_Temp*   temp;
} GUI_CTX = { 0 };

void        GUI_SetContext(GUI_State* state, GUI_Setup* setup, GUI_Temp* temp);
GUI_State*  GUI_GetState();
GUI_Setup*  GUI_GetSetup();
