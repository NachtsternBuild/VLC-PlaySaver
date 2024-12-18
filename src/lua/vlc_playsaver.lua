-- ~/.local/share/vlc/lua/extensions/vlc_playsaver.lua

function descriptor()
    return {
        title = "Start VLC-PlaySaver",
        version = "1.0",
        author = "Elias Mörz",
        description = "Startet und beendet den VLC-PlaySaver automatisch."
    }
end

function activate()
    os.execute("pkill vlc_playsaver") 
    os.execute("/opt/vlc_playsaver/vlc_playsaver &")
    vlc.msg.info("VLC-PlaySaver gestartet.")
end

function deactivate()
    os.execute("pkill vlc_playsaver")
    vlc.msg.info("VLC-PlaySaver beendet.")
end
