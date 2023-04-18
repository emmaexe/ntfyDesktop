package moe.emmaexe.ntfyDesktop.utils;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;

public class Terminal {
    public static void runCommand(String args) {
        try {
            ProcessBuilder builder = new ProcessBuilder();
            String command[] = {"/bin/bash", "-c", args};
            builder.command(command);
            builder.directory(FileManager.getDir());
            Process process = builder.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line = "";
            while ((line = reader.readLine()) != null) {
                LogManager.log(line);
            }
        } catch (IOException e) { LogManager.error(e.toString()); }
    }
    public static void runCustomCommand(String[] command) {
        try {
            ProcessBuilder builder = new ProcessBuilder();
            builder.command(command);
            builder.directory(FileManager.getDir());
            Process process = builder.start();
            BufferedReader reader = new BufferedReader(new InputStreamReader(process.getInputStream()));
            String line = "";
            while ((line = reader.readLine()) != null) {
                LogManager.log(line);
            }
        } catch (IOException e) { LogManager.error(e.toString()); }
    }
}
