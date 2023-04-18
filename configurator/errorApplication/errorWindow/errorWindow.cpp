#include "errorWindow.h"

errorWindow::errorWindow(Config *c): config(c) {
    set_title("Error");
    set_border_width(10); set_default_size(400, 200); set_resizable(false);
    add(grid);

    grid.set_valign(Gtk::ALIGN_CENTER); grid.set_halign(Gtk::ALIGN_CENTER); grid.set_row_spacing(5); grid.set_column_spacing(5);
    grid.attach(infoBar, 0, 0, 3, 1);
    grid.attach(top, 0, 1, 1, 1);
    grid.attach(scrolledWindow, 0, 2, 3, 1);
    grid.attach(copy, 0, 3, 1, 1); grid.attach(reset, 1, 3, 1, 1); grid.attach(quit, 2, 3, 1, 1);
    
    Gtk::Container *infoBarContainer = dynamic_cast<Gtk::Container*>(infoBar.get_content_area());
    infoBarContainer->add(infoBarMessage); 
    infoBar.add_button("_OK", 0);
    infoBar.signal_response().connect(sigc::mem_fun(*this, &errorWindow::on_infobar_ok));
    
    top.set_label("An error has occurred:"); 
    top.set_halign(Gtk::ALIGN_START);
    
    textBuffer = Gtk::TextBuffer::create(); 
    textBuffer->set_text(config->errorMessage);

    textView.set_editable(false); 
    scrolledWindow.add(textView); 
    scrolledWindow.set_policy(Gtk::POLICY_AUTOMATIC, Gtk::POLICY_AUTOMATIC); 
    textView.set_buffer(textBuffer);
    
    copy.set_label("Copy to clipboard"); 
    copy.signal_clicked().connect(sigc::mem_fun(*this, &errorWindow::on_button_copy)); 
    copy.set_margin_start(50);
    
    reset.set_label("Reset all configuration"); 
    reset.signal_clicked().connect(sigc::mem_fun(*this, &errorWindow::on_button_reset));

    quit.set_label("Quit"); 
    quit.signal_clicked().connect(sigc::mem_fun(*this, &errorWindow::on_button_quit));
    
    show_all(); infoBar.hide();
}

void errorWindow::on_button_copy() {
    Glib::RefPtr<Gtk::Clipboard> clipboard = Gtk::Clipboard::get();
    clipboard->set_text(config->errorMessage);
    infoBarMessage.set_text("Copied to clipboard.");
    infoBar.set_message_type(Gtk::MESSAGE_INFO);
    infoBar.show();
}

void errorWindow::on_button_reset() {
    Config c;
    c["version"] = PROGRAM_VERSION;
    c["sources"] = Config::array();
    Gtk::MessageDialog messageDialog("The ntfyDesktop configuration was reset.", false, Gtk::MESSAGE_INFO, Gtk::BUTTONS_CLOSE);
    messageDialog.set_secondary_text("The file <b>~/.config/ntfyDesktop/config.json</b> was reset to default values.", true);
    c.write();
    messageDialog.run();
    hide();
}

void errorWindow::on_button_quit() {
    hide();
}

void errorWindow::on_infobar_ok(int res) {
    infoBarMessage.set_text("");
    infoBar.hide();
}