#include "errorApplication.h"

errorApplication::errorApplication(Config *c): Gtk::Application("moe.emmaexe.ntfyDesktopConfigurator"), config(c) {
    g_set_application_name("ntfyDesktop Configurator");
}

Glib::RefPtr<errorApplication> errorApplication::create(Config *c) {
    return Glib::RefPtr<errorApplication>(new errorApplication(c));
}

void errorApplication::on_startup() {
    Gtk::Application::on_startup();
}

void errorApplication::on_activate() {
    create_window();
}

void errorApplication::create_window() {
    window = new errorWindow(config);
    window->signal_hide().connect(sigc::mem_fun(*this, &errorApplication::on_window_hide));
    add_window(*window);
}

void errorApplication::on_window_hide() {
    delete window;
}