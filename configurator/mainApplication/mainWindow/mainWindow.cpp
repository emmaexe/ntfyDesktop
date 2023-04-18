#include "mainWindow.h"
#include "../mainApplication.h"

mainWindow::mainWindow(Config *c, mainApplication *p, int scrollTo): parentApplication(p), config(c) {
    add(mainGrid);
    
    builder = Gtk::Builder::create_from_resource("/ntfyDesktop/mainApplication/mainWindow/toolbar.ui");
    builder->get_widget("toolbar", toolbar);
    builder->get_widget("newtab", newTab);
    builder->get_widget("save", save);
    builder->get_widget("deletetab", deleteTab);
    
    mainGrid.attach(*toolbar, 0, 0);
    mainGrid.attach(scrolledWindow, 0, 1);

    actionGroup = Gio::SimpleActionGroup::create();
    actionGroup->add_action("newtab", sigc::mem_fun(*this, &mainWindow::on_newtab));
    actionGroup->add_action("save", sigc::mem_fun(*this, &mainWindow::on_save));
    actionGroup->add_action("deletetab", sigc::mem_fun(*this, &mainWindow::on_deletetab));
    insert_action_group("toolbargroup", actionGroup);
    
    notebook.set_scrollable();
    notebook.set_hexpand(true);
    notebook.set_vexpand(true);
    scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC);
    scrolledWindow.set_hexpand(true);
    scrolledWindow.set_vexpand(true);
    scrolledWindow.add(notebook);

    nlohmann::json sources = config->operator[]("sources");
    for (int i = 0;i<sources.size();i++) {
        notebookTabs.push_back(notebookTab(config, i));
        notebook.append_page(notebookTabs.at(i), sources[i]["name"]);
    }
    if (scrollTo != -1) { notebook.set_current_page(scrollTo); }

    mainGrid.set_row_spacing(5);
    mainGrid.set_column_spacing(5);
    mainGrid.set_border_width(10);
    geometry.max_height = 600;
    geometry.max_width = 800;
    geometry.min_height = 320;
    geometry.min_width = 360;
    set_geometry_hints(*this, geometry, Gdk::HINT_MAX_SIZE | Gdk::HINT_MIN_SIZE);
    set_default_size(560, 410);

    show_all_children();
}

void mainWindow::on_newtab() {
    config->changesMade = true;
    nlohmann::json oneSource; oneSource["name"] = ""; oneSource["url"] = "";
    config->operator[]("sources").push_back(oneSource);
    notebookTabs.push_back(notebookTab(config, notebookTabs.size(), true));
    std::string tabName = "Unnamed " + std::to_string(config->operator[]("sources").size());
    notebook.append_page(notebookTabs.at(notebookTabs.size()-1), tabName);
    notebook.set_current_page(-1);
}

void mainWindow::on_save() {
    config->changesMade = false;
    config->operator[]("sources") = Config::array();
    std::vector<notebookTab>::iterator it;
    for (it = notebookTabs.begin(); it != notebookTabs.end(); it++) {
        nlohmann::json oneSource;
        oneSource["name"] = it->nameEntry->get_text();
        oneSource["url"] = "https://" + it->serverEntry->get_text() + "/" + it->topicEntry->get_text() + "/json";
        config->operator[]("sources").push_back(oneSource);
    }
    config->write();
    parentApplication->restart_window(notebook.get_current_page());
}

void mainWindow::on_deletetab() {
    config->changesMade = true;
    int page = notebook.get_current_page();
    notebook.remove_page(page);
    config->operator[]("sources").erase(page);
    notebookTabs.erase(notebookTabs.begin() + page);
}