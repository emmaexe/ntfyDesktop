package moe.emmaexe.ntfyDesktop;

import java.io.IOException;
import java.net.MalformedURLException;
import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;

import org.json.simple.JSONArray;
import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

import moe.emmaexe.ntfyDesktop.utils.Config;
import moe.emmaexe.ntfyDesktop.utils.FileManager;
import moe.emmaexe.ntfyDesktop.utils.LogManager;

public class NtfyDesktopApp {
    public static String version = (String)((JSONObject)JSONValue.parse(FileManager.readFileAsString(FileManager.getTempResource("/config.json")))).get("version");
    public static void main(String[] args) throws IOException, MalformedURLException {
        LogManager.init();
        LogManager.log("Started.");
        JSONArray sourcesArray = (JSONArray)Config.staticConfig.get("sources");
        if (sourcesArray.size() == 0) {
            LogManager.warn("No sources found. To continue, add valid sources to the config file.");
        } else {
            ExecutorService executor = Executors.newFixedThreadPool(sourcesArray.size());
            for (int i = 0;i<sourcesArray.size();i++) {
                NotificationThread thread = new NotificationThread((JSONObject)sourcesArray.get(i));
                executor.execute(thread);
            }
            executor.shutdown();
        }
    }
}