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
#ifndef OPENXCOM_DOGFIGHTSTATE_H
#define OPENXCOM_DOGFIGHTSTATE_H

#include "../Engine/State.h"
#include <vector>
#include <string>

namespace OpenXcom
{

const int STANDOFF_DIST = 560;

class ImageButton;
class Text;
class Surface;
class InteractiveSurface;
class Timer;
class Globe;
class Craft;
class Ufo;
class CraftWeaponProjectile;

/**
 * Shows a dogfight (interception) between a
 * player craft and an UFO.
 */
class DogfightState : public State
{
private:
	Timer *_animTimer, *_moveTimer, *_w1Timer, *_w2Timer, *_ufoWtimer, *_ufoEscapeTimer, *_craftDamageAnimTimer;
	Surface *_window, *_battle, *_range1, *_range2, *_damage;
	InteractiveSurface *_btnMinimize, *_preview, *_weapon1, *_weapon2;
	ImageButton *_btnStandoff, *_btnCautious, *_btnStandard, *_btnAggressive, *_btnDisengage, *_btnUfo;
	ImageButton *_mode;
	ImageButton *_btnMinimizedIcon;
	Text *_txtAmmo1, *_txtAmmo2, *_txtDistance, *_txtStatus, *_txtInterceptionNumber;
	Globe *_globe;
	Craft *_craft;
	Ufo *_ufo;
	int _timeout, _currentDist, _targetDist, _currentRadius, _ufoFireInterval, _ufoHitFrame;
	bool _end, _destroyUfo, _destroyCraft, _ufoBreakingOff, _weapon1Enabled, _weapon2Enabled, _minimized, _endDogfight;
	std::vector<CraftWeaponProjectile*> _projectiles;
	static const int _ufoBlobs[8][13][13];
	static const int _projectileBlobs[4][6][3];
	static const int _timeScale;
	int _ufoSize, _craftHeight, _currentCraftDamageColor, _interceptionsCount, _interceptionNumber;
	int _x, _y, _minimizedIconX, _minimizedIconY;

	// Ends the dogfight.
	void endDogfight();

public:
	/// Creates the Dogfight state.
	DogfightState(Game *game, Globe *globe, Craft *craft, Ufo *ufo);
	/// Cleans up the Dogfight state.
	~DogfightState();
	/// Runs the timers.
	void think();
	/// Animates the window.
	void animate();
	/// Moves the craft.
	void move();
	// Fires the first weapon.
	void fireWeapon1();
	// Fires the second weapon.
	void fireWeapon2();
	// Fires UFO weapon.
	void ufoFireWeapon();
	// Sets the craft to minimum distance.
	void minimumDistance();
	// Sets the craft to maximum distance.
	void maximumDistance();
	/// Changes the status text.
	void setStatus(const std::string &status);
	/// Handler for clicking the Minimize button.
	void btnMinimizeClick(Action *action);
	/// Handler for clicking the Standoff button.
	void btnStandoffClick(Action *action);
	/// Handler for clicking the Cautious Attack button.
	void btnCautiousClick(Action *action);
	/// Handler for clicking the Standard Attack button.
	void btnStandardClick(Action *action);
	/// Handler for clicking the Aggressive Attack button.
	void btnAggressiveClick(Action *action);
	/// Handler for clicking the Disengage button.
	void btnDisengageClick(Action *action);
	/// Handler for clicking the Ufo button.
	void btnUfoClick(Action *action);
	/// Handler for clicking the Preview graphic.
	void previewClick(Action *action);
	/// Makes the UFO break off the interception... or at least tries to.
	void ufoBreakOff();
	/// Draws UFO.
	void drawUfo();
	/// Draws projectiles.
	void drawProjectile(const CraftWeaponProjectile* p);
	/// Animates craft damage.
	void animateCraftDamage();
	/// Updates craft damage.
	void drawCraftDamage();
	/// Toggles usage of weapon 1.
	void weapon1Click(Action *action);
	/// Toggles usage of weapon 2.
	void weapon2Click(Action *action);
	/// Changes colors of weapon icons, range indicators and ammo texts base on current weapon state.
	void recolor(const int weaponNo, const bool currentState);
	/// Returns true if state is minimized.
	bool isMinimized() const;
	/// Sets state minimized or maximized.
	void setMinimized(const bool minimized);
	/// Handler for clicking the minimized interception window icon.
	void btnMinimizedIconClick(Action *action);
	/// Gets interception number.
	int getInterceptionNumber() const;
	/// Sets interception number.
	void setInterceptionNumber(const int number);
	/// Sets interceptions count.
	void setInterceptionsCount(const int count);
	/// Calculates window position according to opened interception windows.
	void calculateWindowPosition();
	/// Moves window to new position.
	void moveWindow();
	/// Checks if the dogfight should be ended.
	bool dogfightEnded() const;
	/// Gets pointer to the UFO in this dogfight.
	Ufo* getUfo() const;
	
};

}

#endif
