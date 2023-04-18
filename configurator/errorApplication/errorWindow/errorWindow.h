#ifndef ERRORWINDOW_H
#define ERRORWINDOW_H

#include <gtkmm/window.h>
#include <gtkmm/grid.h>
#include <gtkmm/messagedialog.h>
#include <gtkmm/label.h>
#include <gtkmm/button.h>
#include <gtkmm/textbuffer.h>
#include <gtkmm/textview.h>
#include <gtkmm/scrolledwindow.h>
#include <gtkmm/infobar.h>
#include "../../config/config.h"
#include "../../programInfo.h"

class errorWindow: public Gtk::Window {
    public:
        errorWindow(Config *c);
    protected:
        Config *config;
        Gtk::Grid grid;
        Gtk::InfoBar infoBar;
        Gtk::Label top, infoBarMessage;
        Gtk::Button copy, reset, quit;
        void on_button_copy(); void on_button_reset(); void on_button_quit();
        void on_infobar_ok(int res);
        Glib::RefPtr<Gtk::TextBuffer> textBuffer;
        Gtk::TextView textView;
        Gtk::ScrolledWindow scrolledWindow;
};

#endif //ERRORWINDOW_H