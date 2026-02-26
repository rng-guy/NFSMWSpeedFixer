
![POV: You should've stopped a looooong time ago.](Thumbnail.jpg "Graphic design is still my passion.")

SpeedFixer lets you **customise the speedbreaker mechanic** in *Need for Speed: Most Wanted* (2005):
* the car speed required to activate it,
* up to how long it can last per activation,
* whether it can be recharged passively,
* the car speed required for passive recharging,
* how long passive recharging takes to complete,
* how effective active recharging (by drifting) is,
* the drifting speed required for active recharging,
* the slip angle required for active recharging,
* the slowdown factor for in-game time,
* the car-mass scale for collisions,
* the gravity scale for downforce,
* the boost to steering friction,
* the maximum steering angle,
* the amount of aerodynamic drag, and
* the amount of steering drag.

&nbsp;

The **sections below** address these questions in detail:
1. [What mods are (in)compatible with SpeedFixer?](#1---what-mods-are-incompatible-with-speedfixer)
2. [How do I install SpeedFixer for my game?](#2---how-do-i-install-speedfixer-for-my-game)
3. [How may I share or bundle SpeedFixer?](#3---how-may-i-share-or-bundle-speedfixer)

&nbsp;

For a detailed **version history** of SpeedFixer, see the plain-text version of this document (`README.txt`).

&nbsp;

&nbsp;

&nbsp;



# 1 - What mods are (in)compatible with SpeedFixer?

All **[VltEd](https://nfs-tools.blogspot.com/2019/02/nfs-vlted-v46-released.html) and [Binary](https://github.com/SpeedReflect/Binary/releases) mods** should be fully compatible with all SpeedFixer configurations. 

Almost all  **other `.asi` mods** should be fully compatible with all SpeedFixer configurations. SpeedFixer overrides all speedbreaker features of the [NFSMW ExtraOptions](https://github.com/ExOptsTeam/NFSMWExOpts/releases) mod by ExOptsTeam.

&nbsp;

&nbsp;

&nbsp;



# 2 - How do I install SpeedFixer for my game?

**Before installing** SpeedFixer:
1. make sure your original copy of the game wasn't a repack or came pre-modified in any way,
2. read and understand the two sections about [mod (in)compatibilities](#1---what-mods-are-incompatible-with-speedfixer) above,
3. make sure your game's `speed.exe` is compatible (i.e. 5.75 MB / 6,029,312 bytes large), and
4. install an `.asi` loader or any mod with one (e.g. the [WideScreenFix](https://github.com/ThirteenAG/WidescreenFixesPack/releases/tag/nfsmw) mod by ThirteenAG).

&nbsp;

**To install** SpeedFixer:
1. download and unzip the `NfSMW_SpeedFixer_v1.02.0.7z` file from its [release page](https://github.com/rng-guy/NFSMWSpeedFixer/releases/tag/v1.02.00);
2. if it doesn't exist already, create a `scripts` folder in your game's installation folder;
3. copy the contents of SpeedFixer's `scripts` folder to your game's `scripts` folder; and
4. if SpeedFixer's `.asi` file gets flagged by your antivirus software, whitelist the file.

&nbsp;

**After installing** SpeedFixer, edit its `NFSMWSpeedFixerSettings.ini` file to your liking.

**To uninstall** SpeedFixer, remove its files from your game's `scripts` folder.

**To update** SpeedFixer, uninstall it and repeat the installation process above. If you update from a version older than v1.02.00, replace the old configuration file.

&nbsp;

&nbsp;

&nbsp;



# 3 - How may I share or bundle SpeedFixer?

You are free to bundle SpeedFixer and its files with your own mod, **no credit required**. In the interest of code transparency, however, consider linking to [SpeedFixer's GitHub repository](https://github.com/rng-guy/NFSMWSpeedFixer) somewhere in your mod's documentation (e.g. README).