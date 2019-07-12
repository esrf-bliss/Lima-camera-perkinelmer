############################################################################
# This file is part of LImA, a Library for Image Acquisition
#
# Copyright (C) : 2009-2011
# European Synchrotron Radiation Facility
# BP 220, Grenoble 38043
# FRANCE
#
# This is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This software is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
############################################################################

import PyTango
import sys

from Lima import Core
from Lima import PerkinElmer as PerkinElmerModule
from Lima.Server import AttrHelper

#==================================================================
#   PerkinElmer Class Description:
#
#
#==================================================================


class PerkinElmer(PyTango.Device_4Impl):

#--------- Add you global variables here --------------------------
    Core.DEB_CLASS(Core.DebModApplication, 'LimaCCDs')

#------------------------------------------------------------------
#    Device constructor
#------------------------------------------------------------------
    def __init__(self,cl, name):
        PyTango.Device_4Impl.__init__(self,cl,name)
        self.init_device()

        self.__CorrectionMode = {'NO' : _PerkinElmerIterface.No,
                                 'OFFSET ONLY' : _PerkinElmerIterface.OffsetOnly,
                                 'OFFSET AND GAIN' : _PerkinElmerIterface.OffsetAndGain}
	
        self.__KeepFirstImage = {'YES' : True,
                                 'NO' : False}
	
        self.__Attribute2FunctionBase = {'gain': 'Gain',
                                         'correction_mode': 'CorrectionMode',
                                         'keep_first_image': 'KeepFirstImage',
            }
#------------------------------------------------------------------
#    Device destructor
#------------------------------------------------------------------
    def delete_device(self):
        pass


#------------------------------------------------------------------
#    Device initialization
#------------------------------------------------------------------
    def init_device(self):
        self.set_state(PyTango.DevState.ON)
        self.get_device_properties(self.get_device_class())

#==================================================================
#
#    PerkinElmer read/write attribute methods
#
#==================================================================

    def __getattr__(self,name) :
        return AttrHelper.get_attr_4u(self,name,_PerkinElmerIterface)
	
#==================================================================
#
#    PerkinElmer command methods
#
#==================================================================
            
#------------------------------------------------------------------
#    getAttrStringValueList command:
#
#    Description: return a list of authorized values if any
#    argout: DevVarStringArray   
#------------------------------------------------------------------
    @Core.DEB_MEMBER_FUNCT
    def getAttrStringValueList(self, attr_name):
        return AttrHelper.get_attr_string_value_list(self, attr_name)

    @Core.DEB_MEMBER_FUNCT
    def startAcqOffsetImage(self,nbImageNtime) :
        nbImage = int(nbImageNtime[0])
        expTime = nbImageNtime[1]
        _PerkinElmerIterface.startAcqOffsetImage(nbImage,expTime)

    @Core.DEB_MEMBER_FUNCT
    def startAcqGainImage(self,nbImageNtime) :
        nbImage = int(nbImageNtime[0])
        expTime = nbImageNtime[1]
        _PerkinElmerIterface.startAcqGainImage(nbImage,expTime)
#==================================================================
#
#    PerkinElmerClass class definition
#
#==================================================================
class PerkinElmerClass(PyTango.DeviceClass):

    #    Class Properties
    class_property_list = {
        }


    #    Device Properties
    device_property_list = {
        }


    #    Command definitions
    cmd_list = {
        'getAttrStringValueList':
        [[PyTango.DevString, "Attribute name"],
         [PyTango.DevVarStringArray, "Authorized String value list"]],
        'startAcqOffsetImage':
        [[PyTango.DevVarDoubleArray, "nb frames,exposure time"],
         [PyTango.DevVoid]],
        'startAcqGainImage':
        [[PyTango.DevVarDoubleArray, "nb frames,exposure time"],
         [PyTango.DevVoid]],
        }


    #    Attribute definitions
    attr_list = {
        'correction_mode':
            [[PyTango.DevString,
            PyTango.SCALAR,
            PyTango.READ_WRITE]],
	'gain':
	[[PyTango.DevLong,
	  PyTango.SCALAR,
	  PyTango.READ_WRITE]],
	'keep_first_image':
	[[PyTango.DevString,
	  PyTango.SCALAR,
	  PyTango.READ_WRITE]],
        }


#------------------------------------------------------------------
#    PerkinElmerClass Constructor
#------------------------------------------------------------------
    def __init__(self, name):
        PyTango.DeviceClass.__init__(self, name)
        self.set_type(name)

#----------------------------------------------------------------------------
# Plugins
#----------------------------------------------------------------------------
_PerkinElmerIterface = None

def get_control(**keys) :
    global _PerkinElmerIterface
    if _PerkinElmerIterface is None:
        _PerkinElmerIterface = PerkinElmerModule.Interface()
    return Core.CtControl(_PerkinElmerIterface)


def get_tango_specific_class_n_device() :
    return PerkinElmerClass,PerkinElmer
