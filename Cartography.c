/*
largura maxima = 100 colunas
tab = 4 espaços
0123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789

	Linguagens e Ambientes de Programação (B) - Projeto de 2019/20

	Cartography.c

	Este ficheiro constitui apenas um ponto de partida para o
	seu trabalho. Todo este ficheiro pode e deve ser alterado
	à vontade, a começar por este comentário. É preciso inventar
	muitas funções novas.

COMPILAÇÃO

  gcc -std=c11 -o Main Cartography.c Main.c -lm

IDENTIFICAÇÃO DOS AUTORES

  Aluno 1: 55833 Miguel Ramalhete
  Aluno 2: 56187 Diogo Paulico

COMENTÁRIO

We use two for loops in the command D, the first one to know how many different districts we have
and the second one to save all those districts.
We ended up using the same squeme for the command C.
In the command F we use the command A to detect all the adjacent parcels to the first position,
if none of them matches the second position we verifie each one of those new pos, 
and there adjacent parcel and recheck if they match until we get a match.

Coloque aqui a identificação do grupo, mais os seus comentários, como
se pede no enunciado.

*/

#define USE_PTS true

#include "Cartography.h"

typedef int Data;
typedef struct Node *List;
typedef struct PartitionNode *ListOfPartions;
typedef struct Node {
    Data data;
    List next;
} Node;

typedef struct PartitionNode {
    List data;
    ListOfPartions next;
} PartitionNode;

static List newNode(Data val, List next)
{
    List n = malloc(sizeof(Node));
    if( n == NULL )
        return NULL;
    n->data = val;
    n->next = next;
    return n;
}

static List listPutAtEnd(List l, Data val)
{
    if( l == NULL )
        return newNode(val, NULL);
    else {
        List p;
        for( p = l ; p->next != NULL ; p = p->next ); // Stop at the last node
        p->next = newNode(val, NULL);  // Assign to the next of the last node
        return l;
    }
}

static ListOfPartions newPartition (int pos, ListOfPartions next)
{
	ListOfPartions new = malloc(sizeof(PartitionNode));
	new->data = newNode(pos, NULL);
	new->next = next; 
	return new;
}

static ListOfPartions addParcelToPartition(int pos, ListOfPartions list){
	list->data = listPutAtEnd(list->data, pos);
	return list;
}

static ListOfPartions partitionAddEnd(int pos, ListOfPartions list){
	if(list == NULL){
		return newPartition(pos, NULL);
	}
	else{
		ListOfPartions p;
		for(p = list; p->next != NULL; p = p->next);
		p->next = newPartition(pos, NULL);
		return p->next;
	}
}

/* STRING -------------------------------------- */

static void showStringVector(StringVector sv, int n) {
	int i;
	for( i = 0 ; i < n ; i++ ) {
		printf("%s\n", sv[i]);
	}
}

/* UTIL */

static void error(String message)
{
	fprintf(stderr, "%s.\n", message);
	exit(1);	// Termina imediatamente a execução do programa
}

static void readLine(String line, FILE *f)	// lê uma linha que existe obrigatoriamente
{
	if( fgets(line, MAX_STRING, f) == NULL )
		error("Ficheiro invalido");
	line[strlen(line) - 1] = '\0';	// elimina o '\n'
}

static int readInt(FILE *f)
{
	int i;
	String line;
	readLine(line, f);
	sscanf(line, "%d", &i);
	return i;
}


/* IDENTIFICATION -------------------------------------- */

static Identification readIdentification(FILE *f)
{
	Identification id;
	String line;
	readLine(line, f);
	sscanf(line, "%s %s %s", id.freguesia, id.concelho, id.distrito);
	return id;
}

static void showIdentification(int pos, Identification id, int z)
{
	if( pos >= 0 ) // pas zero interpretado como não mostrar
		printf("%4d ", pos);
	else
		printf("%4s ", "");
	if( z == 3 )
		printf("%-25s %-13s %-22s", id.freguesia, id.concelho, id.distrito);
	else if( z == 2 )
		printf("%-25s %-13s %-22s", "", id.concelho, id.distrito);
	else
		printf("%-25s %-13s %-22s", "", "", id.distrito);
}

static void showValue(int value)
{
	if( value < 0 ) // value negativo interpretado como char
		printf(" [%c]\n", -value);
	else
		printf(" [%3d]\n", value);
}

static bool sameIdentification(Identification id1, Identification id2, int z)
{
	if( z == 3 )
		return strcmp(id1.freguesia, id2.freguesia) == 0
			&& strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else if( z == 2 )
		return strcmp(id1.concelho, id2.concelho) == 0
			&& strcmp(id1.distrito, id2.distrito) == 0;
	else
		return strcmp(id1.distrito, id2.distrito) == 0;
}


/* COORDINATES -------------------------------------- */

Coordinates coord(double lat, double lon)
{
	Coordinates c = {lat, lon};
	return c;
}

static Coordinates readCoordinates(FILE *f)
{
	double lat, lon;
	String line;
	readLine(line, f);
	sscanf(line, "%lf %lf", &lat, &lon);
	return coord(lat, lon);
}

bool sameCoordinates(Coordinates c1, Coordinates c2)
{
	return c1.lat == c2.lat && c1.lon == c2.lon;
}

static double toRadians(double deg)
{
	return deg * PI / 180.0;
}

// https://en.wikipedia.org/wiki/Haversine_formula
double haversine(Coordinates c1, Coordinates c2)
{
	double dLat = toRadians(c2.lat - c1.lat);
	double dLon = toRadians(c2.lon - c1.lon);

	double sa = sin(dLat / 2.0);
	double so = sin(dLon / 2.0);

	double a = sa * sa + so * so * cos(toRadians(c1.lat)) * cos(toRadians(c2.lat));
	return EARTH_RADIUS * (2 * asin(sqrt(a))); //in kilometers
}


/* RECTANGLE -------------------------------------- */

Rectangle rect(Coordinates tl, Coordinates br)
{
	Rectangle r = {tl, br};
	return r;
}

static void showRectangle(Rectangle r)
{
	printf("{%lf, %lf, %lf, %lf}",
			r.topLeft.lat, r.topLeft.lon,
			r.bottomRight.lat, r.bottomRight.lon);
}

static Rectangle calculateBoundingBox(Coordinates vs[], int n)
{
	int i;
	double smallLat, bigLat, smallLon, bigLon;
	smallLat = bigLat = vs[0].lat;
	smallLon = bigLon = vs[0].lon;
	for(i = 0; i < n; i++){
		if(vs[i].lat < smallLat){smallLat = vs[i].lat;}
		if(vs[i].lat > bigLat){bigLat = vs[i].lat;} 
		if(vs[i].lon < smallLon){smallLon = vs[i].lon;}
		if(vs[i].lon > bigLon){bigLon = vs[i].lon;}
	}
	return rect(coord(bigLat,smallLon), coord(smallLat,bigLon));
}

bool insideRectangle(Coordinates c, Rectangle r)
{
	if(c.lat > r.topLeft.lat || c.lat < r.bottomRight.lat || c.lon < r.topLeft.lon
	|| c.lon > r.bottomRight.lon )
		return false;
	return true;
}


/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	/*if( n > MAX_VERTEXES )
		error("Anel demasiado extenso");*/
	r.nVertexes = n;
	r.vertexes = malloc ((r.nVertexes * sizeof(Coordinates)));
	for( i = 0 ; i < n ; i++ ) {
		r.vertexes[i] = readCoordinates(f);
	}
	r.boundingBox =
		calculateBoundingBox(r.vertexes, r.nVertexes);
	return r;
}


// http://alienryderflex.com/polygon/
bool insideRing(Coordinates c, Ring r)
{
	if( !insideRectangle(c, r.boundingBox) )	// otimização
		return false;
	double x = c.lon, y = c.lat;
	int i, j;
	bool oddNodes = false;
	for( i = 0, j = r.nVertexes - 1 ; i < r.nVertexes ; j = i++ ) {
		double xi = r.vertexes[i].lon, yi = r.vertexes[i].lat;
		double xj = r.vertexes[j].lon, yj = r.vertexes[j].lat;
		if( ((yi < y && y <= yj) || (yj < y && y <= yi))
								&& (xi <= x || xj <= x) ) {
			oddNodes ^= (xi + (y-yi)/(yj-yi) * (xj-xi)) < x;
		}
	}
	return oddNodes;
}

bool adjacentRings(Ring a, Ring b)
{
	int na, nb;
	for(na = 0; na < a.nVertexes; na++){
		for(nb = 0; nb < b.nVertexes; nb++){
			if(sameCoordinates(a.vertexes[na],b.vertexes[nb]))
				return true;
		}
	}
////// FAZER -- tem alguma coordenada comum
	return false;
}


/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	/*if( n > MAX_HOLES )
		error("Poligono com demasiados buracos");*/
	p.edge = readRing(f);
	p.nHoles = n;
	if(p.nHoles > 0){
	p.holes = malloc ((p.nHoles * sizeof(Ring)));
	for( i = 0 ; i < n ; i++ ) {
		p.holes[i] = readRing(f);
	}
	}
	return p;
}

static void showHeader(Identification id)
{
	showIdentification(-1, id, 3);
	printf("\n");
}

static void showParcel(int pos, Parcel p, int lenght)
{
	showIdentification(pos, p.identification, 3);
	showValue(lenght);
}

bool insideParcel(Coordinates c, Parcel p)
{
	if(!insideRectangle(c,p.edge.boundingBox)){
			return false;
	}
	if(p.holes > 0){
		int i;
		for(i = 0; i < p.nHoles; i++){
			if(insideRing(c,p.holes[i])){
				return false;}
		}
	}
		return insideRing(c,p.edge);

}

bool adjacentParcels(Parcel a, Parcel b)
{
	int h;
	if(a.nHoles > 0){
		for(h = 0; h < a.nHoles; h++){
			if(adjacentRings(a.holes[h],b.edge))
				return true;
			}
		}

	if(b.nHoles > 0){
		for(h = 0; h < b.nHoles; h++){
			if(adjacentRings(b.holes[h],a.edge))
				return true;
			}
		}

	if(adjacentRings(a.edge,b.edge)){
		return true;
	}
		return false;
}


/* CARTOGRAPHY -------------------------------------- */

int loadCartography(String fileName, Cartography *cartography)
{
	FILE *f;
	int i;
	f = fopen(fileName, "r");
	if( f == NULL )
		error("Impossivel abrir ficheiro");
	int n = readInt(f);
	*cartography = malloc((n * sizeof(Parcel)));
	/*if( n > MAX_PARCELS )
		error("Demasiadas parcelas no ficheiro");*/
	for( i = 0 ; i < n ; i++ ) {
		//(*cartography)[i] = readParcel(f); 
		((Parcel*)cartography)[i] = readParcel(f);
	}
	fclose(f);
	return n;
}

static int findLast(Cartography cartography, int n, int j, Identification id)
{
	for(  ; j < n ; j++ ) {
		if( !sameIdentification(cartography[j].identification, id, 3) )
			return j-1;
	}
	return n - 1;
}

void showCartography(Cartography cartography, int n)
{
	int last;
	Identification header = {"___FREGUESIA___", "___CONCELHO___", "___DISTRITO___"};
	showHeader(header);
	for( int i = 0 ; i < n ; i = last + 1 ) {
		last = findLast(cartography, n, i, cartography[i].identification);
		showParcel(i, cartography[i], last - i + 1);
	}
}


/* INTERPRETER -------------------------------------- */

static bool checkArgs(int arg)
{
	if( arg != -1 )
		return true;
	else {
		printf("ERRO: FALTAM ARGUMENTOS!\n");
		return false;
	}
}

static bool checkPos(int pos, int n)
{
	if( 0 <= pos && pos < n )
		return true;
	else {
		printf("ERRO: POSICAO INEXISTENTE!\n");
		return false;
	}
}

//L
static void commandListCartography(Cartography cartography, int n)
{
	showCartography(cartography, n);
}

static int compareCounties (const void * a, const void * b) {
  return strcmp((*(String*)a), (*(String*)b));
}

static void sortCountie(String aux[], int n) {
    qsort(aux, n, sizeof(String), compareCounties);
}

static bool notInArray(String text, String array[], int n ){
	for(int i = 0; i < n; i++){
		if(strcmp(text, array[i]) == 0){
			return false;
		}
	}
	return true;
}

// C
static void commandListCounties(Cartography cartography, int n)
{
	int i = 0;
	String last;
	String all[n];
	int b = 0;
	for(i = 0; i < n; i++){
		if(i == 0){ 
			strcpy(all[b], cartography[i].identification.concelho);
			b++;
			strcpy(last, cartography[i].identification.concelho);
		} else{
			if(strcmp(cartography[i].identification.concelho, last) != 0 || notInArray(cartography[i].identification.concelho, all, b)){
				strcpy(all[b], cartography[i].identification.concelho);
				b++;
				strcpy(last, cartography[i].identification.concelho);
			}
		}
	}
	String counties[b];
	for(int i = 0; i < b; i++){
		strcpy(counties[i], all[i]);
	}

	 sortCountie(counties, b);
	 showStringVector(counties, b);
}


static int compareDistricts(const void * a, const void * b) {
  return strcmp((*(String*)a), (*(String*)b));
}

static void sortDistricts(String aux[], int n) {
    qsort(aux, n, sizeof(String), compareDistricts);
}

//D
static void commandListDistricts(Cartography cartography, int n)
{
	int i = 0;
	String last;
	String all[n];
	int b = 0;
	for(i = 0; i < n; i++){
		if(i == 0){ 
			strcpy(all[b], cartography[i].identification.distrito);
			b++;
			strcpy(last, cartography[i].identification.distrito);
		} else{
			if(strcmp(cartography[i].identification.distrito, last) != 0 || notInArray(cartography[i].identification.distrito, all, b)){
				strcpy(all[b], cartography[i].identification.distrito);
				b++;
				strcpy(last, cartography[i].identification.distrito);
			}
		}
	}
	String districts[b];
	for(int i = 0; i < b; i++){
		strcpy(districts[i], all[i]);
	}

	 sortDistricts(districts, b);
	 showStringVector(districts, b);
	}

static void extrems(int pos, Identification id, int z, char direction){	
	showIdentification(pos, id, z); 
	printf(" [%c]\n", direction);
}

//X 
static void commandExtrems(Cartography cartography, int n){
	int i = 0;
	int rLeft, rRight, rTop, rBottom;
	for (i = 0; i < n; i++ ){
		if(i == 0){
		rLeft = rRight = rTop = rBottom = 0;
		}
		else {
		if(cartography[rBottom].edge.boundingBox.bottomRight.lat > 
		cartography[i].edge.boundingBox.bottomRight.lat){
			rBottom = i;
		}
		if(cartography[rTop].edge.boundingBox.topLeft.lat < 
		cartography[i].edge.boundingBox.topLeft.lat){
			rTop = i;
		}
		if(cartography[rRight].edge.boundingBox.bottomRight.lon < 
		cartography[i].edge.boundingBox.bottomRight.lon){
			rRight = i;
		}
		if(cartography[rLeft].edge.boundingBox.topLeft.lon > 
		cartography[i].edge.boundingBox.topLeft.lon){
			rLeft = i;
		}
		}
	}
	extrems( rTop, cartography[rTop].identification, 3, 'N');
	extrems( rRight, cartography[rRight].identification, 3, 'E');
	extrems( rBottom, cartography[rBottom].identification, 3, 'S');
	extrems( rLeft, cartography[rLeft].identification, 3, 'W');
}

static void showParcelPersonalized(int pos, Parcel p, int lenght, int z){
	showIdentification(pos, p.identification, z);
	showValue(lenght);
}

//Q
static void commandHowMany( int pos, Cartography cartography, int n){
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return;
	int i = 0;
	int nfreg = 0;
	int ncount = 0;
	int ndistrict = 0; 
for(i = 0; i < n; i++){
if(strcmp(cartography[pos].identification.freguesia,cartography[i].identification.freguesia)== 0){
			nfreg++;
		}
	if(strcmp(cartography[pos].identification.concelho,cartography[i].identification.concelho)==0){
			ncount = ncount + 1;
		}	
	if(strcmp(cartography[pos].identification.distrito,cartography[i].identification.distrito)==0){
			ndistrict++;
		}
	}
	showParcelPersonalized(pos, cartography[pos],nfreg, 3);
	showParcelPersonalized(pos, cartography[pos],ncount, 2);
	showParcelPersonalized(pos, cartography[pos],ndistrict, 1);
	}

//R
static void commandMetaData(int pos, Cartography cartography, int n){
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return;
	showIdentification(pos, cartography[pos].identification, 3);
	printf("\n     %d ", cartography[pos].edge.nVertexes);
	if(cartography[pos].nHoles > 0){
		int i;
		for(i = 0; i < cartography[pos].nHoles; i++){
			printf("%d ", cartography[pos].holes[i].nVertexes);	
		}
	}
Rectangle r = cartography[pos].edge.boundingBox;
	showRectangle(r);
	printf("\n");
}

//P
static void parcel(Cartography cartography, double lat, double lon, int n){
	int i;
	Coordinates c = coord(lat,lon);
	for(i = 0; i < n; i++){
		if(insideParcel(c, cartography[i])){
			showIdentification(i, cartography[i].identification, 3);
			printf("\n");
			return;
		}	
	}
	printf("FORA DO MAPA\n");
	return;
}

//V
static void trip(Cartography cartography,double lat, double lon, int pos, int n){
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return;
	double minDistance;
	int i = 0;
	Coordinates cord = coord(lat,lon);
	for(; i < cartography[pos].edge.nVertexes; i++){
		Coordinates edgecord = coord(cartography[pos].edge.vertexes[i].lat, 
		cartography[pos].edge.vertexes[i].lon);
		double distance = haversine(cord, edgecord);
		if(distance < minDistance || i == 0)
			minDistance = distance;
	}
	printf(" %f\n", minDistance);
}

static List adjacentAux(int pos, Cartography cartography, int n){
	List firstAdj;
	List adj;
	int count, i;
	count = i = 0;
	for(; i < n; i++){
		if(i != pos){
			if(adjacentParcels(cartography[pos],cartography[i])){
				if(count == 0){
					adj = newNode(i, NULL);
					firstAdj = adj;
					count++;
				}
				else{
				adj = listPutAtEnd(adj, i);
				count++;
				}
			}
		}
	}
	if(count == 0){
		return NULL;
	}
	return firstAdj;
}

// A
static void adjacent(int pos, Cartography cartography, int n){
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return;
	int num;
	List firstAdj = adjacentAux(pos, cartography, n);
	if(firstAdj == NULL){
		printf("NAO HA ADJACENCIAS\n");
		return;
	}

	for(; firstAdj != NULL; firstAdj = firstAdj->next){
		num = firstAdj->data;
		showIdentification(num,cartography[num].identification,3);
		printf("\n");
	}
	
}

// F
static void borders(Cartography cartography, int pos1, int pos2, int n){
	
	if( !checkArgs(pos1)  || !checkPos(pos1, n) || !checkArgs(pos2)  || !checkPos(pos2, n))
		return;
	
	if(pos1 == pos2){
		printf(" 0\n");
		return;
	}
	List queue = NULL;
	int current = 0;
	queue = listPutAtEnd(queue, pos1);
	List first = queue;
	int distances[n];
	for(int i = 0; i < n; i++){
		distances[i] = -1;
	}
	distances[pos1] = 0;
	while(distances[pos2] == -1 && first != NULL){
		current = first->data;
		List adj = adjacentAux(current, cartography,n);
		for(;adj != NULL; adj = adj->next){
			if(distances[adj->data] == -1){
				distances[adj->data] = distances[current] + 1;
				queue = listPutAtEnd(queue, adj->data);
			}
		}
		first = first->next;
	}
	if(distances[pos2] == -1){
		printf("NAO HA CAMINHO\n");
		return;
	}
	printf(" %d\n", distances[pos2]);
}

static bool checkPartitionDist(Cartography cartography, ListOfPartions list, int pos, double di){
	bool foundSmaller = false;
		List tempData = list->data;
		for(; tempData != NULL; tempData = tempData->next){
			if(haversine(cartography[tempData->data].edge.vertexes[0], cartography[pos].edge.vertexes[0]) <= di){
				return true;
		}
	}
	return foundSmaller;
}

static void printPartitions(ListOfPartions list){
	for(; list !=NULL; list = list->next){
		printf(" ");
		int start = list->data->data;
		int current = start;
		int counter = 0;		
		for(list->data = list->data->next ;list->data !=NULL; list->data = list->data->next){
			if(list->data->data == current + 1){
				current++;
			}
			else {
				printf("%d-%d", start, current);

			}
		}
		printf("\n");
	}
}


// T
static void partitions(double distance, Cartography cartography, int n){
	if(n > 0){
		int bitmap[n];
		for(int b = 0; b < n; b++){
			bitmap[b] = 0;
		}
		ListOfPartions list = newPartition(0, NULL);
		ListOfPartions first = list;
		bitmap[0] = 1;
		for(int i = 1; i < n; i++){
			for(; list->next != NULL; list = list->next){
				if(checkPartitionDist(cartography, list, i, distance)){
					bitmap[i] = 1;
					list = addParcelToPartition(i, list);
			}
		}
		if(checkPartitionDist(cartography, list, i, distance)){
					bitmap[i] = 1;
					list = addParcelToPartition(i, list);
			}
		if(bitmap[i] == 0){
			list = partitionAddEnd(i, list);
			bitmap[i] = 1;
		}
		list = first;
	}
	printPartitions(list);
	}

}

// M pos
static void commandMaximum(int pos, Cartography cartography, int n)
{
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return;
	int maxVert = 0;
	int parcelPos = 0;
	int currentPos = pos;
	Identification id = cartography[currentPos].identification;
	while(currentPos >= 0 && sameIdentification(cartography[currentPos].identification, id, 3)) {
		currentPos--;
	}
	currentPos++;
	while(sameIdentification(cartography[currentPos].identification, id, 3)){
		if(cartography[currentPos].edge.nVertexes > maxVert){
			maxVert = cartography[currentPos].edge.nVertexes; 
			parcelPos = currentPos;
		}
		if(cartography[currentPos].nHoles > 0){
			int count = 0;
			for(int i = 0; i < cartography[currentPos].nHoles ; i++){
				count += cartography[currentPos].holes[i].nVertexes;
			}
			count += cartography[currentPos].edge.nVertexes;
			if(count > maxVert){
					  maxVert = count;
					  parcelPos = currentPos;
				}
		}
		currentPos++;
	}
	showParcel(parcelPos, cartography[parcelPos], maxVert);
}

void interpreter(Cartography cartography, int n)
{
	String commandLine;
	for(;;) {	// ciclo infinito
		printf("> ");
		readLine(commandLine, stdin);
		char command = ' ';
		double arg1 = -1.0, arg2 = -1.0, arg3 = -1.0;
		sscanf(commandLine, "%c %lf %lf %lf", &command, &arg1, &arg2, &arg3);
		// printf("%c %lf %lf %lf\n", command, arg1, arg2, arg3);
		switch( commandLine[0] ) {
			case 'L': case 'l':	// listar
				commandListCartography(cartography, n);
				break;

			case 'M': case 'm':	// maximo
				commandMaximum(arg1, cartography, n);
				break;
			
			case 'V': case 'v':
				trip(cartography, arg1, arg2, arg3, n);
				break;
			
			case 'R': case 'r':
				commandMetaData(arg1, cartography, n);
				break;

			case 'C': case 'c': 
				commandListCounties(cartography, n);
				break;
			
			case 'D': case 'd': 
				commandListDistricts(cartography, n);
				break;
			
			case 'Q': case 'q': 
				commandHowMany(arg1,cartography,n);
				break;
			
			case 'X': case 'x': 
				commandExtrems(cartography, n);
				break;
			
			case 'P': case 'p': 
				parcel(cartography, arg1, arg2, n);
				break;

			case 'A': case 'a':	
				adjacent(arg1, cartography, n);
				break;

			case 'F': case 'f':
				borders(cartography, arg1, arg2, n);
				break;

			case 'T': case 't':	
				partitions(arg1, cartography, n);
				break;

			case 'Z': case 'z':	
				printf("Fim de execucao! Volte sempre.\n");
				return;

			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
