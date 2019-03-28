# Dijkstras-Curse
A puzzle game where you are a flood-filling blob

## Object of the game

You are a blob. To survive each level you have to find a route to one of the exit portals without getting trapped or killed in the process.

![Dijkstra's Curse Gameplay Gif](https://github.com/CaffeinePwrdAl/Dijkstras-Curse/blob/master/images/dijkstras_curse_one_path.gif)

In some levels, you may start in multiple places at once, this can be a help or a hinderance.

As long as you reach the portal, you survive and move on to the next level - even if you brush into the spikes at the same time.

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

## Building

The game is written using some elements of C++14, and supports Windows and Linux.

### Windows

No dependencies, other than a compiler capable of C++14, but should build straight out of the box with Visual Studio 2015 or greater (will probably work with older versions if they load the project file ok).

### Linux

Build uses a simple GNU Makefile and requires G++ >= 4.9.

## Author

* Alex Walters - original author - @CaffeinePwrdAl

## Acknowledgements

Thanks to all my friends who have helped with a bit of play testing

And of course, thanks to Edsger Wybe Dijkstra for inventing the algorithm that inspired this game.

The gifs on this page were produced using [https://www.screentogif.com/](https://www.screentogif.com/) (https://github.com/NickeManarin/ScreenToGif)
