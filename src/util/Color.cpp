/* Copyright (C) 2006-2017 PUC-Rio/Laboratorio TeleMidia

This file is part of Ginga (Ginga-NCL).

Ginga is free software: you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

Ginga is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public
License for more details.

You should have received a copy of the GNU General Public License
along with Ginga.  If not, see <http://www.gnu.org/licenses/>.  */

#include "config.h"
#include "util/Color.h"
#include "util/functions.h"

GINGA_UTIL_BEGIN

  	const string Color::swhite = "#FFFFFF";
	const string Color::syellow = "#FFFF00";
	const string Color::sred = "#FF0000";
	const string Color::spink = "#FFC0CB";
	const string Color::sorange = "#FFA500";
	const string Color::smagenta = "#FF00FF";
	const string Color::sgreen = "#008000";
	const string Color::scyan = "#00FFFF";
	const string Color::sblue = "#0000FF";
	const string Color::slightGray = "#D3D3D3";
	const string Color::sgray = "#808080";
	const string Color::sdarkGray = "#A9A9A9";
	const string Color::sblack = "#101010";
	const string Color::ssilver = "#C0C0C0";
	const string Color::smaroon = "#800000";
	const string Color::sfuchsia = "#FF00FF";
	const string Color::spurple = "#800080";
	const string Color::slime = "#00FF00";
	const string Color::solive = "#808000";
	const string Color::snavy = "#000080";
	const string Color::saqua = "#00FFFF";
	const string Color::steal = "#008080";


	void Color::setColorToI(string color) {
		if (color == "") {
			color = "#FFFFFF";
		}

		if (color.substr(0,1)=="#" && color.length() > 6) {
			r = (int)(::ginga::util::stof(color.substr(1, 2)));
			g = (int)(::ginga::util::stof(color.substr(3, 2)));
			b = (int)(::ginga::util::stof(color.substr(5, 2)));
			return;
		}

		if (color=="black") {
			setColor(0x10, 0x10, 0x10);

		} else if (color=="blue") {
			setColor(0x00, 0x00, 0xFF);

		} else if (color=="cyan") {
			setColor(0x00, 0xFF, 0xFF);

		} else if (color=="darkGray") {
			setColor(0xA9, 0xA9, 0xA9);

		} else if (color=="gray") {
			setColor(0x80, 0x80, 0x80);

		} else if (color=="green") {
			setColor(0x00, 0x80, 0x00);

		} else if (color=="lightGray") {
			setColor(0xD3, 0xD3, 0xD3);

		} else if (color=="magenta") {
			setColor(0xFF, 0x00, 0xFF);

		} else if (color=="orange") {
			setColor(0xFF, 0xA5, 0x00);

		} else if (color=="pink") {
			setColor(0xFF, 0xC0, 0xCB);

		} else if (color=="red") {
			setColor(0xFF, 0x00, 0x00);

		} else if (color=="yellow") {
			setColor(0xFF, 0xFF, 0x00);

		} else if (color=="silver") {
			setColor(0xC0, 0xC0, 0xC0);

		} else if (color=="maroon") {
			setColor(0x80, 0x00, 0x00);

		} else if (color=="fuchsia") {
			setColor(0xFF, 0x00, 0xFF);

		} else if (color=="purple") {
			setColor(0x80, 0x00, 0x80);

		} else if (color=="lime") {
			setColor(0x00, 0xFF, 0x00);

		} else if (color=="olive") {
			setColor(0x80, 0x80, 0x00);

		} else if (color=="navy") {
			setColor(0x00, 0x00, 0x80);

		} else if (color=="aqua") {
			setColor(0x00, 0xFF, 0xFF);

		} else if (color=="teal") {
			setColor(0x00, 0x80, 0x80);

		} else {
			setColor(0xFF, 0xFF, 0xFF);
		}
	}

	int Color::colortoi(string color) {
		if (color == "") {
			return Color::white;
		}

		if (color.substr(0,1) == "#" && color.length() > 6) {
			int red, green, blue;

			red   = (int)(::ginga::util::stof(color.substr(1, 2)));
			green = (int)(::ginga::util::stof(color.substr(3, 2)));
			blue  = (int)(::ginga::util::stof(color.substr(5, 2)));

			return 256*256*red + 256*green + blue;
		}

		if (color=="black") {
			return Color::black;
		}
		else if (color=="blue") {
			return Color::blue;
		}
		else if (color=="cyan") {
			return Color::cyan;
		}
		else if (color=="darkGray") {
			return Color::darkGray;
		}
		else if (color=="gray") {
			return Color::gray;
		}
		else if (color=="green") {
			return Color::green;
		}
		else if (color=="lightGray") {
			return Color::lightGray;
		}
		else if (color=="magenta") {
			return Color::magenta;
		}
		else if (color=="orange") {
			return Color::orange;
		}
		else if (color=="pink") {
			return Color::pink;
		}
		else if (color=="red") {
			return Color::red;
		}
		else if (color=="yellow") {
			return Color::yellow;
		}
		else if (color=="silver") {
			return Color::silver;

		} else if (color=="maroon") {
			return Color::maroon;

		} else if (color=="fuchsia") {
			return Color::fuchsia;

		} else if (color=="purple") {
			return Color::purple;

		} else if (color=="lime") {
			return Color::lime;

		} else if (color=="olive") {
			return Color::olive;

		} else if (color=="navy") {
			return Color::navy;

		} else if (color=="aqua") {
			return Color::aqua;

		} else if (color=="teal") {
			return Color::teal;
		}
		else {
			return Color::white;
		}
	};

	Color::Color() {
		this->alpha = 255;
	}

	Color::Color(int r, int g, int b, int alpha) {
		this->r     = r;
		this->g     = g;
		this->b     = b;
		this->alpha = alpha;
	}

	Color::Color(string color, int alpha) {
		this->alpha = alpha;
		setColorToI(color);
	}

	void Color::setColor(string color) {
		setColorToI(color);
	}

	void Color::setColor(int red, int green, int blue) {
		this->r     = red;
		this->g     = green;
		this->b     = blue;
		this->alpha = 0xFF;
	}

	int Color::getR() {
		return r;
	}

	int Color::getG() {
		return g;
	}

	int Color::getB() {
		return b;
	}

	int Color::getAlpha() {
		return alpha;
	}

	uint32_t Color::getRGBA() {
		return (r << 24) & 0xFF000000 +
				(g << 16) & 0x00FF0000 +
				(b << 8) & 0x0000FF00 +
				alpha & 0x000000FF;
	}

	uint32_t Color::getARGB() {
		return (alpha << 24) & 0xFF000000 +
				(r << 16) & 0x00FF0000 +
				(g << 8) & 0x0000FF00 +
				b & 0x000000FF;
	}

GINGA_UTIL_END
