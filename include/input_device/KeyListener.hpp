#pragma once

#include "GLFW/glfw3.h"

class KeyListener
{
private:
  static inline const uint16_t NUMBER_OF_KEYS = 500;
  static inline bool lastKeys[NUMBER_OF_KEYS] = { false };
  static inline bool keys[NUMBER_OF_KEYS] = { false };

  // TODO: Get caps lock status when the app runs.
  // See more in https://github.com/davidalmarinho/PieceOfCake/issues/3. 
  static inline bool capsLock = false;

  static void warning(int keycode);

public:

  static void keyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods);

  static void update();

  static bool isKeyPressed(int keycode);
  static bool isKeyDown(int keycode);
  static bool isKeyUp(int keycode);
  static bool isBindPressed(int modKeycode, int keycode);
  static bool isBindDown(int modKeycode, int keycode);
  static bool isBindUp(int modeKeycode, int keycode);
  static bool isAnyKeyPressed();
  static bool isCapsLock();
};