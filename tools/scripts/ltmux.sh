#!/bin/bash

SESSION="BOOT1"

echo "Launching tmux"

#allow re-launch
/usr/bin/tmux has-session -t $SESSION 2> /dev/null && /usr/bin/tmux kill-session -t $SESSION
/usr/bin/tmux -2 new-session -d -s $SESSION "/home/pi/bin/start_app.sh; exec bash"

/usr/bin/tmux rename-window "console"

/usr/bin/tmux set-option -t $SESSION set-remain-on-exit on

/usr/bin/tmux split-window -h "htop; exec bash"
/usr/bin/tmux split-window -v "echo Welcome aboard, captain; exec bash"
/usr/bin/tmux select-pane -t 0

# Nouvelle session pour VIM
/usr/bin/tmux new-window -n "vim"
/usr/bin/tmux split-window -h

/usr/bin/tmux select-window -t ${SESSION}:0
