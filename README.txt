
── ■ │ WHAT IS NFSMW SPEEDFIXER? (v1.04.01) │ ■ ───────────────────────────────────────────────────

VIEW THIS FILE on GitHub: https://github.com/rng-guy/NFSMWSpeedFixer

SpeedFixer lets you customise the Speedbreaker mechanic:
 • the car speed required to activate it,
 • whether it has a limited active duration,
 • its maximum active duration when fully charged,
 • whether you can recharge it passively over time,
 • the car speed required to trigger passive recharging,
 • how long fully passive recharging takes to complete,
 • how effective active recharging (by drifting) is,
 • the drifting speed required to trigger active recharging,
 • the slip angle     required to trigger active recharging,
 • the dilation multiplier for in-game time,
 • the car-mass multiplier for collisions,
 • the gravity  multiplier for downforce,
 • the time-dilation transition speed,
 • the boost to front-wheel steering friction,
 • the maximum  front-wheel steering angle,
 • the reduction of aerodynamic drag, and
 • the reduction of steering    drag.

The SECTIONS BELOW address these questions in detail:
 1) • How do I install SpeedFixer for my game?
 2) • Which mods are (in)compatible with SpeedFixer?
 3) • How may I share or bundle SpeedFixer?
 4) • What changed in each version of SpeedFixer?






── ■ │ 1 - HOW DO I INSTALL SPEEDFIXER FOR MY GAME? │ ■ ───────────────────────────────────────────

BEFORE INSTALLING SpeedFixer:
 1) • read and understand the section about mod (in)compatibilities below,
 2) • make sure your cracked copy of the game isn't a repack or came pre-modified in any way,
 3) • make sure your game's "speed.exe" is compatible (i.e. 5.75 MB / 6,029,312 bytes large), and
 4) • install an .asi loader or any mod with one (e.g. the "WideScreenFix" mod by ThirteenAG).

TO INSTALL SpeedFixer:
 1) • copy SpeedFixer's "scripts" folder to your game's folder, replacing existing files; and
 2) • if SpeedFixer's .asi file gets flagged by your antivirus software, whitelist the file.

AFTER INSTALLING SpeedFixer, edit its "NFSMWSpeedFixerSettings.ini" file to your liking.

TO UNINSTALL SpeedFixer, remove its files from your game's "scripts" folder.

TO UPDATE SpeedFixer, uninstall it and repeat the installation process above.
If you update from a version older than v1.04.00, replace the old configuration file.






── ■ │ 2 - WHICH MODS ARE (IN)COMPATIBLE WITH SPEEDFIXER? │ ■ ─────────────────────────────────────

All VLTED AND BINARY MODS should be fully compatible with all SpeedFixer configurations.

Almost all OTHER .ASI MODS should be fully compatible with all SpeedFixer configurations. If you
use the "NFSMW ExtraOptions" mod by ExOptsTeam, SpeedFixer overrides all its Speedbreaker features.






── ■ │ 3 - HOW MAY I SHARE OR BUNDLE SPEEDFIXER? │ ■ ──────────────────────────────────────────────

You are free to bundle SpeedFixer and its files with your own mod, NO CREDIT REQUIRED.
In the interest of code transparency, however, consider linking to SpeedFixer's GitHub repository 
(https://github.com/rng-guy/NFSMWSpeedFixer) somewhere in your mod's documentation (e.g. README).






── ■ │ 4 - WHAT CHANGED IN EACH VERSION OF SPEEDFIXER? │ ■ ────────────────────────────────────────

v1.00.00: Initial release

   01.00: Revised documentation and renamed some parameters for clarity
      01: Fixed yet more minor documentation oversights (sigh)
      02: Even more random documentation stuff that annoyed me

   02.00: Improved readability by renaming some more parameters
      01: Simplified internal configuration-file parser
      02: Added unlimited-duration feature
      03: Improved some mod internals

   03.00: Improved accuracy of time-dilation transition speed
      01: Fixed some new documentation oversights (rip)

   04.00: Added dilation-transition feature
      01: Updated installation instructions