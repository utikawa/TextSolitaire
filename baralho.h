#ifndef BARALHO_H_INCLUDED
#define BARALHO_H_INCLUDED

// Definicoes dos valores das cartas. Usa bit a bit para poder criar mascaras
#define CARD_VALUE_INVALID 0x0000
#define CARD_VALUE_ACE     0x0001
#define CARD_VALUE_2       0x0002
#define CARD_VALUE_3       0x0004
#define CARD_VALUE_4       0x0008
#define CARD_VALUE_5       0x0010
#define CARD_VALUE_6       0x0020
#define CARD_VALUE_7       0x0040
#define CARD_VALUE_8       0x0080
#define CARD_VALUE_9       0x0100
#define CARD_VALUE_10      0x0200
#define CARD_VALUE_J       0x0400
#define CARD_VALUE_Q       0x0800
#define CARD_VALUE_K       0x1000

// Tipos de barallhos
#define DECK_TYPE_FULL  (CARD_VALUE_ACE | CARD_VALUE_2 | CARD_VALUE_3 | CARD_VALUE_4 | CARD_VALUE_5  | \
                         CARD_VALUE_6   | CARD_VALUE_7 | CARD_VALUE_8 | CARD_VALUE_9 | CARD_VALUE_10 | \
                         CARD_VALUE_J   | CARD_VALUE_Q | CARD_VALUE_K)

#define DECK_TYPE_TRUCO (CARD_VALUE_ACE | CARD_VALUE_2 | CARD_VALUE_3 | CARD_VALUE_4 | CARD_VALUE_5 | \
                         CARD_VALUE_6   | CARD_VALUE_7 | CARD_VALUE_J | CARD_VALUE_Q | CARD_VALUE_K)

// Enumeracao com a definicao dos naipes. O valor definido refere-se ao caracter ASCII referente ao simbolo do naipe
typedef enum { Diamond=4, Spade=6, Heart=3, Club=5 } Suit;

// Estrutura que define uma carta
typedef struct {
	int  iValue;
	Suit eSuit;
} Card;

// Estrutura que define uma lista de cartas
typedef struct strCardList CardList;
struct strCardList {
	Card     *pCard;
	CardList *pNext;
};

// Funcoes referentes a Cartas
Card * Card_Create       (int iValue, Suit eSuit);
void   Card_Free         (Card *pCard);
char * Card_GetSuitString(Card *pCard);
int    Card_IsBlack      (Card *pCard);
int    Card_GetValue     (Card *pCard);

// Funcoes referentes a Listas de Cartas
CardList * CardList_GetPosition   (CardList **pList, CardList **pRetPrevious, int iPosition);
int        CardList_GetSize       (CardList **pList);
void       CardList_InsertCardList(CardList **pList, CardList *pNew);
void       CardList_InsertCard    (CardList **pList, Card *pCard);
CardList * CardList_GetSubset     (CardList **pList, Card *pCard);
Card     * CardList_GetCard       (CardList **pList);
void       CardList_CreateDeck    (CardList **pList, unsigned int uDeckMask);
void       CardList_Shuffle       (CardList **pList, int iCount);
void       CardList_Free          (CardList **pList);

#endif // BARALHO_H_INCLUDED

