###############################################################################
# 
#  Copyright (2014) Alexander Stukowski
#
#  This file is part of OVITO (Open Visualization Tool).
#
#  OVITO is free software; you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation; either version 2 of the License, or
#  (at your option) any later version.
#
#  OVITO is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################

ADD_SUBDIRECTORY(botan)
ADD_SUBDIRECTORY(qssh)
IF(OVITO_BUILD_GUI)
    ADD_SUBDIRECTORY(qwt)
ENDIF()
IF(OVITO_BUILD_PLUGIN_PARTICLES)
    ADD_SUBDIRECTORY(muparser)
    ADD_SUBDIRECTORY(voro++)
    ADD_SUBDIRECTORY(ptm)
    ADD_SUBDIRECTORY(copr)
ENDIF()
IF(OVITO_BUILD_PLUGIN_TACHYON)
	ADD_SUBDIRECTORY(tachyon)
ENDIF()
IF(OVITO_BUILD_PLUGIN_CRYSTALANALYSIS)
	ADD_SUBDIRECTORY(geogram)
ENDIF()
