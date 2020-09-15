#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <conio.h>

#define MAX_IME_FAJLA 255
#define MAX_KOLONA 50
#define MAX_REDOVA 20
#define MAX_POTEZA 100

#define PLOCA 'o'
#define RUPA '-'
#define CIGLA 'S'
#define KRAJ 'T'

// cvor grafa, sadrzi vrstu i kolonu, kao identifikator cvora(rbr), cvorovi su numerisani od 0 na dalje
typedef struct Cvor {
	int i, j;
	int rbr;
	struct Cvor* sled;
} Cvor;

// graf sadrzi liste susednosti koje su zapravo svi cvorovi grafa, a svaki cvor pokazuje na svoju listu susednosti, n je broj cvorova u grafu
typedef struct Graf {
	Cvor* liste;
	int n;
} Graf;

// igra sadrzi graf, pocetnu poziciju (si, sj), krajnju poziciju (ti, tj), kao i mapu karaktera koja se stampa na konzoli pri svakom potezu, takodje igra sadrzi i broj_redova koji se koriste na mapi
typedef struct Igra {
	Graf* g;
	int si, sj, srbr;
	int ti, tj;
	char mapa[MAX_REDOVA][MAX_KOLONA + 1];
	int broj_redova;
} Igra;

// Red i RedElem se koriste za obilazak grafa po sirini. Red je implementiran u vidu ulancane liste, dok element reda ukazuje na cvor, govori da li je cigla na tom cvoru uspravno postavljena (up), a ako nije govori da li cigla vertikalno lezi sa dva dela(vertical = 1) ili horizontalno (vertical = 0)
// Element reda takodje sadrzi listu poteza kojim se doslo do tog stanja prilikom obilaska grafa
typedef struct RedElem {
	Cvor* cvor;
	int up;
	int vertical;
	char potezi[MAX_POTEZA + 1];
	struct RedElem* sled;
} RedElem;

// Red sadrzi pokazivac na prvi i poslednji element reda, kao i broj elemenata u redu
typedef struct Red {
	RedElem* prvi, *posl;
	int n;
} Red;

// dodaje cvor u liste susednosti svih suseda, a ukoliko je novi_element_liste = 1, to znaci da je potrebno alocirati novu listu susednosti za taj cvor, tacnije da je ova funkcija pozvana iz funkcije dodajPlocu gde lista za novu plocu ne postoji, dok se pri formiranju grafa lista susednosti vec pravi u funkciji formiranjeMape, te je ovaj argument 0 kada se odatle poziva
void dodajCvor(Graf* g, Cvor *c, int novi_element_liste) {
	int i;
	int tekI, tekJ;
	Cvor* kopija, *kopijaZaNovi;
	Cvor* novi;
	if (novi_element_liste) {
		// VODI RACUNA KOD REALLOC MORA sizeof(Cvor) da se pomnozi sa brojem elemenata
		g->liste = (Cvor*) realloc(g->liste, (g->n + 1)* sizeof(Cvor));
		g->liste[g->n].i = c->i;
		g->liste[g->n].j = c->j;
		g->liste[g->n].rbr = c->rbr;
		g->liste[g->n].sled = NULL;

		g->n = g->n + 1;
	}

	for (i = 0; i < g->n; i++) {
		tekI = g->liste[i].i;
		tekJ = g->liste[i].j;

		if (c->i == tekI && c->j == tekJ) continue;

		// DA LI MU JE SUSED
		if ((c->i == tekI && abs(c->j - tekJ) <= 1) || (c->j == tekJ && abs(c->i - tekI) <= 1)) {
			kopija = (Cvor*)malloc(sizeof(Cvor));
			kopija->i = c->i;
			kopija->j = c->j;
			kopija->rbr = c->rbr;
			
			// DODAVANJE NA POCETAK LISTE SUSEDNOSTI
			kopija->sled = g->liste[i].sled;
			g->liste[i].sled = kopija;

			// DODAVANJE NA POCETAK LISTE SUSEDNOSTI NOVOG CVORA
			if (novi_element_liste) {
				kopijaZaNovi = (Cvor*)malloc(sizeof(Cvor));
				kopijaZaNovi->i = tekI;
				kopijaZaNovi->j = tekJ;
				kopijaZaNovi->rbr = g->liste[i].rbr;

				kopijaZaNovi->sled = g->liste[g->n - 1].sled;
				g->liste[g->n - 1].sled = kopijaZaNovi;
			}
		}
	}
}

int formiranjeMape(Igra** igra) {
	char putanja[MAX_IME_FAJLA];
	FILE* fajl;
	char linija[MAX_KOLONA];
	int i, j;
	int broj_reda;
	int broj_cvorova = 0;
	int postoji_s = 0, postoji_t = 0;
	int si, sj;
	int ti, tj;
	int validnaMapa = 0;
	Cvor* listaCvorova, *posl, *novi, *zaBrisanje, *tekuci, * pocetni;

	system("cls");
	printf("Unesite putanju do fajla sa mapom: ");
	getchar();
	gets(putanja);

	fajl = fopen(putanja, "r");

	if (fajl == NULL) {
		printf("Fajl ne postoji.");
		_sleep(2000);
		return 0;
	} 

	listaCvorova = NULL;
	posl = NULL;
	
	broj_reda = 0;
	validnaMapa = 1;
	
	(*igra) = (Igra*)malloc(sizeof(Igra));

	for (i = 0; i < MAX_REDOVA; i++) {
		for (j = 0; j < MAX_KOLONA - 1; j++) {
			(*igra)->mapa[i][j] = RUPA;
		}

		(*igra)->mapa[i][MAX_KOLONA - 1] = '\0';
	}

	while (fgets(linija, MAX_KOLONA, fajl) != NULL) {
		if (linija[strlen(linija) - 1] == '\n') {
			linija[strlen(linija) - 1] = '\0'; // BEZ NEWLINE
		}
		strcpy((*igra)->mapa[broj_reda], linija);
		for (i = 0; i < strlen(linija); i++) {
			switch (linija[i]) {
			case 'o':
				novi = (Cvor*) malloc(sizeof(Cvor));

				novi->i = broj_reda;
				novi->j = i;
				novi->rbr = broj_cvorova;
				novi->sled = NULL;

				if (broj_cvorova == 0) {
					listaCvorova = novi;
				}
				else {
					posl->sled = novi;
				}

				posl = novi;
				broj_cvorova = broj_cvorova + 1;
				break;
			case 'S':
				if (postoji_s) {
					validnaMapa = 0;
				}
				else {
					postoji_s = 1;
					si = broj_reda;
					sj = i;
				}
				break;
			case 'T':
				if (postoji_t) {
					validnaMapa = 0;
				}
				else {
					postoji_t = 1;
					ti = broj_reda;
					tj = i;
				}
				break;
			}

			if (!validnaMapa) {
				break;
			}
		}

		if (!validnaMapa) {
			break;
		}
		broj_reda++;
	}

	fclose(fajl);

	if (!validnaMapa) {
		while (listaCvorova != NULL) {
			zaBrisanje = listaCvorova;
			listaCvorova = listaCvorova->sled;
			free(zaBrisanje);
		}

		free(igra);

		return 0;
	}
	else {
		
		(*igra)->g = (Graf*)malloc(sizeof(Graf));
		(*igra)->si = si;
		(*igra)->sj = sj;
		(*igra)->ti = ti;
		(*igra)->tj = tj;
		(*igra)->broj_redova = broj_reda;

		// +1 zbog pocetnog mesta
		(*igra)->g->liste = (Cvor*)calloc(broj_cvorova + 1, sizeof(Cvor));
		(*igra)->g->n = broj_cvorova + 1;

		tekuci = listaCvorova;
		i = 0;

		// UBACIVANJE ZAGLAVLJA LISTA, ODNOSNO TOG CVORA ZA KOJI SE LISTA PRAVI
		while (tekuci != NULL) {
			(*igra)->g->liste[i].i = tekuci->i;
			(*igra)->g->liste[i].j = tekuci->j;
			(*igra)->g->liste[i].rbr = tekuci->rbr;
			(*igra)->g->liste[i].sled = NULL;
				
			tekuci = tekuci->sled;
			i++;
		}

		(*igra)->g->liste[(*igra)->g->n - 1].i = si;
		(*igra)->g->liste[(*igra)->g->n - 1].j = sj;
		(*igra)->g->liste[(*igra)->g->n - 1].rbr = (*igra)->g->n - 1;
		(*igra)->g->liste[(*igra)->g->n - 1].sled = NULL;
		(*igra)->srbr = (*igra)->g->n - 1;

		// PRVO SMO SE CVOROVE DODALI U LISTU CVOROVA, A SAD IH UBACUJEMO U GRAF
		while (listaCvorova != NULL) {
			zaBrisanje = listaCvorova;
			listaCvorova = listaCvorova->sled;

			dodajCvor((*igra)->g, zaBrisanje, 0);

			free(zaBrisanje);
		}

		pocetni = (Cvor*)malloc(sizeof(Cvor));
		pocetni->i = si;
		pocetni->j = sj;
		pocetni->rbr = (*igra)->g->n - 1;

		// DODAVANJE PLOCE NA KOJOJ SE NALAZI CIGLA NA POCETKU
		dodajCvor((*igra)->g, pocetni, 0);

		free(pocetni);

		return 1;
	}

	
}

void dodajPlocu(Igra* igra) {
	int i;
	int pi, pj;
	int ima_suseda = 0;
	Graf* graf = igra->g;
	Cvor* cvor;
	printf("Unesite x koordinatu ploce: ");
	scanf("%d", &pi);
	printf("Unesite y koordinatu ploce: ");
	scanf("%d", &pj);

	if (pi < 0 || pj < 0 || pi >= MAX_REDOVA || pj >= MAX_KOLONA ) {
		system("cls");
		printf("Nevalidne koordinate");
		_sleep(2000);
		return;
	}

	if ((pi == igra->si && pj == igra->sj) || ( pi == igra->ti && pj == igra->tj )) {
		system("cls");
		printf("Na unesenim koordinata vec postoji cigla ili supljina za kraj igre.");
		_sleep(2000);
		return;
	}

	for (i = 0; i < graf->n; i++) {
		// DA LI POSTOJI TA PLOCA
		if (graf->liste[i].i == pi && graf->liste[i].j == pj) {
			system("cls");
			printf("Na unesenim koordinata vec postoji ploca.");
			_sleep(2000);
			return;
		}
		// DA LI JE SUSED
		else if ((graf->liste[i].i == pi && abs(graf->liste[i].j - pj) <= 1) || (graf->liste[i].j == pj && abs(graf->liste[i].i - pi) <= 1)) {
			ima_suseda = 1;
		}
	}

	if (!ima_suseda) {
		system("cls");
		printf("Ploca mora imati barem jednu susednu plocu");
		_sleep(2000);
		return;
	}

	cvor = (Cvor*)malloc(sizeof(Cvor));
	cvor->i = pi;
	cvor->j = pj;
	cvor->rbr = graf->n - 1;
	dodajCvor(graf, cvor, 1);

	if (cvor->j >= strlen(igra->mapa[cvor->i])) {
		for (i = strlen(igra->mapa[cvor->i]); i < cvor->j; i++) {
			igra->mapa[cvor->i][i] = RUPA;
		}
		igra->mapa[cvor->i][cvor->j + 1] = '\0';
	}

	igra->mapa[cvor->i][cvor->j] = PLOCA;
	if (cvor->i >= igra->broj_redova) {
		// NOVI RED IMA ISTU DUZINU KAO PRETPOSLEDNJI
		igra->mapa[igra->broj_redova][strlen(igra->mapa[igra->broj_redova - 1])] = '\0';
		igra->broj_redova += 1;
	}
}

void obrisiCvor(Graf* g, Cvor* c) {
	Cvor* prev, *tekuci, * zaBrisanje;
	int i;
	int tekI, tekJ;
	int detektovan_za_brisanje = 0; // OVA PROMENLJIVA GOVORI DA SMO PROSLI LISTU SUSEDNOSTI ZA CVOR KOJI SE BRISE I ONDA OSTALE LISTE MORAMO POMERITI ZA JEDNO MESTO UNAZAD KAKO IH REALLOC NE BI OBRISAO
	for (i = 0; i < g->n; i++) {
		tekI = g->liste[i].i;
		tekJ = g->liste[i].j;


		if (tekI == c->i && tekJ == c->j) {
			detektovan_za_brisanje = 1;

			tekuci = g->liste[i].sled;

			while (tekuci != NULL) {
				zaBrisanje = tekuci;
				tekuci = tekuci->sled;

				free(zaBrisanje);
			}
		}
		else {
			prev = &g->liste[i];
			tekuci = g->liste[i].sled;

			while (tekuci != NULL) {
				if (tekuci->i == c->i && tekuci->j == c->j) {
					prev->sled = tekuci->sled;
					free(tekuci);
					break;
				}

				prev = tekuci;
				tekuci = tekuci->sled;
			}

			// AKO JE CVOR ZA BRISANJE VEC DETEKTOVAN POMERI LISTU ZA 1 ELEMENT NA GORE
			if (detektovan_za_brisanje) {
				g->liste[i - 1].i = tekI;
				g->liste[i - 1].j = tekJ;

				g->liste[i - 1].sled = g->liste[i].sled;
			}
		}

		
	}

	g->liste = realloc(g->liste, (g->n - 1) * sizeof(Cvor));
	g->n = g->n - 1;
}

void obrisiPlocu(Igra* igra) {
	int i, j;
	int pi, pj;
	int postoji_ploca = 0;
	int broj_suseda;
	Graf* graf = igra->g;
	Cvor* cvor, * tekuci;
	int ima_ploce_u_poslednjem_redu = 0;

	if (graf->n <= 9) {
		system("cls");
		printf("Ne moze biti manje od 9 ploca");
		_sleep(2000);
		return;
	}
	printf("Unesite x koordinatu ploce: ");
	scanf("%d", &pi);
	printf("Unesite y koordinatu ploce: ");
	scanf("%d", &pj);

	if (pi < 0 || pj < 0 || pi >= MAX_REDOVA || pj >= MAX_KOLONA) {
		system("cls");
		printf("Nevalidne koordinate");
		_sleep(2000);
		return;
	}

	if ((pi == igra->si && pj == igra->sj) || (pi == igra->ti && pj == igra->tj)) {
		system("cls");
		printf("Na unesenim koordinata vec postoji cigla ili supljina za kraj igre.");
		_sleep(2000);
		return;
	}

	if ((igra->ti == pi && abs(igra->tj - pj) <= 1) || (igra->tj == pj && abs(igra->ti - pi) <= 1)) {
		system("cls");
		printf("Ne moze se ukloniti susedna ploca u odnosu na supljinu za kraj igre.");
		_sleep(2000);
		return;
	}

	for (i = 0; i < graf->n; i++) {
		// DA LI POSTOJI TA PLOCA
		if (graf->liste[i].i == pi && graf->liste[i].j == pj) {
			cvor = &graf->liste[i];

			postoji_ploca = 1;

			broj_suseda = 0;

			tekuci = graf->liste[i].sled;

			while (tekuci != NULL) {
				broj_suseda++;
				tekuci = tekuci->sled;
			}

			// DA LI JE PLOCA NA IVICI
			if (broj_suseda == 4) {
				system("cls");
				printf("Ploca nije na ivici mape, te se ne moze izbrisati");
				return;
			}

			break;
		}

	}

	if (!postoji_ploca) {
		system("cls");
		printf("Ne postoji ploca na unesenoj lokaciji.");
		_sleep(2000);
		return;
	}

	igra->mapa[cvor->i][cvor->j] = RUPA;

	for (j = 0; j < strlen(igra->mapa[cvor->i]); j++) {
		if (igra->mapa[cvor->i][j] == PLOCA) {
			ima_ploce_u_poslednjem_redu = 1;
			break;
		}
	}

	if (!ima_ploce_u_poslednjem_redu) {
		igra->broj_redova -= 1;
	}

	obrisiCvor(graf, cvor);
	
}

void ispisiMapu(Igra* igra) {
	int i, j;
	for ( i = 0; i < igra->broj_redova; i++) {
		for (j = 0; j < strlen(igra->mapa[i]); j++) {
			printf("%c", igra->mapa[i][j]);
		}

		printf("\n");
	}
	
}

void sacuvajMapu(Igra* igra) {
	char putanja[MAX_IME_FAJLA];
	FILE* fajl;
	int i;

	system("cls");
	printf("Unesite putanju fajla za cuvanje mape: ");
	getchar();
	gets(putanja);

	fajl = fopen(putanja, "w");

	if (fajl == NULL) {
		printf("Greska pri otvaranju fajla za upis.");
		_sleep(2000);
		return;
	}

	for (i = 0; i < igra->broj_redova; i++) {
		fputs(igra->mapa[i], fajl);

		if (i != igra->broj_redova - 1) {
			fputchar("\n", fajl);
		}
	}

	fclose(fajl);
}

void obrisiGraf(Graf* g) {
	int i;
	Cvor* tekuci, *zaBrisanje;

	for (i = 0; i < g->n; i++) {
		tekuci = g->liste[i].sled;

		while (tekuci != NULL) {
			zaBrisanje = tekuci;
			tekuci = tekuci->sled;

			free(zaBrisanje);
		}
	}

	free(g->liste);
}

void obrisiMapu(Igra* igra) {
	obrisiGraf(igra->g);

	free(igra->g);

	free(igra);
}

// FUNKCIJA KOJA ODREDJUJE DA LI JE CVOR SA IDENTIFIKATOR RBR SUSED CVORU NA POZICIJI i,j
// RBR CE IMATI VREDNOST -1 KADA SE RADI O SUPLJINI ZA KRAJ IGRE, JER CIGLA MOZE LEZATI JEDNOM POLOVINOM NA TOJ SUPLJINI, DOK TO NIJE SLUCAJ KADA SE CIGLA NALAZI NA IVICI MAPE, JER CE S NJE PASTI
Cvor* proveriSusedstvo(Graf* g, int rbr, int i, int j) {
	Cvor* tekuci;
	int z;

	// SLUCAJ KADA JE CIGLA NALEGLA NA RUPU, TADA SE MORATE PROVERITI DA LI POSTOJI PLOCA U GRAFU
	if (rbr == -1) {
		for (z = 0; z < g->n; z++) {
			if (g->liste[z].i == i && g->liste[z].j == j) {
				return &g->liste[z];
			}
		}

		return NULL;
	}

	// UKOLIKO SE RADI O REGULARNOJ PLOCI
	tekuci = g->liste[rbr].sled;

	while (tekuci != NULL) {
		if (tekuci->i == i && tekuci->j == j) {
			return tekuci;
		}

		tekuci = tekuci->sled;
	}

	return NULL;
}

/* 
CIGLA MOZE BITI U SLEDECIM POLOZAJIMA:

USPRAVNO (UP = 1, vrednost VERTICAL nije bitna)
(x1,y1)
-----------------------------------------------------
LEZI VERTIKALNO (UP = 0, VERTICAL = 1)
(x1,y1) 
(x2,y2)
-----------------------------------------------------
LEZI HORIZONTALNO (UP = 0, VERTICAL = 0)
(x1,y1) (x2, y2)

*/
void pokreniIgru(Igra* igra) {
	// VERTICAL SE KORISTI SAMO KAD JE UP = 0, DA ODREDI DA LI CIGLA LEZI VERTIKALNO ILI HORIZONTALNO
	int x1, y1, x2, y2, rbr1, rbr2, vertical, up, lastVertical, lastUp; // x1,y1 su koordinate prve polovine cigle, a x2, y2 druge polovine, dok je rbr1 identifikator ploce na kojoj lezi prva polovina cigle, a rbr2 identifikator druge polovine. Ukoliko cigla stoji uspravno aktivne ce biti samo x1, y1, i koristice se samo rbr1
	int nx1, ny1, nx2, ny2; // nx1, ny1 je nova pozicija prve polovine cigle nakon poteza,  a nx2, ny2 druge polovine ako cigla lezi na mapi
	int kraj = 0, poraz;
	char keyboardC;
	int promena_mape = 0;
	Cvor* sused;
	int i, j;
	char kopijaMape[MAX_REDOVA][MAX_KOLONA + 1];

	for (i = 0; i < MAX_REDOVA; i++) {
		for (j = 0; j < MAX_KOLONA + 1; j++) {
			kopijaMape[i][j] = igra->mapa[i][j];
		}
	}
	

	up = 1;
	x1 = igra->si;
	y1 = igra->sj;
	rbr1 = igra->srbr;
	lastUp = up;

	system("cls");
	ispisiMapu(igra);

	while (!kraj) {
		
		if (_kbhit()) {
			promena_mape = 0;

			keyboardC = getch();

			switch (keyboardC)
			{
			// GORE
			case 'w':
				promena_mape = 1;
				if (up) {
					nx1 = x1 - 2;
					ny1 = y1;
					nx2 = x1 - 1;
					ny2 = y1;

					sused = proveriSusedstvo(igra->g, rbr1, nx2, ny2);

					rbr2 = sused != NULL ?  sused->rbr : -1 ;


					if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, sused->rbr, nx1, ny1);

						rbr1 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}

					}		

					up = 0;
					vertical = 1;
				}
				else if (vertical) {
					nx1 = x1 - 1;
					ny1 = y1;

					if (nx1 == igra->ti && ny1 == igra->tj) {
						kraj = 1;
						poraz = 0;
					}
					else {

						sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

						rbr1 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}	

					}

					up = 1;
					
				}
				else {
					nx1 = x1 - 1;
					ny1 = y1;
					nx2 = x1 - 1;
					ny2 = y2;

					sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

					rbr1 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr2, nx2, ny2);

						rbr2 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
						
					}
				}
				break;
			// LEVO
			case 'a':
				promena_mape = 1;
				if (up) {
					nx1 = x1;
					ny1 = y1 - 2;
					nx2 = x1;
					ny2 = y1 - 1;

					sused = proveriSusedstvo(igra->g, rbr1, nx2, ny2);
					
					rbr2 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {		
						sused = proveriSusedstvo(igra->g, rbr2, nx1, ny1);

						rbr1 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
						
					}

					up = 0;
					vertical = 0;
				}
				else if (vertical) {
					nx1 = x1;
					ny1 = y1 - 1;
					nx2 = x2;
					ny2 = y1 - 1;

					sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

					rbr1 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr2, nx2, ny2);

						rbr2 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
					}
				}
				else {
					nx1 = x1;
					ny1 = y1 - 1;

					if (nx1 == igra->ti && ny1 == igra->tj) {
						kraj = 1;
						poraz = 0;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

						rbr1 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
					}

					up = 1;
				}
				break;
			// DOLE
			case 's':
				promena_mape = 1;
				if (up) {
					nx1 = x1 + 1;
					ny1 = y1;
					nx2 = x1 + 2;
					ny2 = y1;

					sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

					rbr1 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr1, nx2, ny2);
						rbr2 = sused != NULL ? sused->rbr : -1;
						if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}

					}


					up = 0;
					vertical = 1;
				}
				else if (vertical) {
					nx1 = x2 + 1;
					ny1 = y1;

					if (nx1 == igra->ti && ny1 == igra->tj) {
						kraj = 1;
						poraz = 0;
					}
					else {

						sused = proveriSusedstvo(igra->g, rbr2, nx1, ny1);

						rbr1 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}


					}

					up = 1;
				}
				else {
					nx1 = x1 + 1;
					ny1 = y1;
					nx2 = x1 + 1;
					ny2 = y2;

					sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

					rbr1 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr2, nx2, ny2);

						rbr2 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
					}
				}
				break;
			// DESNO
			case 'd':
				promena_mape = 1;
				if (up) {
					nx1 = x1;
					ny1 = y1 + 1;
					nx2 = x1;
					ny2 = y1 + 2;

					sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

					rbr1 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr1, nx2, ny2);
						rbr2 = sused != NULL ? sused->rbr : -1;
						if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
					}

					up = 0;
					vertical = 0;
				}
				else if (vertical) {
					nx1 = x1;
					ny1 = y1 + 1;
					nx2 = x2;
					ny2 = y1 + 1;

					sused = proveriSusedstvo(igra->g, rbr1, nx1, ny1);

					rbr1 = sused != NULL ? sused->rbr : -1;

					if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
						kraj = 1;
						poraz = 1;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr2, nx2, ny2);

						rbr2 = sused != NULL ? sused->rbr : -1;

						if (sused == NULL && (nx2 != igra->ti || ny2 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
					}
				}
				else {
					nx1 = x1;
					ny1 = y2 + 1;

					if (nx1 == igra->ti && ny1 == igra->tj) {
						kraj = 1;
						poraz = 0;
					}
					else {
						sused = proveriSusedstvo(igra->g, rbr2, nx1, ny1);
						rbr1 = sused != NULL ? sused->rbr : -1;
						if (sused == NULL && (nx1 != igra->ti || ny1 != igra->tj)) {
							kraj = 1;
							poraz = 1;
						}
					}

					up = 1;
				}
				break;
			default:
				break;
			}

			if (promena_mape) {
			

				if (lastUp) {
					if (x1 != igra->ti || y1 != igra->tj) {
						igra->mapa[x1][y1] = PLOCA;
					}
					else {
						igra->mapa[x1][y1] = KRAJ;
					}
				}
				else {
					if (x1 != igra->ti || y1 != igra->tj) {
						igra->mapa[x1][y1] = PLOCA;
					}
					else {
						igra->mapa[x1][y1] = KRAJ;
					}

					if (x2 != igra->ti || y2 != igra->tj) {
						igra->mapa[x2][y2] = PLOCA;
					}
					else {
						igra->mapa[x2][y2] = KRAJ;
					}
				}

				x1 = nx1;
				y1 = ny1;

				if (!up) {
					x2 = nx2;
					y2 = ny2;
				}

				igra->mapa[x1][y1] = CIGLA;

				if(!up) {
					igra->mapa[x2][y2] = CIGLA;
				}

				lastUp = up;
				lastVertical = vertical;

				system("cls");
				ispisiMapu(igra);

				if (kraj) {
					if (poraz) {
						printf("\nCigla je ispala sa mape. Izgubili ste.");
					}
					else {
						printf("\nCestitamo. Pobedili ste.");
					}

					_sleep(4000);
				}
			}
		}
	}

	// RESTAURIRANJE ORIGINALNE MAPE JER SE VRACAMO U GLAVNI MENI
	for (i = 0; i < MAX_REDOVA; i++) {
		for (j = 0; j < MAX_KOLONA + 1; j++) {
			igra->mapa[i][j] = kopijaMape[i][j];
		}
	}
}

void ubaciURed(Red* red, RedElem* elem) {
	if (red->n == 0) {
		red->prvi = elem;	
	}
	else {
		red->posl->sled = elem;
	}

	red->posl = elem;

	red->n += 1;
}

RedElem* uzmiIzReda(Red* red) {
	RedElem* re;
	if (red->n == 0) return NULL;
	
	re = red->prvi;

	red->prvi = red->prvi->sled;

	if (red->prvi == NULL) {
		red->posl = NULL;
	}

	red->n -= 1;

	return re;
}

/*
CIGLA MOZE BITI U SLEDECIM POLOZAJIMA:

USPRAVNO (UP = 1, vrednost VERTICAL nije bitna)
(x1,y1)
-----------------------------------------------------
LEZI VERTIKALNO (UP = 0, VERTICAL = 1)
(x1,y1)
(x2,y2)
-----------------------------------------------------
LEZI HORIZONTALNO (UP = 0, VERTICAL = 0)
(x1,y1) (x2, y2)

*/
void resiIgru(Igra* igra) {
	Red red;
	Graf* g = igra->g;
	RedElem* zaUbaciti;
	RedElem* tekuci;
	Cvor* sused, *susedSuseda, *susedSupljina;
	Cvor* susediT, *poslSusediT, *noviT,*zaBrisanje;
	Cvor* dodatniProstor = NULL;
	int i, iProvera, jProvera;
	int visitedRbr;
	int* visited = (int*)calloc(g->n * 3 + 3, sizeof(int));
	int nadjen_put = 0;
	char resenje[MAX_POTEZA + 1];
	char potez;
	char potezi[MAX_POTEZA + 1];
	int tekI, tekJ;


	red.prvi = NULL;
	red.posl = NULL;
	red.n = 0;
	
	for (i = 0; i < g->n * 3; i++) {
		visited[i] = 0;
	}

	// zaUbaciti je pokazivac na element reda koji se treba u datom koraku obrade ubaciti u red, na pocetku je to pocetna ploca
	zaUbaciti = (RedElem*)malloc(sizeof(RedElem));

	zaUbaciti->cvor = &g->liste[igra->srbr];
	zaUbaciti->up = 1;
	zaUbaciti->sled = NULL;

	for (i = 0; i < MAX_POTEZA + 1; i++) {
		zaUbaciti->potezi[i] = '\0';
	}

	ubaciURed(&red, zaUbaciti);

	// visited niz ima 3 puta vise elemenata nego sto ima cvorova grafa + jos 3 elementa za supljinu za kraj igre. Prilikom obilaska grafa svaki cvor mozemo obici tako sto je cigla
	// na njemu uspravna, zatim sto cigla proteze na desni plocu i na donju plocu. Otuda 3 stanja za svaku ciglu i zato visited niz ima vise elemenata.
	visited[zaUbaciti->cvor->rbr] = 1;

	// SUSED SUPLJINA JE CVOR KOJI SE KORISTI KADA JE NEKO MCVORU SUSEDNA SUPLJINA ZA KRAJ IGRE, OVO JE POTREBNO JER SE OVAJ CVOR NE NALAZI U GRAFU
	susedSupljina = (Cvor*) malloc(sizeof(Cvor));
	susedSupljina->rbr = -1;
	susedSupljina->i = igra->ti;
	susedSupljina->j = igra->tj;

	// SVE DOK NE NADJEMO PUT I DOK RED NIJE PRAZAN
	while (!nadjen_put && red.n > 0) {
		
		tekuci = uzmiIzReda(&red);

		susediT = NULL;
		poslSusediT = NULL;

		// UKOLIKO TEKUCI CVOR NIJE SUPLJINA ZA KRAJ IGRE, sa tekuci->cvor = NULL OZNACAVAMO DA SE RADI O SUPLJEM CVORU ZA KRAJ IGRE
		if (tekuci->cvor != NULL) {
			sused = g->liste[tekuci->cvor->rbr].sled;
			
			// AKO JE SUSED SUPLJINA ZA KRAJ, MORAMO JE DODATI NA POCETAK LISTE SUSEDA
			if ((igra->ti == tekuci->cvor->i && abs(igra->tj - tekuci->cvor->j) <= 1) || (igra->tj == tekuci->cvor->j && abs(igra->ti - tekuci->cvor->i) <= 1)) {
				susedSupljina->sled = sused;
				sused = susedSupljina;
			}

		}
		else {
			// FORMIRANJE LISTE SUSEDA ZA SUPLJINU, MORAMO NACI KOJI SU SVE CVOROVI SUSEDNI SUPLJINI I DODATI IH U LISTU SUSEDA
			for (i = 0; i < g->n; i++) {
				if ( (g->liste[i].i == igra->ti && (abs(g->liste[i].j - igra->tj) <= 1)) || (g->liste[i].j == igra->tj && (abs(g->liste[i].i - igra->ti) <= 1)) ) {
					noviT = (Cvor*)malloc(sizeof(Cvor));
					noviT->i = g->liste[i].i;
					noviT->j = g->liste[i].j;
					noviT->rbr = g->liste[i].rbr;
					noviT->sled = NULL;
					if (susediT == NULL) {
						susediT = noviT;				
					}
					else {
						poslSusediT->sled = noviT;
					}

					poslSusediT = noviT;
				}
			}

			sused = susediT;
		}

		// OBILAZAK LISTE SUSEDNOSTI
		while (sused != NULL) {
			zaUbaciti = NULL;

			// AKO SE RADI O SUPLJINI ZA KRAJ IGRE
			if (tekuci->cvor == NULL) {
				tekI = igra->ti;
				tekJ = igra->tj;
			}
			// AKO SE RADI O REGULARNOJ PLOCI
			else {
				tekI = tekuci->cvor->i;
				tekJ = tekuci->cvor->j;
			}

			//DESNO
			if (sused->i == tekI && sused->j == tekJ + 1) {
				potez = 'd';
				// CIGLA STOJI USPRAVNO I POKUSAVAMO U DESNO DA JE POMERIMO
				if (tekuci->up) {
					iProvera = sused->i;
					jProvera = sused->j + 1;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					// AKO POSTOJI PLOCA PORED SUSEDA ILI AKO SE RADI O SUPLJINI ZA KRAJ IGRE
					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));

						// OVO ZNACI DA JE SUSED IZ LISTE SUSEDNOSTI SUPLJINA ZA KRAJ IGRE
						if (sused->rbr != -1) {
							zaUbaciti->cvor = sused;
						}
						else {
							zaUbaciti->cvor = NULL;
						}

						zaUbaciti->up = 0;
						zaUbaciti->vertical = 0;
					}
					
				}
				// CIGLA LEZI VERTIKALNO I POKUSAVAMO U DESNO DA JE POMERIMO
				else if (tekuci->vertical) {
					iProvera = sused->i + 1;
					jProvera = sused->j;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (sused->rbr != -1) {
							zaUbaciti->cvor = sused;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 1;
					}
				}
				// CIGLA LEZI HORIZONTALNO I POKUSAVAMO U DESNO DA JE POMERIMO
				else {
					iProvera = sused->i;
					jProvera = sused->j + 1;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (susedSuseda != NULL) {
							zaUbaciti->cvor = susedSuseda;
						}
						else {
							nadjen_put = 1;
						}
						zaUbaciti->up = 1;
					}
				}
			}
			// LEVO
			else if (sused->i == tekI && sused->j == tekJ - 1) {
				potez = 'a';
				// CIGLA STOJI USPRAVNO I POKUSAVAMO U LEVO DA JE POMERIMO
				if (tekuci->up) {
					iProvera = sused->i;
					jProvera = sused->j - 1;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						// ISPITUJEMO DA LI JE SUSED SUSEDA SUPLJINA ZA KRAJ IGRE
						if (susedSuseda != NULL) {
							zaUbaciti->cvor = susedSuseda;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 0;
					}
					
					
				}
				// CIGLA STOJI LEZI VERTIKALNO I POKUSAVAMO U LEVO DA JE POMERIMO
				else if (tekuci->vertical) {
					iProvera = sused->i + 1;
					jProvera = sused->j;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (sused->rbr != -1) {
							zaUbaciti->cvor = sused;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 1;
					}
				}
				// CIGLA STOJI LEZI HORIZONTALNO I POKUSAVAMO U LEVO DA JE POMERIMO
				else {
					zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
					if (sused->i == igra->ti && sused->j == igra->tj) {
						nadjen_put = 1;
					}
					else {
						zaUbaciti->cvor = sused;
					}
					zaUbaciti->up = 1;			
				}
			}
			// DOLE
			else if (sused->j == tekJ && sused->i == tekI + 1) {
				potez = 's';
				// CIGLA STOJI USPRAVNO I POKUSAVAMO NA DOLE DA JE POMERIMO
				if (tekuci->up) {
					iProvera = sused->i + 1;
					jProvera = sused->j;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (sused->rbr != -1) {
							zaUbaciti->cvor = sused;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 1;
					}
				}
				// CIGLA LEZI VERTIKALNO I POKUSAVAMO NA DOLE DA JE POMERIMO
				else if (tekuci->vertical) {
					iProvera = sused->i + 1;
					jProvera = sused->j;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (susedSuseda->rbr != -1) {
							zaUbaciti->cvor = susedSuseda;
						}
						else {
							nadjen_put = 1;
						}
						zaUbaciti->up = 1;
					}
				}
				// CIGLA LEZI HORIZONTALNO I POKUSAVAMO NA DOLE DA JE POMERIMO
				else {
					iProvera = sused->i;
					jProvera = sused->j + 1;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (sused->rbr != -1) {
							zaUbaciti->cvor = sused;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 0;
					}
				}
			}
			// GORE
			else {
				potez = 'w';
				// CIGLA STOJI USPRAVNO I POKUSAVAMO NA GORE DA JE POMERIMO
				if (tekuci->up) {
					iProvera = sused->i - 1;
					jProvera = sused->j;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (susedSuseda != NULL) {
							zaUbaciti->cvor = susedSuseda;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 1;
					}
				}
				// CIGLA LEZI VERTIKALNO I POKUSAVAMO NA GORE DA JE POMERIMO
				else if (tekuci->vertical) {
					zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
					if (sused->i == igra->ti && sused->j == igra->tj) {
						nadjen_put = 1;
					}
					else {
						zaUbaciti->cvor = sused;
					}
					zaUbaciti->up = 1;		
				}
				// CIGLA LEZI HORIZONTALNO I POKUSAVAMO NA GORE DA JE POMERIMO
				else {
					iProvera = sused->i;
					jProvera = sused->j + 1;

					susedSuseda = proveriSusedstvo(g, sused->rbr, iProvera, jProvera);

					if (susedSuseda != NULL || (iProvera == igra->ti && jProvera == igra->tj)) {
						zaUbaciti = (RedElem*)malloc(sizeof(RedElem));
						if (sused->rbr != -1) {
							zaUbaciti->cvor = sused;
						}
						else {
							zaUbaciti->cvor = NULL;
						}
						zaUbaciti->up = 0;
						zaUbaciti->vertical = 0;
					}
				}
			}

			
			// AKO JE PRONADJEN PUT SACUVAJ RESENJE I NE UBACUJ DALJE U RED
			if (nadjen_put) {
				for (i = 0; i < strlen(tekuci->potezi); i++) {
					resenje[i] = tekuci->potezi[i];
				}
				resenje[i] = potez;
				resenje[i + 1] = '\0';

				if (zaUbaciti != NULL) {
					free(zaUbaciti);
				}
				break;
			}

			if (zaUbaciti != NULL) {
				

				// ZA SUPLJINU
				if (zaUbaciti->cvor == NULL) {
					visitedRbr = g->n * 3;
				}
				else {
					visitedRbr = zaUbaciti->cvor->rbr * 3;
				}

				if (!zaUbaciti->up) {
					if(zaUbaciti->vertical){
						visitedRbr += 1;
					}
					else {
						visitedRbr += 2;
					}
				}

				// AKO NISMO OBISLI STANJE
				if (!visited[visitedRbr]) {
					for (i = 0; i < MAX_POTEZA + 1; i++) {
						zaUbaciti->potezi[i] = '\0';
					}

					for (i = 0; i < strlen(tekuci->potezi); i++) {
						zaUbaciti->potezi[i] = tekuci->potezi[i];
					}

					zaUbaciti->potezi[i] = potez;
					zaUbaciti->sled = NULL;
					ubaciURed(&red, zaUbaciti);
					visited[visitedRbr] = 1;
				}
				else {
					free(zaUbaciti);
				}

			}




			sused = sused->sled;
		}

		// ZBOG DEALOKACIJE
		if (tekuci->cvor == NULL) {

			if (dodatniProstor != NULL) {
				poslSusediT->sled = dodatniProstor;
			}

			dodatniProstor = susediT;
		}

		free(tekuci);	
	}

	// DEALOKACIJA DODATNOG CVORA ZA SUPLJINU ZA KRAJ IGRE
	if (susedSupljina != NULL) {
		free(susedSupljina);
	}

	// DEALOKACIJA DODATNOG PROSTORA ZA SUSEDE SUPLJINE ZA KRAJ IGRE
	while (dodatniProstor != NULL) {
		zaBrisanje = dodatniProstor;
		dodatniProstor = dodatniProstor->sled;

		free(zaBrisanje);
	}

	// IZBACI SVE PREOSTALO U REDU
	while (red.n > 0) {
		free(uzmiIzReda(&red));
	}
	if (nadjen_put) {
		printf("Sekvenca poteza: %s", resenje);
	}
	else {
		printf("Sekvenca ne postoji.");
	}

	_sleep(5000);
}

void main() {
	// Odabirom opcije 0 iz menija program se zavrsava i kraj dobija vrednost 1, program se radi u do-while petlji sve dok je vrednost promenljive kraj 0
	int kraj = 0;
	// Izabrana opcija iz menija
	int opcija;
	// Ukoliko se izabere neka opcija iz menija koja nije ponudjena, korektanUnos promenljiva ce imati vrednost 0 i meni ce se ponovo ispisati sve dok korisnik korektno ne odabere opciju
	int korektanUnos;
	// Promenljive formiranoStablo i unetiRezultati su logickog tipa i sluze da se zabrane ostale akcije dok ove dve promenljive nemaju vrednost 1, tj. dok se ne formira stablo i unesu rezultati (opcije 1 i 2)
	int formiranGraf = 0;
	// Igra koja sadrzi graf i jos dodatnih stvari
	Igra* igra = NULL;
	
	do {
		// brisanje konzole
		system("cls");

		if (formiranGraf) {
			printf("==============================MAPA=============================\n\n");
			ispisiMapu(igra);
			printf("\n\n");
		}

		printf("==============================MENI=============================\n\n");

		printf("1) Formiranje mape\n");
		printf("2) Dodavanje ploce\n");
		printf("3) Brisanje ploce\n");
		printf("4) Cuvanje mape\n");
		printf("5) Igraj\n");
		printf("6) Brisanje mape\n\n");
		printf("7) Resi igru\n\n");
		printf("0) Prekid programa\n");

		do {
			korektanUnos = 1;

			printf("\nUnesite opciju: ");
			scanf("%d", &opcija);

			switch (opcija) {
			case 0:
				kraj = 1;
				break;
			case 1:
				// ako je graf vec formiran potrebno ga je obrisati pre ponovnog formiranja
				if (formiranGraf) {
					obrisiMapu(igra);
					igra = NULL;
					formiranGraf = 0;
				}

				formiranGraf = formiranjeMape(&igra);
	
				break;
			case 2:
				if (formiranGraf) {
					dodajPlocu(igra);
				}
				else {
					printf("Morate formirati mapu pre koriscenja ove funkcije!\n");
					_sleep(2000);
				}
				break;
			case 3:
				if (formiranGraf) {
					obrisiPlocu(igra);
				}
				else {
					printf("Morate formirati mapu pre koriscenja ove funkcije!\n");
					_sleep(2000);
				}
				break;
			case 4:
				if (formiranGraf) {
					sacuvajMapu(igra);
				}
				else {
					printf("Morate formirati mapu pre koriscenja ove funkcije!\n");
					_sleep(2000);
				}
				break;
			case 5:
				if (formiranGraf) {
					pokreniIgru(igra);
				}
				else {
					printf("Morate formirati mapu pre koriscenja ove funkcije!\n");
					_sleep(2000);
				}
				break;
			case 6:
				if (formiranGraf) {
					obrisiMapu(igra);
					igra = NULL;
					formiranGraf = 0;
				}
				else {
					printf("Morate formirati stablo pre koriscenja ove funkcije!\n");
					_sleep(2000);
				}
				break;
			case 7:
				if (formiranGraf) {
					resiIgru(igra);
				}
				else {
					printf("Morate formirati stablo pre koriscenja ove funkcije!\n");
					_sleep(2000);
				}
				break;
			default:
				korektanUnos = 0;
				break;
			}
		} while (!korektanUnos);

	} while (!kraj);

	if (formiranGraf) {
		obrisiMapu(igra);
	}
}