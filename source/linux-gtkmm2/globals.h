//
// Created by dang on 2/11/18.
//

#ifndef C_CLIENT_GLOBALS_H
#define C_CLIENT_GLOBALS_H

#include "../openwl.h"

#include <gtkmm/main.h>
#include <gtkmm/accelgroup.h>
#include <gtkmm/clipboard.h>

extern wlEventCallback eventCallback;
extern Gtk::Main *appMain;
extern Glib::RefPtr<Gtk::AccelGroup> globalAccelGroup;
extern Glib::RefPtr<Gtk::Clipboard> gtkClipboard;

#endif //C_CLIENT_GLOBALS_H
