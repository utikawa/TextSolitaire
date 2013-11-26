#include <string.h>
#include <malloc.h>

#include "paciencia.h"

static const unsigned int FILE_MAGIC = 0x1248a50f;

GameState * Game_Create(void)
{
	GameState *pGame;
	int iCount, iSlot;

	pGame = (GameState *)malloc(sizeof(GameState));
	if(pGame == NULL) return NULL;

	memset(pGame, 0, sizeof(GameState));

	CardList_CreateDeck(&(pGame->pDeck), DECK_TYPE_FULL);
	CardList_Shuffle   (&(pGame->pDeck), 0);

	for(iSlot = 0; iSlot < GAME_SLOT_NUM; iSlot++) {
		pGame->pCard = CardList_GetCard(&(pGame->pDeck));
		CardList_InsertCard(&(pGame->pSlots[iSlot][GAME_SLOT_OPEN]), pGame->pCard);
		for(iCount = iSlot; iCount > 0; iCount--) {
			pGame->pCard = CardList_GetCard(&(pGame->pDeck));
			CardList_InsertCard(&(pGame->pSlots[iSlot][GAME_SLOT_CLOSED]), pGame->pCard);
		}
	}

	pGame->pCard = CardList_GetCard(&(pGame->pDeck));

	return pGame;
}

void Game_Free(GameState *pGame)
{
	int iCount;

	if(pGame == NULL) return;

	Card_Free(pGame->pCard);
	CardList_Free(&pGame->pDeck);
	CardList_Free(&pGame->pWaste);

	for(iCount = 0; iCount < 4; iCount++) {
		CardList_Free(&pGame->pFoundation[iCount]);
	}

	for(iCount = 0; iCount < GAME_SLOT_NUM; iCount++) {
		CardList_Free(&pGame->pSlots[iCount][GAME_SLOT_OPEN  ]);
		CardList_Free(&pGame->pSlots[iCount][GAME_SLOT_CLOSED]);
	}

	free(pGame);
}

static int Game_LoadCardList(FILE *pFile, CardList **pList)
{
	int iSize, iCount;
	Card tCard;

	if(pFile == NULL || pList == NULL) return -1;

	*pList = NULL;
	fread(&iSize, sizeof(iSize), 1, pFile);

	for(iCount = 0; iCount < iSize; iCount++) {
		fread(&tCard, sizeof(Card), 1, pFile);
		CardList_InsertCard(pList, Card_Create(tCard.iValue, tCard.eSuit));
	}

	return iSize;
}

static void Game_WriteCardList(FILE *pFile, CardList *pList)
{
	CardList *pCurrent;
	int iCount, iSize;

	if(pFile == NULL) return;

	// Grava a quantidade de cartas na lista
	iSize = CardList_GetSize(&pList);
	fwrite(&iSize, sizeof(iSize), 1, pFile);

	// Grava as cartas existentes na lista
	for(iCount = 0; iCount < iSize; iCount++) {
		pCurrent = CardList_GetPosition(&pList, NULL, iCount);
		fwrite(pCurrent->pCard, sizeof(Card), 1, pFile);
	}
}

int Game_Load(GameState *pGame, char *szFileName)
{
	FILE         *pFile;
	int           iCount;
	unsigned int  iMagic;

	if(pGame == NULL || szFileName == NULL) return 0;

	pFile = fopen(szFileName, "rb");
	if(pFile == NULL) return 0;

	// Carrega um int e verifica se corresponde ao numero magico
	fread(&iMagic, sizeof(iMagic), 1, pFile);
	if(iMagic != FILE_MAGIC) {
		fclose(pFile);
		return 0;
	}

	// Cria uma carta "falsa" apenas para que seja alocada e inicializada a memoria para a carta
	// Em seguida le a carta do arquivo.
	pGame->pCard = Card_Create(CARD_VALUE_ACE, Diamond);
	fread(pGame->pCard, sizeof(Card), 1, pFile);

	// Carrega as fundacoes
	for(iCount = 0; iCount < 4; iCount++) {
		Game_LoadCardList(pFile, &pGame->pFoundation[iCount]);
	}

	// Carrega os slots
	for(iCount = 0; iCount < GAME_SLOT_NUM; iCount++) {
		Game_LoadCardList(pFile, &pGame->pSlots[iCount][GAME_SLOT_OPEN  ]);
		Game_LoadCardList(pFile, &pGame->pSlots[iCount][GAME_SLOT_CLOSED]);
	}

	// Carrega o baralho
	Game_LoadCardList(pFile, &pGame->pDeck);

	// Carrega a pilha de descartes
	Game_LoadCardList(pFile, &pGame->pWaste);

	fclose(pFile);


	return 1;
}

int Game_Save(GameState *pGame, char *szFileName)
{
	FILE     *pFile;
	int       iCount;

	if(pGame == NULL || szFileName == NULL) return 0;

	pFile = fopen(szFileName, "wb");
	if(pFile == NULL) return 0;

	// Grava o numero magico para identificarmos se eh um arquivo valido
	fwrite(&FILE_MAGIC, sizeof(FILE_MAGIC), 1, pFile);

	// Grava a carta atual
	fwrite(pGame->pCard, sizeof(Card), 1, pFile);

	// Grava as fundacoes
	for(iCount = 0; iCount < 4; iCount++) {
		Game_WriteCardList(pFile, pGame->pFoundation[iCount]);
	}

	// Grava os slots
	for(iCount = 0; iCount < GAME_SLOT_NUM; iCount++) {
		Game_WriteCardList(pFile, pGame->pSlots[iCount][GAME_SLOT_OPEN  ]);
		Game_WriteCardList(pFile, pGame->pSlots[iCount][GAME_SLOT_CLOSED]);
	}

	// Grava o baralho
	Game_WriteCardList(pFile, pGame->pDeck);

	// Grava a pilha de descartes
	Game_WriteCardList(pFile, pGame->pWaste);

	fclose(pFile);

	return 1;
}

CardList ** Game_GetCardList(GameState *pGame, GameCardRef *pRef)
{
	if(pGame != NULL && pRef != NULL) {
		switch(pRef->iRef) {
			case CARD_REF_FOUNDATION:
				if(pRef->iColumn >= 0 && pRef->iColumn < 4)
					return &pGame->pFoundation[pRef->iColumn];
				break;

			case CARD_REF_SLOT:
				if(pRef->iColumn >= 0 && pRef->iColumn < GAME_SLOT_NUM)
					return &pGame->pSlots[pRef->iColumn][GAME_SLOT_OPEN];
				break;
		}
	}

	return NULL;
}

int Game_CanMove(Card *pFromCard, Card *pToCard, int iToRef)
{
	int iCanMove = 0, iFromIsBlack, iToIsBlack;

	if(pFromCard != NULL) {
		iFromIsBlack = Card_IsBlack(pFromCard);
		iToIsBlack   = Card_IsBlack(pToCard  );

		if(iToRef == CARD_REF_FOUNDATION) {
			if(pToCard == NULL && Card_GetValue(pFromCard) == CARD_VALUE_ACE) {
				iCanMove = 1;
			} else if(Card_GetValue(pFromCard) == Card_GetValue(pToCard)+1 &&
					pFromCard->eSuit == pToCard->eSuit) {
				iCanMove = 1;
			}
		} else if(iToRef == CARD_REF_SLOT) {
			if(pToCard == NULL) {
				if(Card_GetValue(pFromCard) == 13) {
					iCanMove = 1;
				}
			} else if(Card_GetValue(pFromCard) == Card_GetValue(pToCard)-1 &&
				  iFromIsBlack != iToIsBlack) {
				iCanMove = 1;
			}
		}
	}

	return iCanMove;
}

void Game_ProcessCommand(GameState **pGame, int iCommand, GameCardRef *pFrom, GameCardRef *pTo)
{
	int iCount;
	Card      *pFromCard,  *pToCard;
	CardList **pFromList, **pToList;
	if(pGame == NULL || *pGame == NULL || pFrom == NULL) return;

	switch(iCommand) {
		case GAME_CMD_CARD_NEW:
			if(pFrom->iRef == CARD_REF_OPEN) {
				if((*pGame)->pCard != NULL) {
					CardList_InsertCard(&(*pGame)->pWaste, (*pGame)->pCard);
				}

				if(CardList_GetSize(&(*pGame)->pDeck) == 0) {
					for(iCount = CardList_GetSize(&(*pGame)->pWaste); iCount > 0; iCount--) {
						CardList_InsertCard(&(*pGame)->pDeck, CardList_GetCard(&(*pGame)->pWaste));
					}
				}

				(*pGame)->pCard = CardList_GetCard(&(*pGame)->pDeck);
			} else if(pFrom->iRef == CARD_REF_SLOT) {
				pFromList = Game_GetCardList(*pGame, pFrom);
				if(pFromList != NULL && *pFromList == NULL) {
					pFromCard = CardList_GetCard(&(*pGame)->pSlots[pFrom->iColumn][GAME_SLOT_CLOSED]);
					if(pFromCard != NULL)
						CardList_InsertCard(pFromList, pFromCard);
				}
			}

			break;

		case GAME_CMD_MOVE:
			if(pTo == NULL || pTo->iRef == CARD_REF_OPEN) break;

			pToList = Game_GetCardList(*pGame, pTo);

			pFromCard = NULL;
			pToCard   = NULL;
			pFromList = NULL;

			if(pToList != NULL) {
				if(*pToList != NULL) {
					pToCard = CardList_GetPosition(pToList, NULL, -1)->pCard;
				}

				if(pFrom->iRef == CARD_REF_OPEN) {
					pFromCard = (*pGame)->pCard;
				} else {
					pFromList = Game_GetCardList(*pGame, pFrom);

					if(pFromList != NULL && *pFromList != NULL) {
						if(*pToList != NULL) {
							iCount = CardList_GetSize(pFromList);
							while(iCount-- > 0) {
								pFromCard = CardList_GetPosition(pFromList, NULL, iCount)->pCard;
								if(Game_CanMove(pFromCard, pToCard, pTo->iRef))
									break;
							}
							if(iCount < 0) {
								pFromCard = NULL;
							}
						} else {
							pFromCard = CardList_GetPosition(pFromList, NULL, pTo->iRef == CARD_REF_SLOT ? 0 : -1)->pCard;
						}
					}
				}
			}

			if(pFromCard != NULL && pToList != NULL && Game_CanMove(pFromCard, pToCard, pTo->iRef)) {
				if(pFromList != NULL) {
					CardList_InsertCardList(pToList, CardList_GetSubset(pFromList, pFromCard));
				} else {
					CardList_InsertCard(pToList, pFromCard);
				}

				if(pFrom->iRef == CARD_REF_OPEN) {
					(*pGame)->pCard = CardList_GetCard(&(*pGame)->pWaste);
					if((*pGame)->pCard == NULL) {
						Game_ProcessCommand(pGame, GAME_CMD_CARD_NEW, NULL, NULL);
					}
				}
			}
	}
}

int Game_Finished(GameState **pGame)
{
	int iCount;
	CardList *pList;
	if(pGame == NULL || *pGame == NULL) return 0;

	for(iCount = 0; iCount < 4; iCount ++) {
		pList = CardList_GetPosition(&(*pGame)->pFoundation[iCount], NULL, -1);
		if(pList == NULL || Card_GetValue(pList->pCard) != 13)
			return 0;
	}

	return 1;
}

