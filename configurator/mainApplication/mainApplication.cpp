#include "mainApplication.h"

mainApplication::mainApplication(Config *c): Gtk::Application("moe.emmaexe.ntfyDesktopConfigurator"), config(c) {
    g_set_application_name("ntfyDesktop Configurator");
}

Glib::RefPtr<mainApplication> mainApplication::create(Config *c) {
    return Glib::RefPtr<mainApplication>(new mainApplication(c));
}

void mainApplication::on_startup() {
    Gtk::Application::on_startup();

    builder = Gtk::Builder::create_from_resource("/ntfyDesktop/mainApplication/mainApplication.ui");
    add_action("start", sigc::mem_fun(*this, &mainApplication::on_menu_ntfydesktop_start));
    add_action("stop", sigc::mem_fun(*this, &mainApplication::on_menu_ntfydesktop_stop));
    add_action("about", sigc::mem_fun(*this, &mainApplication::on_menu_help_about));
    add_action("quit", sigc::mem_fun(*this, &mainApplication::on_menu_quit));
    add_action("save", sigc::mem_fun(*this, &mainApplication::on_menu_file_save));
    add_action("reload", sigc::mem_fun(*this, &mainApplication::on_menu_file_reload));
    
    Glib::RefPtr<Glib::Object> object;
    object = builder->get_object("mainmenu"); 
    Glib::RefPtr<Gio::Menu> gmenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
    object = builder->get_object("appmenu");
    Glib::RefPtr<Gio::Menu> appMenu = Glib::RefPtr<Gio::Menu>::cast_dynamic(object);
    
    set_app_menu(appMenu);
    set_menubar(gmenu);
}

void mainApplication::on_activate() {
    create_window();
}

void mainApplication::create_window(int scrollTo) {
    window = new mainWindow(config, this, scrollTo);
    window->signal_delete_event().connect(sigc::bind<Gtk::Window*>(sigc::mem_fun(*this, &mainApplication::on_window_delete), window));
    add_window(*window);
    window->show_all();
}

bool mainApplication::on_window_delete(GdkEventAny* event, Gtk::Window* w) {
    return !check_quit();
}

void mainApplication::restart_window(int scrollTo) {
    if (check_quit()) {
        window->hide();
        create_window(scrollTo);
        config->read();
        config->changesMade = false;
    }
}

void mainApplication::on_menu_ntfydesktop_start() {
    printf("Start\n"); //TODO: Interact with systemd to start the java service
}

void mainApplication::on_menu_ntfydesktop_stop() {
    printf("Stop\n"); //TODO: Interact with systemd to stop the java service
}

void mainApplication::on_menu_quit() {
    if (check_quit()) {
        quit();
        /*
            std::vector<Gtk::Window*> windows = get_windows();
            for (int i = 0;i<windows.size();i++) { windows[i]->hide(); }
        */
        window->hide();
    }
}

void mainApplication::on_menu_help_about() {
    printf("About\n"); //TODO: Delete about button and add buttons that open github and docs.
}

void mainApplication::on_menu_file_save() {
    window->on_save();
}

void mainApplication::on_menu_file_reload() {
    restart_window();
}

bool mainApplication::check_quit() {
    if (config->changesMade == false) { config->changesMade = check_changes_made(); }
    if (config->changesMade) {
        Gtk::MessageDialog messageDialog("Quit without saving?", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_OK_CANCEL);
        messageDialog.set_secondary_text("You have unsaved changes.\nPress <b>OK</b> to quit anyway or <b>CANCEL</b> to cancel.", true);
        int response = messageDialog.run();
        if (response == Gtk::RESPONSE_OK) {
            return true;
        } else if (response == Gtk::RESPONSE_CANCEL) {
            return false;
        }
    }
    return true;
}

bool mainApplication::check_changes_made() {
    if (config->changesMade) { return true; }
    if (window->notebookTabs.size() != config->operator[]("sources").size()) { return true; }
    std::vector<notebookTab>::iterator nbt;
    nlohmann::json::iterator cnf;
    for (nbt = window->notebookTabs.begin(), cnf = config->operator[]("sources").begin(); nbt != window->notebookTabs.end() && cnf != config->operator[]("sources").end(); nbt++, cnf++ ) {
        std::string name = nbt->nameEntry->get_text();
        std::string url = "https://" + nbt->serverEntry->get_text() + "/" + nbt->topicEntry->get_text() + "/json";
        if (cnf->operator[]("name") != name || cnf->operator[]("url") != url) {
            return true;
        }
    }
    return false;
}