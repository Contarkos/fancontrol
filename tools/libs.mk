# TODO : generate file on the run

FANCONTROL_PWD = /home/martin/dev/template_project/

# Environment directory
CMD_DIR = $(FANCONTROL_PWD)/env/CMD
export CMD_DIR

COM_DIR = $(FANCONTROL_PWD)/env/COM
export COM_DIR

MAIN_DIR = $(FANCONTROL_PWD)/env/MAIN
export MAIN_DIR

MODULES_DIR = $(FANCONTROL_PWD)/env/MODULES
export MODULES_DIR

OS_DIR = $(FANCONTROL_PWD)/env/OS
export OS_DIR

# Modules directory
FAN = $(FANCONTROL_PWD)/modules/fan
export FAN

TEMP = $(FANCONTROL_PWD)/modules/temp
export TEMP

REMOTE = $(FANCONTROL_PWD)/modules/remote
export REMOTE
