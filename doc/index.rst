.. _camera-perkinelmer:

.. toctree::
  :maxdepth: 3

Perkin Elmer camera
-------------------

.. image:: perkinelmer.png
   :scale: 50

Intoduction
```````````
PerkinElmer is a world leader in the design, development, and manufacture of Amorphous Silicon (aSi) Flat Panel Detectors (FPD) designed to perform across a wide range of medical, veterinary, and industrial, Non-Destructive Testing (NDT) applications. Our XRD family of detectors provide superior image resolution, high frame rates up to 30 frames per seconds (fps), energy levels form 20 keV -15 MeV and easy information storage and retrieval.

The detector model we tested (ESRF) is : XRD 1621 CN ES
 
Prerequisite Windows 7
``````````````````````
Previously to this you have to install the Perkinelmer Windows7 SDK to the default path.


Installation & Module configuration
````````````````````````````````````

-  follow first the steps for the linux installation :ref:`linux_installation`

The minimum configuration file is *config.inc* :

.. code-block:: sh

  COMPILE_CORE=1
  COMPILE_SIMULATOR=0
  COMPILE_SPS_IMAGE=1
  COMPILE_ESPIA=0
  COMPILE_FRELON=0
  COMPILE_MAXIPIX=0
  COMPILE_PILATUS=-1
  COMPILE_PERKINELMER=1
  COMPILE_CBF_SAVING=0
  export COMPILE_CORE COMPILE_SPS_IMAGE COMPILE_SIMULATOR \
         COMPILE_ESPIA COMPILE_FRELON COMPILE_MAXIPIX COMPILE_PILATUS \
         COMPILE_PERKINELMER COMPILE_CBF_SAVING


-  start the compilation :ref:`linux_compilation`

-  finally for the Tango server installation :ref:`tango_installation`

Initialisation and Capabilities
````````````````````````````````

Camera initialisation
......................

The camera will be initialized  by created the PerkinElmer::Interface object. The contructor
will take care of your detector configuration according to the SDK installation setup done before.

Std capabilites
................

This plugin has been implement in respect of the mandatory capabilites but with some limitations which
are due to the camera and SDK features.  We provide here further information for a better understanding
of the detector specific capabilities.

* HwDetInfo
  
  getCurrImageType/getDefImageType():  Bpp16 only.

  setCurrImageType(): this method do not change the image type which is fixed to Bpp16.

* HwSync

  get/setTrigMode(): the supported mode are IntTrig, ExtStartStop, ExtTrigReadout
  
Optional capabilites
........................
In addition to the standard capabilities, we make the choice to implement some optional capabilities which
are supported by the SDK and the I-Kon cameras. A Shutter control, a hardware ROI and a hardware Binning are available.

* HwBin 
  
  Some camera models support binning 4x4, 2x2, 4x2 4x2 and 1x1  and others support only 2x2.
  Camera type si provided when initing the sdk (_InitDetector()) and only camera of type 15 supports
  the long range of binning.

Configuration
`````````````

 - Nothing special to do, but read the manual for proper installation. 


How to use
````````````
This is a python code example for a simple test:

.. code-block:: python

  from Lima import PerkinElmer
  from lima import Core

  hwint = PerkinElmer.Interface()
  ct = Core.CtControl(hwint)

  acq = ct.acquisition()

  # set offset and gain calibration, one image 1.0 second exposure 
  hwint.startAcqOffsetImage(1, 1.0)
  hwint.startAcqGainImage(1, 1.0)

  # set further hardware configuration
  print (hwint.getGain())
  hwint.setCorrectionMode(hwint.OffsetAndGain) # or No or OffsetOnly
  hwint.setKeepFirstImage(False)

  # setting new file parameters and autosaving mode
  saving=ct.saving()

  pars=saving.getParameters()
  pars.directory='/buffer/lcb18012/opisg/test_lima'
  pars.prefix='test1_'
  pars.suffix='.edf'
  pars.fileFormat=Core.CtSaving.EDF
  pars.savingMode=Core.CtSaving.AutoFrame
  saving.setParameters(pars)

  # set accumulation mode

  acq_pars= acq.getPars()

  #0-normal,1-concatenation,2-accumu
  acq_pars.acqMode = 2
  acq_pars.accMaxExpoTime = 0.05
  acq_pars.acqExpoTime =1
  acq_pars.acqNbFrames = 1

  acq.setPars(acq_pars)
  # here we should have 21 accumalated images per frame
  print (acq.getAccNbFrames())

  # now ask for 2 sec. exposure and 10 frames
  acq.setAcqExpoTime(2)
  acq.setNbImages(10) 
  
  ct.prepareAcq()
  ct.startAcq()

  # wait for last image (#9) ready
  lastimg = ct.getStatus().ImageCounters.LastImageReady
  while lastimg !=9:
    time.sleep(1)
    lastimg = ct.getStatus().ImageCounters.LastImageReady
 
  # read the first image
  im0 = ct.ReadImage(0)

