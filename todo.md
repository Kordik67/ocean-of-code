About my wood to bronze code :
==============================

I finaly managed to get Bronze. That was pretty easy since most people in this elo use random moves, random torpedoes, no silence and no sonar.
Only top 5 people at this elo will start using those.

## Ideas to make my code better
- [ ] Translate from French to English for those who don't speak French (mainly comments)
- [ ] Make it cleaner. I made so much things that now the code is fat and harder to understand
- [ ] Review my way of craiting the grid
- [ ] The current main issue to detect enemy position is the silence. Of the enemy use it too often you won't be able to guess his position precisely
- [ ] Hunting down the enemy is based on his directions, his torpedo targets and the sector in which it surfaces. There is an issue when the opponent orders are reversed, like `TORPEDO 3 4 | MOVE N` instead of `MOVE N | TORPEDO 3 4`. Currently if it happens it would tell you that your there is 0 possible positions for the enemy
- [ ] Review my way of moving my submarine
- [ ] I don't use silence which makes me more vulnerable
- [ ] When enemy use silence it can be a good idea to charge a sonar to find him faster again
- [ ] I was thinking about creating a map for each enemy possibilities to save his path, and maybe other stuff. If a map becomes wrong, I would delete it to make my code faster
