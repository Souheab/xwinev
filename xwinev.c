#include <X11/Xlib.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

Display *dpy;
Window root;

static void log_fatalf(const char *str, ...) {
  va_list args;
  va_start(args, str);
  printf("FATAL ERROR: ");
  vprintf(str, args);
  va_end(args);
  exit(EXIT_FAILURE);
}

static void log_f(const char *str, ...) {
  va_list args;
  va_start(args, str);
  printf("LOG: ");
  vprintf(str, args);
  va_end(args);
}

static void log_warn_f(const char *str, ...) {
  va_list args;
  va_start(args, str);
  printf("WARNING: ");
  vprintf(str, args);
  va_end(args);
}

static int error_handler(Display *dpy, XErrorEvent *ev) {
  char error_text[256];
  XGetErrorText(dpy, ev->error_code, error_text, sizeof(error_text));
  log_warn_f(
      "X11 error: %s (request code: %d, error code: %d, resource id: %lu)\n",
      error_text, ev->request_code, ev->error_code, ev->resourceid);
  return 0;
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  Window root_return, parent_return;
  Window *children;
  unsigned int nchildren;
  int scr;

  dpy = XOpenDisplay(NULL);

  if (!dpy) {
    log_fatalf("Failed to open X11 display\n");
  } else {
    log_f("Opened X11 display: %s\n", DisplayString(dpy));
  }

  scr = DefaultScreen(dpy);
  root = RootWindow(dpy, scr);
  XSetErrorHandler(error_handler);
  XSynchronize(dpy, True);

  XSelectInput(dpy, root,
               SubstructureNotifyMask |
               ExposureMask |
               StructureNotifyMask |
               PropertyChangeMask);

  XQueryTree(dpy, root, &root_return, &parent_return, &children, &nchildren);
  XEvent ev;
  bool compositing_done = false;
  while (XPending(dpy)) {
    XNextEvent(dpy, &ev);
    compositing_done = false;
    switch (ev.type) {
    case CreateNotify:
      break;
    case DestroyNotify:
      break;
    case ConfigureNotify:
      break;
    case MapNotify:
      break;
    case UnmapNotify:
      break;
    case Expose:
    default:
      break;
    }
  }

  XCloseDisplay(dpy);
  return 0;
}
