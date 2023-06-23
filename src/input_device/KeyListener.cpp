#include "KeyListener.hpp"
#include <iostream>

void KeyListener::keyCallback(GLFWwindow* window, int keycode, int scancode, int action, int mods)
{
  if (action == GLFW_PRESS) {
    if (keycode == GLFW_KEY_CAPS_LOCK) {
      capsLock = !capsLock;
    }

    keys[keycode] = true;
  }
  else if (action == GLFW_RELEASE) {
    keys[keycode] = false;
  }
}

// TODO: Put this in an Error / Warning handler file.
void KeyListener::warning(int keycode)
{
  std::cout << "Warning: Tried to reach " << keycode << " but it is out of " << 
  "range of the number of stablished as available keys: " << 
  NUMBER_OF_KEYS << ".";
}

void KeyListener::update()
{
  for (int i = 0; i < NUMBER_OF_KEYS; i++) {
    lastKeys[i] = keys[i];
  }
}

bool KeyListener::isKeyPressed(int keycode)
{
  if (keycode < NUMBER_OF_KEYS) {
    return keys[keycode];
  }

  warning(keycode);
  return false;
}

bool KeyListener::isKeyDown(int keycode)
{
  if (keycode < NUMBER_OF_KEYS) {
    return keys[keycode] && !lastKeys[keycode];
  }

  warning(keycode);
  return false;
}

bool KeyListener::isKeyUp(int keycode)
{
  if (keycode < NUMBER_OF_KEYS) {
    return !keys[keycode] && lastKeys[keycode];
  }

  warning(keycode);
  return false;
}

bool KeyListener::isBindPressed(int modKeycode, int keycode)
{
  return isKeyPressed(modKeycode) && isKeyPressed(keycode);
}

bool KeyListener::isBindDown(int modKeycode, int keycode)
{
  return isKeyPressed(modKeycode) && isKeyDown(keycode);
}

bool KeyListener::isBindUp(int modKeycode, int keycode)
{
  return isKeyPressed(modKeycode) && isKeyUp(keycode);
}

bool KeyListener::isAnyKeyPressed()
{
  for (auto key : keys) {
    if (key)
      return true;
  }

  return false;
}

bool KeyListener::isCapsLock()
{
  return capsLock;
}