#Distributed Maze Game#

##Overall Functionality##
* The maze is N-by-N grid, consisting of N*N cells.Each cell can be the location of a “treasure”. At the start of the game, there are M treasures placed randomly at the grid.
* The number of players is arbitrary. Each player aims to collect as many treasures as possible. A player collects one or more treasures when the player’s location is the same as the treasure’s location. A player cannot occupy a cell that another player occupies. The game is over when there are no more treasures on the grid.
* The updated game state is sent to the other players only when they make their moves. In other words, the other players will often see a game state that is slightly stale.
* 20 seconds after a first player requests a new game from the server, the game should start. Note that by that time several players may have joined the game. After the start of the game, no additional players should be accepted by the server.

##Implementation Details##
* This is a maze game in C++ using Qt.
* Network communications are implemented with sockets.
* Using peer-to-peer architecture. There will be no dedicated server, and the system only has players where each player is also called a peer.
* Among all the n players, one player should act as the Primary Server (in addition to being a player itself), and another player should act as the Backup Server. 
* If the Primary Server crashes, your system should be able to “regenerate” the Primary Server on another (uncrashed) player. You need to properly make sure that the game state on the new Primary Server is brought up to date.
* The same applies if the Backup Server crashes.
* The primary server is binded to port 30000 at the beginning.

##Screenshot##
* images/Screenshot.png