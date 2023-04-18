#ifndef NOTEBOOKTAB_H
#define NOTEBOOKTAB_H

#include <regex>
#include <gtkmm/grid.h>
#include <gtkmm/label.h>
#include <gtkmm/entry.h>
#include <gtkmm/builder.h>
#include <gtkmm/messagedialog.h>
#include "../../config/config.h"

class notebookTab: public Gtk::Grid {
    public:
        notebookTab(Config *c, int tI, bool newTab = false);
        Gtk::Entry *nameEntry, *serverEntry, *topicEntry;
    protected:
        Config *config;
        int tabIndex;
        Gtk::Label *name, *server, *topic;
        Glib::RefPtr<Gtk::Builder> builder;
        //void on_changes_made_name(); void on_changes_made_server(); void on_changes_made_topic();
};

#endif