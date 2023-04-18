package moe.emmaexe.ntfyDesktop.utils;

import java.io.File;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

public class Config {
    public static JSONObject staticConfig = readConfig();
    private static JSONObject readConfig() {
        String configString = FileManager.readFileAsString(new File(FileManager.getConfigDir(), "config.json"));
        if (StringUtils.stringHasLetterNumberSymbol(configString)) {
            JSONObject config = (JSONObject)JSONValue.parse(configString);
            populateEssential(config);
            return config;
        } else {
            LogManager.error("Config file empty or does not exist. Creating template config file.");
            createTemplateConfig();
            System.exit(0);
            return new JSONObject(); // To please the IDE.
        }
    }
    private static void createTemplateConfig() {
        File configFile = new File(FileManager.getConfigDir(), "config.json");
        FileManager.getResource("/config.json", configFile);
    }
    @SuppressWarnings("unchecked")
    private static JSONObject populateEssential(JSONObject config) {
        if (!config.containsKey("version")) {
            config.putIfAbsent("version", "1.0.0");
            LogManager.error("Missing key \"version\" in config.json");
        }
        if (!config.containsKey("sources")) {
            config.putIfAbsent("sources", new JSONArray());
            LogManager.error("Missing key \"sources\" in config.json");
        }
        return config;
    }
}