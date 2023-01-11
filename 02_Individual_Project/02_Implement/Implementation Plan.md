# **Implementation Plan**

The game I am trying to develop is `Bomberman`. 

The game consists of players, enemies, bombs and bricks, which I want to load from files. But in the beginning, I need to use some simple polygon meshes for the main flow of the game. For example, the cubes represent bricks, the balls represent players and enemies, and the balls of different colors represent bombs. This will help improve the compilation efficiency of the program and improve my development efficiency. I found the model of the player and the bomb as follows, and I will apply these model to the project after the main process development is complete.

<img src="Implementation Plan.assets/image-20221107172441955.png" alt="image-20221107172441955" style="zoom: 33%;" />

Regarding the operation and interaction part, the original intention of this game is to let the user control the player, release bombs, and defeat the enemy, so as to win the game. So I plan to use only the keyboard to increase the control of the player's actions such as up and down, left and right, and release bombs. In addition, for the control of the entire game, setting options based on the player's perspective and God's perspective(top down view) will be added, and will also be controlled by keyboard shortcuts. The specific function key correspondence table is as follows:

| Key   | Function                   |
| ----- | -------------------------- |
| W     | Walk forward               |
| S     | Walk backward              |
| A     | Walk to the left           |
| D     | Walk to the right          |
| V     | Change the views of camera |
| Space | Drop bomb                  |
| Esc   | Exit game                  |

Then, the game needs to randomize the position of the block, which can be achieved by using random numbers. In this way, the game map will be different every time you enter the game. The main core part of the game is collision detection. The game is full of parts that need to verify collision detection. For example, players and enemies will not be able to move forward if they encounter blocks and bombs during the march. When the player meets the enemy, the player is hit by a bomb, and the enemy is hit by a bomb, collision detection verification is required.

Regarding the perspective of the game, I plan to use the top down view to realize the rough prototype first, and then consider how to present the first person view, because for this game, if the game perspective is changed to the first player perspective, it will greatly increase. Big game difficulty, so you will need to consider a better way of presentation.

Based on the above ideas, we are already trying to realize the project. The current progress is as follows:

<img src="Implementation Plan.assets/image-20221107181100957.png" alt="image-20221107181100957" style="zoom: 33%;" />