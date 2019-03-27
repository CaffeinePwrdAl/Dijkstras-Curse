# Dijkstras-Curse
A puzzle game where you are a flood-filling blob

## Object of the game

You are a blob. To survive each level you have to find a route to one of the exit portals without getting trapped or killed in the process.

![Dijkstra's Curse Gameplay Gif](https://github.com/CaffeinePwrdAl/Dijkstras-Curse/blob/master/images/dijkstras_curse_one_path.gif)

In some levels, you may start in multiple places at once, this can be a help or a hinderance.

But as long as you hit the portal in the same move, you live on and move on to the next level - even if you brush into the spikes at the same time.

On some levels, this is _**much**_ harder than it looks.

![Dijkstra's Curse Gameplay Gif](https://github.com/CaffeinePwrdAl/Dijkstras-Curse/blob/master/images/dijkstras_curse_1.gif)

## Background - Dijkstra's Algorithm

The name of this game is for the well known [Dijkstra's Algorithm](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm)
for path finding between nodes on a graph, conceived by [Edsger W. Dijkstra, 1956](https://en.wikipedia.org/wiki/Edsger_W._Dijkstra).

Though it will always find the best path, it is sometimes referred to as _'flood filling'_ the search space, and will visit more nodes than
some later heuristic extensions to the path-finding space such as A* - This is _**'it's curse'**_, if you will, and the point that inspired this game.

The oddity, as it turned out, is that the game _**hasn't actually needed to employ a path finding algorithm!**_

Here is a rather good example image of the algorithm in operation by [SubH83](https://commons.wikimedia.org/wiki/User:Subh83) / [CC BY 3.0](https://creativecommons.org/licenses/by/3.0)

[![Dijkstra's Algorithm](https://github.com/CaffeinePwrdAl/Dijkstras-Curse/blob/master/images/dijkstras_progress_animation.gif)](https://en.wikipedia.org/wiki/Dijkstra%27s_algorithm#/media/File:Dijkstras_progress_animation.gif)
