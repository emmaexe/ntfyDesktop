#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <gtkmm/applicationwindow.h>
#include <gtkmm/builder.h>
#include <gtkmm/button.h>
#include <gtkmm/grid.h>
#include <gtkmm/notebook.h>
#include <gtkmm/toolbar.h>
#include <gtkmm/scrolledwindow.h>
#include <giomm/simpleactiongroup.h>
#include <vector>
#include "notebookTab.h"
#include "../../config/config.h"

class mainApplication;

class mainWindow: public Gtk::ApplicationWindow {
    public:
        mainWindow(Config *c, mainApplication *p, int scrollTo = -1);
        void on_save();
        std::vector<notebookTab> notebookTabs;
    protected:
        mainApplication *parentApplication;
        Config *config;
        Gdk::Geometry geometry;
        Gtk::Grid mainGrid;
        Gtk::ScrolledWindow scrolledWindow;
        Gtk::Notebook notebook;
        Gtk::Toolbar *toolbar; Gtk::ToolButton *newTab, *save, *deleteTab;
        void on_newtab(); void on_deletetab();
        Glib::RefPtr<Gtk::Builder> builder;
        Glib::RefPtr<Gio::SimpleActionGroup> actionGroup;
};

#endif //MAINWINDOW_H