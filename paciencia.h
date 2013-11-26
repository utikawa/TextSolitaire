#ifndef PACIENCIA_H_INCLUDED
#define PACIENCIA_H_INCLUDED

#include "baralho.h"

#define GAME_SLOT_NUM 7

#define GAME_SLOT_CLOSED 0
#define GAME_SLOT_OPEN   1

#define GAME_CMD_NONE     0
#define GAME_CMD_QUIT     1
#define GAME_CMD_CARD_NEW 2
#define GAME_CMD_MOVE     3

#define CARD_REF_INVALID    0
#define CARD_REF_OPEN       1
#define CARD_REF_FOUNDATION 2
#define CARD_REF_SLOT       3

typedef struct {
	Card     *pCard;
	CardList *pDeck, *pWaste, *pFoundation[4], *pSlots[GAME_SLOT_NUM][2];
} GameState;

typedef struct {
	int iRef;
	int iColumn;
	int iIndex;
} GameCardRef;

GameState  * Game_Create        (void);
void         Game_Free          (GameState *pGame);
int          Game_Load          (GameState *pGame, char *szFileName);
int          Game_Save          (GameState *pGame, char *szFileName);
CardList  ** Game_GetCardList   (GameState *pGame, GameCardRef *pRef);
int          Game_CanMove       (Card *pFromCard, Card *pToCard, int iToRef);
void         Game_ProcessCommand(GameState **pGame, int iCommand, GameCardRef *pFrom, GameCardRef *pTo);
int          Game_Finished      (GameState **pGame);

#endif

