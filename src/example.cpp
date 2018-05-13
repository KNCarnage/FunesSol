#include "oxygine-framework.h"
#include <functional>

#define CLUB 0
#define DIAMOND 1
#define SPADE 2
#define HEART 3

#define BLACK 1
#define RED 2

using namespace oxygine;

//it is our resources
//in real project you would have more than one Resources declarations.
//It is important on mobile devices with limited memory and you would load/unload them
Resources gameResources;

class MainDeck : public Actor
{
public:
	ResAnim* _deck;
	ResAnim* _back;

	typedef struct
	{
		Point	pCard;
		bool	bOpen;
	} struct_datacards;

	bool bInuse;
	spSprite sInuse[2];
	spSprite sStackPile[2];

	struct_datacards CardData[52];
	Vector2 CardBaseSize;
	Vector2 EndPos[2];
	int used[52];

	MainDeck()
	{
		Vector2 Pos;
		spSprite card;

		_deck = gameResources.getResAnim("cards");
		_back = gameResources.getResAnim("cardsback");

		CardsCreate();

		for (int col = 0; col < 12; col++)
		{
			for (int row = 0; row < 4; row++)
			{
				card = new Sprite();
				addChild(card);
				CardData[row + (col * 4)].pCard = CardsShf(row + (col*4));
				CardData[row + (col * 4)].bOpen = false;
				card->setAnimFrame((_back));
				Pos.x = col * (card->getWidth() + 10);
				Pos.y = row * (card->getHeight() + 10);
				card->setPosition(Pos);
				card->setUserData(&CardData[row + (col * 4)]);
				EventCallback cb = CLOSURE(this, &MainDeck::cardStartFlip);
				card->addEventListener(TouchEvent::CLICK, cb);
			}
		}
		for (int col = 12; col < 13; col++)
		{
			for (int row = 0; row < 4; row++)
			{
				card = new Sprite();
				addChild(card);
				CardData[row + (col * 4)].pCard = CardsShf(row + (col * 4));
				CardData[row + (col * 4)].bOpen = false;
				card->setAnimFrame((_back));
				Pos.x = row * (card->getWidth() + 10);
				Pos.y = 4 * (card->getHeight() + 10);
				card->setPosition(Pos);
				card->setUserData(&CardData[row + (col * 4)]);
				EventCallback cb = CLOSURE(this, &MainDeck::cardStartFlip);
				card->addEventListener(TouchEvent::CLICK, cb);
			}
		}
		for (int col = 9; col < 11; col++)
		{
			sStackPile[col - 9] = new Sprite();
			addChild(sStackPile[col - 9]);
			sStackPile[col - 9]->setAnimFrame((_back),2);
			Pos.x = col * (sStackPile[col - 9]->getWidth() + 10);
			Pos.y = 4 * (sStackPile[col - 9]->getHeight() + 10);
			sStackPile[col - 9]->setPosition(Pos);
			EndPos[col - 9] = Pos;
		}
		CardBaseSize.x = card->getWidth();
		CardBaseSize.y = card->getHeight();
		return;
	}

	void cardStopFlip(Event* event)
	{
		spSprite* sprite;
		spSprite card;
		Vector2 srcPos;
		struct_datacards* dataCard;

		card = new Sprite();
		addChild(card);
		sprite = (spSprite*)&(event->target);
		dataCard = (struct_datacards*)(*sprite)->getUserData();
		srcPos = (*sprite)->getPosition();
		card->setPosition(srcPos);
		if (dataCard->bOpen == false)
		{
			dataCard->bOpen = true;
			card->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		}
		else
		{
			dataCard->bOpen = false;
			card->setAnimFrame((_back));
			EventCallback cb = CLOSURE(this, &MainDeck::cardStartFlip);
			card->addEventListener(TouchEvent::CLICK, cb);
		}
		card->setUserData(dataCard);
		
		if (dataCard->bOpen == false)
			return;

		if (bInuse)
		{
			sInuse[1] = card;
			FlipCards(CheckCards());
		}
		else
		{
			sInuse[0] = card;
			bInuse = true;
		}
		return;
	}

	void cardEndFlip(Event* event)
	{
		spSprite* sprite;
		spTweenQueue tweenQueue = new TweenQueue();
		Vector2 srcPos, destPos;
		struct_datacards* dataCard;

		sprite = (spSprite*)&(event->target);
		dataCard = (struct_datacards*)(*sprite)->getUserData();
		if (dataCard->bOpen)
			(*sprite)->setAnimFrame((_back));
		else
			(*sprite)->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		(*sprite)->addTween(Actor::TweenWidth(CardBaseSize.x), 600, 0, true);
		srcPos = (*sprite)->getPosition();
		destPos = Vector2(srcPos.x - CardBaseSize.x / 2, srcPos.y);
		tweenQueue->add(Sprite::TweenPosition(destPos), 285, 1);

		tweenQueue->addDoneCallback(CLOSURE(this, &MainDeck::cardStopFlip));
		(*sprite)->addTween(tweenQueue);
		tweenQueue->detachWhenDone();
		return;

	}

	void cardStartFlip(Event* event)
	{
		spSprite* sprite;
		spTweenQueue tweenQueue = new TweenQueue();
		Vector2 srcPos,destPos;

		sprite = (spSprite*)&(event->target);
		(*sprite)->addTween(Actor::TweenWidth(0), 600, 0, true);
		srcPos = (*sprite)->getPosition();
		destPos = Vector2(srcPos.x + (*sprite)->getWidth()/2, srcPos.y);
		tweenQueue->add(Sprite::TweenPosition(destPos), 300, 1);
		tweenQueue->addDoneCallback(CLOSURE(this, &MainDeck::cardEndFlip));
		(*sprite)->addTween(tweenQueue);
		return;
	}

	void cardStackPile(Event* event)
	{
		spSprite* sprite;
		Vector2 srcPos;
		struct_datacards* dataCard;

		sprite = (spSprite*)&(event->target);
		dataCard = (struct_datacards*)(*sprite)->getUserData();
		srcPos = (*sprite)->getPosition();
		if ((srcPos.x == EndPos[0].x) && (srcPos.y == EndPos[0].y))
			sStackPile[0]->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		else
			sStackPile[1]->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		return;
	}

	void FlipCards(bool bHit)
	{
		spTween tween[2];
		bInuse = false;

		tween[0] = sInuse[0]->addTween(TweenDummy(), 1000);
		tween[1] = sInuse[1]->addTween(TweenDummy(), 1000);

		if (bHit)
		{
			tween[0]->detachWhenDone();
			tween[1]->detachWhenDone();
			sInuse[0]->addTween(Sprite::TweenPosition(EndPos[0]), 1000, 1);
			sInuse[1]->addTween(Sprite::TweenPosition(EndPos[1]), 1000, 1);
			tween[0]->addDoneCallback(CLOSURE(this, &MainDeck::cardStackPile));
			tween[1]->addDoneCallback(CLOSURE(this, &MainDeck::cardStackPile));

		}
		else
		{
			tween[0]->addDoneCallback(CLOSURE(this, &MainDeck::cardStartFlip));
			tween[1]->addDoneCallback(CLOSURE(this, &MainDeck::cardStartFlip));
		}

		return;
	}

	void CardsCreate(void)
	{
		bInuse = 0;
		for (int i = 0; i < 52; i++)
		{
			used[i] = 0;
		}
	}

	Point CardsShf(int i)
	{
		time_t t1;
		int select, found;
		Point Carta;

		(void)time(&t1);
		srand((long)t1);

		found = 0;
		while (!found)
		{

			select = rand() % 52;
			if (used[select] == 0)
			{
				used[select] = 1;
				found = 1;
			}
		}
		Carta.x = select / 4;
		Carta.y = select % 4;
		return Carta;
	}

	int CheckColor(int cardColor)
	{
		if ((cardColor == CLUB) || (cardColor == SPADE))
			return BLACK;
		else
			return RED;
	}

	bool CheckCards(void)
	{
		Point color;
		Point cara;
		struct_datacards* dataCard[2];

		dataCard[0] = (struct_datacards*)sInuse[0]->getUserData();
		dataCard[1] = (struct_datacards*)sInuse[1]->getUserData();

		color.x = CheckColor(dataCard[0]->pCard.y);
		color.y = CheckColor(dataCard[1]->pCard.y);

		cara.x = dataCard[0]->pCard.x;
		cara.y = dataCard[1]->pCard.x;

		if ((cara.x == cara.y) && (color.x == color.y))
			return true;
		else
			return false;
	}
};

typedef oxygine::intrusive_ptr<MainDeck> spMainDeck;

void example_preinit() {}

//called from main.cpp
void example_init()
{
    //load xml file with resources definition
    gameResources.loadXML("res.xml");


    //lets create our client code simple actor
    //spMainActor was defined above as smart intrusive pointer (read more: http://www.boost.org/doc/libs/1_60_0/libs/smart_ptr/intrusive_ptr.html)
	spMainDeck actor = new MainDeck;

    //and add it to Stage as child
    getStage()->addChild(actor);
}


//called each frame from main.cpp
void example_update()
{
}

//called each frame from main.cpp
void example_destroy()
{
    //free previously loaded resources
    gameResources.free();
}
