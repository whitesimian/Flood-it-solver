# Flood-it solver

Heuristic approach to solve the NP-Hard "Flood-It" problem.
This project was made for the "Graph Theory" course at UFOP, JUL/2018.

#### Premises:
- Connected graph.
- Always flooding (not flooded), i.e., the graph ends with the same colour as the initial pivot.
- Colours identifiers start at 1.

## The ideia

I find a initial solution and try to improve it by using a descent and first improvement heuristic.

### Initial solution

At each step (chosing a colour to flood), it is chosen the colour that maximizes the number of new adjacent areas of the same colour.
#### Example: 
- If I choose colour RED to flood I'll have 3 new adjacent areas of colour YELLOW and 5 new BLACK.
- If I choose colour BLACK to flood I'll have 1 new adjacent areas of colour YELLOW and 7 new RED.

Then I choose colour BLACK, because 7 > 5.
Besides that, it is not completely ignored the existing adjacent areas. Except that, instead of counting the frequency of these initial adjacent areas, they are counted as 1, regardless of how many colours RED or YELLOW are adjacent to the pivot area.

This seems a naïve approach, but it generates a pretty decent initial solution.

### Descent, First Improvement Heuristic

Given the initial solution (sequence of chosen colours), I try to remove steps to improve the solution (less steps).

#### Algorithm:
- Remove the first colour of the solution.
- If the remaining sequence dos not generates a viable solution, step back to the previous viable solution and remove the next colour.
- If the remaining sequance generates a viable solution, remove permanently the colour at that index of the solution and all the steps ahead not used.
- If the position of the colour to be removed of the solution exceeds the end, end the heuristic and adopt the current viable solution as the final one.
