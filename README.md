# Program labirynt.c:
- generuje losowy labirynt o wymiarach n x n (n podane jako argument wywołania programu)
- używa algorytmu DFS (depth-first search) do utworzenia labiryntu bez cykli
- traktuje każdą komórkę labiryntu jako wierzchołek grafu ważonego
- krawędziami tego grafu są przejścia między sąsiednimi komórkami (rozumiane jako brak ściany między nimi)
- każda krawędź ma losową wagę od 0.0 do 10.0
- używa algorytmu Dijkstra
