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

 Coloque aqui a identificação do grupo, mais os seus comentários, como
 se pede no enunciado.

*/

#include "Cartography.h"

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
	Rectangle rect = {tl, br};
	return rect;
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
	if(c.lat < r.topLeft.lat || c.lat > r.bottomRight.lat || c.lon > r.topLeft.lon
	|| c.lon < r.bottomRight.lon )
		return false;
	return true;
}


/* RING -------------------------------------- */

static Ring readRing(FILE *f)
{
	Ring r;
	int i, n = readInt(f);
	if( n > MAX_VERTEXES )
		error("Anel demasiado extenso");
	r.nVertexes = n;
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
////// FAZER -- tem alguma coordenada comum
	return false;
}


/* PARCEL -------------------------------------- */

static Parcel readParcel(FILE *f)
{
	Parcel p;
	p.identification = readIdentification(f);
	int i, n = readInt(f);
	if( n > MAX_HOLES )
		error("Poligono com demasiados buracos");
	p.edge = readRing(f);
	p.nHoles = n;
	for( i = 0 ; i < n ; i++ ) {
		p.holes[i] = readRing(f);
	}
	return p;
}

static void showHeader(Identification id)
{
	showIdentification(-1, id, 3);
	printf("\n");
	//STRING ARRAY[10] PASSAR POS PARA STRING E VER STRING LEGTH?
}

static void showParcel(int pos, Parcel p, int lenght)
{
	showIdentification(pos, p.identification, 3);
	showValue(lenght);
}

bool insideParcel(Coordinates c, Parcel p)
{
////// FAZER - se não tiber buracos é só ver se pertence ao anel, se tiver buraco, não pode estar no buraco, bounding box é useful no ultimo caso
	return false;
}

bool adjacentParcels(Parcel a, Parcel b)
{
	////// FAZER - a porção de fronteira comum aparece repetida nos dois anéis envolvidos, com rigorosamente os mesmos vértices comuns. 
	//Há só duas variações possíveis: esses vértices aparecem pela mesma ordem nos dois anéis envolvidos, ou aparecem por ordens opostas nos dois anéis envolvidos. 
	//Repare que basta existir um vértice em comum, para as duas parcelas serem consideradas adjacentes. 
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
	if( n > MAX_PARCELS )
		error("Demasiadas parcelas no ficheiro");
	for( i = 0 ; i < n ; i++ ) {
		(*cartography)[i] = readParcel(f);
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

// L
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

// C
static void commandListCounties(Cartography cartography, int n)
{
	int i = 0;
	String last;
	int b = 0;
	for(i = 0; i < n; i++){
		if(i == 0){ 
			b++;
			strcpy(last, cartography[i].identification.concelho);
		} else{
			if(strcmp(cartography[i].identification.concelho, last) != 0){
				b++;
				strcpy(last, cartography[i].identification.concelho);
			}
		}
	}
	String counties[b];
	int c = 0;
	for(i = 0; i < n; i++){
		if(i == 0){ 
			strcpy(counties[c], cartography[i].identification.concelho);
			strcpy(last, cartography[i].identification.concelho);
			c++;
		} else{
			if(strcmp(cartography[i].identification.concelho, last) != 0){
				strcpy(counties[c], cartography[i].identification.concelho);
				strcpy(last, cartography[i].identification.concelho);
				c++;
			}
		}
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
	int b = 0;
	for(i = 0; i < n; i++){
		if(i == 0){ 
			b++;
			strcpy(last, cartography[i].identification.distrito);
		} else{
			if(strcmp(cartography[i].identification.distrito, last) != 0){
				b++;
				strcpy(last, cartography[i].identification.distrito);
			}
		}
	}
	String districts[b];
	int c = 0;
	for(i = 0; i < n; i++){
		if(i == 0){ 
			strcpy(districts[c], cartography[i].identification.distrito);
			strcpy(last, cartography[i].identification.distrito);
			c++;
		} else{
			if(strcmp(cartography[i].identification.distrito, last) != 0){
				strcpy(districts[c], cartography[i].identification.distrito);
				strcpy(last, cartography[i].identification.distrito);
				c++;
			}
		}
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
		if(cartography[rBottom].edge.boundingBox.bottomRight.lat > cartography[i].edge.boundingBox.bottomRight.lat){
			rBottom = i;
		}
		if(cartography[rTop].edge.boundingBox.topLeft.lat < cartography[i].edge.boundingBox.topLeft.lat){
			rTop = i;
		}
		if(cartography[rRight].edge.boundingBox.bottomRight.lon < cartography[i].edge.boundingBox.bottomRight.lon){
			rRight = i;
		}
		if(cartography[rLeft].edge.boundingBox.topLeft.lon > cartography[i].edge.boundingBox.topLeft.lon){
			rLeft = i;
		}
		}
	}
	extrems( rTop, cartography[rTop].identification, 3, 'N');
	extrems( rRight, cartography[rRight].identification, 3, 'E');
	extrems( rBottom, cartography[rBottom].identification, 3, 'S');
	extrems( rLeft, cartography[rLeft].identification, 3, 'W');
}

//R
static void commandMetaData(int pos, Cartography cartography, int n){
	if( !checkArgs(pos) || !checkPos(pos, n) )
		return;
	showIdentification(pos, cartography[pos].identification, 3);
	printf("\n     %d ", cartography[pos].edge.nVertexes);
	if(cartography[pos].nHoles > 0){
		int i = cartography[pos].nHoles - 1;
		for(; i >= 0; i--){
			printf("%d ", cartography[pos].holes[i].nVertexes);	
		}
	}
Rectangle r = cartography[pos].edge.boundingBox;
		printf("{%f, %f, %f, %f}\n", r.topLeft.lat, r.topLeft.lon,r.bottomRight.lat, r.bottomRight.lon);
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
			int i = cartography[currentPos].nHoles - 1;
			for(; i >= 0 ; i--){
				if(cartography[currentPos].holes[i].nVertexes > maxVert){
					  maxVert = cartography[currentPos].holes[i].nVertexes;
					  parcelPos = currentPos;
				}
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

			case 'C': case 'c': //concelhos 
				commandListCounties(cartography, n);
				break;
			
			case 'D': case 'd': //distritos
				commandListDistricts(cartography, n);
				break;
			
			case 'X': case 'x': //extremos
				commandExtrems(cartography, n);
				break;

			case 'Z': case 'z':	// terminar
				printf("Fim de execucao! Volte sempre.\n");
				return;

			default:
				printf("Comando desconhecido: \"%s\"\n", commandLine);
		}
	}
}
