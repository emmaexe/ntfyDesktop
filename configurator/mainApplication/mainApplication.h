#ifndef MAINAPPLICATION_H
#define MAINAPPLICATION_H

#include "mainWindow/mainWindow.h"
#include "../config/config.h"
#include <gtkmm/application.h>
#include <gtkmm/builder.h>

class mainApplication: public Gtk::Application {
    protected:
        mainApplication(Config *c);
    public:
        static Glib::RefPtr<mainApplication> create(Config *c);
        void restart_window(int scrollTo = -1);
    protected:
        void on_startup() override;
        void on_activate() override;
    private:
        Config *config;
        mainWindow *window;
        void create_window(int scrollTo = -1);
        bool on_window_delete(GdkEventAny* event, Gtk::Window* w);
        void on_menu_ntfydesktop_start();
        void on_menu_ntfydesktop_stop();
        void on_menu_help_about();
        void on_menu_quit();
        void on_menu_file_save();
        void on_menu_file_reload();
        Glib::RefPtr<Gtk::Builder> builder;
        bool check_quit();
        bool check_changes_made();
};

#endif //MAINAPPLICATION_H