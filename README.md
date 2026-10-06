# Sofle — configuration ZMK AZERTY

Clavier Sofle en deux moitiés, contrôleurs nice!nano v2, ZMK v0.3, écrans
nice_oled, deux encodeurs et RGB. ZMK Studio est activé en USB sur la moitié
gauche. Les modules externes sont fixés à des commits dans `config/west.yml`.

## Mapping

Le mapping initial est dans [`config/sofle.keymap`](config/sofle.keymap).
Les grilles de commentaires correspondent aux bindings : `TRNS` reprend
l'action d'une couche active inférieure ; `---` désactive la touche.

| Couche | Accès | Fonction |
| --- | --- | --- |
| main / BASE | Par défaut | AZERTY, modificateurs, espace et entrée |
| special / LOWER | Maintenir la touche de pouce à gauche de l'espace | Chiffres, symboles, F1 à F10 et F12 |
| raise / RAISE | Maintenir la touche de pouce à droite de l'entrée | Navigation, Bluetooth, raccourcis et déverrouillage Studio |
| adjust / ADJUST | Maintenir LOWER puis la touche tout en haut à droite | RGB, alimentation et sélection de sortie USB/Bluetooth |
| DEV | Maintenir LOWER et RAISE | Symboles de programmation |

Les encodeurs règlent le volume à gauche et envoient Page précédente/suivante
à droite sur main, special, raise et DEV. ADJUST ne définit pas de bindings
d'encodeur. Les séquences `=>`, `->`, `/*`, etc. ne sont pas des macros
configurées : les touches correspondantes de DEV sont transparentes.

Les codes `FR_*` ciblent la disposition **Français AZERTY classique** sur
l'ordinateur. Une disposition macOS, belge ou AZERTY différente peut produire
d'autres caractères. Les accents morts tels que `^`, `~` et l'accent grave
peuvent nécessiter une pression sur Espace pour être affichés seuls.

## Installer le firmware

GitHub Actions compile les deux moitiés à chaque push/PR et sur lancement
manuel. Télécharger l'archive `firmware` depuis le workflow terminé.
Les fichiers attendus sont `sofle_left_studio.uf2` et `sofle_right.uf2`.

1. Brancher la moitié gauche avec un câble USB de données.
2. Appuyer deux fois rapidement sur Reset pour faire apparaître le disque
   du bootloader, généralement `NICENANO`.
3. Copier `sofle_left_studio.uf2` sur ce disque et attendre le redémarrage.
4. Répéter avec la moitié droite et `sofle_right.uf2`.

Un dossier local `firmware/` peut aussi contenir les fichiers compilés ; il
est exclu de Git. Ne pas flasher un firmware de remise à zéro des paramètres
Bluetooth pour cette installation.

## Modifier et tester avec ZMK Studio

1. Brancher la moitié **gauche** en USB et ouvrir
   [ZMK Studio](https://zmk.studio/) dans Chrome ou Edge.
2. Se connecter au port série du clavier proposé par l'interface.
3. Maintenir **RAISE**, puis appuyer sur la touche **tout en haut à droite**
   pour déverrouiller Studio. Relâcher les touches.
4. Sélectionner une couche, une touche et sa nouvelle action ; enregistrer
   les changements avec la commande de sauvegarde de Studio.
5. Tester sur le clavier dans un éditeur de texte. Par exemple, maintenir
   LOWER + RAISE et saisir `/ ? ~ |` pour vérifier la couche DEV.

Studio peut afficher les codes avec des libellés US : un `Q` affiché peut
produire `A` sur l'ordinateur configuré en français. Vérifier les caractères
réellement saisis ; la sélection de disposition de l'ordinateur dans Studio
figure encore parmi les fonctionnalités prévues dans la documentation.

Si le clavier est aussi connecté en Bluetooth, sélectionner la sortie USB :
maintenir LOWER, puis la touche tout en haut à droite pour atteindre ADJUST ;
appuyer sur la première touche de la deuxième rangée de la moitié droite
(position Y sur main). La touche suivante (position U) sélectionne Bluetooth.

Studio modifie le mapping enregistré **sur le clavier**, sans recompilation
à chaque changement. Il ne synchronise pas `config/sofle.keymap` avec GitHub.
Après des changements dans le dépôt et un nouveau flash, utiliser
**Restore Stock Settings** dans Studio pour reprendre le mapping compilé ;
cette action remplace les personnalisations faites dans Studio.

Les règles de couches conditionnelles, les définitions de macros et les
bindings d'encodeurs restent configurés dans les fichiers du dépôt.
Voir les [capacités et instructions officielles](https://zmk.dev/docs/features/studio).

Pour éditer les fichiers graphiquement, utiliser aussi
[Keymap Editor](https://nickcoutsos.github.io/keymap-editor/) : source Clipboard
avec le contenu du `.keymap`, ou intégration GitHub. Dans ce cas, les changements
passent par une compilation et un flash.

## Compiler localement

Avec west, les dépendances Python de Zephyr et un SDK Zephyr compatibles,
créer un workspace séparé et y copier le dossier `config`. Depuis ce workspace :

```sh
west init -l config
west update --fetch-opt=--filter=tree:0
west zephyr-export
west build -s zmk/app -d build/left -b nice_nano_v2 -S studio-rpc-usb-uart -- \
  -DZMK_CONFIG="$PWD/config" -DSHIELD="sofle_left nice_oled" -DCONFIG_ZMK_STUDIO=y
west build -s zmk/app -d build/right -b nice_nano_v2 -- \
  -DZMK_CONFIG="$PWD/config" -DSHIELD="sofle_right nice_oled"
```

Les fichiers sont `build/left/zephyr/zmk.uf2` et
`build/right/zephyr/zmk.uf2`. Les noms de cartes ci-dessus correspondent à
ZMK v0.3 ; conserver cette version lors de la compilation.

## Points corrigés lors de l'audit

- Suppression des sept aliases DEV : utilisation directe des codes français
  officiels, correction de `/`, `?`, `~` et `|`.
- Correction du `0` dans RAISE, qui envoyait auparavant la touche physique `à`.
- Mise à jour des grilles : vrais chiffres dans LOWER, touche Menu et `0`
  dans RAISE, transparences de DEV.
- Réglages OLED propres à chaque moitié séparés ; Raw HID reste à gauche.
- USB désactivé explicitement sur le périphérique pour supprimer un
  avertissement de dépendance Kconfig.
- Sommeil profond, mise en inactivité automatique et extinction de l'écran/RGB
  pour inactivité désactivés sur les deux moitiés pour tester les blocages
  au réveil de l'ordinateur.
- Liaison Bluetooth entre moitiés configurée sans événements de connexion
  sautés ; rendu OLED de priorité inférieure à la transmission des touches.
- Versions des modules externes fixées ; instructions du dépôt actualisées.

Pour désactiver l'inactivité automatique sur les deux moitiés sous ZMK v0.3,
`CONFIG_ZMK_IDLE_TIMEOUT=2147483647` utilise la valeur maximale du compteur
signé 32 bits : la condition de passage en inactivité ne peut pas être vraie.
Une valeur de zéro ferait au contraire entrer immédiatement en inactivité.
Les écrans restent actifs et le RGB ne s'éteint plus pour inactivité, ce qui augmente
la consommation sur batterie. Le RGB garde son extinction automatique en USB.

### Réactivité et liaison entre les moitiés

La droite transmet ses touches à la gauche en Bluetooth, même lorsque la
gauche est branchée en USB à l'ordinateur. La gauche demande maintenant
`CONFIG_ZMK_SPLIT_BLE_PREF_LATENCY=0` au lieu du défaut de 30 : la droite
participe à chaque événement de connexion. L'intervalle reste à 7,5 ms
(`CONFIG_ZMK_SPLIT_BLE_PREF_INT=6`) et le délai de détection d'une liaison
perdue reste à 4 secondes. Ces valeurs décrivent la connexion, pas une mesure
de latence des touches ; le défaut de 30 ne signifie pas que chaque touche
attendait 30 intervalles.

Les deux écrans utilisent une file de travail dédiée de priorité 10 au lieu
de 5, inférieure à celle de la transmission des touches Bluetooth. Ce réglage
est conseillé pour privilégier la frappe dans le
[guide du module OLED](https://github.com/mctechnology17/zmk-nice-oled/blob/46f824abb2bd41f1287c5c68abd14122af6042a3/docs/OPTIMIZE.md).
Les écrans peuvent être moins fluides pendant une frappe rapide.

Flasher **les deux moitiés** avec les nouveaux fichiers, puis comparer une
frappe alternée gauche/droite et plusieurs cycles de veille/réveil de
l'ordinateur. Pour isoler la liaison avec l'ordinateur, refaire le test en
USB sur la gauche avec la sortie USB sélectionnée. Le gain de réactivité et
la disparition des blocages restent à vérifier sur le matériel.

À vérifier sur le matériel : connexion/déverrouillage Studio, caractères
AZERTY, retour Bluetooth après sélection USB, fonctionnement des deux moitiés,
encodeurs, OLED et RGB. Les widgets Raw HID nécessitent toujours
`zmk-hid-host` sur l'ordinateur.

## Validation de cette configuration

Les deux firmwares ont été compilés localement avec l'image
`zmkfirmware/zmk-build-arm:stable`, le SDK Zephyr 0.16.9 et les révisions
déclarées. Studio, CDC ACM et les deux interfaces HID sont présents à gauche ;
Studio et le Raw HID sont absents à droite. Les cinq couches ont chacune
60 bindings et conservent leur ordre et leurs règles d'activation.

La moitié gauche utilise environ 42 % de la flash et 52 % de la RAM ; la
droite environ 33 % et 36 %. Un avertissement sur le symbole déprécié
`NRF_STORE_REBOOT_TYPE_GPREGRET` subsiste dans la définition nice!nano de
ZMK v0.3. La compilation ne valide pas le comportement sur le matériel.
