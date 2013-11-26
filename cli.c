#include <stdio.h>
#include <malloc.h>
#include <string.h>

#include "paciencia.h"

#define GAME_KEY_CARD_NEW  '+'
#define GAME_KEY_CARD_OPEN '*'
#define GAME_KEY_HELP      '?'
#define GAME_KEY_QUIT      'X'
#define GAME_KEY_NEW       'N'
#define GAME_KEY_SAVE      'S'
#define GAME_KEY_LOAD      'L'

#define CARD_STRING_CLOSED "|###|"
#define CARD_STRING_EMPTY  "|   |"
#define CARD_STRING_NONE   "     "

void PrintCard(Card *pCard)
{
	char  szCardString[6];
	char *szCard[] = { "??", " A", " 2", " 3", " 4", " 5", " 6", " 7", " 8", " 9", "10", " J", " Q", " K" }, cSuit = '?';

	if(pCard != NULL) {
		switch(pCard->eSuit) {
			case Diamond: cSuit = 'o'; break;
			case Spade  : cSuit = 'e'; break;
			case Heart  : cSuit = 'c'; break;
			case Club   : cSuit = 'p'; break;
		}

		sprintf(szCardString, "|%s%c|", szCard[Card_GetValue(pCard)], cSuit);
	} else {
		strcpy(szCardString, CARD_STRING_EMPTY);
	}

	printf("%s", szCardString);
}

void Game_PrintBoard(GameState **pGame)
{
	Card     *pCard;
	CardList *pSlotList;
	int iCount, iSlotSize[GAME_SLOT_NUM], iSlot, iCardPrinted;

	if(pGame == NULL || *pGame == NULL) return;

	printf("\n\n");

	if(CardList_GetSize(&(*pGame)->pDeck) > 0) {
		printf("%s ", CARD_STRING_CLOSED);
	} else {
		printf("%s ", CARD_STRING_EMPTY );
	}

	PrintCard((*pGame)->pCard);
	printf(" %s ", CARD_STRING_NONE);

	for(iCount=0; iCount<4; iCount++) {
		pCard = NULL;
		if((*pGame)->pFoundation[iCount] != NULL) {
			pCard = CardList_GetPosition(&(*pGame)->pFoundation[iCount], NULL, -1)->pCard;
		}

		PrintCard(pCard);
		printf(" ");
	}

	printf("\n|(%c)| |(%c)|       |(a)| |(b)| |(c)| |(d)|\n\n", GAME_KEY_CARD_NEW, GAME_KEY_CARD_OPEN);

	for(iSlot = 0; iSlot < GAME_SLOT_NUM; iSlot++) {
		printf("|(%d)| ", iSlot);
		iSlotSize[iSlot] = CardList_GetSize(&(*pGame)->pSlots[iSlot][GAME_SLOT_CLOSED]);
	}

	printf("\n");

	iCount = 0;
	do {
		iCardPrinted = 0;
		
		for(iSlot = 0; iSlot < GAME_SLOT_NUM; iSlot++) {
			if(iCount < iSlotSize[iSlot]) {
				iCardPrinted = 1;
				printf("%s ", CARD_STRING_CLOSED);
			} else {
				pSlotList = CardList_GetPosition(&(*pGame)->pSlots[iSlot][GAME_SLOT_OPEN], NULL, iCount - iSlotSize[iSlot]);
				if(pSlotList != NULL) {
					iCardPrinted = 1;
					PrintCard(pSlotList->pCard);
					printf(" ");
				} else {
					printf("%s ", CARD_STRING_NONE);
				}
			}
		}

		iCount++;
		printf("\n");
	} while(iCardPrinted);

	printf("\n");
}

void Game_GetRefFromKey(GameCardRef *pRef, char cKey)
{
	if(pRef == NULL) return;

	if(cKey >= '0' && cKey <= '9') { // Maximo: 10 slots
		cKey -= '0';
		if(cKey < GAME_SLOT_NUM) {
			pRef->iRef    = CARD_REF_SLOT;
			pRef->iColumn = cKey;
		}
	} else if(cKey >= 'a' && cKey <= 'd') {
		pRef->iRef    = CARD_REF_FOUNDATION;
		pRef->iColumn = cKey - 'a';
	} else if(cKey == GAME_KEY_CARD_OPEN) {
		pRef->iRef    = CARD_REF_OPEN;
		pRef->iColumn = 0;
	} else {
		pRef->iRef = CARD_REF_INVALID;
	}
}

void PrintHelp(void)
{
	printf("Lista de Comandos:\n\n");

	printf("%c - Virar Nova Carta do Baralho\n", GAME_KEY_CARD_NEW);
	printf("%c - Encerrar\n", GAME_KEY_QUIT);
	printf("%c - Novo Jogo\n", GAME_KEY_NEW);
	printf("%c - Salvar Jogo\n", GAME_KEY_SAVE);
	printf("%c - Carregar Jogo\n", GAME_KEY_LOAD);

	printf("Escolha das Cartas:\n\n");

	printf("Digite na sequência as letras da pilha de origem e destino.\n");
	printf("Se digitar apenas a origem e for slot, abre a carta fechada (se nenhuma aberta).\n");
}

int main(void)
{
	char        cKey;
	int         iCommand = GAME_CMD_QUIT;
	GameState   *pGame = Game_Create();
	GameCardRef tFrom, tTo;

	do {
		Game_PrintBoard(&pGame);

		printf("Digite o comando e pressione ENTER: ");
		cKey = getchar();

		iCommand = GAME_CMD_NONE;
		switch(cKey) {
			case GAME_KEY_CARD_NEW:
				iCommand = GAME_CMD_CARD_NEW;
				Game_GetRefFromKey(&tFrom, GAME_KEY_CARD_OPEN);
				break;

			case GAME_KEY_LOAD:
				Game_Free(pGame);
				Game_Load(pGame, "teste.pac");
				break;

			case GAME_KEY_SAVE:
				Game_Save(pGame, "teste.pac");
				break;

			case GAME_KEY_NEW:
				Game_Free(pGame);
				pGame = Game_Create();
				break;

			case GAME_KEY_HELP:
				PrintHelp();
				break;

			case GAME_KEY_QUIT:
				iCommand = GAME_CMD_QUIT;
				break;

			case '\n':
			case '\r':
				break;

			default:
				Game_GetRefFromKey(&tFrom, cKey);
				if(tFrom.iRef != CARD_REF_INVALID) {
					cKey = getchar();
					if(cKey == '\n') {
						if(tFrom.iRef == CARD_REF_SLOT)
							iCommand = GAME_CMD_CARD_NEW;
					} else {
						Game_GetRefFromKey(&tTo, cKey);
						if(tTo.iRef != CARD_REF_INVALID) {
							iCommand = GAME_CMD_MOVE;
						}
					}
				}
				if(iCommand == GAME_CMD_NONE)
					printf("\n\nComando invalido!\n\n");

				break;
		}

		Game_ProcessCommand(&pGame, iCommand, &tFrom, &tTo);

		while(cKey != '\n') {
			cKey = getchar();

		if(Game_Finished(&pGame)) {
			printf("Voce ganhou!!!\n");
			iCommand = GAME_CMD_QUIT;
		}
		}
	} while(iCommand != GAME_CMD_QUIT);

	return 0;
}

