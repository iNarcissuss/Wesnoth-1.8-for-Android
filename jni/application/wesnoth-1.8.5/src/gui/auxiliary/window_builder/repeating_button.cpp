/* $Id: repeating_button.cpp 48450 2011-02-08 20:55:18Z mordante $ */
/*
   Copyright (C) 2009 - 2011 by Mark de Wever <koraq@xs4all.nl>
   Part of the Battle for Wesnoth Project http://www.wesnoth.org/

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License version 2
   or at your option any later version.
   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY.

   See the COPYING file for more details.
*/

#define GETTEXT_DOMAIN "wesnoth-lib"

#include "gui/auxiliary/window_builder/repeating_button.hpp"

#include "config.hpp"
#include "gui/auxiliary/log.hpp"
#include "gui/widgets/repeating_button.hpp"

namespace gui2 {

namespace implementation {

tbuilder_repeating_button::tbuilder_repeating_button(const config& cfg)
	: tbuilder_control(cfg)
{
}

twidget* tbuilder_repeating_button::build() const
{
	trepeating_button* widget = new trepeating_button();

	init_control(widget);

	DBG_GUI_G << "Window builder: placed repeating button '"
			<< id << "' with defintion '"
			<< definition << "'.\n";

	return widget;
}

} // namespace implementation

} // namespace gui2

/*WIKI_MACRO
 * @start_macro = repeating_button_description
 *
 *        A repeating_button is a control that can be pushed down and repeat a
 *        certain action. Once the button is down every x milliseconds it is
 *        down a new down event is triggered.
 * @end_macro
 */

/*WIKI
 * @page = GUIWidgetInstanceWML
 * @order = 2_repeating_button
 *
 * == Repeating button ==
 *
 * @macro = repeating_button_description
 *
 */
