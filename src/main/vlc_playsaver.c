/*
 *-------------------------------------------*
 *               VLC PlaySaver               *
 *-------------------------------------------*
 *    (C) Copyright 2024 Elias Mörz          *
 *-------------------------------------------*
 *             vlc_playsaver                 *
 *-------------------------------------------*
 */
/* headers */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define LOG_FILE "video_stop_log.txt"

// function to run dbus-command
void execute_dbus_command(const char *command, char *result, size_t size) 
{
    FILE *fp = popen(command, "r");
    if (fp == NULL) 
    {
        perror("Error when executing the D-Bus command.\n");
        return;
    }

    if (fgets(result, size, fp) != NULL) 
    {
        // Remove the closing newline character
        size_t len = strlen(result);
        if (len > 0 && result[len - 1] == '\n') 
        {
            result[len - 1] = '\0';
        }
    }
    pclose(fp);
}

// Function to convert the position in seconds 
void format_time(long seconds, char *buffer, size_t size) 
{
    int hours = seconds / 3600;
    int minutes = (seconds % 3600) / 60;
    int secs = seconds % 60;
    snprintf(buffer, size, "%02d:%02d:%02d", hours, minutes, secs);
}

/* main function */
int main() 
{
    char state[16];
    char position_raw[32];
    char video_name_raw[256];
    char stop_time[16];

    printf("Run vlc_playsaver.\n");

    while (1) 
    {
        // Check the playback status of VLC
        execute_dbus_command(
            "dbus-send --print-reply --dest=org.mpris.MediaPlayer2.vlc "
            "/org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get "
            "string:org.mpris.MediaPlayer2.Player string:PlaybackStatus | grep -o 'Playing\\|Paused'",
            state, sizeof(state));

        if (strcmp(state, "Paused") == 0) 
        {
            // Get the current position in nanoseconds
            execute_dbus_command(
                "dbus-send --print-reply --dest=org.mpris.MediaPlayer2.vlc "
                "/org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get "
                "string:org.mpris.MediaPlayer2.Player string:Position | grep -o '[0-9]*'",
                position_raw, sizeof(position_raw));
            long position_ns = atol(position_raw);  // convert them to seconds
            long position_sec = position_ns / 1000000;

            // get name of the video
            execute_dbus_command(
                "dbus-send --print-reply --dest=org.mpris.MediaPlayer2.vlc "
                "/org/mpris/MediaPlayer2 org.freedesktop.DBus.Properties.Get "
                "string:org.mpris.MediaPlayer2.Player string:Metadata | grep -m1 'xesam:title' | grep -o '\".*\"' | tr -d '\"'",
                video_name_raw, sizeof(video_name_raw));

            // Format the stop time
            format_time(position_sec, stop_time, sizeof(stop_time));

            // Save the data in the file
            FILE *file = fopen(LOG_FILE, "a");
            if (file) 
            {
                fprintf(file, "Video: %s\nStop-Zeit: %s\n\n", video_name_raw, stop_time);
                fclose(file);
                printf("Data saved: Video: %s, Stop-Zeit: %s\n", video_name_raw, stop_time);
            } 
            
            else 
            {
                perror("Error when writing to the log file.\n");
            }
        }

        // Wait 2 seconds before checking again
        sleep(2);
    }

    return 0;
}
