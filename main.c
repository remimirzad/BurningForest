#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define MAX_LINE_LENGTH 100
#define INTACT 0
#define BURNING 1
#define BURNED 2

// Structure pour stocker les coordonnées des cases en feu
typedef struct {
    int row;
    int col;
} BurningCell;

// Fonction pour extraire la valeur d'une clé dans une ligne de configuration
float extractConfigValue(const char *line) {
    char *valueString = strchr(line, '=');
    if (valueString != NULL) {
        valueString++; // Ignorer le "=" et avancer vers la valeur
        return atof(valueString);
    }
    return -1; // Erreur de lecture
}

// Fonction pour extraire les coordonnées d'une cellule en feu à partir d'une ligne de configuration
BurningCell extractBurningCell(const char *line) {
    BurningCell cell;
    char *coordString = strchr(line, '=');
    if (coordString != NULL) {
        coordString++; // Ignorer le "=" et avancer vers les coordonnées
        sscanf(coordString, "%d,%d", &cell.row, &cell.col);
    }
    else {
        cell.row = -1;
        cell.col = -1;
    }
    return cell;
}

// Fonction pour initialiser la forêt avec des cases en feu aléatoires
void initForest(int height, int width, int numBurning, BurningCell *burningCells, float propagationProbability, int forest[height][width]) {
    // Initialiser toutes les cases comme étant intactes
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            forest[i][j] = INTACT;
        }
    }

    // Choisir aléatoirement les cases initialement en feu
    for (int i = 0; i < numBurning; i++) {
        int row = burningCells[i].row;
        int col = burningCells[i].col;
        if (row >= 0 && row < height && col >= 0 && col < width) {
            forest[row][col] = BURNING;
        }
    }
}

// Fonction pour afficher l'état actuel de la forêt
void printForest(int height, int width, int forest[height][width]) {
    int totalIntact = 0;
    int totalBurning = 0;
    int totalBurnt = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (forest[i][j] == INTACT) {
                printf(".");
                totalIntact++;
            } else if (forest[i][j] == BURNING) {
                printf("F");
                totalBurning++;
            } else {
                printf("c");
                totalBurnt++;
            }
        }
        printf("\n");
    }

    if (totalBurning != 0) {
        printf("Intacte : %3d, En feu : %3d, Cendres : %3d\n", totalIntact, totalBurning, totalBurnt);
    } else {
        printf("%3.2f%% de la forêt a brulée.\n", totalBurnt * 100.0 / (totalIntact + totalBurnt));
    }
    printf("\n");
}

// Fonction pour mettre à jour l'état de la forêt à l'étape suivante
void updateForest(int height, int width, float p, int forest[height][width]) {
    int newForest[height][width];
    srand(time(NULL));
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (forest[i][j] == BURNING) {
                newForest[i][j] = BURNED; // Le feu s'éteint dans cette case
            } else if (forest[i][j] == INTACT) {
                // Vérifier les cases adjacentes pour voir si le feu peut se propager
                if (((i > 0 && forest[i - 1][j] == BURNING) ||
                    (i < height - 1 && forest[i + 1][j] == BURNING) ||
                    (j > 0 && forest[i][j - 1] == BURNING) ||
                    (j < width - 1 && forest[i][j + 1] == BURNING)) &&
                    (float)rand() / RAND_MAX <= p) {
                    newForest[i][j] = BURNING; // La case prend feu
                } else {
                    newForest[i][j] = INTACT; // La case reste intacte
                }
            } else {
                newForest[i][j] = BURNED; // La case reste brûlée
            }
        }
    }
    // Copier la nouvelle forêt dans l'ancienne
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            forest[i][j] = newForest[i][j];
        }
    }
}

// Fonction pour vérifier si la forêt est entièrement éteinte
int isFireOut(int height, int width, int forest[height][width]) {
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < width; j++) {
            if (forest[i][j] == BURNING) {
                return 0; // Il y a encore des cases en feu
            }
        }
    }
    return 1; // La forêt est entièrement éteinte
}

int main() {

    // Lecture des configurations à partir du fichier config.txt
    FILE *configFile = fopen("config.txt", "r");
    if (configFile == NULL) {
        printf("Erreur lors de l'ouverture du fichier config.txt.\n");
        return 1;
    }

    // Variables pour stocker les configurations
    int height = 0;
    int width = 0;
    int numBurning = 0;
    int numOutOfRange = 0;
    BurningCell *burningCells = NULL;
    float propagationProbability = 0.0;

    char line[MAX_LINE_LENGTH];
    while (fgets(line, sizeof(line), configFile)) {
        // Ignorer les lignes de commentaire
        if (line[0] == '#') {
            continue;
        }

        // Extraire les valeurs des clés
        if (strncmp(line, "HEIGHT", 6) == 0) {
            height = (int)extractConfigValue(line);
        } else if (strncmp(line, "WIDTH", 5) == 0) {
            width = (int)extractConfigValue(line);
        } else if (strncmp(line, "BURNING_CELL", 12) == 0) {
            BurningCell cell = extractBurningCell(line);
            if (cell.row != -1 && cell.col != -1) {
                if (cell.col <= height && cell.row <= width) {
                    numBurning++;
                    burningCells = realloc(burningCells, numBurning * sizeof(BurningCell));
                    burningCells[numBurning - 1] = cell;
                } else {
                    numOutOfRange++;
                }
                
            }
        } else if (strncmp(line, "PROPAGATION_PROBABILITY", 23) == 0) {
            propagationProbability = extractConfigValue(line);
        }
    }

    fclose(configFile);

    printf("Hauteur : %d Largeur : %d \n", height, width);
    printf("Nombre de cases initiallement en feu : %2d \n", numBurning);
    printf("Possibilité de Propagation : %3.2f%% \n", propagationProbability * 100.0);
    printf("Nombre de cases impossibles : %2d \n", numOutOfRange);

    int forest[height][width];

    // Initialisation de la forêt avec un nombre donné de cases en feu
    initForest(height, width, numBurning, burningCells, propagationProbability, forest);

    // Boucle principale de la simulation
    int step = 0;
    while (!isFireOut(height, width, forest)) {
        printf("Étape %d:\n", step);
        printForest(height, width, forest);
        updateForest(height, width, propagationProbability, forest);
        step++;
    }

    printf("Étape %d: Forêt éteinte!\n", step);
    printForest(height, width, forest);

    printf("Simulation terminée. La forêt est entièrement éteinte.\n");

    return 0;
}
