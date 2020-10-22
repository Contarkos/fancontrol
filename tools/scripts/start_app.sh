#!/bin/bash
# Script de démarrage de l'appli

###########################
#       Variables         #
###########################
LOCAL_BIN=sw_local.bin
DATA_DIR=DATA
LOCAL_DIR=/mnt/flash
NFS_DIR=/mnt/nfs
NFS_CONF=nfs_conf

MOD_DIR=/lib/modules/$(uname -r)/kernel/fancontrol
MOD_ADC_DIR=${MOD_DIR}/adc
MOD_TIME_DIR=${MOD_DIR}/time
MOD_ADC_NAME=kisr_adc
MOD_TIME_NAME=kisr_time
MOD_ADC=${MOD_ADC_NAME}.ko
MOD_TIME=${MOD_TIME_NAME}.ko

COPY_DIR=${LOCAL_DIR}

NFS_IP=192.168.0.14

###########################
#       Fonctions         #
###########################

start_flash ()
{
  /bin/echo "        ____________________.__          ";
  /bin/echo "        \______   \______   \__|         ";
  /bin/echo "  ______ |       _/|     ___/  |  ______ ";
  /bin/echo " /_____/ |    |   \|    |   |  | /_____/ ";
  /bin/echo "         |____|_  /|____|   |__|         ";
  /bin/echo "                \/                       ";
  /bin/echo "      _____.__                .__        ";
  /bin/echo "    _/ ____\  | _____    _____|  |__     ";
  /bin/echo "    \   __\|  | \__  \  /  ___/  |  \    ";
  /bin/echo "     |  |  |  |__/ __ \_\___ \|   Y  \   ";
  /bin/echo "     |__|  |____(____  /____  >___|  /   ";
  /bin/echo "                     \/     \/     \/    ";
  /bin/echo
  /bin/echo "========================================================"
  /bin/echo

  /bin/echo "Setting directory to ${LOCAL_DIR}"
  COPY_DIR=${LOCAL_DIR}
}

start_nfs ()
{
  /bin/echo "     ____  ____  ____                ";
  /bin/echo " ___(  _ \\(  _ \\(_  _)___            ";
  /bin/echo "(___))   / )___/ _)(_(___)           ";
  /bin/echo "    (_)\\_)(__)  (____)               ";
  /bin/echo "                     _  _  ____  ___ ";
  /bin/echo "                    ( \\( )( ___)/ __)";
  /bin/echo "                     )  (  )__) \\__ \\";
  /bin/echo "                    (_)\\_)(__)  (___/";
  /bin/echo
  /bin/echo "========================================================"
  /bin/echo

  # Ping de l'adresse du serveur tftp (5 secondes de timeout)
  /bin/ping ${NFS_IP} -c 1 -W 2
  ALIVE=$?
  /bin/echo

  if [[ ${ALIVE} -eq 0 ]];
  then
    /bin/echo "Mounting NFS from ${NFS_IP}"

    # Montage du dossier NFS
    /usr/bin/sudo /bin/umount ${NFS_DIR}
    /usr/bin/sudo /bin/mount -t nfs -o rw ${NFS_IP}:/tftpboot ${NFS_DIR}

    # Selection du dossier de copie
    COPY_DIR=${NFS_DIR}
  else
    /bin/echo "No echo from TFTP Server, using FLASH instead"
    COPY_DIR=${LOCAL_DIR}
  fi

}

launch_app ()
{
  # Suppression des modules
  /usr/bin/sudo /sbin/rmmod ${MOD_TIME} ${MOD_ADC}

  # Insertion des modules
  /usr/bin/sudo /sbin/insmod ${MOD_TIME_DIR}/${MOD_TIME}
  /usr/bin/sudo /sbin/insmod ${MOD_ADC_DIR}/${MOD_ADC}

  # Demarrage de l'applicatif avec options
  /bin/echo "Starting...."
  /bin/echo

  /usr/bin/sudo /tmp/${LOCAL_BIN}
}

copy_data ()
{
  # On verifie que le soft est bien la
  if ! [[ -f ${COPY_DIR}/${LOCAL_BIN} ]];
  then
    # Fallback si erreur
    /bin/echo "No file to launch... Fallback on flash"
    COPY_DIR=${LOCAL_DIR}
  fi

  # Copie des modules kernel pour interruption
  if [[ -f ${COPY_DIR}/${DATA_DIR}/${MOD_TIME} ]];
  then
    /usr/bin/sudo /bin/cp ${COPY_DIR}/${DATA_DIR}/${MOD_TIME} ${MOD_TIME_DIR}/${MOD_TIME}
  else
    /bin/echo "No kernel modules to load. Exiting..."
    exit 1
  fi

  if [[ -f ${COPY_DIR}/${DATA_DIR}/${MOD_ADC} ]];
  then
    /usr/bin/sudo /bin/cp ${COPY_DIR}/${DATA_DIR}/${MOD_ADC} ${MOD_ADC_DIR}/${MOD_ADC}
  else
    /bin/echo "No kernel modules to load. Exiting..."
    exit 1
  fi

  # Copie des binaires en RAM
  /bin/echo "Copying the binaries from ${COPY_DIR}..."

  # Copie en RAM et bit d'execution
  /bin/cp ${COPY_DIR}/${LOCAL_BIN} /tmp/${LOCAL_BIN}

  if [[ $? -eq 0 ]];
  then
    /bin/echo "Done !"
    /bin/chmod +x /tmp/${LOCAL_BIN}
  else
    /bin/echo "Error while copying files... Exiting"
    exit 1
  fi
}

###########################
#        Script           #
###########################

# Pour avoir le temps de démarrage
/bin/echo "Boot time : $(/bin/cat /proc/uptime | /usr/bin/cut -d ' ' -f1) seconds"

/bin/echo "========================================================"
/bin/echo
/bin/echo "  -- Starting APP --"
/bin/echo
/bin/echo "========================================================"

# Choix de la conf de demarrage
if [[ -f ${LOCAL_DIR}/${NFS_CONF} ]]; then
  start_nfs
else
  start_flash
fi

# Preparation des donnees
/bin/echo "========================================================"
/bin/echo
copy_data

# On lance l'appli préparée
/bin/echo "========================================================"
/bin/echo
launch_app

