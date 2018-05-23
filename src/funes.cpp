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
	spTextField _text;
	spSprite _button;

	typedef struct
	{
		Point	pCard;
		bool	bOpen;
		bool	bFlip;
		spSprite card;
	} struct_datacards;

	bool bInuse;
	spSprite sInuse[2];
	spSprite sStackPile[2];

	struct_datacards CardData[52];
	Vector2 CardBaseSize;
	float scaleFactor;
	Vector2 EndPos[2];
	Vector2 CardSep;
	int used[52];
	int jugadas, aciertos,trampas;

	MainDeck()
	{
		spTextField button_text;
		int CardW, CardH, SizeW;
		_deck = gameResources.getResAnim("cards");
		_back = gameResources.getResAnim("cardsback");
		_text = new TextField;
		_button = new Sprite();

		CardW = _deck->getAttribute("cardwidth").as_int(0);
		CardH = _deck->getAttribute("cardheight").as_int(0);

		SizeW = (int)(getStage()->getWidth() / 13);
		CardSep.x = (float)(int)(SizeW / 11);
		CardSep.y = CardSep.x;
		scaleFactor = (float)SizeW / (float)CardW;
		CardBaseSize.x = (float)(int)(CardW*scaleFactor);
		CardBaseSize.y = (float)(int)(CardH*scaleFactor);

		sStackPile[0] = new Sprite();
		addChild(sStackPile[0]);
		sStackPile[1] = new Sprite();
		addChild(sStackPile[1]);

		_button->setResAnim(gameResources.getResAnim("button"));
		_button->setAnchor(0.5f, 0.5f);
		_button->setX(getStage()->getWidth() / 2);
		_button->setY(getStage()->getHeight() - CardBaseSize.y + _button->getHeight()/2);
		_button->addEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::TipHelp));
		_button->addEventListener(TouchEvent::TOUCH_DOWN, CLOSURE(this, &MainDeck::ButtonResize));
		addChild(_button);
		button_text = new TextField();
		button_text->setName("button_text");
		button_text->attachTo(_button);
		button_text->setPosition(_button->getSize() / 2);
		TextStyle style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
		button_text->setStyle(style);
		button_text->removeEventListener(TouchEvent::TOUCH_UP, CLOSURE(this, &MainDeck::TipHelp));
		CardsShuffle();
		return;
	}

	void CardsShuffle(void)
	{
		Vector2 Pos;
		spSprite card;
		spTextField button_text;

		bInuse = 0;
		jugadas = 0;
		aciertos = 0;
		trampas = 0;


		button_text = this->getDescendantT<TextField>("button_text", ep_ignore_error);
		button_text->setText("Ayuda!");

		for (int i = 0; i < 52; i++)
		{
			used[i] = 0;
		}
		for (int col = 0; col < 12; col++)
		{
			for (int row = 0; row < 4; row++)
			{
				card = new Sprite();
				addChild(card);
				CardData[row + (col * 4)].pCard = CardsShf(row + (col * 4));
				CardData[row + (col * 4)].bOpen = false;
				CardData[row + (col * 4)].bFlip = false;
				CardData[row + (col * 4)].card = card;
				card->setScale(scaleFactor);
				card->setAnchor(0.5f, 0.5f);
				card->setAnimFrame((_back));
				Pos.x = col * (CardBaseSize.x + CardSep.x) + CardBaseSize.x/2;
				Pos.y = row * (CardBaseSize.y + CardSep.y) + CardBaseSize.y/2;
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
				CardData[row + (col * 4)].bFlip = false;
				CardData[row + (col * 4)].card = card;
				card->setScale(scaleFactor);
				card->setAnchor(0.5f, 0.5f);
				card->setAnimFrame((_back));
				Pos.x = row * (CardBaseSize.x + CardSep.x) + CardBaseSize.x/2;
				Pos.y = 4 * (CardBaseSize.y + CardSep.y) + CardBaseSize.y/2;
				card->setPosition(Pos);
				card->setUserData(&CardData[row + (col * 4)]);
				EventCallback cb = CLOSURE(this, &MainDeck::cardStartFlip);
				card->addEventListener(TouchEvent::CLICK, cb);
			}
		}
		for (int col = 0; col < 2; col++)
		{
			sStackPile[col]->setScale(scaleFactor);
			sStackPile[col]->setAnchor(0.5f, 0.5f);
			sStackPile[col]->setAnimFrame((_back), 2);
			Pos.x = (col+9) * (CardBaseSize.x + CardSep.x) + CardBaseSize.x / 2;
			Pos.y = 4 * (CardBaseSize.y + CardSep.y) + CardBaseSize.y / 2;
			sStackPile[col]->setPosition(Pos);
			EndPos[col] = Pos;
		}
		DrawText();
		return;
	}

	void cardStopFlip(Event* event)
	{
		spSprite sprite;
		spSprite card;
		Vector2 srcPos;
		struct_datacards* dataCard;

		card = new Sprite();
		addChild(card);
		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		srcPos = sprite->getPosition();
		card->setPosition(srcPos);
		card->setScale(scaleFactor);
		card->setAnchor(0.5f, 0.5f);
		dataCard->bFlip = false;
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
		dataCard->card = card;
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
		spSprite sprite;
		spTween tween;
		struct_datacards* dataCard;

		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		sprite->setScale(scaleFactor);
		sprite->setAnchor(0.5f, 0.5f);
		if (dataCard->bOpen)
			sprite->setAnimFrame((_back));
		else
			sprite->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		tween = sprite->addTween(Actor::TweenWidth(CardBaseSize.x/scaleFactor), 300, 1, false);
		tween->addDoneCallback(CLOSURE(this, &MainDeck::cardStopFlip));
		tween->detachWhenDone();
		return;

	}

	void cardStartFlip(Event* event)
	{
		spSprite sprite;
		spTween tween;
		struct_datacards* dataCard;

		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		if (dataCard->bFlip == true)
			return;
		dataCard->bFlip = true;
		sprite->setUserData(dataCard);
		tween = sprite->addTween(Actor::TweenWidth(0), 300, 1, false);
		tween->addDoneCallback(CLOSURE(this, &MainDeck::cardEndFlip));
		return;
	}

	void cardStackPile(Event* event)
	{
		spSprite sprite;
		Vector2 srcPos;
		struct_datacards* dataCard;

		sprite = safeSpCast<Sprite>(event->target);
		dataCard = (struct_datacards*)sprite->getUserData();
		srcPos = sprite->getPosition();
		if ((srcPos.x == EndPos[0].x) && (srcPos.y == EndPos[0].y))
			sStackPile[0]->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);
		else
			sStackPile[1]->setAnimFrame((_deck), dataCard->pCard.x, dataCard->pCard.y);

		if (aciertos > 25)
			EndGame();
		return;
	}

	void FlipCards(bool bHit)
	{
		spTween tween[2];
		bInuse = false;
		struct_datacards* dataCard;

		tween[0] = sInuse[0]->addTween(TweenDummy(), 1000);
		tween[1] = sInuse[1]->addTween(TweenDummy(), 1000);

		jugadas++;
		if (bHit)
		{
			aciertos++;
			tween[0]->detachWhenDone();
			tween[1]->detachWhenDone();
			dataCard = (struct_datacards*)sInuse[0]->getUserData();
			dataCard->card = NULL;
			dataCard = (struct_datacards*)sInuse[1]->getUserData();
			dataCard->card = NULL;
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

	void DrawText(void)
	{
		char text[255];

		safe_sprintf(text, "Jugadas: %d Aciertos: %d", jugadas, aciertos);
		TextStyle style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
		_text->setStyle(style);
		_text->setText(text);
		_text->setVAlign(TextStyle::VALIGN_BASELINE);
		_text->setHAlign(TextStyle::HALIGN_MIDDLE);
		_text->setX(getStage()->getWidth() / 2);
		_text->setY(getStage()->getHeight() - CardSep.y);
		_text->attachTo(getStage());
		return;
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

	void ButtonResize(Event* event)
	{
		_button->setScale(1.0f);
		_button->addTween(Actor::TweenScale(0.9f), 250, 1, false);
		return;
	}

	void RestartGame(Event* event)
	{
		if (aciertos > 25)
			CardsShuffle();
		return;
	}

	void TipHelp(Event* event)
	{
		spSprite sTip[2];
		spTween tween[2];
		struct_datacards* dataCard;
		Point	pSearchCard;
		time_t t1;
		int select, found;

		(void)time(&t1);
		srand((long)t1);

		found = 0;

		_button->addTween(Actor::TweenScale(1.0f), 250, 1, false);

		if (aciertos > 25)
		{
			CheckEndGame();
			return;
		}

		for (select = 0; select < 52; select++)
		{
			if (CardData[select].bFlip)
				return;
		}

		if (bInuse)
			sTip[0] = sInuse[0];
		else
		{
			while (!found)
			{
				select = rand() % 52;
				if (CardData[select].card != NULL)
					found = 1;
			}
			sTip[0] = CardData[select].card;
		}
		dataCard = (struct_datacards*)sTip[0]->getUserData();
		pSearchCard.x = dataCard->pCard.x;
		switch (dataCard->pCard.y)
		{
			case CLUB:
				pSearchCard.y = SPADE;
			break;
			case DIAMOND:
				pSearchCard.y = HEART;
			break;
			case HEART:
				pSearchCard.y = DIAMOND;
			break;
			case SPADE:
				pSearchCard.y = CLUB;
			break;
		}
		for (select = 0; select < 52; select++)
		{
			if ((CardData[select].pCard.x == pSearchCard.x) && (CardData[select].pCard.y == pSearchCard.y))
			{
				sTip[1] = CardData[select].card;
				break;
			}
		}

		if (!bInuse)
		{
			dataCard = (struct_datacards*)sTip[0]->getUserData();
			dataCard->bOpen = true;
			tween[0] = sTip[0]->addTween(TweenDummy(), 100);
			tween[0]->addDoneCallback(CLOSURE(this, &MainDeck::cardStartFlip));
		}
		dataCard = (struct_datacards*)sTip[1]->getUserData();
		if (!dataCard->bFlip)
		{
			dataCard->bOpen = true;
			tween[1] = sTip[1]->addTween(TweenDummy(), 100);
			tween[1]->addDoneCallback(CLOSURE(this, &MainDeck::cardStartFlip));
			trampas = trampas + 1;
		}

		return;
	}

	void EndGame(void)
	{
		spTextField Endtext;
		char text[255];

		Endtext = getStage()->getDescendantT<TextField>("EndText", ep_ignore_error);
		if (Endtext)
			return;

		safe_sprintf(text, "Felicitaciones ha terminado el Juego en\nJugadas: %d", jugadas);
		if (trampas)
			safe_sprintf(text, "%s\nHe hizo %d TRAMPAS", text, trampas);
		TextStyle style = TextStyle(gameResources.getResFont("main")).withColor(Color::White).alignMiddle();
		Endtext = new TextField;
		Endtext->setStyle(style);
		Endtext->setText(text);
		Endtext->setVAlign(TextStyle::VALIGN_BASELINE);
		Endtext->setHAlign(TextStyle::HALIGN_MIDDLE);
		Endtext->setX(getStage()->getWidth() / 2);
		Endtext->setY(getStage()->getHeight() / 2);
		Endtext->setName("EndText");
		Endtext->attachTo(getStage());

		Endtext = this->getDescendantT<TextField>("button_text", ep_ignore_error);
		Endtext->setText("Jugar");
		return;
	}

	void CheckEndGame(void)
	{
		spTextField Endtext;
		spTween tween;

		Endtext = getStage()->getDescendantT<TextField>("EndText", ep_ignore_error);
		if (Endtext)
		{
			tween = Endtext->addTween(TweenDummy(), 100);
			tween->detachWhenDone();
			tween->addDoneCallback(CLOSURE(this, &MainDeck::RestartGame));
		}
		return;
	}
};

typedef oxygine::intrusive_ptr<MainDeck> spMainDeck;
spMainDeck MainActor;

void funes_preinit() {}

//called from main.cpp
void funes_init()
{
    //load xml file with resources definition
    gameResources.loadXML("res.xml");


    //lets create our client code simple actor
    //spMainDeck was defined above as smart intrusive pointer (read more: http://www.boost.org/doc/libs/1_60_0/libs/smart_ptr/intrusive_ptr.html)
	MainActor = new MainDeck;

    //and add it to Stage as child
    getStage()->addChild(MainActor);
}


//called each frame from main.cpp
void funes_update()
{
	MainActor->DrawText();
}

//called each frame from main.cpp
void funes_destroy()
{
    //free previously loaded resources
    gameResources.free();
}
