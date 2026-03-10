imKairos

- Fuentes: usar libreria para cargar fuentes TTF con alta calidad. Que VT323 se vea como aquí en un editor.

- Scroll button menu -> botones mas abajo de lo q deberian OK
- Consistency: todos los q permitan colores y fuentes deberian recibir param en la funcion. Usar global solo para obtener defaults desde afuera.
- Textbox improvements
    - int y float
    - scroll contents           OK
    - copy/paste
    - jump between spaces
- Limpieza:
  - Overlay     OK
    - Grids     OK
    - Window    OK
    - Controls  OK
- THEME
  - Revisar tema oscuro
  - Revisar tema claro

- Program menu
    - New
    - Open
    - Close             OK

- Tabs!
    - Inside window

- Resize
    - Works?            OK
    - Add other points

- Icons
    - windows       OK
    - switch

- Implement Slider.
    - Play with hue-slider to test theme colors.

- Implement docking.
    - First by code. (Kinda?)
    - W/Interactions

- Organizar ajustes para temas                          OK
    - Remover padding y reemplazarlo por un delta       OK
    - Editor


DONE

- Reemplazar ids por automaticos asi:
    - Global[OK]/Window[? si es necesario]:
        id == posicion de memoria                       OK
        esto se usa solo para saber si estoy focused    ^
        Limitación menor.
- Add transparency          OK

- Determinar tamaño icons al 0.5 del original.          OK

- Cut controls
    - Test          (OK)
    - Do all rest    OK

- Change cursor when hover a textbox.   OK
    - Support multiple cursors.         OK
    - Change cursor style.              OK


- Windows z-index
    - WindowStruct                                  (OK)
    - GUI_Window adds a Window GUI_RenderWindows    (OK)

- Interact conditions -> Functions to allow further customization.  OK

- Scrolling.                                OK
    - Auto scroll (do not set pre-value)    OK
        - Render stores window size and     OK
        -  uses it for next render.         OK

- Button menu           OK
    - Draw at the end   OK
    - Overflow          OK
    - Icon sizes        OK
    - Generalize
        - Postrender    OK
        - Macros        OK

- Add statuses  OK
- Fix IsCursorOverGui()    OK
- Asserts to be project-wise (rayext)   OK
- Close windows     OK
- Review GUI_CTX    OK
