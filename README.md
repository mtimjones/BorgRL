# BorgRL - 2023 7 Day Roguelike (7drl)

BorgRL is a Borg Roguelike.  The player is the Borg and travels across space, fighting enemies scavenging resources, mining planets, assimilating drones, healing and recycling drones, and converting resources into upgrades for its evolving fleet of drones.  The player explores the sector and uses the star gate to jump to the next.  The player must optimize resources to heal drones assimilate or kill enemy drones, and ultimately kill the final boss to win.

## Features
- Permadeath
- Procedurally generated sectors (random walk with cellular automata smoothing)
- Turn-based and real-time gameplay (user can pause)
- Random enemies, wrecks, planets, academies, enemies, gas-clouds, etc.
- Tactical combat, strategic resource management

## Game Start
The player begins with the Borg (no attack ability), a scavenger drone, a miner drone, and a small amount of resources.  A random upgrade is also provided.

## End Game
The player must fight and kill the Boss.  The Boss emits combat drones to attack the player and its drones.  Surviving these waves and killing the Boss represents a win

## Controls

### Movement:

- Right-click in the local map to move the Borg, or use the arrow keys to move the Borg.

- Left-click a drone (inventory or map) and right-click to move (or right-click an object on the map).

### Basic Commands

- Tactical Pause/Unpause ('p').
- Single step. (' ').
- help ('?').
- Map legend ('l').
- Surrender ('X').

### Actions:

#### Left-click a drone in the inventory window:

- heal the drone (using resources). 'h'
- recycle the drone (for resources). 'r'
    
#### Left-click a friendly drone in the inventory window:

- dock the drone into the Borg. 'd'

- press 'D' to dock all undocked combat drones. 'D'

#### Left-click on an object in the map:

- contextual information. 'i'

#### Left-click an enemy drone in the map window:

- assimilate (probabilistic). 'a'

#### Left-click a combat drone in the inventory window:

- press 's' to morph into a scavenger drone. 's'
- press 'm' to morph into a miner drone. 'm'
- press 'j' to morph into a javelin drone. 'j'

### Operations:

- Left-click a scavenger drone in the inventory window (or map) and right-click a wreck to scavenge.

- Left-click a miner drone in the inventory window (or map) and right-click a planet to mine.

- Left-click a friendly drone in the inventory window (or map) and right-click the academy to upgrade.

- Left-click a combat drone in the inventory window (or map) and right-click an enemy to attack.

