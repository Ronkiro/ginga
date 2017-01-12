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

#ifndef ITimeBaseListener_H_
#define ITimeBaseListener_H_

#include <stdint.h>

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace system {
namespace time {
	class ITimeBaseListener {
		public:
			virtual ~ITimeBaseListener(){};

			virtual void timeBaseNaturalEnd(
					unsigned char contentId,
					double timeValue)=0;

			virtual void updateTimeBaseId(
					unsigned char oldContentId,
					unsigned char newContentId)=0;

			virtual void valueReached(
					unsigned char timeBaseId, double timeValue)=0;

			virtual void loopDetected()=0;
	};
}
}
}
}
}
}
}

#endif /*ITimeBaseListener_H_*/
