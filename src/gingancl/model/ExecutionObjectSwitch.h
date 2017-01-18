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

#ifndef _EXECUTIONOBJECTSWITCH_H_
#define _EXECUTIONOBJECTSWITCH_H_

#include "ncl/Node.h"
using namespace ::ginga::ncl;

#include "ncl/LambdaAnchor.h"
using namespace ::ginga::ncl;

#include "CompositeExecutionObject.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::components;

#include "SwitchEvent.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::switches;

#include "FormatterEvent.h"
#include "PresentationEvent.h"
using namespace ::br::pucrio::telemidia::ginga::ncl::model::event;

#include <string>
using namespace std;

BR_PUCRIO_TELEMIDIA_GINGA_NCL_MODEL_SWITCHES_BEGIN

  class ExecutionObjectSwitch : public CompositeExecutionObject {
	protected:
		ExecutionObject* selectedObject;

	public:
		ExecutionObjectSwitch(
				string id,
				Node* switchNode,
				bool handling,
				ILinkActionListener* seListener);

		virtual ~ExecutionObjectSwitch(){};
		ExecutionObject* getSelectedObject();
		void select(ExecutionObject* executionObject);
		bool addEvent(FormatterEvent* event);
  };

BR_PUCRIO_TELEMIDIA_GINGA_NCL_MODEL_SWITCHES_END
#endif //_EXECUTIONOBJECTSWITCH_H_
