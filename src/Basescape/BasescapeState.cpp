/*
 * Copyright 2010-2013 OpenXcom Developers.
 *
 * This file is part of OpenXcom.
 *
 * OpenXcom is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * OpenXcom is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with OpenXcom.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "BasescapeState.h"
#include "../Engine/Game.h"
#include "../Resource/ResourcePack.h"
#include "../Engine/Language.h"
#include "../Engine/Palette.h"
#include "../Engine/Options.h"
#include "../Interface/TextButton.h"
#include "../Interface/Text.h"
#include "../Interface/TextEdit.h"
#include "BaseView.h"
#include "MiniBaseView.h"
#include "../Savegame/SavedGame.h"
#include "../Savegame/Base.h"
#include "../Savegame/BaseFacility.h"
#include "../Ruleset/RuleBaseFacility.h"
#include "../Savegame/Region.h"
#include "../Ruleset/RuleRegion.h"
#include "../Geoscape/GeoscapeState.h"
#include "../Menu/ErrorMessageState.h"
#include "DismantleFacilityState.h"
#include "../Geoscape/BuildNewBaseState.h"
#include "../Engine/Action.h"
#include "../Savegame/Craft.h"
#include "BaseInfoState.h"
#include "SoldiersState.h"
#include "CraftsState.h"
#include "BuildFacilitiesState.h"
#include "ResearchState.h"
#include "ManufactureState.h"
#include "PurchaseState.h"
#include "SellState.h"
#include "TransferBaseState.h"
#include "CraftInfoState.h"
#include "../Geoscape/AllocatePsiTrainingState.h"

namespace OpenXcom
{

/**
 * Initializes all the elements in the Basescape screen.
 * @param game Pointer to the core game.
 * @param base Pointer to the base to get info from.
 * @param globe Pointer to the Geoscape globe.
 */
BasescapeState::BasescapeState(Game *game, Base *base, Globe *globe) : State(game), _base(base), _globe(globe)
{
	// Create objects
	_txtFacility = new Text(192, 9, 0, 0);
	_view = new BaseView(192, 192, 0, 8);
	_mini = new MiniBaseView(128, 16, 192, 41);
	_edtBase = new TextEdit(127, 17, 193, 0);
	_txtLocation = new Text(126, 9, 194, 16);
	_txtFunds = new Text(126, 9, 194, 24);
	_btnNewBase = new TextButton(128, 12, 192, 58);
	_btnBaseInfo = new TextButton(128, 12, 192, 71);
	_btnSoldiers = new TextButton(128, 12, 192, 84);
	_btnCrafts = new TextButton(128, 12, 192, 97);
	_btnFacilities = new TextButton(128, 12, 192, 110);
	_btnResearch = new TextButton(128, 12, 192, 123);
	_btnManufacture = new TextButton(128, 12, 192, 136);
	_btnTransfer = new TextButton(128, 12, 192, 149);
	_btnPurchase = new TextButton(128, 12, 192, 162);
	_btnSell = new TextButton(128, 12, 192, 175);
	_btnGeoscape = new TextButton(128, 12, 192, 188);

	// Set palette
	_game->setPalette(_game->getResourcePack()->getPalette("PALETTES.DAT_1")->getColors());

	add(_view);
	add(_mini);
	add(_txtFacility);
	add(_edtBase);
	add(_txtLocation);
	add(_txtFunds);
	add(_btnNewBase);
	add(_btnBaseInfo);
	add(_btnSoldiers);
	add(_btnCrafts);
	add(_btnFacilities);
	add(_btnResearch);
	add(_btnManufacture);
	add(_btnTransfer);
	add(_btnPurchase);
	add(_btnSell);
	add(_btnGeoscape);

	centerAllSurfaces();

	// Set up objects
	_view->setFonts(_game->getResourcePack()->getFont("Big.fnt"), _game->getResourcePack()->getFont("Small.fnt"));
	_view->setTexture(_game->getResourcePack()->getSurfaceSet("BASEBITS.PCK"));
	_view->onMouseClick((ActionHandler)&BasescapeState::viewLeftClick, SDL_BUTTON_LEFT);
	_view->onMouseClick((ActionHandler)&BasescapeState::viewRightClick, SDL_BUTTON_RIGHT);
	_view->onMouseOver((ActionHandler)&BasescapeState::viewMouseOver);
	_view->onMouseOut((ActionHandler)&BasescapeState::viewMouseOut);

	_mini->setTexture(_game->getResourcePack()->getSurfaceSet("BASEBITS.PCK"));
	_mini->setBases(_game->getSavedGame()->getBases());
	for (unsigned int i = 0; i < _game->getSavedGame()->getBases()->size(); ++i)
	{
		if (_game->getSavedGame()->getBases()->at(i) == _base)
		{
			_mini->setSelectedBase(i);
			break;
		}
	}
	_mini->onMouseClick((ActionHandler)&BasescapeState::miniClick);

	_txtFacility->setColor(Palette::blockOffset(13)+10);

	_edtBase->setColor(Palette::blockOffset(15)+1);
	_edtBase->setBig();
	_edtBase->onKeyboardPress((ActionHandler)&BasescapeState::edtBaseKeyPress);

	_txtLocation->setColor(Palette::blockOffset(15)+6);

	_txtFunds->setColor(Palette::blockOffset(13)+10);

	_btnNewBase->setColor(Palette::blockOffset(13)+5);
	_btnNewBase->setText(_game->getLanguage()->getString("STR_BUILD_NEW_BASE_UC"));
	_btnNewBase->onMouseClick((ActionHandler)&BasescapeState::btnNewBaseClick);

	_btnBaseInfo->setColor(Palette::blockOffset(13)+5);
	_btnBaseInfo->setText(_game->getLanguage()->getString("STR_BASE_INFORMATION"));
	_btnBaseInfo->onMouseClick((ActionHandler)&BasescapeState::btnBaseInfoClick);

	_btnSoldiers->setColor(Palette::blockOffset(13)+5);
	_btnSoldiers->setText(_game->getLanguage()->getString("STR_SOLDIERS_UC"));
	_btnSoldiers->onMouseClick((ActionHandler)&BasescapeState::btnSoldiersClick);

	_btnCrafts->setColor(Palette::blockOffset(13)+5);
	_btnCrafts->setText(_game->getLanguage()->getString("STR_EQUIP_CRAFT"));
	_btnCrafts->onMouseClick((ActionHandler)&BasescapeState::btnCraftsClick);

	_btnFacilities->setColor(Palette::blockOffset(13)+5);
	_btnFacilities->setText(_game->getLanguage()->getString("STR_BUILD_FACILITIES"));
	_btnFacilities->onMouseClick((ActionHandler)&BasescapeState::btnFacilitiesClick);

	_btnResearch->setColor(Palette::blockOffset(13)+5);
	_btnResearch->setText(_game->getLanguage()->getString("STR_RESEARCH"));
	_btnResearch->onMouseClick((ActionHandler)&BasescapeState::btnResearchClick);

	_btnManufacture->setColor(Palette::blockOffset(13)+5);
	_btnManufacture->setText(_game->getLanguage()->getString("STR_MANUFACTURE"));
	_btnManufacture->onMouseClick((ActionHandler)&BasescapeState::btnManufactureClick);

	_btnTransfer->setColor(Palette::blockOffset(13)+5);
	_btnTransfer->setText(_game->getLanguage()->getString("STR_TRANSFER_UC"));
	_btnTransfer->onMouseClick((ActionHandler)&BasescapeState::btnTransferClick);

	_btnPurchase->setColor(Palette::blockOffset(13)+5);
	_btnPurchase->setText(_game->getLanguage()->getString("STR_PURCHASE_RECRUIT"));
	_btnPurchase->onMouseClick((ActionHandler)&BasescapeState::btnPurchaseClick);

	_btnSell->setColor(Palette::blockOffset(13)+5);
	_btnSell->setText(_game->getLanguage()->getString("STR_SELL_SACK_UC"));
	_btnSell->onMouseClick((ActionHandler)&BasescapeState::btnSellClick);

	_btnGeoscape->setColor(Palette::blockOffset(13)+5);
	_btnGeoscape->setText(_game->getLanguage()->getString("STR_GEOSCAPE_UC"));
	_btnGeoscape->onMouseClick((ActionHandler)&BasescapeState::btnGeoscapeClick);
	_btnGeoscape->onKeyboardPress((ActionHandler)&BasescapeState::btnGeoscapeClick, (SDLKey)Options::getInt("keyCancel"));
}

/**
 *
 */
BasescapeState::~BasescapeState()
{
	// Clean up any temporary bases
	bool exists = false;
	for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end() && !exists; ++i)
	{
		if (*i == _base)
		{
			exists = true;
      break;
		}
	}
	if (!exists)
	{
		delete _base;
	}
}

/**
 * The player can change the selected base
 * or change info on other screens.
 */
void BasescapeState::init()
{
	if (_game->getSavedGame()->getBases()->size() > 0)
	{
		bool exists = false;
		for (std::vector<Base*>::iterator i = _game->getSavedGame()->getBases()->begin(); i != _game->getSavedGame()->getBases()->end() && !exists; ++i)
		{
			if (*i == _base)
			{
				exists = true;
        break;
			}
		}
		// If base was removed, select first one
		if (!exists)
		{
			_base = _game->getSavedGame()->getBases()->front();
			_mini->setSelectedBase(0);
		}
	}
	else
	{
		// Use a blank base for special case when player has no bases
		_base = new Base(_game->getRuleset());
	}

	_view->setBase(_base);
	_mini->draw();
	_edtBase->setText(_base->getName());

	// Get area
	for (std::vector<Region*>::iterator i = _game->getSavedGame()->getRegions()->begin(); i != _game->getSavedGame()->getRegions()->end(); ++i)
	{
		if ((*i)->getRules()->insideRegion(_base->getLongitude(), _base->getLatitude()))
		{
			_txtLocation->setText(_game->getLanguage()->getString((*i)->getRules()->getType()));
			break;
		}
	}

	std::wstring s = _game->getLanguage()->getString("STR_FUNDS");
	s += Text::formatFunding(_game->getSavedGame()->getFunds());
	_txtFunds->setText(s);

	_btnNewBase->setVisible(_game->getSavedGame()->getBases()->size() < 8);
}

/**
 * Changes the base currently displayed on screen.
 * @param base Pointer to new base to display.
 */
void BasescapeState::setBase(Base *base)
{
	_base = base;
	for (unsigned int i = 0; i < _game->getSavedGame()->getBases()->size(); ++i)
	{
		if (_game->getSavedGame()->getBases()->at(i) == _base)
		{
			_mini->setSelectedBase(i);
			break;
		}
	}
	init();
}

/**
 * Goes to the Build New Base screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnNewBaseClick(Action *)
{
	Base *base = new Base(_game->getRuleset());
	_game->popState();
	_game->pushState(new BuildNewBaseState(_game, base, _globe, false));
}

/**
 * Goes to the Base Info screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnBaseInfoClick(Action *)
{
	_game->pushState(new BaseInfoState(_game, _base, this));
}

/**
 * Goes to the Soldiers screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnSoldiersClick(Action *)
{
	_game->pushState(new SoldiersState(_game, _base));
}

/**
 * Goes to the Crafts screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnCraftsClick(Action *)
{
	_game->pushState(new CraftsState(_game, _base));
}

/**
 * Opens the Build Facilities window.
 * @param action Pointer to an action.
 */
void BasescapeState::btnFacilitiesClick(Action *)
{
	_game->pushState(new BuildFacilitiesState(_game, _base, this, true));
}

/**
 * Goes to the Research screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnResearchClick(Action *)
{
	_game->pushState(new ResearchState(_game, _base));
}

/**
 * Goes to the Manufacture screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnManufactureClick(Action *)
{
	_game->pushState(new ManufactureState(_game, _base));
}

/**
 * Goes to the Purchase screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnPurchaseClick(Action *)
{
	_game->pushState(new PurchaseState(_game, _base));
}

/**
 * Goes to the Sell screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnSellClick(Action *)
{
	_game->pushState(new SellState(_game, _base));
}

/**
 * Goes to the Select Destination Base window.
 * @param action Pointer to an action.
 */
void BasescapeState::btnTransferClick(Action *)
{
	_game->pushState(new TransferBaseState(_game, _base));
}

/**
 * Returns to the previous screen.
 * @param action Pointer to an action.
 */
void BasescapeState::btnGeoscapeClick(Action *)
{
	_game->popState();
}

/**
 * Processes clicking on facilities.
 * @param action Pointer to an action.
 */
void BasescapeState::viewLeftClick(Action *)
{
	BaseFacility *fac = _view->getSelectedFacility();
	if (fac != 0)
	{
		// Pre-calculate values to ensure base stays connected
		int x = -1, y = -1, squares = 0;
		for (std::vector<BaseFacility*>::iterator i = _base->getFacilities()->begin(); i != _base->getFacilities()->end(); ++i)
		{
			if ((*i)->getRules()->isLift())
			{
				x = (*i)->getX();
				y = (*i)->getY();
			}
			squares += (*i)->getRules()->getSize() * (*i)->getRules()->getSize();
		}
		squares -= fac->getRules()->getSize() * fac->getRules()->getSize();

		// Is facility in use?
		if (fac->inUse())
		{
			_game->pushState(new ErrorMessageState(_game, "STR_FACILITY_IN_USE", Palette::blockOffset(15)+1, "BACK13.SCR", 6));
		}
		// Would base become disconnected? (occupied squares connected to Access Lift < total squares occupied by base)
		else if (_view->countConnected(x, y, 0, fac) < squares)
		{
			_game->pushState(new ErrorMessageState(_game, "STR_CANNOT_DISMANTLE_FACILITY", Palette::blockOffset(15)+1, "BACK13.SCR", 6));
		}
		else
		{
			_game->pushState(new DismantleFacilityState(_game, _base, _view, fac));
		}
	}
}

/**
 * Processes right clicking on facilities.
 * @param action Pointer to an action.
 */
void BasescapeState::viewRightClick(Action *)
{
	BaseFacility *f = _view->getSelectedFacility();
	if (f == 0)
		_game->pushState(new BaseInfoState(_game, _base, this));

	else if (f->getRules()->getCrafts() > 0)
	{
		if (f->getCraft() == 0)
			_game->pushState(new CraftsState(_game, _base));
		else
			for (size_t craft = 0; craft < _base->getCrafts()->size(); ++craft)
			{
				if (f->getCraft() == _base->getCrafts()->at(craft))
				{
					_game->pushState(new CraftInfoState(_game, _base, craft));
					break;
				}
			}
	}
	else if (f->getRules()->getStorage() > 0)
		_game->pushState(new SellState(_game, _base));

	else if (f->getRules()->getPersonnel() > 0)
		_game->pushState(new SoldiersState(_game, _base));

	else if (f->getRules()->getPsiLaboratories() > 0 && Options::getBool("anytimePsiTraining") && _base->getAvailablePsiLabs() > 0)
		_game->pushState(new AllocatePsiTrainingState(_game, _base));

	else if (f->getRules()->getLaboratories() > 0)
		_game->pushState(new ResearchState(_game, _base));

	else if (f->getRules()->getWorkshops() > 0)
		_game->pushState(new ManufactureState(_game, _base));

	else if (f->getRules()->isLift() || f->getRules()->getRadarRange() > 0)
		_game->popState();
}

/**
 * Displays the name of the facility the mouse is over.
 * @param action Pointer to an action.
 */
void BasescapeState::viewMouseOver(Action *)
{
	BaseFacility *f = _view->getSelectedFacility();
	std::wstring t;
	if (f == 0)
		t = L"";
	else if (f->getRules()->getCrafts() == 0 || f->getBuildTime() > 0)
		t = _game->getLanguage()->getString(f->getRules()->getType());
	else
	{
		t.reserve(31);
		t =  _game->getLanguage()->getString(f->getRules()->getType());
		t += L" ";
		t += _game->getLanguage()->getString("STR_CRAFT_");
		if (f->getCraft() != 0)
			t += f->getCraft()->getName(_game->getLanguage());
	}
	_txtFacility->setText(t);
}

/**
 * Clears the facility name.
 * @param action Pointer to an action.
 */
void BasescapeState::viewMouseOut(Action *)
{
	_txtFacility->setText(L"");
}

/**
 * Selects a new base to display.
 * @param action Pointer to an action.
 */
void BasescapeState::miniClick(Action *)
{
	unsigned int base = _mini->getHoveredBase();
	if (base < _game->getSavedGame()->getBases()->size())
	{
		_mini->setSelectedBase(base);
		_base = _game->getSavedGame()->getBases()->at(base);
		init();
	}
}

/**
 * Changes the Base name.
 * @param action Pointer to an action.
 */
void BasescapeState::edtBaseKeyPress(Action *action)
{
	if (action->getDetails()->key.keysym.sym == SDLK_RETURN ||
		action->getDetails()->key.keysym.sym == SDLK_KP_ENTER)
	{
		_base->setName(_edtBase->getText());
	}
}
}
