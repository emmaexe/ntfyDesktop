#ifndef ERRORAPPLICATION_H
#define ERRORAPPLICATION_H

#include "errorWindow/errorWindow.h"
#include "../config/config.h"
#include <gtkmm/application.h>

class errorApplication: public Gtk::Application {
    protected:
        errorApplication(Config *c);
    public:
        static Glib::RefPtr<errorApplication> create(Config *c);
    protected:
        void on_startup() override;
        void on_activate() override;
    private:
        Config *config;
        errorWindow *window;
        void create_window();
        void on_window_hide();
};

#endif //ERRORAPPLICATION_H