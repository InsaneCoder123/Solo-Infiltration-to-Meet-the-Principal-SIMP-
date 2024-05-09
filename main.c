#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "mapData.c"

#define ANSI_COLOR_RED     "\x1b[31m"
#define ANSI_COLOR_GREEN   "\x1b[32m"
#define ANSI_COLOR_YELLOW  "\x1b[33m"
#define ANSI_COLOR_BLUE    "\x1b[34m"
#define ANSI_COLOR_MAGENTA "\x1b[35m"
#define ANSI_COLOR_CYAN    "\x1b[36m"
#define ANSI_COLOR_RESET   "\x1b[0m"

#define SCREEN_WIDTH 68
#define SCREEN_HEIGHT 21

#define CAMERA_WIDTH 41
#define CAMERA_HEIGHT 21

#define DEFAULT_MOVE_UP 'w'
#define DEFAULT_MOVE_LEFT 'a'
#define DEFAULT_MOVE_DOWN 's'
#define DEFAULT_MOVE_RIGHT 'd'

typedef struct {
	int x;
	int y;
	int areaID;
} position;

typedef struct {
	int mentalHealth;
	int Pesos;
	int Charisma;
} playerStats;

typedef struct {
	char moveUp;
	char moveLeft;
	char moveDown;
	char moveRight;
} Keybinds;

typedef struct {
	int id;
	int type;
	int quantity;
} Items;


typedef struct {
	playerStats stats;
	Items* inventory;
	position position;
	int itemsNumber;
} Player;

typedef struct {
	position coordinate;
	int id;
	int type;
} NPC;

typedef struct {
	Keybinds keybinds;
	int sceneID;
	int relativeCameraCoordinate[2];
	long timeWhenGameStarted;
	int timeInSeconds;
	int timeInMinutes;
	int day;
} Game;

int isPlayerInPosition(Player* player, int x, int y, int x2, int y2) {
	if (x2 == x && y2 == y) {
		return 1;
	}
	return 0;
}

char mapTypeToChar(int n) {
	switch (n) {
	case 0:
		return '.';
	case 1:
		return '#';
	case 2:
		return ' ';
	}
}

char intToChar(int n) {
	switch (n) {
	case 1:
		return 'A';
	case 2:
		return 'B';
	case 3:
		return 'C';
	case 4:
		return 'D';
	case 5:
		return 'E';
	case 6:
		return 'F';
	case 7:
		return 'G';
	case 8:
		return 'H';
	case 9:
		return 'I';
	case 10:
		return 'J';
	case 11:
		return 'K';
	case 12:
		return 'L';
	case 13:
		return 'M';
	case 14:
		return 'N';
	case 15:
		return 'O';
	case 16:
		return 'P';
	case 17:
		return 'Q';
	case 18:
		return 'R';
	case 19:
		return 'S';
	case 20:
		return 'T';
	case 21:
		return 'U';
	case 22:
		return 'V';
	case 23:
		return 'W';
	case 24:
		return 'X';
	case 25:
		return 'Y';
	case 26:
		return 'Z';
	case 27:
		return '-';
	case 28:
		return ':';
	case 29:
		return ' ';
	default:
		return '?';
	}
}


void updateCameraRelativeCoordinate(Game *game, Player *player) {
	game->relativeCameraCoordinate[0] = player->position.x - (CAMERA_WIDTH - 1) / 2;
	game->relativeCameraCoordinate[1] = player->position.y - (CAMERA_HEIGHT - 1) / 2;
}

int calculateIndexFromCoordinate(int x, int y, int width) { 
	return (y * width) + x;
}

void renderUI(Game *game, Player* player) {
	int screenPointerX = 0;
	int screenPointerY = 0;

	int userInterfacePointerX = 0;
	int userInterfacePointerY = 0;

	int dataPointer = 0;

	for (int screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
		for (int screenX = 0; screenX < SCREEN_WIDTH; screenX++) {
			updateCameraRelativeCoordinate(game, player);

			//Game Screen Rendering
			if (screenX < CAMERA_WIDTH) {
				if (isPlayerInPosition(player, (CAMERA_WIDTH-1)/2, (CAMERA_HEIGHT-1)/2, screenPointerX, screenPointerY)) {
					printf("@");
					++screenPointerX;
					continue;
				}
				if (game->relativeCameraCoordinate[0]+screenPointerX < 0 || game->relativeCameraCoordinate[1]+screenPointerY < 0 || game->relativeCameraCoordinate[0]+screenPointerX >= TOTAL_WIDTH_MAP || game->relativeCameraCoordinate[1]+screenPointerY >= TOTAL_HEIGHT_MAP) {
					printf(" ");
					++screenPointerX;
					continue;
				}
				printf("%c", mapTypeToChar(extractMapData(calculateIndexFromCoordinate(game->relativeCameraCoordinate[0] + screenPointerX, game->relativeCameraCoordinate[1] + screenPointerY, TOTAL_WIDTH_MAP))));
				++screenPointerX;
			}

			//User Interface Rendering
			if (screenX >= CAMERA_WIDTH) {
				if (screenY < TOTAL_HEIGHT_UI) {
					int currentInterfaceIndex = calculateIndexFromCoordinate(userInterfacePointerX, userInterfacePointerY, TOTAL_WIDTH_UI);
					switch (retrieveUIData()[currentInterfaceIndex] % 100) {
						case 53:
							switch (dataPointer) {
								case 0:
									printf("%d", player->stats.mentalHealth / 100);
									++userInterfacePointerX;
									++dataPointer;
									continue;
								case 1:
									printf("%d", (player->stats.mentalHealth % 100) / 10);
									++userInterfacePointerX;
									++dataPointer;
									continue;
								case 2:
									printf("%d", player->stats.mentalHealth % 10);
									dataPointer = 0;
									++userInterfacePointerX;
									continue;
								}
							break;
						case 54:
							switch (dataPointer) {
							case 0:
								printf("%d", player->stats.Pesos / 100);
								++userInterfacePointerX;
								++dataPointer;
								continue;
							case 1:
								printf("%d", (player->stats.Pesos % 100) / 10);
								++userInterfacePointerX;
								++dataPointer;
								continue;
							case 2:
								printf("%d", player->stats.Pesos % 10);
								dataPointer = 0;
								++userInterfacePointerX;
								continue;
							}
							break;
						case 55:
							switch (dataPointer) {
							case 0:
								printf("%d", player->stats.Charisma / 100);
								++userInterfacePointerX;
								++dataPointer;
								continue;
							case 1:
								printf("%d", (player->stats.Charisma % 100) / 10);
								++userInterfacePointerX;
								++dataPointer;
								continue;
							case 2:
								printf("%d", player->stats.Charisma % 10);
								dataPointer = 0;
								++userInterfacePointerX;
								continue;
							}
							break;
						case 56:
							switch (dataPointer) {
							case 0:
								printf("%d", game->timeInMinutes / 10);
								++userInterfacePointerX;
								++dataPointer;
								continue;
							case 1:
								printf("%d", game->timeInMinutes % 10);
								dataPointer = 0;
								++userInterfacePointerX;
								continue;
							}
							break;
						case 57:
							switch (dataPointer) {
							case 0:
								printf("%d", (game->timeInSeconds%60) / 10);
								++userInterfacePointerX;
								++dataPointer;
								continue;
							case 1:
								printf("%d", (game->timeInSeconds%60) % 10);
								dataPointer = 0;
								++userInterfacePointerX;
								continue;
							}
							break;
						case 58:
							printf("%d", game->day);
							++userInterfacePointerX;
							continue;
						}
					printf("%c", intToChar(extractInterfaceData(currentInterfaceIndex)));
					++userInterfacePointerX;
					}
				}
			}
		printf("\n");
		++screenPointerY;
		++userInterfacePointerY;
		screenPointerX = 0;
		userInterfacePointerX = 0;
	}
}

void changeposition(Player* player, int x, int y, int areaID) {
	player->position.x += x;
	player->position.y += y;
	player->position.areaID = areaID;
}

int canPlayerMoveThere(Player* player, Game* game, int x, int y) {
	// Type 0 - Can move
	// Type 1 - Can't Move
	// Type n will eventually check if player has item, then he can move
	if (retrieveMapData()[calculateIndexFromCoordinate(player->position.x + x, player->position.y + y, TOTAL_WIDTH_MAP)] % 10 == 1) {
		return 0;
	}
	return 1;
}


void movePlayer(Game* game, Player* player) {
	char userInput = _getch();
	if (userInput == game->keybinds.moveUp) {
		if (canPlayerMoveThere(player, game, 0, -1) != 0) {
			changeposition(player, 0, -1, game->sceneID);
		}
	}
	else if (userInput == game->keybinds.moveLeft) {
		if (canPlayerMoveThere(player, game, -1, 0) != 0) {
			changeposition(player, -1, 0, game->sceneID);
		}
	}
	else if (userInput == game->keybinds.moveDown) {
		if (canPlayerMoveThere(player, game, 0, 1) != 0) {
			changeposition(player, 0, 1, game->sceneID);
		}
	}
	else if (userInput == game->keybinds.moveRight) {
		if (canPlayerMoveThere(player, game, 1, 0) != 0) {
			changeposition(player, 1, 0, game->sceneID);
		}
	}
}

void addItemInInventory(Player *player, int id, int type, int quantity) {

	player->inventory = (Items*)realloc(player->inventory, sizeof(Items)*(player->itemsNumber+1));
	++player->itemsNumber;
	(player->inventory+(player->itemsNumber-1))->id = id;
	(player->inventory + (player->itemsNumber - 1))->type = type;
	(player->inventory + (player->itemsNumber - 1))->quantity = quantity;
}

void updateTime(Game *game) {
	game->timeInSeconds = 540 + (time(NULL) - game->timeWhenGameStarted) * 3;
	game->timeInMinutes = game->timeInSeconds / 60;
}

void debugMode(Player* player, Game* game) {
	int relativeTopLeftCoordinate[2] = { player->position.x - (CAMERA_WIDTH - 1) / 2, player->position.y - (CAMERA_HEIGHT - 1) / 2 };
	printf("Player Absolute Coordinate: [%d, %d]\n", player->position.x, player->position.y);
	printf("Camera Relative Coordinate: [%d, %d]\n", relativeTopLeftCoordinate[0], relativeTopLeftCoordinate[1]);
	printf("Player Mental Health: %d\n", player->stats.mentalHealth);
	printf("Time: %d%d:%d%d\n", game->timeInMinutes / 10, game->timeInMinutes % 10, game->timeInSeconds % 60 / 10, (game->timeInSeconds % 60) % 10);
	printf("Day:  %d", game->day);
}

int main(void) {
	Player player = { {185, 152, 553}, (Items*)malloc(sizeof(Items)*0), {20, 10, 0}, 0};
	Game game = { {DEFAULT_MOVE_UP, DEFAULT_MOVE_LEFT, DEFAULT_MOVE_DOWN, DEFAULT_MOVE_RIGHT}, 0, {0, 0}, time(NULL), 0, 1};
	int gameRunning = 1;
	updateCameraRelativeCoordinate(&game, &player);
	printf("\33[?25l"); // Hide the cursor
	//printf("\33[?25h"); // Re enable cursor
	while (gameRunning == 1) {
		updateTime(&game);
		renderUI(&game, &player);
		//debugMode(&player, &game);
		movePlayer(&game, &player);
		system("cls");
	}
}