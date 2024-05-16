#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

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

#define TOTAL_WIDTH_MAP 149
#define TOTAL_HEIGHT_MAP 43

#define TOTAL_WIDTH_UI 28
#define TOTAL_HEIGHT_UI 21

#define DEFAULT_MOVE_UP 'w'
#define DEFAULT_MOVE_LEFT 'a'
#define DEFAULT_MOVE_DOWN 's'
#define DEFAULT_MOVE_RIGHT 'd'

#define MAP_DATASIZE 3
#define UI_DATASIZE 2

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
	int statModifier;
} Items;


typedef struct {
	playerStats stats;
	Items inventory[13];
	position position;
	int itemsNumber;
} Player;

typedef struct {
	position coordinate;
	int id;
} NPC;

typedef struct Option Option;

typedef struct {
	char* dialouge;
	int currentOption;
	int numberOfOptions;
	Option* option;
} ActivePrompt;


typedef struct {
	Keybinds keybinds;
	int sceneID;
	int relativeCameraCoordinate[2];
	long timeWhenGameStarted;
	int timeInSeconds;
	int timeInMinutes;
	int day;
	NPC* npcList;
	char* mapData;
	char* interfaceData;
	int numberOfNPC;
	ActivePrompt activePrompt;
	int isInteractionActive;
} Game;

typedef void (*EventAction)(Game* game, Player* player);
struct Option{
	EventAction eventAction;
	char OptionText[50];
	int id;
};

void clearActivePrompt(Game* game, Player* player);

void printTest(Game *game, Player *player) {
	game->isInteractionActive = 0;
	clearActivePrompt(game, player);
	game->activePrompt.option = (Option*)realloc(game->activePrompt.option, 0);
	game->activePrompt.numberOfOptions = 0;
}


void AddOption(Game *game, char *dialouge, int id, void (*f)(Game, Player)) {
	int newNumberOfOptions = game->activePrompt.numberOfOptions + 1;
	Option* temp = (Option*)realloc(game->activePrompt.option, sizeof(Option)*newNumberOfOptions);
	if (temp == NULL) {
		printf(ANSI_COLOR_RED "\n[Memory Allocation Field]\n" ANSI_COLOR_RESET);
		return;
	}
	game->activePrompt.option = temp;
	game->activePrompt.numberOfOptions = newNumberOfOptions;

	strncpy(game->activePrompt.option[newNumberOfOptions - 1].OptionText, dialouge, sizeof(game->activePrompt.option[newNumberOfOptions - 1].OptionText) - 1);
	game->activePrompt.option[newNumberOfOptions - 1].OptionText[sizeof(game->activePrompt.option[newNumberOfOptions - 1].OptionText) - 1] = '\0'; // Ensure null-termination
	game->activePrompt.option[newNumberOfOptions - 1].id = id;
	game->activePrompt.option[newNumberOfOptions - 1].eventAction = f;
	
}

int isPlayerInPosition(Player* player, int x, int y, int x2, int y2) {
	if (x2 == x && y2 == y) {
		return 1;
	}
	return 0;
}

char mapTypeToChar(int n) {
	char mapTypeArr[] = ".# ";
	return mapTypeArr[n];
}

char intToChar(int n) {
	char uiArr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ-: ?";
	return uiArr[n-1];
}


void updateCameraRelativeCoordinate(Game *game, Player *player) {
	game->relativeCameraCoordinate[0] = player->position.x - (CAMERA_WIDTH - 1) / 2;
	game->relativeCameraCoordinate[1] = player->position.y - (CAMERA_HEIGHT - 1) / 2;
}

int calculateIndexFromCoordinate(int x, int y, int width, int dataSize, int dataIndex) { 
	return (y * width * dataSize) + (x * dataSize) + dataIndex;
}

void updateCurrentActivePrompt(Game *game, char *dialouge, int choice) {

	size_t newSize = strlen(dialouge) + 1;
	char *temp = (char*)realloc(game->activePrompt.dialouge, newSize);
	if (temp == NULL) {
		printf(ANSI_COLOR_RED "\n[Memory Allocation Field]\n" ANSI_COLOR_RESET);
		return;
	}
	
	game->activePrompt.dialouge = temp;
	strcpy(game->activePrompt.dialouge, dialouge);
	game->activePrompt.currentOption = choice;
}

void clearActivePrompt(Game *game, Player *player) {
	updateCurrentActivePrompt(game, " ", -1);
}

void spawnNPC(Game* game, int x, int y, int id) {
	game->npcList[game->numberOfNPC].coordinate.x = x;
	game->npcList[game->numberOfNPC].coordinate.x = y;
	game->npcList[game->numberOfNPC].id = id;
	game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] = id + '0';
	game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)] = 0 + '0';
}

char onOption(int currentOption, int optionID) {
	if (currentOption == optionID) {
		return '>';
	}
	return ' ';
}

void renderUI(Game *game, Player* player) {
	int screenPointerX = 0;
	int screenPointerY = 0;

	int userInterfacePointerX = 0;
	int userInterfacePointerY = 0;

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
				if (game->mapData[calculateIndexFromCoordinate(game->relativeCameraCoordinate[0] + screenPointerX, game->relativeCameraCoordinate[1] + screenPointerY, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] - '0' == 7) {
					printf(ANSI_COLOR_BLUE "@" ANSI_COLOR_RESET);
					++screenPointerX;
					continue;
				}
				printf("%c", mapTypeToChar(game->mapData[calculateIndexFromCoordinate(game->relativeCameraCoordinate[0] + screenPointerX, game->relativeCameraCoordinate[1] + screenPointerY,
					TOTAL_WIDTH_MAP, MAP_DATASIZE, 1)] - '0'));
				++screenPointerX;
			}

			//User Interface Rendering
			if (screenX >= CAMERA_WIDTH) {
				if (screenY < TOTAL_HEIGHT_UI) {
					int currentInterfaceIndex = calculateIndexFromCoordinate(userInterfacePointerX, userInterfacePointerY, TOTAL_WIDTH_UI, UI_DATASIZE, 0);
					char uiData[2] = { game->interfaceData[currentInterfaceIndex], game->interfaceData[currentInterfaceIndex + 1] };
					int uiIntData = atoi(uiData);
					switch (uiIntData) {
						case 53:
							printf("%03d", player->stats.mentalHealth);
							userInterfacePointerX += 3;
							continue;
						case 54:
							printf("%03d", player->stats.Pesos);
							userInterfacePointerX += 3;
							continue;
						case 55:
							printf("%03d", player->stats.Charisma);
							userInterfacePointerX += 3;
							continue;
						case 56:
							printf("%02d", game->timeInMinutes);
							userInterfacePointerX += 2;
							continue;
						case 57:
							printf("%02d", game->timeInSeconds % 60);
							userInterfacePointerX += 2;
							continue;
						case 58:
							printf("%d", game->day);
							++userInterfacePointerX;
							continue;
						}
					printf("%c", intToChar(uiIntData));
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

	printf("%s\n", game->activePrompt.dialouge);
	if (game->activePrompt.option != NULL) {
		for (int i = 0; i < game->activePrompt.numberOfOptions; i++) {
			printf("%c %s\n", onOption(game->activePrompt.currentOption, game->activePrompt.option[i].id), 
				game->activePrompt.option[i].OptionText);
		}
	}
}

void changeposition(Player* player, int x, int y, int areaID) {
	player->position.x += x;
	player->position.y += y;
	player->position.areaID = areaID;
}

void interactWithNPC(Game *game) {
	updateCurrentActivePrompt(game, "What do you want child?", 0);
	AddOption(game, "Testddd", 0, printTest);
	AddOption(game, "Leave", 1, printTest);
	game->isInteractionActive = 1;
}

int canPlayerMoveThere(Player* player, Game* game, int x, int y) {
	int indexOfObjectAhead = calculateIndexFromCoordinate(player->position.x + x, player->position.y + y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0);
	if (game->mapData[indexOfObjectAhead + 2] - '0' == 0) {
		if (game->mapData[indexOfObjectAhead] - '0' == 7) {
			interactWithNPC(game);
		}
		return 0;
	}
	return 1;
}

void addItemInInventory(Player* player, int id, int type, int statModifier, int quantity) {
	player->inventory[player->itemsNumber - 1].id = id;
	player->inventory[player->itemsNumber - 1].type = type;
	player->inventory[player->itemsNumber - 1].statModifier = statModifier;
	player->inventory[player->itemsNumber - 1].quantity = quantity;
	++(player->itemsNumber);
}


int readInput(Game* game, Player* player) {
	char userInput = _getch();
	if (game->isInteractionActive == 0) {
		if (userInput == game->keybinds.moveUp) {
			if (canPlayerMoveThere(player, game, 0, -1) == 1) {
				changeposition(player, 0, -1, game->sceneID);
			}
		}
		else if (userInput == game->keybinds.moveLeft) {
			if (canPlayerMoveThere(player, game, -1, 0) == 1) {
				changeposition(player, -1, 0, game->sceneID);
			}
		}
		else if (userInput == game->keybinds.moveDown) {
			if (canPlayerMoveThere(player, game, 0, 1) == 1) {
				changeposition(player, 0, 1, game->sceneID);
			}
		}
		else if (userInput == game->keybinds.moveRight) {
			if (canPlayerMoveThere(player, game, 1, 0) == 1) {
				changeposition(player, 1, 0, game->sceneID);
			}
		}
	}
	else {
		if (userInput == game->keybinds.moveUp && game->activePrompt.currentOption != 0) {
			game->activePrompt.currentOption -= 1;
		}
		else if (userInput == game->keybinds.moveDown && game->activePrompt.currentOption != game->activePrompt.numberOfOptions - 1) {
			game->activePrompt.currentOption += 1;
		}
		else if (userInput == 'm') {
			game->activePrompt.option[game->activePrompt.currentOption].eventAction(game, player);
		}
	}
	if (userInput == 'l') {
		spawnNPC(game, 10, 18, 7);
	}
	if (userInput == 'b') {
		return 0;
	}
	return 1;
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

int initiateDatas(Game *game) {
	FILE* fptr;
	fptr = fopen("mapData.txt", "r");
	if (fptr == NULL) { return 0; };

	game->mapData = (char*)realloc(game->mapData, sizeof(char)*(TOTAL_WIDTH_MAP*TOTAL_HEIGHT_MAP * MAP_DATASIZE));
	fgets(game->mapData, TOTAL_WIDTH_MAP * TOTAL_HEIGHT_MAP * MAP_DATASIZE, fptr);

	fptr = fopen("interfaceData.txt", "r");

	game->interfaceData = (char*)realloc(game->interfaceData, sizeof(char) * (TOTAL_WIDTH_UI * TOTAL_HEIGHT_UI * UI_DATASIZE));
	fgets(game->interfaceData, TOTAL_WIDTH_UI * TOTAL_HEIGHT_UI * UI_DATASIZE, fptr);

	fclose(fptr);

	return 1;
}

int main(void) {
	Player player = { {185, 53, 553}, {0, 0, 0}, {2, 21, 0}, 0 };
	Game game = { {DEFAULT_MOVE_UP, DEFAULT_MOVE_LEFT, DEFAULT_MOVE_DOWN, DEFAULT_MOVE_RIGHT},
		0, {0, 0}, time(NULL), 0, 1, 1, (NPC*)malloc(sizeof(NPC)), (char*)malloc(sizeof(char)), (char*)malloc(sizeof(char)), 0, {NULL, 0, 0, NULL}, 0};
	updateCameraRelativeCoordinate(&game, &player);
	updateCurrentActivePrompt(&game, "Objective: Meet the Principal", -1);
	printf("\33[?25l"); // Hide the cursor
	//printf("\33[?25h"); // Re enable cursor
	if (initiateDatas(&game) == 0) { printf(ANSI_COLOR_RED "{No Map Data}"); return; };

	renderUI(&game, &player);
	while (readInput(&game, &player) == 1) {
		system("cls");
		updateTime(&game);
		//debugMode(&player, &game);
		renderUI(&game, &player);
	}
	free(game.npcList);
	free(game.interfaceData);
	free(game.mapData);
	free(game.activePrompt.dialouge);
}