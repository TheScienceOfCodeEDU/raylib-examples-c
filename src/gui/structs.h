
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
#define GUI_NO_WIN          -1


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

typedef enum {
    EGUI_WinActionNone,
    EGUI_WinActionMoving,
    EGUI_WinActionResizing
} EGUI_WinAction;

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


// > WINDOW STRUCTS
//   STABILITY : █████████░  90%
//   NOTES     : Resize. Close and open.

#define MAX_WINDOW_TITLE 16

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


// > STATE
//   STABILITY : ███░░░░░░░  30%
//   NOTES     : Save and restore

typedef struct {
    RenderTexture2D buffer;
    float           scale;
    int             force_z_index;

    GUI_Window      window_s[GUI_MAX_OPEN_WINS];

    // z_index stores windows indexes or zero as empty.
    int             z_index[GUI_MAX_OPEN_WINS];
} GUI_State;

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
    // This is NOT the active window focused by the player. Active win_idx is ==> GUI_State.z_index[0]
    int             current_window_idx;       // Current window being drawn
    Rectangle       current_window_workspace; // Current window workspace
    bool            force_overflow;
} GUI_LayoutTemp;

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
