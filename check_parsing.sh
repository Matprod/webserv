#!/bin/bash

# Programme à exécuter
PROGRAM="./webserv"

# Vérifie que le programme est exécutable
if [ ! -x "$PROGRAM" ]; then
    echo "Erreur : '$PROGRAM' n'est pas exécutable."
    exit 1
fi

# Tableau des fichiers de configuration à tester
CONFIG_FILES_GOOD=(
    "config/good/allowMethod.conf"
    "config/good/customError.conf"
    "config/good/listDirectory.conf"
    "config/good/maxBodySize.conf"
    "config/good/multipleServer.conf"
    "config/good/multipleServerName.conf"
    "config/good/redirect.conf"
    "config/good/webserv.conf"
    "config/good/UnclosedServerBlock.conf"
)

CONFIG_FILES_BAD=(
    "config/bad/DuplicateLocationPath.conf"
    "config/bad/DuplicateRoot.conf"
    "config/bad/DuplicateRootAndAliasInLocation.conf"
    "config/bad/DuplicateServerName.conf"
    "config/bad/InvalidDirective.conf"
    "config/bad/InvalidIp.conf"
    "config/bad/InvalidPort.conf"
    "config/bad/EmptyFile.conf"
    "config/bad/EmptyRoot.conf"
    "config/bad/InvalidAutoindex.conf"
    "config/bad/InvalidListenFormat.conf"
    "config/bad/InvalidMaxBodySize.conf"
    "config/bad/InvalidRedirectStatus.conf"
    "config/bad/MissingListenValue.conf"
    "config/bad/MultipleRootValues.conf"
    "config/bad/NegativeIP.conf"
    "config/bad/OnlyComments.conf"
    "config/bad/OverflowPort.conf"
    "config/bad/OverlappingLocations.conf"
    "config/bad/UnknownServerDirective.conf"
)

all_passed_good=1  # 1 = tout va bien, 0 = au moins un échec
all_passed_bad=1
failed_good=()     # Tableau pour stocker les fichiers GOOD échoués
failed_bad=()      # Tableau pour stocker les fichiers BAD échoués

for config in "${CONFIG_FILES_GOOD[@]}"; do
    echo "=== Test avec '$config' ==="

    if [ ! -f "$config" ]; then
        echo "❌ Fichier non trouvé : $config"
        all_passed_good=0
        failed_good+=("$config")
        continue
    fi

    "$PROGRAM" "$config"
    RET=$?

    if [ "$RET" -eq 0 ]; then
        echo "✅  Succès (exit code: 0)"
    else
        echo "❌ Échec (exit code: $RET)"
        all_passed_good=0
        failed_good+=("$config")
    fi

    echo
done

if [ "$all_passed_good" -eq 1 ]; then
    echo "🎉 Tous les tests de GOOD se sont bien passés."
else
    echo "⚠️  Au moins un test a échoué. Fichiers échoués : ${failed_good[*]}"
fi

for config in "${CONFIG_FILES_BAD[@]}"; do
    echo "=== Test avec '$config' ==="

    if [ ! -f "$config" ]; then
        echo "❌ Fichier non trouvé : $config"
        all_passed_bad=0
        failed_bad+=("$config")
        continue
    fi

    "$PROGRAM" "$config"
    RET=$?

    if [ "$RET" -eq 1 ]; then
        echo "✅  Succès (exit code: 1)"
    else
        echo "❌ Échec (exit code: $RET)"
        all_passed_bad=0
        failed_bad+=("$config")
    fi

    echo
done

if [ "$all_passed_bad" -eq 1 ]; then
    echo "🎉 Tous les tests de BAD se sont bien passés."
else
    echo "🚩  Au moins un test a échoué. Fichiers échoués : ${failed_bad[*]}"
fi

if [[ "$all_passed_bad" -eq 1 && "$all_passed_good" -eq 1 ]]; then
    echo "🎉 Tous les tests de BAD ET GOOD se sont bien passés."
else
    echo "🚩 Au moins un test a échoué. Fichiers échoués (GOOD) : ${failed_good[*]}, (BAD) : ${failed_bad[*]}"
fi