/* Panel.cpp
Copyright (c) 2014 by Michael Zahniser

Endless Sky is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option) any later version.

Endless Sky is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
this program. If not, see <https://www.gnu.org/licenses/>.
*/

#include "Panel.h"

#include "Color.h"
#include "Command.h"
#include "DelaunayTriangulation.h"
#include "Dialog.h"
#include "FillShader.h"
#include "GamePad.h"
#include "text/Format.h"
#include "GameData.h"
#include "Point.h"
#include "Preferences.h"
#include "Screen.h"
#include "Sprite.h"
#include "SpriteShader.h"
#include "UI.h"

#include <SDL2/SDL.h>

using namespace std;



Panel::Panel() noexcept
{
	// Clear any triggered commands when starting a new panel
	Command::InjectClear();
}



Panel::~Panel()
{
}



// Draw a sprite repeatedly to make a vertical edge.
void Panel::DrawEdgeSprite(const Sprite *edgeSprite, int posX)
{
	if(edgeSprite->Height())
	{
		// If the screen is high enough, the edge sprite should repeat.
		double spriteHeight = edgeSprite->Height();
		Point pos(
			posX + .5 * edgeSprite->Width(),
			Screen::Top() + .5 * spriteHeight);
		for( ; pos.Y() - .5 * spriteHeight < Screen::Bottom(); pos.Y() += spriteHeight)
			SpriteShader::Draw(edgeSprite, pos);
	}
}



// Move the state of this panel forward one game step.
void Panel::Step()
{
	// It is ok for panels to be stateless.
}



// Return true if this is a full-screen panel, so there is no point in
// drawing any of the panels under it.
bool Panel::IsFullScreen() const noexcept
{
	return isFullScreen;
}



// Return true if, when this panel is on the stack, no events should be
// passed to any panel under it. By default, all panels do this.
bool Panel::TrapAllEvents() const noexcept
{
	return trapAllEvents;
}



// Check if this panel can be "interrupted" to return to the main menu.
bool Panel::IsInterruptible() const noexcept
{
	return isInterruptible;
}



// Clear the list of clickable zones.
void Panel::ClearZones()
{
	zones.clear();
}



// Add a clickable zone to the panel.
void Panel::AddZone(const Rectangle &rect, const function<void()> &fun)
{
	// The most recently added zone will typically correspond to what was drawn
	// most recently, so it should be on top.
	zones.emplace_front(rect, fun);
}



// Add a clickable zone to the panel.
void Panel::AddZone(const Rectangle &rect, const function<void(const Event&)> &fun)
{
	// The most recently added zone will typically correspond to what was drawn
	// most recently, so it should be on top.
	zones.emplace_front(rect, fun);
}



void Panel::AddZone(const Rectangle &rect, SDL_Keycode key)
{
	AddZone(rect, [this, key](){ this->KeyDown(key, 0, Command(), true); });
}



void Panel::AddZone(const Rectangle &rect, Command command)
{
	zones.emplace_front(rect, command);
}



// Add a clickable zone to the panel.
void Panel::AddZone(const Point &center, float radius, const function<void()> &fun)
{
	// The most recently added zone will typically correspond to what was drawn
	// most recently, so it should be on top.
	zones.emplace_front(center, radius, fun);
}



// Add a clickable zone to the panel.
void Panel::AddZone(const Point &center, float radius, const function<void(const Event &)> &fun)
{
	// The most recently added zone will typically correspond to what was drawn
	// most recently, so it should be on top.
	zones.emplace_front(center, radius, fun);
}



void Panel::AddZone(const Point &center, float radius, SDL_Keycode key)
{
	AddZone(center, radius, [this, key](){ this->KeyDown(key, 0, Command(), true); });
}



void Panel::AddZone(const Point &center, float radius, Command command)
{
	zones.emplace_front(center, radius, command);
}



// Check if a click at the given coordinates triggers a clickable zone. If
// so, apply that zone's action and return true.
bool Panel::ZoneMouseDown(const Point &point, int id)
{
	for(const Zone &zone : zones)
	{
		if(zone.Contains(point))
		{
			// If the panel is in editing mode, make sure it knows that a mouse
			// click has broken it out of that mode, so it doesn't interpret a
			// button press and a text character entered.
			EndEditing();
			zone.MouseDown(point, id);
			return true;
		}
	}
	return false;
}



// Check if a click at the given coordinates triggers a clickable zone. If
// so, apply that zone's action and return true.
bool Panel::ZoneFingerDown(const Point &point, int id)
{
	for(const Zone &zone : zones)
	{
		if(zone.Contains(point))
		{
			// If the panel is in editing mode, make sure it knows that a mouse
			// click has broken it out of that mode, so it doesn't interpret a
			// button press and a text character entered.
			EndEditing();
			zone.FingerDown(point, id);
			return true;
		}
	}
	return false;
}



// Check if a click at the given coordinates are on a zone.
bool Panel::HasZone(const Point &point)
{
	for(const Zone &zone : zones)
	{
		if(zone.Contains(point))
			return true;
	}
	return false;
}



// Panels will by default not allow fast-forward. The ones that do allow
// it will override this (virtual) function and return true.
bool Panel::AllowsFastForward() const noexcept
{
	return false;
}



// Only override the ones you need; the default action is to return false.
bool Panel::KeyDown(SDL_Keycode key, Uint16 mod, const Command &command, bool isNewPress)
{
	return false;
}



bool Panel::Click(int x, int y, int clicks)
{
	return false;
}



bool Panel::RClick(int x, int y)
{
	return false;
}



bool Panel::Hover(int x, int y)
{
	return false;
}



bool Panel::Drag(double dx, double dy)
{
	return false;
}



bool Panel::Scroll(double dx, double dy)
{
	return false;
}



bool Panel::Release(int x, int y)
{
	return false;
}



bool Panel::FingerDown(int x, int y, int fid)
{
	return false;
}



bool Panel::FingerMove(int x, int y, int fid)
{
	return false;
}



bool Panel::FingerUp(int x, int y, int fid)
{
	return false;
}



bool Panel::Gesture(Gesture::GestureEnum gesture)
{
	return false;
}



bool Panel::ControllersChanged()
{
	return false;
}



bool Panel::ControllerButtonDown(SDL_GameControllerButton button)
{
	return false;
}



bool Panel::ControllerButtonUp(SDL_GameControllerButton button)
{
	return false;
}



bool Panel::ControllerAxis(SDL_GameControllerAxis axis, int position)
{
	return false;
}



bool Panel::ControllerTriggerPressed(SDL_GameControllerAxis axis, bool positive)
{
	return false;
}



bool Panel::ControllerTriggerReleased(SDL_GameControllerAxis axis, bool positive)
{
	return false;
}



void Panel::SetIsFullScreen(bool set)
{
	isFullScreen = set;
}



void Panel::SetTrapAllEvents(bool set)
{
	trapAllEvents = set;
}



void Panel::SetInterruptible(bool set)
{
	isInterruptible = set;
}



// Dim the background of this panel.
void Panel::DrawBackdrop() const
{
	if(!GetUI()->IsTop(this))
		return;

	// Darken everything but the dialog.
	const Color &back = *GameData::Colors().Get("dialog backdrop");
	FillShader::Fill(Point(), Point(Screen::Width(), Screen::Height()), back);
}



UI *Panel::GetUI() const noexcept
{
	return ui;
}



// This is not for overriding, but for calling KeyDown with only one or two
// arguments. In this form, the command is never set, so you can call this
// with a key representing a known keyboard shortcut without worrying that a
// user-defined command key will override it.
bool Panel::DoKey(SDL_Keycode key, Uint16 mod)
{
	return KeyDown(key, mod, Command(), true);
}



// A lot of different UI elements allow a modifier to change the number of
// something you are buying, so the shared function is defined here:
int Panel::Modifier()
{
	SDL_Keymod mod = SDL_GetModState();

	int modifier = 1;
	if(mod & KMOD_ALT)
		modifier *= 500;
	if(mod & (KMOD_CTRL | KMOD_GUI))
		modifier *= 20;
	if(mod & KMOD_SHIFT)
		modifier *= 5;

	return modifier;
}



// Display the given help message if it has not yet been shown
// (or if force is set to true). Return true if the message was displayed.
bool Panel::DoHelp(const string &name, bool force) const
{
	string preference = "help: " + name;
	if(!force && Preferences::Has(preference))
		return false;

	const string &message = GameData::HelpMessage(name);
	if(message.empty())
		return false;

	Preferences::Set(preference);
	ui->Push(new Dialog(Format::Capitalize(name) + ":\n\n" + message));

	return true;
}



void Panel::SetUI(UI *ui)
{
	this->ui = ui;
}
