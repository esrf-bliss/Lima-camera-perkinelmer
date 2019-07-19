PerkinElmer Tango device
========================

This is the reference documentation of the PerkinElmer Tango device.

you can also find some useful information about the camera models/prerequisite/installation/configuration/compilation in the :ref:`PerkinElmer camera plugin <camera-perkinelmer>` section.


Properties
----------
This device has no property.



Attributes
----------
======================= ======= ======================= ======================================================================
Attribute name		RW	Type			Description
======================= ======= ======================= ======================================================================
correction_mode		rw	DevString	 	'NO', 'OFFSET ONLY' or 'OFFSET AND GAIN'
gain                    rw      DevLong                 The gain value, from 0 to 63
keep_first_image        rw      DevString               'YES' or 'NO', you can decide to trash the 1st image
======================= ======= ======================= ======================================================================

Commands
--------
=======================	======================== ======================= ===========================================
Command name		Arg. in		         Arg. out		 Description
=======================	======================== ======================= ===========================================
Init			DevVoid 	         DevVoid		 Do not use
State			DevVoid		         DevLong		 Return the device state
Status			DevVoid		         DevString		 Return the device state as a string
getAttrStringValueList	DevString:	         DevVarStringArray:	 Return the authorized string value list for
			Attribute name	         String value list	 a given attribute name
startAcqOffsetImage     DevVarDoubleArray:       DevVoid                 Start acquisition for an offset calibration
                        nb_frames, exposure_time
startAcqGainImage       DevVarDoubleArray:       DevVoid                 Start an acquisition for an gain calibration
                        nb_frames, exposure_time 
=======================	======================== ======================= ===========================================
