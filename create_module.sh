#!/bin/bash
#
# Creation d'un dossier de module
#
#

################################
#          Variables           #
################################


################################
#          Functions           #
################################

print_help () {
  echo "Usage :"
  echo "./create_module.sh [module_name] [path/to/module]"

  exit 1
}

make_mode () {
  echo "==================================================="
  echo "Creation du module ${MOD_NAME} dans ${MOD_DIR}"
  echo "==================================================="
  echo

  cd ${MOD_DIR}

  if [ -d "${MOD_NAME}" ]; then
    echo "Module deja existant... Exit"
    exit 1
  else
    mkdir ${MOD_NAME}
    cd ${MOD_NAME}

    #Creation des dossiers du module
    echo "Creation des dossiers...."
    mkdir api inc lib obj src tst xml

    # Fichier de compilation spécifique au module
    echo "Creation fichier de compilation"
    touch include.mk
    ln -s ../../tools/module.mk

    echo "Done !"
  fi
}

################################
#         Main Script          #
################################

while getopts ":h" opt; do
  case $opt in
    h)
      print_help
      ;;
    *)
      print_help
      ;;
  esac
done

if [ -z $1 ] || [ -z $2 ]; then
  echo "Error : Missing parameter"
  print_help
else
  MOD_NAME=$1
  MOD_DIR=$2

  make_mode
fi
