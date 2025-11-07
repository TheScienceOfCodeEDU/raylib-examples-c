#ifndef INCLUDES_BASE_H
 #define INCLUDES_BASE_H
 #include <stdio.h>
 #include <string.h>
 #include <stdlib.h>
 #include <raylib.h>
 #include <rlgl.h>
 #include <raymath.h>
 #include "env.h"
 #include "rayext.h"

 #if defined(INCLUDE_GUI)
  #include "gui_setup.h"
  #include "gui_structs.h"
  #include "gui.h"
 #endif
 #if defined(INCLUDE_GAME)
  #include "game_structs.h"
  #include "game_gui.h"
  #include "game.h"
 #endif
#endif
