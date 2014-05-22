# **************************************************************************
# *
# * Authors:    Jose Gutierrez (jose.gutierrez@cnb.csic.es)
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
# *  e-mail address 'jmdelarosa@cnb.csic.es'
# *
# **************************************************************************

import os
from os.path import basename
import xmipp

from views_util import loadProtocolProject, getResourceCss, getResourceJs, getResourceIcon
from views_protocol import updateProtocolParams
from pyworkflow.em import * 

#from pyworkflow.manager import Manager
#from pyworkflow.project import Project
#from django.shortcuts import render_to_response
#from pyworkflow.gui import getImage, getPILImage
#from django.http import HttpResponse
#from pyworkflow.em.convert import ImageHandler
#
#from pyworkflow.em import SetOfImages, SetOfMicrographs, Volume, SetOfParticles, SetOfVolumes, ProtCTFMicrographs
#from pyworkflow.em.wizard import EmWizard
#
#from pyworkflow.em.packages.xmipp3.convert import xmippToLocation, locationToXmipp
#from pyworkflow.em.packages.spider.convert import locationToSpider
#from pyworkflow.em.packages.spider.wizard import filter_spider
#from pyworkflow.em.packages.xmipp3.constants import *


# Imports for web wizards
from pyworkflow.web.app.wizards.xmipp_wizard import *
from pyworkflow.web.app.wizards.spider_wizard import *
from pyworkflow.web.app.wizards.relion_wizard import *



#===============================================================================
#    Wizard base function (to call the others)
#===============================================================================

def wizard(request):
    # Get the Wizard Name
    requestDict = getattr(request, "POST")
#    functionName = requestDict.get("wizName")

    # Get and instance the wizard class
    className = requestDict.get("wizClassName")
    wizClass = globals().get(className, None)()

    # Get the protocol object
    project, protocol = loadProtocolProject(request)
    updateProtocolParams(request, protocol, project)

    print "======================= in wizard: " + str(wizClass)
    
    # Obtain the parameters for the wizard
    return wizClass._run(protocol, request)


#===============================================================================
#    Wizard common resources (to build the context)
#===============================================================================

def wiz_base(request, context):
    context_base = {'general_style': getResourceCss('general'),
               'wizard_style': getResourceCss('wizard'),
               'jquery_ui_style': getResourceCss('jquery_ui'),
               'font_awesome': getResourceCss('font_awesome'),
               'jquery': getResourceJs('jquery'),
               'jquery_ui': getResourceJs('jquery_ui'),
               'jquery_ui_touch': getResourceJs('jquery_ui_touch'),
               'wizard_utils': getResourceJs('wizard_utils'),
               'raphael':getResourceJs('raphael'),
               'projectName': request.session['projectName'],
               'loading' : getResourceIcon('loading'),
               }
    
    context.update(context_base)
    return context

