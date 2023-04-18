package moe.emmaexe.ntfyDesktop.utils;

import java.io.File;
import java.io.IOException;
import java.net.URL;
import java.nio.charset.Charset;
import java.util.UUID;

import org.apache.commons.io.FileUtils;

public class FileManager {
    public static File getDir() {
        URL pathURL = FileManager.class.getProtectionDomain().getCodeSource().getLocation();
        File jarPath = FileUtils.toFile(pathURL);
        File path = new File(jarPath.getParent());
        return path;
    }
    public static File getConfigDir() {
        String configStr = System.getenv("HOME");
        configStr = configStr + "/.config/ntfyDesktop/";
        File file = new File(configStr);
        if (!file.exists()) { file.mkdir(); }
        return file;
    }
    public static File getTempDir() {
        String TMPDIR = System.getenv("TMPDIR");
        if (TMPDIR != null) {
            return new File(TMPDIR);
        } else {
            return new File("/tmp/");
        }
    }
    public static File getLogDir() {
        return getDir();
    }
    public static String readFileAsString(File file) {
        String data = "";
        try {
            data = FileUtils.readFileToString(file, Charset.forName("UTF-8"));
        } catch (IOException e) { LogManager.error(e.toString()); }
        return data;
    }
    public static void writeStringToFile(String string, File file) {
        try {
            FileUtils.writeStringToFile(file, string, Charset.forName("UTF-8"));
        } catch (IOException e) { LogManager.error(e.toString()); }
    }
    public static void writeStringToFile(String string, File file, boolean append) {
        try {
            FileUtils.writeStringToFile(file, string, Charset.forName("UTF-8"), append);
        } catch (IOException e) { LogManager.error(e.toString()); }
    }
    public static void deleteFile(File file) {
        try {
            FileUtils.delete(file);
        } catch (IOException e) { LogManager.error(e.toString()); }
    }
    public static void renameFile(File file, String string) {
        File parent = new File(file.getParent());
        File newFile = new File(parent, string);
        file.renameTo(newFile);
    }
    public static void downloadFile(URL url, File file) {
        try {
            FileUtils.copyURLToFile(url, file, 60000, 60000);
        } catch (IOException e) { LogManager.error(e.toString()); }
    }
    public static File downloadTempFile(URL url) {
        UUID fileUUID = UUID.randomUUID();
        File file = new File(FileManager.getTempDir(), fileUUID.toString());
        FileManager.downloadFile(url, file);
        return file;
    }
    public static void getResource(String resourceString, File destinationFile) {
        URL resourceUrl = FileManager.class.getResource(resourceString);
        FileManager.downloadFile(resourceUrl, destinationFile);
    }
    public static File getTempResource(String resourceString) {
        URL resourceUrl = FileManager.class.getResource(resourceString);
        UUID fileUUID = UUID.randomUUID();
        File file = new File(FileManager.getTempDir(), fileUUID.toString());
        FileManager.downloadFile(resourceUrl, file);
        return file;
    }
}
