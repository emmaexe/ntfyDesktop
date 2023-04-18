package moe.emmaexe.ntfyDesktop.utils;
 
import java.util.regex.Matcher;
import java.util.regex.Pattern;

public class StringUtils {
    public static boolean stringHasLetterNumberSymbol(String string) {
        Pattern letter = Pattern.compile("[a-zA-z]");
        Pattern digit = Pattern.compile("[0-9]");
        Pattern special = Pattern.compile ("[!@#$%&*()_+=|<>?{}\\[\\]~-]");
        Matcher hasLetter = letter.matcher(string);
        Matcher hasDigit = digit.matcher(string);
        Matcher hasSpecial = special.matcher(string);
        return( hasLetter.find() || hasDigit.find() || hasSpecial.find() );
    }
    public static String surroundInQuotes(String string) {
        return new String("\"" + string + "\"");
    }
}