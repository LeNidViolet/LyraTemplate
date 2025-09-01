
Unreal Engine Version

5.6.1 (Installed Build)

Lyra Version

Compatible with UE 5.6

⸻⸻⸻⸻⸻⸻⸻⸻⸻⸻

Modifications Made
1. Removed all original Content assets.
2. Cleared any Project Settings references to the original Content.
3. Added a minimal Experience implementation under the Default folder.
4. Enabled Enhanced Input Support in Project Settings → Game → Common Input Settings.
5. Removed the following plugins:
   * LyraExtTool
   * LyraExampleContent
6. Added the plugin:
   * EnhancedInputIcons
   * GameFeaturePlugin->SimpleExperience


⸻⸻⸻⸻⸻⸻⸻⸻⸻⸻

When the game starts, it will load B_Experience_Default (via LyraGameMode).
After that, it scans the available Game Feature Plugins (GFP) and creates a loading button for each GFP.
SimpleExperience demonstrates how to implement a minimal GFP experience.
It relies on the Lyra framework to provide basic character movement, crouching, and jumping, as well as some simple applications of CommonUI.