/******************************************************************************
Este arquivo eh parte da implementacao do ambiente declarativo do middleware
Ginga (Ginga-NCL).

Direitos Autorais Reservados (c) 1989-2007 PUC-Rio/Laboratorio TeleMidia

Este programa eh software livre; voce pode redistribui-lo e/ou modificah-lo sob
os termos da Licenca Publica Geral GNU versao 2 conforme publicada pela Free
Software Foundation.

Este programa eh distribuido na expectativa de que seja util, porem, SEM
NENHUMA GARANTIA; nem mesmo a garantia implicita de COMERCIABILIDADE OU
ADEQUACAO A UMA FINALIDADE ESPECIFICA. Consulte a Licenca Publica Geral do
GNU versao 2 para mais detalhes.

Voce deve ter recebido uma copia da Licenca Publica Geral do GNU versao 2 junto
com este programa; se nao, escreva para a Free Software Foundation, Inc., no
endereco 59 Temple Street, Suite 330, Boston, MA 02111-1307 USA.

Para maiores informacoes:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
******************************************************************************
This file is part of the declarative environment of middleware Ginga (Ginga-NCL)

Copyright: 1989-2007 PUC-RIO/LABORATORIO TELEMIDIA, All Rights Reserved.

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by
the Free Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE.  See the GNU General Public License version 2 for more
details.

You should have received a copy of the GNU General Public License version 2
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA

For further information contact:
ncl @ telemidia.puc-rio.br
http://www.ncl.org.br
http://www.ginga.org.br
http://www.telemidia.puc-rio.br
*******************************************************************************/

#include "config.h"

#include "tuner/providers/frontends/isdbt/ISDBTProvider.h"

#include "util/functions.h"
using namespace ::br::pucrio::telemidia::util;

namespace br {
namespace pucrio {
namespace telemidia {
namespace ginga {
namespace core {
namespace tuning {
	const string ISDBTProvider::iniFileName(
			SystemCompat::appendGingaFilesPrefix("tuner/isdbt.ini"));

	ISDBTProvider::ISDBTProvider(string feName) {
		this->frontend       = NULL;
		this->fileName       = feName;
		this->feDescriptor   = -1;
		this->channels       = new vector<IChannel*>;
		this->currentChannel = channels->end();
		this->listener       = NULL;
		this->capabilities   = (
				DPC_CAN_FETCHDATA |
				DPC_CAN_DEMUXBYHW |
				DPC_CAN_FILTERPID |
				DPC_CAN_FILTERTID);
	}

	ISDBTProvider::~ISDBTProvider() {
		close();
	}

	void ISDBTProvider::setListener(ITProviderListener* listener) {
		this->listener = listener;
	}

	short ISDBTProvider::getCaps() {
		return capabilities;
	}

	void ISDBTProvider::attachFilter(IFrontendFilter* filter) {
		if (frontend != NULL) {
			frontend->attachFilter(filter);
		}
	}

	void ISDBTProvider::removeFilter(IFrontendFilter* filter) {
		if (frontend != NULL) {
			frontend->removeFilter(filter);
		}
	}

	void ISDBTProvider::initializeChannels() {
		ifstream fis;
		string token, id, name, freq, seg;
		bool skipLine = false;
		IChannel* channel;

		channels->clear();

		fis.open(iniFileName.c_str(), ifstream::in);

		if (!fis.is_open()) {
			clog << "ISDBTProvider::initializeChannels ";
			clog << "can't open '" << iniFileName << "'" << endl;

		} else {
			fis >> token;
			while (fis.good()) {
				if (token.substr(0, 1) == "#") {
					skipLine = true;
				} else {
					skipLine = false;
				}

				if (token == "id") {
					fis >> token;
					id = token;
					fis >> token;
					if (token == "name") {
						fis >> token;
						name = token;
						fis >> token;
						if (token == "frequency") {
							fis >> token;
							freq = token;
							fis >> token;
							if (token == "segment") {
								fis >> token;
								seg = token;

								if (!skipLine) {
									channel = new Channel();
									channel->setId((short)(stof(id)));
									channel->setName(name);

									channel->setFrequency(
											(unsigned int)(stof(freq)));

									if (seg == "FULLSEG") {
										channel->setSegment(true);
									} else {
										channel->setSegment(false);
									}

									channels->push_back(channel);
								}

							} else if (!skipLine) {
								clog << "ISDBTProvider::initializeChannels ";
								clog << " token segment not found";
								clog << " current token is '" << token;
								clog << "'" << endl;
							}

						} else if (!skipLine) {
							clog << "ISDBTProvider::initializeChannels token";
							clog << " frequency not found";
							clog << " current token is '" << token;
							clog << "'" << endl;
						}

					} else if (!skipLine) {
						clog << "ISDBTProvider::initializeChannels token";
						clog << " name not found";
						clog << " current token is '" << token;
						clog << "'" << endl;
					}

				} else if (!skipLine) {
					clog << "ISDBTProvider::initializeChannels token";
					clog << " id not found";
					clog << " current token is '" << token;
					clog << "'" << endl;
				}
				fis >> token;
			}
			fis.close();
		}

		if (channels->empty()) {
			frontend->scanFrequencies(channels);
		}
	}

	bool ISDBTProvider::tune() {
		bool tuned = false;
		IChannel* channel;

		if (frontend != NULL) {
			return true;
		}

		if ((feDescriptor = open(
				ISDBTFrontend::IFE_FE_DEV_NAME.c_str(), O_RDWR)) < 0) {

			clog << "ISDBTProvider::tune failed" << endl;
			return false;
		}

		frontend = new ISDBTFrontend(feDescriptor);
		if (frontend->hasFrontend()) {
			initializeChannels();
			if (channels->empty()) {
				clog << "ISDBTProvider::tune no frequencies found";
				clog << endl;
				return false;
			}
			currentChannel = channels->begin();
			while (!tuned) {
				channel = *currentChannel;
				tuned = frontend->changeFrequency(channel->getFrequency());
				if (!tuned) {
					++currentChannel;
					if (currentChannel == channels->end()) {
						clog << "ISDBTProvider::tune all frequencies failed";
						clog << endl;
						break;
					}

				} else {
					clog << "ISDBTProvider::tune tuned at '";
					clog << channel->getFrequency() << "' - ";
					clog << channel->getName() << endl;
				}
			}

			return tuned;

		} else {
			close();
			return false;
		}
	}

	IChannel* ISDBTProvider::getCurrentChannel() {
		IChannel* channel = NULL;

		if (currentChannel != channels->end()) {
			channel = *currentChannel;
		}

		return channel;
	}

	bool ISDBTProvider::getSTCValue(uint64_t* stc, int* valueType) {
		if (frontend == NULL) {
			return false;
		}

		return frontend->getSTCValue(stc, valueType);
	}

	bool ISDBTProvider::changeChannel(int factor) {
		int freq = 0;
		IChannel* channel;

		if (channels->empty() || frontend == NULL) {
			clog << "ISDBTProvider::changeChannel return false: ";
			clog << "number of channels is " << channels->size();
			clog << " and frontend address is " << frontend << endl;
			return false;
		}

		if (factor == 1) {
			++currentChannel;
			if (currentChannel == channels->end()) {
				currentChannel = channels->begin();
			}

		} else if (factor == -1) {
			if (currentChannel == channels->begin()) {
				currentChannel = channels->end();
			}
			--currentChannel;
		}

		channel = *currentChannel;
		freq = (*currentChannel)->getFrequency();

		if (frontend->changeFrequency(freq)) {
			clog << "ISDBTProvider::changeChannel tuned at '" << freq;
			clog << "' - " << channel->getName() << "" << endl;
			return true;
		}

		return false;
	}

	bool ISDBTProvider::setChannel(string channelValue) {
		return frontend->changeFrequency(stof(channelValue));
	}

	int ISDBTProvider::createPesFilter(
			int pid, int pesType, bool compositeFiler) {

		if (frontend != NULL) {
			return frontend->createPesFilter(pid, pesType, compositeFiler);
		}

		return -1;
	}

	string ISDBTProvider::getPesFilterOutput() {
		return ISDBTFrontend::IFE_DVR_DEV_NAME;
	}

	void ISDBTProvider::close() {
		if (feDescriptor > 0) {
			::close(feDescriptor);
			feDescriptor = 0;
		}

		if (frontend != NULL) {
			delete frontend;
			frontend = NULL;
		}
	}

	int ISDBTProvider::receiveData(char* buff) {
		if (feDescriptor > 0) {
			return read(feDescriptor, (void*)buff, BUFFSIZE);
		}

		return 0;
	}
}
}
}
}
}
}
