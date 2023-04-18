package moe.emmaexe.ntfyDesktop.utils;

import java.io.File;
import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.Date;

public class LogManager {
    private static final DateFormat date = new SimpleDateFormat("yyyy-MM-dd HH:mm:ss");
    public static void init() {
        for (int i = 10;i >= 0;i--) {
            if (i == 10) {
                File logFile = new File(FileManager.getLogDir(), "ntfy.log.10");
                if (logFile.exists()) {
                    FileManager.deleteFile(logFile);
                }
            } else if (i == 0) {
                File logFile = new File(FileManager.getLogDir(), "ntfy.log");
                if (logFile.exists()) {
                    FileManager.renameFile(logFile, "ntfy.log.1");
                }
            } else {
                File logFile = new File(FileManager.getLogDir(), "ntfy.log." + Integer.toString(i));
                if (logFile.exists()) {
                    FileManager.renameFile(logFile, "ntfy.log." + Integer.toString(i+1));
                }
            }
        }
    }
    public static void log(String message) {
        System.out.println(message);
        String timestamp = date.format(new Date());
        FileManager.writeStringToFile("[" + timestamp + "][log] " + message + "\n", new File(FileManager.getLogDir(), "ntfy.log"), true);
    }
    public static void warn(String message) {
        System.out.println(message);
        String timestamp = date.format(new Date());
        FileManager.writeStringToFile("[" + timestamp + "][warn] " + message + "\n", new File(FileManager.getLogDir(), "ntfy.log"), true);
    }
    public static void error(String message) {
        System.out.println(message);
        String timestamp = date.format(new Date());
        FileManager.writeStringToFile("[" + timestamp + "][error] " + message + "\n", new File(FileManager.getLogDir(), "ntfy.log"), true);
    }
}
