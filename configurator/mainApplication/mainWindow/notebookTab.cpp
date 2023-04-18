#include "notebookTab.h"

notebookTab::notebookTab(Config *c, int tI, bool newTab): config(c), tabIndex(tI) {
    builder = Gtk::Builder::create_from_resource("/ntfyDesktop/mainApplication/mainWindow/notebookTab.ui");
    builder->get_widget("name", name); builder->get_widget("server", server); builder->get_widget("topic", topic);
    builder->get_widget("nameentry", nameEntry); builder->get_widget("serverentry", serverEntry); builder->get_widget("topicentry", topicEntry);

    attach(*name, 0, 0); attach(*nameEntry, 1, 0); 
    //nameEntry->signal_changed().connect(sigc::mem_fun(*this, &notebookTab::on_changes_made_name));

    attach(*server, 0, 1); attach(*serverEntry, 1, 1); 
    //serverEntry->signal_changed().connect(sigc::mem_fun(*this, &notebookTab::on_changes_made_server));
    
    attach(*topic, 0, 2); attach(*topicEntry, 1, 2); 
    //topicEntry->signal_changed().connect(sigc::mem_fun(*this, &notebookTab::on_changes_made_topic));
    
    nameEntry->set_text(config->operator[]("sources")[tabIndex]["name"]);    
    std::string url = config->operator[]("sources")[tabIndex]["url"];
    std::regex valid_url("https:\\/\\/[a-zA-Z0-9]+\\.[a-zA-Z0-9]+\\/.+\\/json");
    if (std::regex_match(url, valid_url)) {
        url = url.substr(8, url.size()-13); //remove "https://" and "/json"
        serverEntry->set_text(url.substr(0, url.find_last_of('/')));
        topicEntry->set_text(url.substr(url.find_last_of('/')+1, url.size()-url.find_last_of('/')-1));
    } else if (!newTab) {
        Gtk::MessageDialog dialog("Malformed URL warning: in config file in sources array at index: " + std::to_string(tabIndex) + "\nThis URL will be ignored.");
        dialog.run();
    }
    
    set_valign(Gtk::ALIGN_CENTER); set_halign(Gtk::ALIGN_CENTER);
    
    set_row_spacing(5); 
    set_column_spacing(20);
    set_border_width(5);

    show_all();
}

/*
void notebookTab::on_changes_made_name() {
    printf("Changes made to name\n");
}

void notebookTab::on_changes_made_server() {
    printf("Changes made to server\n");
}

void notebookTab::on_changes_made_topic() {
    printf("Changes made to topic\n");
}
*/