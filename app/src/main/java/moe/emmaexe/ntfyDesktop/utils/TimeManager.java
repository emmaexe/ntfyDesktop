package moe.emmaexe.ntfyDesktop.utils;

import java.util.Timer;
import java.util.TimerTask;

public class TimeManager {
    public static void asyncSetTimeout(Runnable runnable, int delay) {
        new Thread(() -> {
            try {
                Thread.sleep(delay);
                runnable.run();
            } catch (Exception e){ LogManager.error(e.toString()); }
        }).start();
    }
    public static void setTimeout(Runnable runnable, int delay) {
        try {
            Thread.sleep(delay);
            runnable.run();
        } catch (Exception e){ LogManager.error(e.toString()); }
    }
    public static void setInterval(Runnable runnable, int delay) {
        new Timer().scheduleAtFixedRate(new TimerTask() {
            public void run(){
                runnable.run();
            }
        },0,delay);
    }
}
