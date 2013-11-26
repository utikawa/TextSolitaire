#include <time.h>
#include <stdlib.h>
#include <malloc.h>

#include "baralho.h"

// Cria uma carta, alocando a memoria e carregando os valores recebidos
Card * Card_Create(int iValue, Suit eSuit)
{
	Card *pNew = (Card *)malloc(sizeof(Card));

	if(pNew != NULL) {
		pNew->iValue = iValue;
		pNew->eSuit  = eSuit;
	}

	return pNew;
}

// Descarrega a carta da memoria
void Card_Free(Card *pCard)
{
	if(pCard != NULL) {
		free(pCard);
	}
}

// Retorna o valor crescente (1 a 13) referente a carta
int Card_GetValue(Card *pCard)
{
	int iValue;

	if(pCard != NULL) {
		for(iValue = 0; iValue < CARD_VALUE_K; iValue++) {
			if(pCard->iValue == (1<<iValue)) {
				return iValue+1;
			}
		}
	}

	return CARD_VALUE_INVALID;
}

// Retorna a string com o nome do naipe da carta
char * Card_GetSuitString(Card *pCard)
{
	if(pCard != NULL) {
		switch(pCard->eSuit) {
			case Diamond: return "Ouros"  ;
			case Spade  : return "Espadas";
			case Heart  : return "Copas"  ;
			case Club   : return "Paus"   ;
		}
	}

	return "";
}

// Retorna 1 se a carta for preta (Espadas ou Paus). Caso contrario retorna 0.
int Card_IsBlack(Card *pCard)
{
	if(pCard != NULL && (pCard->eSuit == Spade || pCard->eSuit == Club)) return 1;

	return 0;
}

// Retorna um ponteiro para a lista de cartas a partir da posicao especificada.
// pRetPrevious contera a posicao anterior a posicao retornada. Parametro opcional, NULL se nao usado.
// Se iPosition receber um valor negativo como parametro, retorna a ultima posicao da lista
CardList * CardList_GetPosition(CardList **pList, CardList **pRetPrevious, int iPosition)
{
	CardList *pCurrent, *pPrevious = NULL;
	if(pList == NULL || *pList == NULL) return NULL;

	pCurrent = *pList;

	// Varre a lista enquanto existir elementos
	while(pCurrent != NULL) {
		if(iPosition < 0 && pCurrent->pNext == NULL) break; // Encontrou ultimo item
		if(iPosition-- == 0) break; // Encontrou item procurado

		pPrevious = pCurrent;
		pCurrent = pCurrent->pNext;
	}

	// Se pRetPrevious nao for nulo, carrega com a posicao anterior a posicao solicitada
	if(pRetPrevious != NULL) {
		if(pCurrent != NULL) {
			*pRetPrevious = pPrevious;
		} else {
			*pRetPrevious = NULL;
		}
	}

	// Se nao encontrou, retorna nulo
	return pCurrent;
}

// Retorna o numero de cartas na lista
int CardList_GetSize(CardList **pList)
{
	int iSize = 0;
	CardList *pCurrent;

	if(pList != NULL && *pList != NULL) {
		pCurrent = *pList;
		while(pCurrent != NULL) {
			iSize++;
			pCurrent = pCurrent->pNext;
		}
	}

	return iSize;
}

// Insere uma lista de cartas no final da lista atual
void CardList_InsertCardList(CardList **pList, CardList *pNew)
{
	CardList *pCurrent;
	if(pList == NULL || pNew == NULL) return;

	if(*pList == NULL) { // Se lista vazia, adiciona na primeira posicao
		*pList = pNew;
	} else {
		pCurrent = CardList_GetPosition(pList, NULL, -1);
		pCurrent->pNext = pNew;
	}
}

// Insere uma carta ao final da lista
void CardList_InsertCard(CardList **pList, Card *pCard)
{
	CardList *pNew;
	if(pList == NULL || pCard == NULL) return;

	// Cria uma nova lista contendo apenas a carta passada como parametro
	pNew = (CardList *)malloc(sizeof(CardList));
	pNew->pCard = pCard;
	pNew->pNext = NULL;

	// Insere a lista criada ao final da lista fornecida como parametro
	CardList_InsertCardList(pList, pNew);
}

// Retorna uma sublista a partir da carta passada como parametro, dividindo a lista atual
CardList * CardList_GetSubset(CardList **pList, Card *pCard)
{
	CardList *pCurrent, *pPrevious = NULL;
	if(pList == NULL || *pList == NULL || pCard == NULL) return NULL;

	// Procura a carta na lista
	pCurrent = *pList;
	while(pCurrent != NULL) {
		if(pCurrent->pCard->iValue == pCard->iValue && pCurrent->pCard->eSuit == pCard->eSuit) {
			break; // Achou a carta!
		} else {
			pPrevious = pCurrent;
			pCurrent = pCurrent->pNext;
		}
	}

	// Atualiza a lista, gravando nulo no indicador de proximo item na posicao da carta encontrada
	if(pPrevious != NULL) {
		pPrevious->pNext = NULL;
	} else if(pCurrent != NULL) {
		*pList = NULL;
	}

	// Se achou, pCurrent tera a posicao onde inicia a lista a partir de pCard, senao tera NULO.
	return pCurrent;
}

// Remove a ultima carta da lista, retornando-a
Card * CardList_GetCard(CardList **pList)
{
	Card     *pCard = NULL;
	CardList *pCurrent, *pPrevious = NULL;
	if(pList == NULL || *pList == NULL) return NULL;

	pCurrent = CardList_GetPosition(pList, &pPrevious, -1);

	if(pPrevious != NULL) {
		pPrevious->pNext = NULL;
	} else { // Lista vazia, carrega NULO
		*pList = NULL;
	}

	// Se achou, pCurrent tera a posicao onde inicia a lista a partir de pCard, senao tera NULO.
	if(pCurrent != NULL) {
		pCard = pCurrent->pCard;
		free(pCurrent);
	}

	return pCard;
}

// Funcao que cria o baralho
void CardList_CreateDeck(CardList **pList, unsigned int uDeckMask)
{
	int iValue;
	if(pList == NULL) return;

	// Loop que verifica bit a bit a mascara passada como parametro para verificar se a carta deve ser criada
	for(iValue=0; (1<<iValue) <= CARD_VALUE_K; iValue++) {
		if(uDeckMask & (1<<iValue)) {
			// Bit para posicao atual ligado na mascara, cria todos os naipes da carta indicada
			CardList_InsertCard(pList, Card_Create(1<<iValue, Diamond));
			CardList_InsertCard(pList, Card_Create(1<<iValue, Spade  ));
			CardList_InsertCard(pList, Card_Create(1<<iValue, Heart  ));
			CardList_InsertCard(pList, Card_Create(1<<iValue, Club   ));
		}
	}
}

// Embaralha uma lista de cartas. iCount contem o numero de trocas que devem ocorrer.
// Se iCount for zero, o numero de trocas sera o numero de cartas na lista, vezes 4.
void CardList_Shuffle(CardList **pList, int iCount)
{
	int iSize;
	CardList *pPrevious, *pStart, *pEnd;

	if(pList == NULL || *pList == NULL) return;

	// Carrega o numero de itens na lista e, se iCount for zero, carrega 4 vezes o tamanho da lista.
	iSize = CardList_GetSize(pList);
	if(iCount <= 0) {
		iCount = 4*iSize;
	}

	// Inicializa os numeros randomicos com a hora atual.
	srand(time(NULL));

	while(iCount--) {
		// Carrega posicao inicial aleatoriamente
		pStart = CardList_GetPosition(pList, &pPrevious, rand()%iSize);
		if(pPrevious == NULL) {
			*pList           = pStart->pNext;
		} else {
			pPrevious->pNext = pStart->pNext;
		}

		// Carrega posicao final aleatoriamente
		pEnd = CardList_GetPosition(pList, NULL, rand()%(iSize-1));
		pStart->pNext = pEnd->pNext;
		pEnd  ->pNext = pStart;
	}
}

// Descarrega uma lista de cartas da memoria
void CardList_Free(CardList **pList)
{
	CardList *pCurrent;
	if(pList == NULL || *pList == NULL) return;

	while(*pList != NULL) {
		pCurrent = *pList;
		*pList   = pCurrent->pNext;

		// Descarrega a carta na posicao atual e, em seguida, a lista
		Card_Free(pCurrent->pCard);
		free(pCurrent);
	}
}

