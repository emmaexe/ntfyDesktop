#include "mainApplication/mainApplication.h"
#include "errorApplication/errorApplication.h"
#include "config/config.h"
#include "resources.h"

int main(int argc, char *argv[]) {
    GResource *resource = resources_get_resource(); g_resources_register(resource);
    Config config;
    if (config.ok) {
        auto app = mainApplication::create(&config);
        return app->run(argc, argv);
    } else {
        auto app = errorApplication::create(&config);
        return app->run(argc, argv);
    }
}