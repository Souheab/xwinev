#include <X11/Xlib.h>
#include <locale.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>

Display *dpy;
Window root;
volatile sig_atomic_t running = 1;

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

static void sigint_handler(int signum) {
  running = 0;
  log_f("Received SIGINT. Exiting...\n");
}

static int error_handler(Display *dpy, XErrorEvent *ev) {
  char error_text[256];
  XGetErrorText(dpy, ev->error_code, error_text, sizeof(error_text));
  log_warn_f(
      "X11 error: %s (request code: %d, error code: %d, resource id: %lu)\n",
      error_text, ev->request_code, ev->error_code, ev->resourceid);
  return 0;
}

static void handle_create_notify(XCreateWindowEvent *ev) {
  log_f("Window created: 0x%lx\n", ev->window);
}

static void handle_destroy_notify(XDestroyWindowEvent *ev) {
  log_f("Window destroyed: 0x%lx\n", ev->window);
}

static void handle_configure_notify(XConfigureEvent *ev) {
  log_f("Window 0x%lx configured: pos(%d, %d), size(%d, %d)\n", ev->window,
        ev->x, ev->y, ev->width, ev->height);
}

static void handle_map_notify(XMapEvent *ev) {
  log_f("Window mapped: 0x%lx\n", ev->window);
}

static void handle_unmap_notify(XUnmapEvent *ev) {
  log_f("Window unmapped: 0x%lx\n", ev->window);
}

static void handle_expose(XExposeEvent *ev) {
  log_f("Window 0x%lx exposed: pos(%d, %d), size(%d, %d)\n", ev->window, ev->x,
        ev->y, ev->width, ev->height);
}

static void handle_property_notify(XPropertyEvent *ev) {
  log_f("PropertyNotify for window 0x%lx: atom %lu, time %lu, state %s\n",
        ev->window, ev->atom, ev->time,
        (ev->state == PropertyNewValue) ? "NewValue" : "Deleted");
}

int main(int argc, char **argv) {
  setlocale(LC_ALL, "");
  int scr;

  dpy = XOpenDisplay(NULL);

  if (!dpy) {
    log_fatalf("Failed to open X11 display\n");
  } else {
    log_f("Opened X11 display: %s\n", DisplayString(dpy));
  }

  struct sigaction sa;
  sa.sa_handler = sigint_handler;
  if (sigaction(SIGINT, &sa, NULL) == -1) {
    log_fatalf("Failed to set up SIGINT handler\n");
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

  XEvent ev;
  bool event_processed = false;

  while (running) {
    if (XPending(dpy)) {
      XNextEvent(dpy, &ev);
      event_processed = false;
    }

    if (!event_processed) {
      switch (ev.type) {
      case CreateNotify:
        handle_create_notify(&ev.xcreatewindow);
        break;
      case DestroyNotify:
        handle_destroy_notify(&ev.xdestroywindow);
        break;
      case ConfigureNotify:
        handle_configure_notify(&ev.xconfigure);
        break;
      case MapNotify:
        handle_map_notify(&ev.xmap);
        break;
      case UnmapNotify:
        handle_unmap_notify(&ev.xunmap);
        break;
      case Expose:
        handle_expose(&ev.xexpose);
        break;
      case PropertyNotify:
        handle_property_notify(&ev.xproperty);
        break;
      default:
        log_f("Unhandled event type: %d\n", ev.type);
        break;
      }
    }

    event_processed = true;
  }

  XCloseDisplay(dpy);
  return 0;
}
