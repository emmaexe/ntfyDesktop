package moe.emmaexe.ntfyDesktop;

import java.io.BufferedReader;
import java.io.File;
import java.io.InputStreamReader;
import java.net.URL;

import org.json.simple.JSONObject;
import org.json.simple.JSONValue;

import moe.emmaexe.ntfyDesktop.utils.FileManager;
import moe.emmaexe.ntfyDesktop.utils.LogManager;
import moe.emmaexe.ntfyDesktop.utils.StringUtils;
import moe.emmaexe.ntfyDesktop.utils.Terminal;
import moe.emmaexe.ntfyDesktop.utils.TimeManager;

public class NotificationThread extends Thread {
    public JSONObject notificationSource;
    public NotificationThread(JSONObject notificationSource) {
        this.notificationSource = notificationSource;
    }
    public void run() {
        LogManager.log("Started thread " + (String)notificationSource.get("name") + "; Now listening to: " + (String)notificationSource.get("url"));
        try {
            URL url = new URL((String)notificationSource.get("url"));
            BufferedReader reader = new BufferedReader( new InputStreamReader(url.openStream()) );
            String inputLine;
            while ((inputLine = reader.readLine()) != null) { handleNotification(inputLine); }
            reader.close();
        } catch (Exception e) { LogManager.error(e.toString()); }
        LogManager.log("Closed thread " + (String)notificationSource.get("name") + "; No longer listening to: " + (String)notificationSource.get("url"));
    }
    public static void handleNotification(String strNotification) {
        JSONObject notification = (JSONObject)JSONValue.parse(strNotification);
        if (notification.get("message") != null) {
            String message = notification.get("message").toString();
            String title = "ntfyDesktop";
            if (notification.get("title") != null) {
                title = (String)notification.get("title");
            }
            title = StringUtils.surroundInQuotes(title);
            message = StringUtils.surroundInQuotes(message);
            if (notification.get("icon") != null) {
                String iconString = notification.get("icon").toString();
                try {
                    URL iconUrl = new URL(iconString);
                    File file = FileManager.downloadTempFile(iconUrl);
                    Terminal.runCommand("notify-send -i \"" + file.toString() + "\" " + title + " " + message);
                    TimeManager.asyncSetTimeout(() -> { FileManager.deleteFile(file); }, 5000); // Added 5 second delay to make sure the notification daemon can grab the file before it is deleted (otherwise the icon is sometimes not displayed)
                } catch (Exception e) { LogManager.error(e.toString()); }
            } else {
                File file = FileManager.getTempResource("/defaultNotificationIcon.png");
                Terminal.runCommand("notify-send -i \"" + file.toString() + "\" " + title + " " + message);
                TimeManager.asyncSetTimeout(() -> { FileManager.deleteFile(file); }, 5000); // Added 5 second delay to make sure the notification daemon can grab the file before it is deleted (otherwise the icon is sometimes not displayed)
            }
        }
    }
}
