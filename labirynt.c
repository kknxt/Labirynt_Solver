#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

// Struktura reprezentujaca pojedyncza komorke labiryntu
typedef struct Cell{
    int i; // Wspolrzedna kolumny labiryntu komorki
    int j; // Wspolrzedna wiersza labiryntu komorki
    
    /*Okresla z jakich stron komorki znajduja sie sciany
    0 - gora
    1 - prawo
    2 - dol
    3 - lewo */
    bool walls[4]; 
    
    bool visited; // Czy komorka zostala przeiterowana w generacji labiryntu lub szukaniu najkrotszej sciezki
    float weight; // Waga komorki
    int totalWeight; // Calkowita "waga sciezki" od poczatku szukania najkrotszej sciezki na danej komorce
} Cell;

// Inicjalizacja komorki o podanych wsporzednych i oraz j
void initializeCell(Cell* cell, int i, int j){
    cell->i=i;
    cell->j=j;
    
    // Otoczenie komorki z kazdej strony scianami
    cell->walls[0]=true;
    cell->walls[1]=true;
    cell->walls[2]=true;
    cell->walls[3]=true;

    cell->visited=false;
}

/* Zmienia dwuwymiarowe wspolrzedne i oraz j
na odpowiadajacy im indeks w jednowymiarowej tablicy*/
int getIndex(int i, int j, int n){
    // Sprawdzenie czy dane wspolrzedne sa czescia labiryntu
    if(i<0 || j<0 || i>n-1 || j>n-1){
        return -1;
    }
    // Indeks komorki z kolumny i, wiersza j w labiryncie n x n 
    return i + j * n;
}

// Sprawdza sasiadujace komorki i zwraca sasiadujaca komorke ktora nie zostala odwiedzona w iteracji.
Cell checkNeighbours(Cell cell, int n, Cell list[]){
    Cell neighbours[4];
    int counter=0;
    // Wypelnia tablice neighbours domyslnymi komorkami
    for(int i=0; i<4; i++){
        Cell newCell;
        initializeCell(&newCell,-1,-1);
        neighbours[i]=newCell;
    }
    // Pobranie indeksu komorki powyzej
    int topIndex = getIndex(cell.i,cell.j-1,n);
    if(topIndex != -1){ // Czy komorka jest w labiryncie?
        Cell top = list[topIndex]; 
        /* Jesli komorka powyzej nie byla jeszcze odwiedzona,
        dodaje komorke do listy nieodwiedzonych sasiadow, 
        w odpowiednim indeksie tablicy dla tego kierunku*/
        if(!top.visited){
            neighbours[0]=top;
            counter++;
        }
    }
    /*Analogicznie to samo dla wszyskich pozostalych kierunkow*/
    int rightIndex = getIndex(cell.i+1,cell.j,n);
    if(rightIndex != -1){
        Cell right = list[rightIndex];
        if(!right.visited){
            neighbours[1]=right;
            counter++;
        }
    }
    int bottomIndex = getIndex(cell.i,cell.j+1,n);
    if(bottomIndex != -1){
        Cell bottom = list[bottomIndex];
        if(!bottom.visited){
            neighbours[2]=bottom;
            counter++;
        }
    }
    int leftIndex = getIndex(cell.i-1,cell.j,n);
    if(leftIndex != -1){
        Cell left = list[leftIndex];
        if(!left.visited){
            neighbours[3]=left;
            counter++;
        }
    }
    if(counter > 0){ // Jesli istnieja sasiadujace komorki, ktore nie zostaly odwiedzone w iteracji
        Cell* validNeighbours = malloc(counter * sizeof(Cell));
        int validCounter = 0;
        /* Zapisanie w liscie validNeighbours
        tylko nieodwiedzonych,
        sasiadujacych z komorka o wspolrzednych i, j komorek */
        for(int i = 0; i < 4; i++){
            if(neighbours[i].i !=-1 ){
                validNeighbours[validCounter] = neighbours[i];
                validCounter++;
            }
        }
        int randomIndex = rand() % validCounter;
        // Losowy wybor sasiadujacej komorki z listy nieodwiedzonych sasiadow
        Cell randomNeighbour = validNeighbours[randomIndex];
        free(validNeighbours);
        return randomNeighbour;
    }
    /* W przypadku braku nieodwiedzonych w iteracji sasiadow,
    funkcja zwraca domyslna komorke o ujemnych wspolrzednych */
    else{
        Cell newCell;
        initializeCell(&newCell,-1,-1);
        return newCell;
    }
}

// Usuwa sciany miedzy dwoma sasiadujacymi komorkami
void removeWalls(Cell *current, Cell *next){
    int x = current->i - next->i;
    // Jesli nastepna (next) komorka jest na lewo
    if(x==1){
        current->walls[3]=false; // Usuwanie lewej sciany obecnej (current) komorki
        next->walls[1]=false; // Usuwanie prawej sciany nastepnej komorki
    }
    // Jesli nastepna komorka jest na prawo, analogicznie
    else if(x==-1){
        current->walls[1]=false; // Usuwanie prawej sciany obecnej komorki
        next->walls[3]=false; // Usuwanie lewej sciany nastepnej komorki
    }
    int y = current->j - next->j;
    // Jesli nastepna komorka jest powyzej obecnej
    if(y==1){
        current->walls[0]=false; // Usuwanie gornej sciany obecnej komorki
        next->walls[2]=false; // Usuwanie dolnej sciany nastepnej komorki
    }
    // Jesli nastepna komorka jest ponizej obecnej
    else if(y==-1){
        current->walls[2]=false; // Usuwanie dolnej sciany obecnej komorki
        next->walls[0]=false; // Usuwanie gornej sciany nastepnej komorki
    }
}

// Definicja struktury stosu, koniecznej do generacji labiryntu przy uzyciu DFS
typedef struct Stack {
    Cell* cells; // Stos przechowuje komorki
    int top;
} Stack;

// Dodaje komorke na "gore" stosu
void push(Stack* stack, Cell cell) {
    stack->cells[++stack->top] = cell;
}

// Usuwa i zwraca "gorna" komorke
Cell pop(Stack* stack) {
    return stack->cells[stack->top--];
}

// Sprawdza czy stos jest pusty
bool isEmpty(Stack* stack) {
    return stack->top == -1; // Zwraca True, gdy stos jest pusty
}

/* Definicja struktury kolejki priorytetowej,
uzytej w algorytmie Dijkstry do znajdowania najkrotszej sciezki */
typedef struct PriorityQueue {
    Cell* cells; // Kolejka przechowuje instancje struktury komorkiei
    float* priorities; // Priorytety komorek labiryntu, zalezne od wagi w danej komorce
    int size; // Liczba komorek w kolejce 
    int maxSize;
} PriorityQueue;

// Dodaje komorke wraz z jej waga (jako priorytet) do kolejki
void enqueue(PriorityQueue* queue, Cell cell, int priority) {
    if (queue->size == queue->maxSize) {
        printf("Przepelnienie kolejki\n");
        exit(1);
    }
    int i;
    // Znajdowanie odpowiedniej pozycji dla komorki w kolejce wg. jej priorytetu
    // Ustawia priorytety komorek wg. zmiennej totalWeight rosnaco
    for (i = queue->size; i > 0 && queue->priorities[i - 1] > priority; i--) {
        queue->cells[i] = queue->cells[i - 1];
        queue->priorities[i] = queue->priorities[i - 1];
    }

    // Wstawienie komorki do kolejki
    queue->cells[i] = cell;
    queue->priorities[i] = priority;
    queue->size++;
}

// Usuwa komorke o najwiekszej calkowitej wadze z konca kolejki
Cell dequeue(PriorityQueue* queue) {
    if (queue->size == 0) {
        printf("Zbyt malo elementow w kolejce\n");
        exit(1);
    }
    // Zmniejsza rozmiar kolejki o jedna komorke
    // Zwraca wartosc ostatniej komorki
    return queue->cells[--queue->size];
}

// Sprawdza czy kolejka jest pusta
bool isQueueEmpty(PriorityQueue* queue) {
    return queue->size == 0;
}

// Sprawdza czy dana komorka jest dodana do kolejki
bool isInQueue(PriorityQueue* queue, Cell cell) {
    for (int i = 0; i < queue->size; i++) {
        if (queue->cells[i].i == cell.i && queue->cells[i].j == cell.j) {
            return true;
        }
    }
    return false;
}

// Zmienia wartosc priorytetu danej komorki
void updatePriority(PriorityQueue* queue, Cell cell, int newPriority) {
    for (int i = 0; i < queue->size; i++) {
        if (queue->cells[i].i == cell.i && queue->cells[i].j == cell.j) {
            queue->priorities[i] = newPriority;
            break;
        }
    }
}


// Wizualna reprezentacja labiryntu i najkrotszej sciezki
void printMaze(Cell* cellArray, Cell* path, int n, int starti, int endi, int startj, int endj) {
    for (int i = 0; i < n; i++) {
        printf("+---"); // Gorna granica labiryntu
    }
    printf("+\n"); // Przejscie do kolejnego wiersza

    // Iteracja po rzedach labiryntu
    for (int j = 0; j < n; j++) {
        printf("|"); // Lewa sciana graniczna labiryntu
        for (int i = 0; i < n; i++) {
            if(i==starti && j == startj){
                printf(" S "); // Oznaczenie wierzcholka startowego grafu
            }
            else if(i==endi && j == endj) {
                printf(" E "); // Oznaczenie wyjscia z labiryntu, ostatniego wierzcholka grafu
            }
            else{
                printf("   "); // wierzcholek grafu
            }
            printf("%s", cellArray[i + j * n].walls[1] ? "|" : " "); // Drukowanie prawej sciany komorki, jesli istnieje
        }
        printf("\n+");
        // Drukowanie dolnych scian komorek labiryntu, jesli istnieja
        for (int i = 0; i < n; i++) {
            printf("%s", cellArray[i + j * n].walls[2] ? "---" : "   ");
            printf("+");
        }
        printf("\n");
    }
    // Iteracja drukujaca wizualizacje najkrotszej sciezki labiryntu
    for (int j = 0; j < n; j++) {
        printf("|");
        for (int i = 0; i < n; i++) {
            bool inPath = false;
            // Sprawdzenie czy dana komorka jest czescia najkrotszej sciezki przez labirynt
            for (int k = 0; k < n * n; k++) {
                if (path[k].i == i && path[k].j == j) {
                    inPath = true;
                    break;
                }
            }
            /* Jesli komorka jest czescia najkrotszej sciezki,
            jest przedstawiona jako S jesli to wierzcholek poczatkowy grafu,
            E jesli jest to wierzcholek koncowy,
            lub "*" jesli jest to element sciezki miedzy poczatkiem a koncem */
            if (inPath) {
                if(i==starti && j == startj){
                    printf(" S ");
                }
                else if(i==endi && j == endj) {
                    printf(" E ");
                }
                else{
                    printf(" * ");
                }
            // Jesli komorka nie nalezy do najkrotszej sciezki, pozostaje pusta
            } else {
                printf("   ");
            }
            printf("%s", cellArray[i + j * n].walls[1] ? "|" : " ");  // Drukowanie prawej sciany komorki, jesli istnieje
        }
        printf("\n+");
        for (int i = 0; i < n; i++) {
            // Drukowanie dolnych scian komorek labiryntu, jesli istnieja
            printf("%s", cellArray[i + j * n].walls[2] ? "---" : "   ");
            printf("+");
        }
        printf("\n");
    }
}

// Algorytm Dijkstry, do znalezienia sciezki o najmniejszej wadze w labiryncie
Cell* dijkstra(Cell* cellArray, Cell start, Cell end, int n, float* pathWeight) {
    // Stworzenie pustej kolejki priorytetowej
    PriorityQueue queue;
    queue.cells = malloc(n * n * sizeof(Cell));
    queue.priorities = malloc(n * n * sizeof(float));
    queue.size = 0;
    queue.maxSize = n * n;
    start.totalWeight = start.weight;
    // Dodanie do kolejki komorki startowej
    enqueue(&queue, start, start.totalWeight);

    // Stworzenie tablicy "zapisujacej" komorki poprzedzajaca aktualna komorke
    Cell* predecessor = malloc(n * n * sizeof(Cell));
    for (int i = 0; i < n * n; i++) {
        predecessor[i] = cellArray[i];
    }

    // Glowna petla algorytmu Dijkstry
    while (!isQueueEmpty(&queue)) {
        Cell current = dequeue(&queue); 
        if (current.i == end.i && current.j == end.j) { // Sprawdzenie czy iteracja dotarla do komorki wyjscia z labiryntu
            break;
        }
        if (!current.visited) {
            current.visited = true;
            cellArray[getIndex(current.i, current.j, n)].visited = true;
            for (int i = 0; i < 4; i++) { // Sprawdzenie sasiednich komorek ze wszystkich stron
                if (!current.walls[i]) { // Jeśli nie ma sciany w danym kierunku, to:
                    int ni = current.i + (i == 1) - (i == 3); // Jeśli komórka jest z prawej, i++, z lewej, i--
                    int nj = current.j + (i == 2) - (i == 0); // Analogicznie góra/dół
                    if (ni >= 0 && ni < n && nj >= 0 && nj < n) { // Czy komorka jest w labiryncie
                        Cell* neighbor = &cellArray[ni + nj * n];
                        int newWeight = current.totalWeight + neighbor->weight; // Obliczenie wagi po odwiedzeniu sasiadujacej komorki
                        if (!neighbor->visited || newWeight < neighbor->totalWeight) { // Jesli nowa sciezka jest krotsza
                            neighbor->totalWeight = newWeight; 
                            if (isInQueue(&queue, *neighbor)) {
                                updatePriority(&queue, *neighbor, neighbor->totalWeight); // Aktualizacja priorytetu sasiadujacej komorki
                            } else {
                                enqueue(&queue, *neighbor, neighbor->totalWeight); // Dodanie sasiadujacej komorki do kolejki
                            }
                            predecessor[getIndex(neighbor->i, neighbor->j, n)] = current; // Aktualizacja poprzedzajacej komorki
                        }
                    }
                }
            }
        }
    }

    *pathWeight = 0.0;

    Cell* path = malloc(n * n * sizeof(Cell));
    int pathLength = 0;
    Cell current = end; // Iteracja od konca po poprzednich komorkach w celu skonstruowania najkrotszej sciezki
    while (!(current.i == start.i && current.j == start.j)) {
        path[pathLength++] = current; // Dodanie komorki do sciezki
        *pathWeight += current.weight; // Dodanie wagi komorki do wagi sciezki
        current = predecessor[getIndex(current.i, current.j, n)]; // Przejscie do poprzedniej komorki
    }

    // Dodanie punktu startowego do sciezki
    path[pathLength++] = start;
    *pathWeight += start.weight;

    free(queue.cells);
    free(queue.priorities);
    free(predecessor);

    return path;
}

// Funkcja przeprowadzajaca dzialanie programu
void setup(int n){
    int cols = n;
    int rows = n;
    int counter = 0;
    Cell cellArray[n*n]; // Przechowuje komorki labiryntu
    Stack stack; // Uzyty do algorytmu DFS do generacji labiryntu
    stack.cells = malloc(n * n * sizeof(Cell));
    stack.top = -1;

    //Inicjalizacja komorek labiryntu
    for (int j = 0; j< cols;j++){
        for(int i = 0; i<rows;i++){
            Cell newCell;
            initializeCell(&newCell,i,j);
            float max = 10.0; // Maksymalna waga wierzcholka grafu = 10.0
            // Nadanie komorce losowej wagi z przedzialu 0.0 - 10.0
            newCell.weight=(float)rand() / (float)(RAND_MAX/max);
            cellArray[counter] = newCell;
            counter++;
        }
    }
    // Generacja labiryntu przy uzyciu algorytmu DFS
    if (n*n > 0){
        Cell current = cellArray[0]; // wierzcholek poczatkowy
        current.visited=true; // Zaznaczanie "odwiedzenie" komorki
        int currentIndex = getIndex(current.i, current.j, n);
        cellArray[currentIndex].visited = true;
        push(&stack, current); // Dodanie pierwszej komorki do stosu
        while(!isEmpty(&stack)){ // Dopoki wszystkie komorki nie zostana odwiedzone
            Cell next = checkNeighbours(current, n, cellArray); // Znajdowanie nieodwiedzonej, sasiadujacej komorki
            if(next.i != -1){ // Jesli istnieje nieodwiedzony "sasiad"
                next.visited=true;
                int nextIndex = getIndex(next.i, next.j, n); // Pobranie indeksu sasiadujacej komorki
                cellArray[nextIndex].visited = true;
                currentIndex = getIndex(current.i, current.j, n);
                removeWalls(&cellArray[currentIndex],&cellArray[nextIndex]); // Usuniecie scian pomiedzy komorkami
                push(&stack, next);
            }
            else{
                Cell cell = pop(&stack); // Usuwanie komorki ze stosu
                current = cell;
            }
        }
    }
    free(stack.cells);
    
    Cell start = cellArray[0]; // wierzcholek startowy grafu
    Cell end = cellArray[rand()%n  + (n-1)*(n)]; // wierzcholek koncowy grafu

    // Resetowanie atrybutu visited do szukania sciezki w labiryncie
    for(int i = 0;i<n*n;i++){
        cellArray[i].visited=false;
    }
    start.visited=false;
    end.visited=false;

    float pathWeight = 0.0;
    // Uzycie algorytmu Dijkstry do znalezienia sciezki o najmniejszej wadze
    Cell* path = dijkstra(cellArray, start, end, n, &pathWeight);
    // Drukowanie labiryntu
    printMaze(cellArray, path, n,start.i,end.i, start.j, end.j);
    printf("Dlugosc (oparta na wagach) najkrotszej sciezki przedstawionej powyzej: %f\n", pathWeight);
    free(path);
}


int main(int argc, char *argv[]){
    
    int n = atoi(argv[1]);

    // Generator liczb losowych w oparciu o milisekundy
    struct timeval time;
    gettimeofday(&time, NULL);
    unsigned long seed = (time.tv_sec * 1000) + (time.tv_usec / 1000);
    srand(seed);
    
    // Utworzenie labiryntu o wymiarach podanych w argumencie uruchomienia programu
    setup(n);
    
    return 0;
}