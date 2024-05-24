#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <conio.h>

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

#define TOTAL_WIDTH_MAP 165
#define TOTAL_HEIGHT_MAP 54

#define TOTAL_WIDTH_UI 28
#define TOTAL_HEIGHT_UI 21

#define DEFAULT_MOVE_UP 'w'
#define DEFAULT_MOVE_LEFT 'a'
#define DEFAULT_MOVE_DOWN 's'
#define DEFAULT_MOVE_RIGHT 'd'

#define MAP_DATASIZE 4
#define UI_DATASIZE 2

typedef struct {
	int x;
	int y;
	int areaID;
} Position;


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
	Position coordinate;
} DroppedItem;

typedef struct {
	int id;
	int type;
	int quantity;
	int statModifier;
} Items;

typedef struct {
	int requirementID; // The ID of the requirement, in case of an NPC having multiple requirements
	int type; // Type 1 - Item, Type 2 - PHP, Type 3 - CHA
	int itemID; // If -1, it is not needed
	int statRequired; // The corresponding amount of stat requirement for the particular stat requirement
} Requirement;

typedef struct {
	playerStats stats;
	Items inventory[13];
	Position position;
	int itemsNumber;
} Player;

typedef struct {
	char** dialouges;
	int dialougeID;
	int pointedDialouge;
} Dialouge;

typedef struct {
	int id;
	int amountOfRequirements;
	Position coordinate;
	Requirement* requirements;
	Dialouge dialouge;
} NPC;

typedef struct Option Option;

typedef struct {
	char* dialouge;
	int currentOption;
	int numberOfOptions;
	int dialougeStage;
	int attachedDataType;
	char* attachedDataString;
	Option* option;
} ActivePrompt;

typedef struct {
	int timeSnapshot[10];
	int timeInSeconds;
	int timeInMinutes;
	int day;
	long timeWhenDayStarted;
} GameTime;

typedef struct {
	NPC* npcList;
	NPC currentActiveNPC;
	ActivePrompt activePrompt;
	int lostCHA;
	int lostPHP;
	int gainCHA;
	int gainPHP;
	int numberOfNPC;
	int isInteractionActive;
} SceneManager;

typedef struct {
	Keybinds keybinds;
	GameTime gameTime;
	DroppedItem itemList[13];
	Requirement hallwayRequirements[3];
	int itemIDRequired;
	int requiredPHP;
	int requiredCHA;
	int itemNumberInMap;
	int relativeCameraCoordinate[2];
	char* mapData;
	char* interfaceData;
	int isGameRunning;
	int debugMode;
	int gameState;
	int hasRequiredItemSpawned;
} GameManager;

typedef void (*EventAction)(GameManager* game, SceneManager* scene, Player* player, int dialougeID);
struct Option{
	EventAction eventAction;
	char OptionText[50];
	int id;
	int pointedDialougeID;
};

void AddOption(SceneManager* scene, char* dialouge, int id, int pointedDialougeID, void (*f)(GameManager, SceneManager, Player, int));
void updateCurrentActivePrompt(SceneManager *scene, char* dialouge, int choice, int dialougeStage);
void changeMapState(GameManager* game, int areaID, int isHide);
void deleteCurrentActiveNPC(GameManager* game, SceneManager* scene);
void addItemInInventory(Player* player, int id, int type, int statModifier, int quantity);
void removeItemInInventory(Player* player, int itemIndex);
char *randomDialougeStudent();
int spawnNPC(GameManager* game, SceneManager* scene, Requirement* requirements, int amountOfRequirements, int x, int y, int areaID, int id, char** dialouges, int numberOfDialouges);

void clearConsole() {
	printf("\033[2J\033[1;1H");
}

void updateCurrentActiveNPC(SceneManager *scene, NPC activeNPC) {
	scene->currentActiveNPC = activeNPC;
}

void clearPromptAndOptions(GameManager *game, SceneManager *scene, Player* player) {
	updateCurrentActivePrompt(scene, " ", -1, 0, NULL);
	scene->activePrompt.option = (Option*)realloc(scene->activePrompt.option, 0);
	scene->activePrompt.numberOfOptions = 0;
}

void exitInteractionWithNPC(GameManager* game, SceneManager *scene, Player* player, int dialougeID) {
	updateCurrentActivePrompt(scene, " ", -1, 0, NULL);
	scene->activePrompt.option = (Option*)realloc(scene->activePrompt.option, 0);
	scene->activePrompt.numberOfOptions = 0;
	scene->isInteractionActive = 0;
}

void goToDialougeAndOptions(GameManager *game, SceneManager *scene, Player *player, int dialougeID, int dialougeStage) {
	clearPromptAndOptions(game, scene, player);
	updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[dialougeID], 0, dialougeStage);
	AddOption(scene, "Leave", 0, -1, exitInteractionWithNPC);
}

int doesPlayerHaveItem(Player *player, int itemIdRequirement) {
	for (int i = 0; i < player->itemsNumber; i++) {
		if (player->inventory[i].id == itemIdRequirement) {
			return i;
		}
	}
	return -1;
}

void goToDialougeAndOptionsIfRequirementMet(GameManager* game, SceneManager* scene, Player* player, int dialougeID) {
	clearPromptAndOptions(game, scene, player);
	switch (scene->currentActiveNPC.requirements[dialougeID-2].type) {
	case 1: // Item Requirement
		if (doesPlayerHaveItem(player, scene->currentActiveNPC.requirements[0].itemID) != -1) {
			updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[dialougeID], 0, 30, NULL);
			AddOption(scene, "Leave", 0, -1, exitInteractionWithNPC);
			removeItemInInventory(player, doesPlayerHaveItem(player, scene->currentActiveNPC.requirements[0].itemID));
		}
		else {
			goToDialougeAndOptions(game, scene, player, 1, 11);
		}
		break;
	case 2: // PHP Requirement
		if (player->stats.Pesos >= scene->currentActiveNPC.requirements[0].statRequired) {
			updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[dialougeID], 0, 30, NULL);
			AddOption(scene, "Leave", 0, -1, exitInteractionWithNPC);
		}
		else {
			goToDialougeAndOptions(game, scene, player, 1, 10);
		}
		break;
	case 3: // CHA Requirement
		if (player->stats.Charisma >= scene->currentActiveNPC.requirements[0].statRequired) {
			updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[dialougeID], 0, 30, NULL);
			AddOption(scene, "Leave", 0, -1, exitInteractionWithNPC);
		}
		else { 
			goToDialougeAndOptions(game, scene, player, 1, 10);
		}
		break;
	}
}

void getItemOrStatIfRequirementMet(GameManager *game, SceneManager *scene, Player *player, int dialougeID) {
	clearPromptAndOptions(game, scene, player);
}


void AddOption(SceneManager *scene, char *dialouge, int id, int pointedDialougeID, void (*f)(GameManager, SceneManager, Player, int)) {
	// Add an option when interacting with an NPC, or possibly an object
	// The option added will have an attached function assigned in runtime that can be executed when the player selects the option
	// The option may also have an attached pointed dialouge ID that will update the current prompt to that dialouge
	int newNumberOfOptions = scene->activePrompt.numberOfOptions + 1;
 	Option* temp = (Option*)realloc(scene->activePrompt.option, sizeof(Option)*newNumberOfOptions);
	if (temp == NULL) {
		printf(ANSI_COLOR_RED "\n[Memory Allocation Field]\n" ANSI_COLOR_RESET);
		return;
	}
	scene->activePrompt.option = temp;
	scene->activePrompt.numberOfOptions = newNumberOfOptions;

	strncpy(scene->activePrompt.option[newNumberOfOptions - 1].OptionText, dialouge, sizeof(scene->activePrompt.option[newNumberOfOptions - 1].OptionText) - 1);
	scene->activePrompt.option[newNumberOfOptions - 1].OptionText[sizeof(scene->activePrompt.option[newNumberOfOptions - 1].OptionText) - 1] = '\0'; // Ensure null-termination
	scene->activePrompt.option[newNumberOfOptions - 1].id = id;
	scene->activePrompt.option[newNumberOfOptions - 1].pointedDialougeID = pointedDialougeID;
	scene->activePrompt.option[newNumberOfOptions - 1].eventAction = f;
	
}

int isTwoCoordinatesTheSame(int x, int y, int x2, int y2) {
	// Compares to coordinate if they are the same
	// Function was made to increase readability of code
	// Used to determine if a specific object is in the pointed coordinate in the nested loops while rendering
	if (x2 == x && y2 == y) {
		return 1;
	}
	return 0;
}

char* itemIDtoString(int n) {
	char* itemName;
	switch (n) {
	case 1:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "GUARD KEY" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "GUARD KEY" ANSI_COLOR_RESET;
		return itemName;
	case 2:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "BSCPE ID SLING" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "BSCPE ID SLING" ANSI_COLOR_RESET;
		return itemName;
	case 3:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "EXCEL'S GLUE" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "EXCEL'S GLUE" ANSI_COLOR_RESET;
		return itemName;
	case 4:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "WILFRED'S STAPLER" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "WILFRED'S STAPLER" ANSI_COLOR_RESET;
		return itemName;
	case 5:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "TRIXIE'S BALLPEN" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "TRIXIE'S BALLPEN" ANSI_COLOR_RESET;
		return itemName;
	case 6:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "VJ'S T-SQUARE" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "VJ'S T-SQUARE" ANSI_COLOR_RESET;
		return itemName;
	case 7:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "SHAUN'S NOTE (+10 CHA)" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "SHAUN'S NOTE (+10 CHA)" ANSI_COLOR_RESET;
		return itemName;
	case 8:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "KEM'S CREDIT CARD (+20 PHP)" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "KEM'S CREDIT CARD (+20 PHP)" ANSI_COLOR_RESET;
		return itemName;
	case 9:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "MAICA'S EARPHONE (+8 CHA)" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "MAICA'S EARPHONE (+8 CHA)" ANSI_COLOR_RESET;
		return itemName;
	case 10:
		itemName = (char*)malloc(sizeof(char) * strlen(ANSI_COLOR_BLUE "DEEP ROCK (+15 CHA)" ANSI_COLOR_RESET));
		itemName = ANSI_COLOR_BLUE "DEEP ROCK (+15 CHA)" ANSI_COLOR_RESET;
		return itemName;
	}
	return NULL;
}

char* areaIDtoString(int n) {
	char* areaName;
	switch (n) {
	case 0:
		areaName = (char*)malloc(sizeof(char) * strlen("[ENTRANCE]"));
		areaName = "[ENTRANCE]";
		return areaName;
	case 1:
		areaName = (char*)malloc(sizeof(char) * strlen("[LIBRARY]"));
		areaName = "[LIBRARY]";
		return areaName;
	case 2:
		areaName = (char*)malloc(sizeof(char) * strlen("[PRINCIPAL'S OFFICE]"));
		areaName = "[PRINCIPAL'S OFFICE]";
		return areaName;
	case 3:
		areaName = (char*)malloc(sizeof(char) * strlen("[STUDY AREA]"));
		areaName = "[STUDY AREA]";
		return areaName;
	case 4:
		areaName = (char*)malloc(sizeof(char) * strlen("[GARDEN]"));
		areaName = "[GARDEN]";
		return areaName;
	case 5:
		areaName = (char*)malloc(sizeof(char) * strlen("[GLECROOM]"));
		areaName = "[GLECROOM]";
		return areaName;
	case 6:
		areaName = (char*)malloc(sizeof(char) * strlen("[GYM]"));
		areaName = "[GYM]";
		return areaName;
	case 7:
		areaName = (char*)malloc(sizeof(char) * strlen("[FACULTY OFFICE]"));
		areaName = "[FACULTY OFFICE]";
		return areaName;
	case 8:
		areaName = (char*)malloc(sizeof(char) * strlen("[HALLWAY]"));
		areaName = "[HALLWAY]";
		return areaName;
	}
	return NULL;
}

char mapTypeToChar(int n) {
	// Converts an int into a char
	// Used in the conversion from map data to a char representation of the data to use in the render of the map data
	char mapTypeArr[] = ".# _/\\|-+=";
	return mapTypeArr[n];
}

char intToChar(int n) {
	// Converts an int into a char
	// Used in the conversion from user interface data to a char representation of the data to use in the render of the user interface
	char uiArr[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ-: ?";
	return uiArr[n-1];
}

char* requirementTypeToString(int n) {
	char* s = '\0';
	switch (n) {
	case 2:
		s = "PHP";
		return s;
	case 3:
		s = "CHA";
		return s;
	}
}


NPC findNPCwithCoordinate(SceneManager *scene, int x, int y) {
	// Iliterates through the list of NPC and then returns the NPC that has a specific coordinate
	for (int i = 0; i < scene->numberOfNPC; i++) {
		if (isTwoCoordinatesTheSame(scene->npcList[i].coordinate.x, scene->npcList[i].coordinate.y, x, y)) {
			return scene->npcList[i];
		}
	}
	NPC temp;
	temp.id = -100;
	return temp;
}

void updateCameraRelativeCoordinate(GameManager *game, Player *player) {
	// Updates the absolute coordinate of the point in the top left of the player based on the render distance around the player
	// This is used as the reference coordinate when rendering the map
	game->relativeCameraCoordinate[0] = player->position.x - (CAMERA_WIDTH - 1) / 2;
	game->relativeCameraCoordinate[1] = player->position.y - (CAMERA_HEIGHT - 1) / 2;
}

int calculateIndexFromCoordinate(int x, int y, int width, int dataSize, int dataIndex) { 
	// Calculates the index of the data of a element in a one dimensional array
	// Refer to the documentation for the corresponding data of the elements (map and user interface)
	// Each data of an element has a particular amount of data attached to it. 
	// Example: A wall may have three data: isMoveable, State, Map Element Type. Which is represented as 011 in data form
	return (y * width * dataSize) + (x * dataSize) + dataIndex;
}

void updateCurrentActivePrompt(SceneManager *scene, char *dialouge, int choice, int dialougeStage) {

	// Updates the current active prompt in the screen

	size_t newSize = strlen(dialouge) + 1;
	char *temp = (char*)realloc(scene->activePrompt.dialouge, newSize);
	if (temp == NULL) {
		printf(ANSI_COLOR_RED "\n[Memory Allocation Field]\n" ANSI_COLOR_RESET);
		return;
	}
	scene->activePrompt.dialouge = temp;
	scene->activePrompt.dialougeStage = dialougeStage;
	strcpy(scene->activePrompt.dialouge, dialouge);
	scene->activePrompt.currentOption = choice;
}


int spawnNPC(GameManager* game, SceneManager *scene, Requirement *requirements, int amountOfRequirements, int x, int y, int areaID, int id, char **dialouges, int numberOfDialouges) {
	// Spawns an NPC by initializing its coordinates, dialouges and then modifying the map data to indicate the presence of the NPC
	// Additionally, add the NPC to the scene manager NPC list
	if (findNPCwithCoordinate(scene, x, y).id != -100) { return 0; }
	if (game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] == '1') { return 0; }
	scene->npcList = (NPC*)realloc(scene->npcList, sizeof(NPC) * (scene->numberOfNPC + 1));
	NPC *lastFreeSpaceInNPCList = &scene->npcList[scene->numberOfNPC];
	lastFreeSpaceInNPCList->coordinate.x = x;
	lastFreeSpaceInNPCList->coordinate.y = y;
	lastFreeSpaceInNPCList->id = id;
	lastFreeSpaceInNPCList->coordinate.areaID = areaID;
	lastFreeSpaceInNPCList->dialouge.dialouges = (char**)malloc(numberOfDialouges*sizeof(char*));
	int dialougeIdAssignment = 0;
	if (amountOfRequirements != 0) {
		lastFreeSpaceInNPCList->requirements = (Requirement*)malloc(sizeof(Requirement)*amountOfRequirements);
		for (int i = 0; i < amountOfRequirements; i++) {
			lastFreeSpaceInNPCList->amountOfRequirements = amountOfRequirements;
			lastFreeSpaceInNPCList->requirements[i].itemID = requirements[i].itemID;
			lastFreeSpaceInNPCList->requirements[i].requirementID = requirements[i].requirementID;
			lastFreeSpaceInNPCList->requirements[i].statRequired = requirements[i].statRequired;
			lastFreeSpaceInNPCList->requirements[i].type = requirements[i].type;
		}
	}
	for (int i = 0; i < numberOfDialouges; i++) {
		lastFreeSpaceInNPCList->dialouge.dialouges[i] = (char*)malloc((strlen(dialouges[i])+1) * sizeof(char));
		lastFreeSpaceInNPCList->dialouge.dialougeID = dialougeIdAssignment++;
		strcpy(lastFreeSpaceInNPCList->dialouge.dialouges[i], dialouges[i]);
	}


	game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] = id + '0';
	game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)] = 0 + '0';

	++scene->numberOfNPC;
	return 1;
}

char onOption(int currentOption, int optionID) {
	// If the current option of the player matches the option id of the argument, it will a '>' which indicates in the render what
	// option the player is selecting
	if (currentOption == optionID) {
		return '>';
	}
	return ' ';
}

int whatNPClocatedAt(GameManager *game, int referenceCoordinateX, int referenceCoordinateY, int screenPointerX, int screenPointerY) {
	return game->mapData[calculateIndexFromCoordinate(referenceCoordinateX + screenPointerX, referenceCoordinateY + screenPointerY, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] - '0';
}

void renderUI(GameManager *game, SceneManager *scene, Player* player) {
	// Utilizes two independent tracker for the current coordinate that the nested loops points to relative to the coordinate designated in
	// the map data and the user interface data which creates the illusion of boundary and limited render distance around the player
	// Additionally, it allows the render of player stats data in the right side of the render of the map data as independent entities

	int screenPointerX = 0;
	int screenPointerY = 0;

	int userInterfacePointerX = 0;
	int userInterfacePointerY = 0;

	int currentInventorySlot = 0;

	for (int screenY = 0; screenY < SCREEN_HEIGHT; screenY++) {
		for (int screenX = 0; screenX < SCREEN_WIDTH; screenX++) {
			updateCameraRelativeCoordinate(game, player);

			// Game Screen Rendering

			if (screenX < CAMERA_WIDTH) {
				if (game->relativeCameraCoordinate[0]+screenPointerX < 0 || game->relativeCameraCoordinate[1]+screenPointerY < 0 || game->relativeCameraCoordinate[0]+screenPointerX >= TOTAL_WIDTH_MAP || game->relativeCameraCoordinate[1]+screenPointerY >= TOTAL_HEIGHT_MAP) {
					printf(" ");
					++screenPointerX;
					continue;
				}
				if ((game->mapData[calculateIndexFromCoordinate(game->relativeCameraCoordinate[0] + screenPointerX, game->relativeCameraCoordinate[1] + screenPointerY,
					TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)] - '0') == 2) {
					printf("?");
					++screenPointerX;
					continue;
				}
				if ((game->mapData[calculateIndexFromCoordinate(game->relativeCameraCoordinate[0] + screenPointerX, game->relativeCameraCoordinate[1] + screenPointerY,
					TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] - '0') == 1) {
					printf(ANSI_COLOR_YELLOW "$" ANSI_COLOR_RESET);
					++screenPointerX;
					continue;
				}
				if (isTwoCoordinatesTheSame((CAMERA_WIDTH-1)/2, (CAMERA_HEIGHT-1)/2, screenPointerX, screenPointerY)) {
					printf("@");
					++screenPointerX;
					continue;
				}
				switch (whatNPClocatedAt(game, game->relativeCameraCoordinate[0], game->relativeCameraCoordinate[1], screenPointerX, screenPointerY)) {
				case 7:
					printf(ANSI_COLOR_BLUE "@" ANSI_COLOR_RESET);
					++screenPointerX;
					continue;
				case 5:
					printf(ANSI_COLOR_RED "@" ANSI_COLOR_RESET);
					++screenPointerX;
					continue;
				case 9:
					printf(ANSI_COLOR_MAGENTA "@" ANSI_COLOR_RESET);
					++screenPointerX;
					continue;
				}

				printf("%c", mapTypeToChar(game->mapData[calculateIndexFromCoordinate(game->relativeCameraCoordinate[0] + screenPointerX, game->relativeCameraCoordinate[1] + screenPointerY,
					TOTAL_WIDTH_MAP, MAP_DATASIZE, 1)] - '0'));
				++screenPointerX;
			}

			// User Interface Rendering

			if (screenX >= CAMERA_WIDTH) {
				if (screenY < TOTAL_HEIGHT_UI) {
					int currentInterfaceIndex = calculateIndexFromCoordinate(userInterfacePointerX, userInterfacePointerY, TOTAL_WIDTH_UI, UI_DATASIZE, 0);
					if (currentInterfaceIndex < TOTAL_WIDTH_UI*TOTAL_HEIGHT_UI*UI_DATASIZE) {
						char uiData[2] = { game->interfaceData[currentInterfaceIndex], game->interfaceData[currentInterfaceIndex + 1] };
						int uiIntData = atoi(uiData);
						switch (uiIntData) {
							case 27:
								if (itemIDtoString(player->inventory[currentInventorySlot].id) != NULL) {
									printf(" - ");
									printf("%s", itemIDtoString(player->inventory[currentInventorySlot].id));
									printf(" - ");
								}
								else { printf(" [EMPTY SLOT]"); }
								++currentInventorySlot;
								userInterfacePointerX += TOTAL_WIDTH_UI * TOTAL_HEIGHT_UI * UI_DATASIZE;
								continue;
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
								printf("%02d", game->gameTime.timeInMinutes);
								userInterfacePointerX += 2;
								continue;
							case 57:
								printf("%02d", game->gameTime.timeInSeconds % 60);
								userInterfacePointerX += 2;
								continue;
							case 58:
								printf("%d", game->gameTime.day);
								++userInterfacePointerX;
								continue;
							case 59:
								printf("%s", areaIDtoString(player->position.areaID));
								userInterfacePointerX += strlen(areaIDtoString(player->position.areaID));
								continue;
							}
						printf("%c", intToChar(uiIntData));
					}
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

	if (scene->lostCHA != 0) {
		printf(ANSI_COLOR_RED "You are slowly losing sanity, thus also losing your confidence " ANSI_COLOR_RESET);
		printf(ANSI_COLOR_RED "(-%d CHA)\n" ANSI_COLOR_RESET, scene->lostCHA);
		if (time(NULL) - game->gameTime.timeSnapshot[0] >= 5) {
			scene->lostCHA = 0;
		}
	}
	if (scene->lostPHP != 0) {
		printf(ANSI_COLOR_RED "You are becoming insane by the second. You lost some money " ANSI_COLOR_RESET);
		printf(ANSI_COLOR_RED "(-%d PHP)\n" ANSI_COLOR_RESET, scene->lostPHP);
		if (time(NULL) - game->gameTime.timeSnapshot[0] >= 5) {
			scene->lostPHP = 0;
		}
	}
	if (scene->gainCHA != 0) {
		printf(ANSI_COLOR_GREEN "You gained confidence by talking with a student, but time passed " ANSI_COLOR_RESET);
		printf(ANSI_COLOR_GREEN "(-%d CHA)" ANSI_COLOR_RESET, scene->gainCHA);
		printf(ANSI_COLOR_RED " (+1 HR)\n" ANSI_COLOR_RESET);
		if (time(NULL) - game->gameTime.timeSnapshot[2] >= 5) {
			scene->gainCHA = 0;
		}
	}
	if (scene->gainPHP != 0) {
		printf(ANSI_COLOR_GREEN "You actually begged for money, you lost some pride and self-esteem " ANSI_COLOR_RESET);
		printf(ANSI_COLOR_GREEN "(+%d PHP)" ANSI_COLOR_RESET, scene->gainPHP);
		printf(ANSI_COLOR_RED " (-%d MH)\n" ANSI_COLOR_RESET, scene->gainPHP);
		if (time(NULL) - game->gameTime.timeSnapshot[2] >= 5) {
			scene->gainPHP = 0;
		}
	}
	printf("%s", scene->activePrompt.dialouge);
	printf("\n");
	if (scene->activePrompt.dialougeStage == 10) {
		printf("(You require %d %s to complete action)\n", scene->currentActiveNPC.requirements[0].statRequired,requirementTypeToString(scene->currentActiveNPC.requirements[0].type));
	}
	if (scene->activePrompt.dialougeStage == 11) { 
		printf("(You require %s to complete action)\n", itemIDtoString(scene->currentActiveNPC.requirements[0].type));
	}
	if (scene->activePrompt.dialougeStage == 12) {
		printf("(You require %s to pass through!)\n", itemIDtoString(game->itemIDRequired));
		clearPromptAndOptions(game, scene, player);
	}
	if (scene->activePrompt.dialougeStage == 13) {
		printf("(You require %d PHP to pass through!)\n", game->requiredPHP);
		clearPromptAndOptions(game, scene, player);
	}
	if (scene->activePrompt.dialougeStage == 14) {
		printf("(You require %d CHA to pass through!)\n", game->requiredCHA);
		clearPromptAndOptions(game, scene, player);
	}

	if (scene->activePrompt.option != NULL) {
		for (int i = 0; i < scene->activePrompt.numberOfOptions; i++) {
			printf("%c %s\n", onOption(scene->activePrompt.currentOption, scene->activePrompt.option[i].id), 
				scene->activePrompt.option[i].OptionText);
		}
	}
}

void changeposition(Player* player, int x, int y) {
	player->position.x += x;
	player->position.y += y;
}

void interactWithEntranceGuard(SceneManager *scene) {
	// The current prompt will become the dialouge stored in that particular type of NPC
	// To be implemented: Template of interaction for each NPC
	updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[0], 0, 0);
	AddOption(scene, "Where is the Principal?", 0, 2, goToDialougeAndOptionsIfRequirementMet);
	AddOption(scene, "Leave", 1, -1, exitInteractionWithNPC);
	scene->isInteractionActive = 1;
}

void endGame(GameManager* game, SceneManager* scene, Player* player, int dialougeID) {
	game->isGameRunning = 0;
	system("cls");
	printf("Congrats! You have met the Principal.");
	printf("\n\n\n\n\n\n");
	printf("Game Developed by:\n");
	printf("Kem Philip S. David\n");
	printf("Excel Duran\n\n\n");
	printf("Game Project made for CPE162\n\n\n");
	printf("Thanks for Playing!!!\n\n\n\n\n");
	printf("Statistics:\n");
	printf("Days Spent Finding the Principal: %d\n", game->gameTime.day);
	printf("Player Mental Health: %03d\n", player->stats.mentalHealth);
	printf("Player Pesos: %03d\n", player->stats.Pesos);
	printf("Player Charisma: %03d\n\n\n", player->stats.Charisma);
}

void interactWithPrincipal(SceneManager* scene) {
	updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[0], 0, 0);
	AddOption(scene, "Yes", 0, -1, endGame);
	scene->isInteractionActive = 1;
}


void changeStatIfRequirementMet(GameManager* game, SceneManager* scene, Player* player, int dialougeID) {
	clearPromptAndOptions(game, scene, player);
}

void talkToStudentAndExit(GameManager* game, SceneManager* scene, Player* player, int dialougeID) {
	srand(time(NULL));
	int statRandomizer = 2 + (rand() % 15);
	player->stats.Charisma += statRandomizer;
	game->gameTime.timeWhenDayStarted -= 10;
	game->gameTime.timeSnapshot[2] = time(NULL);
	scene->gainCHA = statRandomizer;
	exitInteractionWithNPC(game, scene, player, 0);
	deleteCurrentActiveNPC(game, scene);
}

void begToStudentAndExit(GameManager* game, SceneManager* scene, Player* player, int dialougeID) {
	srand(time(NULL));
	int statRandomizer = 2 + (rand() % 15);
	player->stats.Pesos += statRandomizer;
	if (player->stats.mentalHealth - statRandomizer < 0) {
		player->stats.mentalHealth = 0;
	}
	else { player->stats.mentalHealth -= statRandomizer; }
	game->gameTime.timeSnapshot[2] = time(NULL);
	scene->gainPHP = statRandomizer;
	exitInteractionWithNPC(game, scene, player, 0);
	deleteCurrentActiveNPC(game, scene);
}

void interactWithStudent(SceneManager *scene) {
	updateCurrentActivePrompt(scene, scene->currentActiveNPC.dialouge.dialouges[0], 0, 0);
	AddOption(scene, "Talk", 0, -1, talkToStudentAndExit);
	AddOption(scene, "Beg", 1, -1, begToStudentAndExit);
	AddOption(scene, "Leave", 2, -1, exitInteractionWithNPC);
	scene->isInteractionActive = 1;
}

void spawnStudentNPC(GameManager *game, SceneManager *scene) {
	// Refer to documentation for specific area bonds for the different areas
	srand(time(NULL));
	int areaRandomizer = rand() % 4;
	int xRandomizer = 0, yRandomizer = 0, areaID = 0;
	if (game->gameTime.timeSnapshot[4] == 0) {
		game->gameTime.timeSnapshot[4] = time(NULL);
	}
	if (time(NULL) - game->gameTime.timeSnapshot[4] >= 15) {
		switch (areaRandomizer) {
		case 0: // Study Area
			areaID = 3;
			xRandomizer = 4 + rand() % (55-4+1);
			yRandomizer = 41 + rand() % (51-41+1);
			break;
		case 1: // Gym
			areaID = 6;
			xRandomizer = 90 + rand() % (163-90+1);
			yRandomizer = 39 + rand() % (47-39+1);
			break;
		case 2: // Glecroom
			areaID = 5;
			xRandomizer = 68 + rand() % (94-68+1);
			yRandomizer = 12 + rand() % (25-12+1);
			break;
		case 3: // Library
			areaID = 1;
			xRandomizer = 21 + rand() % (45-21+1);
			yRandomizer = 12 + rand() % (25-12+1);
			break;
		}
		char* npcDialouges[1] = { '\0' };
		npcDialouges[0] = randomDialougeStudent();
		game->gameTime.timeSnapshot[4] = 0;
		spawnNPC(game, scene, NULL, 0, xRandomizer, yRandomizer, areaID, 9, npcDialouges, 1);
	}
}

int removeDroppedItem(GameManager* game, SceneManager* scene, int indexOfObjectAhead, int x, int y) {
	int itemID = 0;
	game->mapData[indexOfObjectAhead] = 0 + '0';
	for (int i = 0; i < game->itemNumberInMap; i++) {
		if (isTwoCoordinatesTheSame(game->itemList[i].coordinate.x, game->itemList[i].coordinate.y, x, y)) {
			itemID = game->itemList[i].id;
			game->itemList[i].id = 0;
			break;
		}
	}
	--game->itemNumberInMap;
	return itemID;

}

void moveDroppedItemToInventory(GameManager *game, SceneManager *scene, Player *player, int indexOfObjectAhead, int x, int y) {
	switch (removeDroppedItem(game, scene, indexOfObjectAhead, x, y)) {
	case 1:
		addItemInInventory(player, 1, 0, -1, 1);
		break;
	case 2:
		addItemInInventory(player, 2, 0, -1, 1);
		break;
	case 3:
		addItemInInventory(player, 3, 0, -1, 1);
		break;
	case 4:
		addItemInInventory(player, 4, 0, -1, 1);
		break;
	case 5:
		addItemInInventory(player, 5, 0, -1, 1);
		break;
	case 6:
		addItemInInventory(player, 6, 0, -1, 1);
		break;
	case 7:
		addItemInInventory(player, 7, 1, 10, 1);
		break;
	case 8:
		addItemInInventory(player, 8, 2, 20, 1);
		break;
	case 9:
		addItemInInventory(player, 9, 1, 8, 1);
		break;
	case 10:
		addItemInInventory(player, 10, 1, 15, 1);
		break;
	}

}

int promptHallwayRequirement(GameManager *game, Player *player, SceneManager *scene, int location) {
	switch (game->hallwayRequirements[location].type) {
	case 1: // Item
		for (int j = 0; j < player->itemsNumber; j++) {
			if (player->inventory[location].id == game->hallwayRequirements[location].itemID) {
				return 1;
			}
		}
		game->itemIDRequired = game->hallwayRequirements[location].itemID;
		updateCurrentActivePrompt(scene, ANSI_COLOR_RED "You need something to go through!" ANSI_COLOR_RESET, 0, 12);
		return 0;
	case 2: // PHP
		if (player->stats.Pesos < game->hallwayRequirements[location].statRequired) {
			game->requiredPHP = game->hallwayRequirements[location].statRequired;
			updateCurrentActivePrompt(scene, ANSI_COLOR_RED "You lack money!" ANSI_COLOR_RESET, 0, 13);
			return 0;
		}
		else {
			return 1;
		}
	case 3: // CHA
		if (player->stats.Charisma < game->hallwayRequirements[location].statRequired) {
			game->requiredCHA = game->hallwayRequirements[location].statRequired;
			updateCurrentActivePrompt(scene, ANSI_COLOR_RED "You lack confidence!" ANSI_COLOR_RESET, 0, 14);
			return 0;
		}
		else {
			return 1;
		}
	}
	return 0;
}

int canPlayerMoveThere(Player* player, SceneManager *scene, GameManager* game, int x, int y) {
	// Checks if a player can move to a position forward. Gets the data of the object the player wants to move to and then decide
	// an action depending on the attached data on that object
	int indexOfObjectAhead = calculateIndexFromCoordinate(player->position.x + x, player->position.y + y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0);
	if (game->mapData[indexOfObjectAhead] - '0' != 0) {
		switch (whatNPClocatedAt(game, player->position.x, player->position.y, x, y)) {
			// Refer to documentation. A [7] attached data in the first index refers to a Guard NPC occupying that area
			// A player attempting to move to that area will trigger an interaction with the NPC.
			// The NPC in the coordinate the player attempted to move to will become the current active NPC that can be accessed in the scene manager
		case 1:
			moveDroppedItemToInventory(game, scene, player, indexOfObjectAhead, player->position.x + x, player->position.y + y);
			return 1;
		case 2:
			if (player->position.x + x >= 25 && player->position.y + y >= 8 && player->position.x + x <= 30 && player->position.y + y <= 8) {
				// Principal Office
				return promptHallwayRequirement(game, player, scene, 0);
			}
			else if (player->position.x + x >= 71 && player->position.y + y >= 37 && player->position.x + x <= 76 && player->position.y + y <= 37) {
				// Garden
				return promptHallwayRequirement(game, player, scene, 1);
			}
			else if (player->position.x + x >= 142 && player->position.y + y >= 30 && player->position.x + x <= 147 && player->position.y + y <= 30) {
				// Faculty Office
				return promptHallwayRequirement(game, player, scene, 2);
			}
			break;
		case 9:
			updateCurrentActiveNPC(scene, findNPCwithCoordinate(scene, player->position.x + x, player->position.y + y));
			interactWithStudent(scene);
			break;
		case 7:
			updateCurrentActiveNPC(scene, findNPCwithCoordinate(scene, player->position.x + x, player->position.y + y));
			interactWithEntranceGuard(scene);
			break;
		case 5:
			updateCurrentActiveNPC(scene, findNPCwithCoordinate(scene, player->position.x + x, player->position.y + y));
			interactWithPrincipal(scene);
			break;
		}
		return 0;
	}
	else if (game->mapData[indexOfObjectAhead + 2] - '0' == 2) {
		// The following if elses check if the area the player is going to move to is one of the hidden areas
		// If it is, then change that area's data to not hidden so it will render with the area not hidden
		if (player->position.x + x >= 11 && player->position.x + x <= 44) {
			if (player->position.y + y >= 1 && player->position.y + y <= 6) {
				changeMapState(game, 2, 1);
			}
		}
		else if (player->position.x + x >= 62 && player->position.x + x <= 86) {
			if (player->position.y + y >= 39 && player->position.y + y <= 52) {
				changeMapState(game, 4, 1);
			}
		}
		else if (player->position.x + x >= 138 && player->position.x + x <= 159) {
			if (player->position.y + y >= 19 && player->position.y + y <= 28) {
				changeMapState(game, 7, 1);
			}
		}
		return 1;
	}
	else {
		if (game->mapData[indexOfObjectAhead + 3] - '0' != 9) {
			player->position.areaID = game->mapData[indexOfObjectAhead + 3] - '0';
		}
	}
	if (game->mapData[indexOfObjectAhead + 2] - '0' == 0) { // Refer to documentation. A [0] attached data in the third index refers to an area that can't be moved to
		return 0;
	}
	return 1;
}

void addItemInInventory(Player* player, int id, int type, int statModifier, int quantity) {
	// Add item to inventory
	if (player->itemsNumber+1 < 13) {
		player->inventory[player->itemsNumber].id = id;
		player->inventory[player->itemsNumber].type = type;
		player->inventory[player->itemsNumber].statModifier = statModifier;
		player->inventory[player->itemsNumber].quantity = quantity;
		if (player->inventory[player->itemsNumber].type == 1) {
			player->stats.Charisma += statModifier;
		}
		else if (player->inventory[player->itemsNumber].type == 2) {
			player->stats.Pesos += statModifier;
		}
		++(player->itemsNumber);
	}
}

void removeItemInInventory(Player* player, int itemIndex) {
	if (player->inventory[itemIndex].id == 0 || player->inventory[itemIndex].id == -1) { return; }
	for (int i = 0; i < player->itemsNumber; i++) {
		if (i == itemIndex) {
			player->inventory[i] = player->inventory[i + 1];
			Items temp = {-1, -1, -1, -1};
			player->inventory[i + 1] = temp;
			break;
		}
	}
	--player->itemsNumber;
}


char* randomDialougeStudent() {
	int* dialouge = '\0';
	srand(time(NULL));
	int r = 1 + (rand() % 2);
	switch (r) {
		case 1:
			dialouge = (char*)malloc(sizeof(char)*(strlen("Oh, how are you?")));
			strcpy(dialouge, "Oh, how are you?");
			break;
		case 2:
			dialouge = (char*)malloc(sizeof(char) * (strlen("Hello, good weather today huh.")));
			strcpy(dialouge, "Hello, good weather today huh.");
			break;
	}
	return dialouge;
}


int readInput(GameManager* game, SceneManager *scene, Player *player) {
	// Read the char input from the user. Keys are assigned to specific tasks
	char userInput = _getch();
	if (scene->isInteractionActive == 0) {
		if (userInput == game->keybinds.moveUp) {
			if (canPlayerMoveThere(player, scene, game, 0, -1) == 1) {
				changeposition(player, 0, -1);
			}
		}
		else if (userInput == game->keybinds.moveLeft) {
			if (canPlayerMoveThere(player, scene, game, -1, 0) == 1) {
				changeposition(player, -1, 0);
			}
		}
		else if (userInput == game->keybinds.moveDown) {
			if (canPlayerMoveThere(player, scene, game, 0, 1) == 1) {
				changeposition(player, 0, 1);
			}
		}
		else if (userInput == game->keybinds.moveRight) {
			if (canPlayerMoveThere(player, scene, game, 1, 0) == 1) {
				changeposition(player, 1, 0);
			}
		}
	}
	else {
		if (userInput == game->keybinds.moveUp && scene->activePrompt.currentOption != 0) {
			scene->activePrompt.currentOption -= 1;
		}
		else if (userInput == game->keybinds.moveDown && scene->activePrompt.currentOption != scene->activePrompt.numberOfOptions - 1) {
			scene->activePrompt.currentOption += 1;
		}
		else if (userInput == ' ') {
			Option currentOption = scene->activePrompt.option[scene->activePrompt.currentOption];
			if (currentOption.pointedDialougeID != -5) {
				currentOption.eventAction(game, scene, player, currentOption.pointedDialougeID);
			}
		}
	}
	if (userInput == 'b') {
		if (game->debugMode == 0) { game->debugMode = 1; }
		else game->debugMode = 0;
	}
	if (userInput == 'r') {
		return -1;
	}
	switch (userInput) {
	case '1':
		removeItemInInventory(player, 0);
		break;
	case '2':
		removeItemInInventory(player, 1);
		break;
	case '3':
		removeItemInInventory(player, 2);
		break;
	case '4':
		removeItemInInventory(player, 3);
		break;
	case '5':
		removeItemInInventory(player, 4);
		break;
	case '6':
		removeItemInInventory(player, 5);
		break;
	case '8':
		removeItemInInventory(player, 6);
		break;
	case '9':
		removeItemInInventory(player, 7);
		break;
	}
	if (game->debugMode == 1) {
		if (userInput == 'k') { // Student
			// To be implemenmted template for NPC spawning
			char* npcDialouges[1] = { '\0' };
			npcDialouges[0] = randomDialougeStudent();

			spawnNPC(game, scene, NULL, 0, 8, 29, -1, 9, npcDialouges, 1);
		}
		if (userInput == '4') {
			changeposition(player, -1, 0);
		}
		if (userInput == '8') {
			changeposition(player, 0, -1);
		}
		if (userInput == '6') {
			changeposition(player, 1, 0);
		}
		if (userInput == '2') {
			changeposition(player, 0, 1);
		}
	}

	return 1;
}


void updateTime(GameManager *game) {
	// Update the time with 540 seconds as the starting time in order for the game to start at 9:00
	game->gameTime.timeInSeconds = 540 + (time(NULL) - game->gameTime.timeWhenDayStarted) * 6;
	game->gameTime.timeInMinutes = game->gameTime.timeInSeconds / 60;
	if (game->gameTime.timeInSeconds >= 1020) {
		game->gameTime.timeWhenDayStarted = time(NULL);
		++game->gameTime.day;
		if (game->gameTime.day == 5) {
			game->gameState = 2;
		}
	}
}

void debugMode(Player* player, GameManager* game, SceneManager *scene) {
	int relativeTopLeftCoordinate[2] = { player->position.x - (CAMERA_WIDTH - 1) / 2, player->position.y - (CAMERA_HEIGHT - 1) / 2 };
	printf("________________________________________\n");
	printf("Player Absolute Coordinate: (%d, %d)\n", player->position.x, player->position.y);
	printf("Camera Relative Coordinate: (%d, %d)\n", relativeTopLeftCoordinate[0], relativeTopLeftCoordinate[1]);
	printf("Time: %d\n", game->gameTime.timeInSeconds);
	printf("Number of NPCs: %d\n", scene->numberOfNPC);
	printf("Principal Location: %s\n", areaIDtoString(scene->npcList[0].coordinate.areaID));
	printf("Principal Location Data Index 2: %c\n", game->mapData[calculateIndexFromCoordinate(scene->npcList[0].coordinate.x, scene->npcList[0].coordinate.y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)]);
	printf("Hallway Requirements: [%d, %d, %d]\n", game->hallwayRequirements[0].type, game->hallwayRequirements[1].type, game->hallwayRequirements[2].type);
	printf("________________________________________\n");
}

void changeMapState(GameManager *game, int areaID, int isHide) {
	int x1 = 0, y1 = 0, x2 = 0, y2=0;
	switch (areaID) {
	case 2: // Set the bounds of the coordinate to the Principal's Office area
		x1 = 11;
		y1 = 1;
		x2 = 44;
		y2 = 6;
		break;
	case 4: // Garden
		x1 = 62;
		y1 = 39;
		x2 = 86;
		y2 = 52;
		break;
	case 7: // Faculty Office
		x1 = 138;
		y1 = 19;
		x2 = 159;
		y2 = 28;
		break;
	}

	// Iliterates through the map data equivalent within the set bound and set all their first data index to 2 to indicate it needs to be hidden
	for (int y = y1; y <= y2; y++) {
		for (int x = x1; x <= x2; x++) {
			game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)] = isHide + '0';
		}
	}
}

int initiateDatas(GameManager *game) {
	// Open the txt files containing the data for the map and user interface, then  store it into the game manager structure as a member
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

void initiateGuardNPCspawn(GameManager *game, SceneManager *scene) {
	char* npcDialouges[3] = { '\0' };
	npcDialouges[0] = "What do you want Child?";
	npcDialouges[1] = "I can't tell you.";
	switch (scene->npcList[0].coordinate.areaID) {
	case 2:
		npcDialouges[2] = "The principal is at the [PRINCIPAL'S OFFICE]";
		break;
	case 4:
		npcDialouges[2] = "The principal is at the [GARDEN]";
		break;
	case 7:
		npcDialouges[2] = "The principal is at the [FACULTY OFFICE]";
		break;
	default:
		npcDialouges[2] = "Principal AreaID not found";
		break;
	}

	srand(time(NULL));
	int requirementTypeRandomizer = 1 + (rand() % 3);
	int statRequiredRandomizer = 25 + rand() % (50 - 25+1);
	Requirement req[] = { {0, requirementTypeRandomizer, 1, statRequiredRandomizer} };
	spawnNPC(game, scene, req, 1, 16, 31, -1, 7, npcDialouges, 3);
}

void initiatePrincipalNPCspawn(GameManager* game, SceneManager* scene) {
	srand(time(NULL));
	int spawnRandomizer = 1 + (rand() % 3);
	char* npcDialouges[1] = { '\0' };
	int x = 0, y = 0, areaID = 0;
	switch (spawnRandomizer) {
	case 1: // Principal Office Spawn
		x = 27;
		y = 2;
		areaID = 2;
		break;
	case 2: // Garden Spawn
		x = 67;
		y = 48;
		areaID = 4;
		break;
	case 3: // Faculty Office Spawn
		x = 155;
		y = 22;
		areaID = 7;
		break;
	}
	npcDialouges[0] = "What? You want to complain about your grades?";

	spawnNPC(game, scene, NULL, 0, x, y, areaID, 5, npcDialouges, 1);
	changeMapState(game, 2, 2);
	changeMapState(game, 4, 2);
	changeMapState(game, 7, 2);
}

void initiatePlayerStats(GameManager *game, Player *player, SceneManager *scene) {
	srand(time(NULL));
	int moneyRandomizer = rand() % 20;
	int charismaRandomizer = rand() % 10;
	player->stats.Pesos = 5 + moneyRandomizer;
	player->stats.Charisma = 10 + charismaRandomizer;
}

void deleteCurrentActiveNPC(GameManager *game, SceneManager *scene) {
	NPC* temp = (NPC*)malloc(sizeof(NPC)*scene->numberOfNPC);
	for (int i = 0; i < scene->numberOfNPC; i++) {
		if (isTwoCoordinatesTheSame(scene->currentActiveNPC.coordinate.x, scene->currentActiveNPC.coordinate.y, scene->npcList[i].coordinate.x, 
			scene->npcList[i].coordinate.y) == 1) {
			temp[i].id = -1;
			continue;
		}
		temp[i] = scene->npcList[i];
	}
	scene->numberOfNPC -= 1;
	scene->npcList = (NPC*)realloc(scene->npcList, sizeof(NPC) * scene->numberOfNPC);
	for (int i = 0, j = 0; i < scene->numberOfNPC; i++) {
		if (temp[i].id == -1) {
			j = 1;
		}
		scene->npcList[i] = temp[i+j];
	}

	game->mapData[calculateIndexFromCoordinate(scene->currentActiveNPC.coordinate.x, scene->currentActiveNPC.coordinate.y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] = 0 + '0';
	game->mapData[calculateIndexFromCoordinate(scene->currentActiveNPC.coordinate.x, scene->currentActiveNPC.coordinate.y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)] = 1 + '0';
}

void reducePlayerStatRandomly(GameManager *game, SceneManager *scene, Player *player) {
	if (game->gameTime.timeSnapshot[1] == 0) {
		game->gameTime.timeSnapshot[1] = time(NULL);
	}
	if (time(NULL) - game->gameTime.timeSnapshot[1] >= 10) {
		srand(time(NULL));
		int willPlayerLostStatRandomizer = 1 + (rand() % 100);
		int statLosingRandomizer = 1 + (rand() % 2);
		int moneyLossRandomizer = 10 + (rand() % 10);
		int charismaLossRandomizer = 5 + (rand() % 5);
		if (willPlayerLostStatRandomizer >= 1 && willPlayerLostStatRandomizer <= 100 - player->stats.mentalHealth) {
			switch (statLosingRandomizer) {
			case 1: // Loss of PHP
				scene->lostPHP = moneyLossRandomizer;
				if (player->stats.Pesos - moneyLossRandomizer < 0) { 
					player->stats.Pesos = 0;
					break; 
				}
				player->stats.Pesos -= moneyLossRandomizer;
				break;
			case 2: // Loss of Charisma
				scene->lostCHA = charismaLossRandomizer;
				if (player->stats.Charisma - charismaLossRandomizer < 0) {
					player->stats.Charisma = 0;
					break;
				}
				player->stats.Charisma -= charismaLossRandomizer;
				break;
			}
			game->gameTime.timeSnapshot[0] = time(NULL);
			game->gameTime.timeSnapshot[1] = 0;
		}
	}
}

void spawnItem(GameManager *game, SceneManager *scene, DroppedItem item) {
	if (game->mapData[calculateIndexFromCoordinate(item.coordinate.x, item.coordinate.y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] == 9 + '0') { return; }
	if (game->mapData[calculateIndexFromCoordinate(item.coordinate.x, item.coordinate.y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] == 1 + '0') { return; }
	game->mapData[calculateIndexFromCoordinate(item.coordinate.x, item.coordinate.y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] = 1 + '0';
	++game->itemNumberInMap;
	for (int i = 0; i < game->itemNumberInMap; i++) {
		if (game->itemList[i].id == 0) {
			game->itemList[i] = item;
			break;
		}
	}

}


void spawnGuardKey(GameManager *game, SceneManager *scene) {
	srand(time(NULL));
	int spawnRandomizer = rand() % 3;
	DroppedItem item;
	item.id = 1;
	switch (spawnRandomizer) {
	case 0:
		item.coordinate.x = 163;
		item.coordinate.y = 47;
		spawnItem(game, scene, item);
		break;
	case 1:
		item.coordinate.x = 92;
		item.coordinate.y = 12;
		spawnItem(game, scene, item);
		break;
	case 2:
		item.coordinate.x = 51;
		item.coordinate.y = 4;
		spawnItem(game, scene, item);
		break;
	}
}

void changeAreaDataState(GameManager* game, int x, int y, int dataState) {
	game->mapData[calculateIndexFromCoordinate(x, y, TOTAL_WIDTH_MAP, MAP_DATASIZE, 0)] = dataState + '0';
}
void changeAreaMoveableState(GameManager *game, int x1, int y1, int x2, int y2, int isBlock, int dataState) {
	for (int i = y1; i <= y2; i++) {
		for (int j = x1; j <= x2; j++) {
			game->mapData[calculateIndexFromCoordinate(j, i, TOTAL_WIDTH_MAP, MAP_DATASIZE, 2)] = isBlock + '0';
			changeAreaDataState(game, j, i, dataState);
		}
	}
}

void initiateHallwayRequirements(GameManager *game) {
	int typeRandomizer = 0;
	int statRequirementRandomizer = 0;
	int itemTypeRandomizer = 0;
	for (int i = 0; i < 3; i++) {
		srand(time(NULL) + (i*time(NULL)));
		typeRandomizer = 1 + rand() % 3;
		statRequirementRandomizer = 40 + rand() % 41; // 40 to 80
		switch (typeRandomizer) {
			case 1: // Item Requirement
				itemTypeRandomizer = 2 + rand() % 5; // 2 to 6
				game->hallwayRequirements[i].type = 1;
				game->hallwayRequirements[i].itemID = itemTypeRandomizer;
				break;
			case 2: // PHP Requirement
				game->hallwayRequirements[i].type = 2;
				game->hallwayRequirements[i].statRequired = statRequirementRandomizer;
				break;
			case 3: // CHA Requirement
				game->hallwayRequirements[i].type = 3;
				game->hallwayRequirements[i].statRequired = statRequirementRandomizer;
				break;
		}
	}
}

void itemRandomSpawning(GameManager *game, Player *player, SceneManager *scene) {
	srand(time(NULL));
	int areaRandomizer = rand() % 6;
	int itemRandomizer = 2 + rand() % 9; // 2 to 10
	int priorityItemRandomizer = rand() % 3;
	DroppedItem Item;
	Item.id = itemRandomizer;
	if (game->gameTime.timeSnapshot[7] == 0) {
		game->gameTime.timeSnapshot[7] = time(NULL);
	}
	if (game->hasRequiredItemSpawned < 3) {
		for (int i = game->hasRequiredItemSpawned; i < 3; i++) {
			if (game->hallwayRequirements[i].type == 1) {
				Item.id = game->hallwayRequirements[i].itemID;
				++game->hasRequiredItemSpawned;
				break;
			}
		}
	}
	if (time(NULL) - game->gameTime.timeSnapshot[7] >= 10) {
		switch (areaRandomizer) {
		case 0: 
			Item.coordinate.x = 1;
			Item.coordinate.y = 29;
			spawnItem(game, scene, Item);
			break;
		case 1:
			Item.coordinate.x = 163;
			Item.coordinate.y = 39;
			spawnItem(game, scene, Item);
			break;
		case 2: 
			Item.coordinate.x = 117;
			Item.coordinate.y = 44;
			spawnItem(game, scene, Item);
			break;
		case 3: 
			Item.coordinate.x = 70;
			Item.coordinate.y = 15;
			spawnItem(game, scene, Item);
			break;
		case 4:
			Item.coordinate.x = 25;
			Item.coordinate.y = 50;
			spawnItem(game, scene, Item);
			break;
		case 5:
			Item.coordinate.x = 36;
			Item.coordinate.y = 13;
			spawnItem(game, scene, Item);
			break;
		}
		game->gameTime.timeSnapshot[7] = 0;
		
	}
}

void printMainMenu(GameManager *game) {
	printf("SOLO\n");
	printf("INFILTRATION TO\n");
	printf("MEET THE\n");
	printf("PRINCIPAL\n");
	printf("\n\n\n");
	printf("[Press any button to start]");
	char chr = _getch();
	game->gameState = 1;
}

void initiateGame(GameManager *game, Player *player, SceneManager *scene) {
	updateCameraRelativeCoordinate(game, &player);
	updateCurrentActivePrompt(scene, "Objective: Meet the Principal", -1, 0);
	printf("\33[?25l"); // Hide the cursor
	//printf("\33[?25h"); // Re enable cursor
	if (initiateDatas(game) == 0) { printf(ANSI_COLOR_RED "{No Map Data}"); return; };

	initiatePrincipalNPCspawn(game, scene);
	initiateGuardNPCspawn(game, scene);
	initiatePlayerStats(game, player, scene);
	initiateHallwayRequirements(game);

	changeAreaMoveableState(game, 25, 8, 30, 8, 0, 2);
	changeAreaMoveableState(game, 71, 37, 76, 37, 0, 2);
	changeAreaMoveableState(game, 142, 30, 147, 30, 0, 2);

	if (scene->npcList[1].requirements[0].type == 1) {
		spawnGuardKey(game, scene);
	}
}

void tryAgainScreen(GameManager *game) {
	clearConsole();
	printf("Oh no... you ran out of time...\n\n\n\n");
	printf("[Press Space to Retry]\n");
	printf("[Press n to exit]\n");
	char userInput = _getch();
	if (userInput == ' ') {
		game->gameState = 3;
		clearConsole();
	}
	else if (userInput == 'n') {
		game->isGameRunning = 0;
	}
}

Player initiatePlayerManager() {
	Player player = {
		{100, 50, 30}, // Stats: MH, PHP, CHA
		{0}, // Items
		{2, 31, 0}, // Position: x, y, areaID
		0 // Number of Items
	};
	return player;
}
GameManager initiateGameManager() {
	GameManager game = {
		{DEFAULT_MOVE_UP, DEFAULT_MOVE_LEFT, DEFAULT_MOVE_DOWN, DEFAULT_MOVE_RIGHT}, // Navigation keybinds
		{{0}, 0, 0, 1, time(NULL)}, // Game Time
		{0}, // Item List in Map,
		{0}, // Hallway Requirements
		0, // Item ID required
		0, // PHP Required
		0, // CHA Required
		0, // Total Item in Map
		{0, 0}, // Relative Camera Coordinate
		(char*)malloc(sizeof(char)), // Map Data Initial Allocation
		(char*)malloc(sizeof(char)), // UI Data Initial Allocation
		1, // Game Running Indicator
		0, // Debug Mode
		0, // Game State
		0 // Has required item spawned
	};
	return game;
}

SceneManager initiateSceneManager() {
	SceneManager scene = {
		NULL, // NPC List Initial Allocation
		NULL, // Current NPC Interacted Initialization
		NULL, // Current Active Prompt Initialization
		0, // Current Lost CHA
		0, // Current lost PHP
		0, // Current gain CHA
		0, // Current gain PHP
		0, // Number of NPC Active in Map
		0 // Interaction Between Player to NPC Active {0 = false, 1=true}
	};
	return scene;
}
int main(void) {
	Player player = initiatePlayerManager();
	GameManager game = initiateGameManager();
	SceneManager scene = initiateSceneManager();

	while (game.isGameRunning == 1) {
		if (game.gameState == 0) { // Main Menu
			printMainMenu(&game);
			clearConsole();
			initiateGame(&game, &player, &scene);
			updateTime(&game);
			renderUI(&game, &scene, &player);
		}
		else if (game.gameState == 1) { // Main game
			clearConsole(); //Fixed the hallway requirement not reading item in inventory
			if (game.debugMode == 1) { debugMode(&player, &game, &scene); }
			renderUI(&game, &scene, &player);
			reducePlayerStatRandomly(&game, &scene, &player);
			spawnStudentNPC(&game, &scene);
			itemRandomSpawning(&game, &player, &scene);
			updateTime(&game);
			if (readInput(&game, &scene, &player) == -1) { game.gameState = 3; clearConsole(); continue; }
		}
		else if (game.gameState == 2) { // Try Again Screen
			tryAgainScreen(&game);
		}
		else if (game.gameState == 3) { // Restart Game
			player = initiatePlayerManager();
			game = initiateGameManager();
			scene = initiateSceneManager();
			initiateGame(&game, &player, &scene);
			updateTime(&game);
			renderUI(&game, &scene, &player);
			game.gameState = 1;
		}
	}
	free(scene.activePrompt.option);
	free(scene.npcList);
	free(game.interfaceData);
	free(game.mapData);
	free(scene.activePrompt.dialouge);
}

// TODO
// Item may spawn randomly in the map, the item that will spawn should be required by one of the NPCs
// Guard Random Requirement
// Student giving stats
// Guard blocking the way to a potential principal spot which requires random requirement