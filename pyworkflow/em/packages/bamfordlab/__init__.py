# **************************************************************************
# *
# * Authors:     J.M. De la Rosa Trevin (jmdelarosa@cnb.csic.es)
# *
# * Unidad de  Bioinformatica of Centro Nacional de Biotecnologia , CSIC
# *
# * This program is free software; you can redistribute it and/or modify
# * it under the terms of the GNU General Public License as published by
# * the Free Software Foundation; either version 2 of the License, or
# * (at your option) any later version.
# *
# * This program is distributed in the hope that it will be useful,
# * but WITHOUT ANY WARRANTY; without even the implied warranty of
# * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# * GNU General Public License for more details.
# *
# * You should have received a copy of the GNU General Public License
# * along with this program; if not, write to the Free Software
# * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA
# * 02111-1307  USA
# *
# *  All comments concerning this program package may be sent to the
# *  e-mail address 'scipion@cnb.csic.es'
# *
# **************************************************************************
import os

from pyworkflow.utils import commandExists

_logo = None

ETHAN_HOME = 'ETHAN_HOME'
ETHAN = 'ethan'

from bibtex import _bibtex
from protocol_ethan_picking import ProtEthanPicker
# from wizard import DogPickerWizard    --> Not ready yet: uncomment when ready.


_references = ['Kivioja2000']


def validateInstallation():
    """ This function will be used to check if RELION is properly installed. """
    missingPaths = []

    if not (os.path.exists(os.environ.get(ETHAN_HOME))
            or commandExists(ETHAN)):
        missingPaths.append("%s or %s: %s" % (ETHAN_HOME, ETHAN,
                                              "Ethan not found in the system"))

    if missingPaths:
        return ["Missing variables:"] + missingPaths
    else:
        return [] # No errors