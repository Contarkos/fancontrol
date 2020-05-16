#!/bin/sh
# Script de démarrage de l'appli

###########################
#       Variables         #
###########################
LOCAL_BIN=sw_local.bin
LOCAL_DIR=/boot/fancontrol
NFS_DIR=/mnt/nfs
NFS_CONF=nfs_conf

MOD_DIR=/lib/modules/$(uname -r)/kernel/fancontrol
MOD_ADC_DIR=${LOCAL_DIR}
MOD_TIME_DIR=${LOCAL_DIR}
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
  echo "        ____________________.__          ";
  echo "        \______   \______   \__|         ";
  echo "  ______ |       _/|     ___/  |  ______ ";
  echo " /_____/ |    |   \|    |   |  | /_____/ ";
  echo "         |____|_  /|____|   |__|         ";
  echo "                \/                       ";
  echo "      _____.__                .__        ";
  echo "    _/ ____\  | _____    _____|  |__     ";
  echo "    \   __\|  | \__  \  /  ___/  |  \    ";
  echo "     |  |  |  |__/ __ \_\___ \|   Y  \   ";
  echo "     |__|  |____(____  /____  >___|  /   ";
  echo "                     \/     \/     \/    ";
  echo
  echo "========================================================"
  echo

  echo "Setting directory to ${LOCAL_DIR}"
  COPY_DIR=${LOCAL_DIR}
}

start_nfs ()
{
  echo "     ____  ____  ____                ";
  echo " ___(  _ \\(  _ \\(_  _)___            ";
  echo "(___))   / )___/ _)(_(___)           ";
  echo "    (_)\\_)(__)  (____)               ";
  echo "                     _  _  ____  ___ ";
  echo "                    ( \\( )( ___)/ __)";
  echo "                     )  (  )__) \\__ \\";
  echo "                    (_)\\_)(__)  (___/";
  echo
  echo "========================================================"
  echo

  # Ping de l'adresse du serveur tftp (5 secondes de timeout)
  ping ${NFS_IP} -c 1 -W 2
  ALIVE=$?
  echo

  if [[ ${ALIVE} -eq 0 ]];
  then
    echo "Mounting NFS from ${NFS_IP}"

    # Montage du dossier NFS
    umount ${NFS_DIR}
    mount -t nfs -o rw ${NFS_IP}:/tftpboot ${NFS_DIR}

    # Selection du dossier de copie
    COPY_DIR=${NFS_DIR}
  else
    echo "No echo from TFTP Server, using FLASH instead"
    COPY_DIR=${LOCAL_DIR}
  fi

}

launch_app ()
{
  # Suppression des modules
  rmmod ${MOD_TIME} ${MOD_ADC}

  # Insertion des modules
  insmod ${MOD_TIME_DIR}/${MOD_TIME}
  insmod ${MOD_ADC_DIR}/${MOD_ADC}

  # Demarrage de l'applicatif avec options
  echo "Starting...."
  echo

  /tmp/${LOCAL_BIN}
}

copy_data ()
{
  # On verifie que le soft est bien la
  if ! [[ -f ${COPY_DIR}/${LOCAL_BIN} ]];
  then
    # Fallback si erreur
    echo "No file to launch... Fallback on flash"
    COPY_DIR=${LOCAL_DIR}
  fi

  # Copie des binaires en RAM
  echo "Copying the binaries from ${COPY_DIR}..."

  # Copie en RAM et bit d'execution
  cp ${COPY_DIR}/${LOCAL_BIN} /tmp/${LOCAL_BIN}

  if [[ $? -eq 0 ]];
  then
    echo "Done !"
    chmod +x /tmp/${LOCAL_BIN}
  else
    echo "Error while copying files... Exiting"
    exit 1
  fi
}

###########################
#        Script           #
###########################

# Pour avoir le temps de démarrage
echo "Boot time : $(cat /proc/uptime | cut -d ' ' -f1) seconds"

echo "========================================================"
echo
echo "  -- Starting APP --"
echo
echo "========================================================"

# Choix de la conf de demarrage
if [[ -f ${LOCAL_DIR}/${NFS_CONF} ]]; then
  start_nfs
else
  start_flash
fi

# Preparation des donnees
echo "========================================================"
echo
copy_data

# On lance l'appli préparée
echo "========================================================"
echo
launch_app

