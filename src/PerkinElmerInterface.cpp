//###########################################################################
// This file is part of LImA, a Library for Image Acquisition
//
// Copyright (C) : 2009-2011
// European Synchrotron Radiation Facility
// BP 220, Grenoble 38043
// FRANCE
//
// This is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 3 of the License, or
// (at your option) any later version.
//
// This software is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, see <http://www.gnu.org/licenses/>.
//###########################################################################
#include <cmath>
#include "PerkinElmerInterface.h"
#include "PerkinElmerDetInfoCtrlObj.h"
#include "PerkinElmerSyncCtrlObj.h"
#include "PerkinElmerBinCtrlObj.h"
#include <Acq.h>

#include "processlib/SinkTask.h"
#include "processlib/TaskMgr.h"
 
using namespace lima;
using namespace lima::PerkinElmer;

// CALLBACKS
void CALLBACK lima::PerkinElmer::_OnEndAcqCallback(HANDLE handle)
{
  Interface *anInterfacePt;
  Acquisition_GetAcqData(handle,(DWORD*)(&anInterfacePt));
  anInterfacePt->SetEndAcquisition();
}

void CALLBACK lima::PerkinElmer::_OnEndFrameCallback(HANDLE handle)
{
  Interface *anInterfacePt;
  Acquisition_GetAcqData(handle,(DWORD*)(&anInterfacePt));
  anInterfacePt->newFrameReady();
}
// _StopAcq
class _StopAcq : public SinkTaskBase
{
public:
  _StopAcq(Interface &anInterface) : m_interface(anInterface) {}
  virtual ~_StopAcq() {}
  virtual void process(Data&)
  {
    m_interface.stopAcq();
  }
private:
  Interface& m_interface;
};

Interface::Interface() :
  m_acq_desc(NULL),
  m_acq_started(false),
  m_acq_mode(Normal),
  m_first_thrown(false)
{
  DEB_CONSTRUCTOR();

  _InitDetector(m_max_columns,m_max_rows);
  setGain(0);
  m_det_info = new DetInfoCtrlObj(m_acq_desc,m_max_rows,m_max_columns);
  m_sync = new SyncCtrlObj(m_acq_desc);
  m_bin = new BinCtrlObj(m_acq_desc);
  // TMP Double Buffer
  m_tmp_buffer = _aligned_malloc(m_max_columns * m_max_rows * sizeof(unsigned short) * 2,16);

  if(Acquisition_SetCallbacksAndMessages(m_acq_desc, NULL, 0,
					 0, _OnEndFrameCallback, _OnEndAcqCallback) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Could not set callback";
  
  if(Acquisition_SetAcqData(m_acq_desc,(DWORD)this) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Unable to register Acq Data";

  m_cap_list.push_back(HwCap(m_det_info));
  m_cap_list.push_back(HwCap(&m_buffer_ctrl_obj));
  m_cap_list.push_back(HwCap(m_sync));
  m_cap_list.push_back(HwCap(m_bin));
}

Interface::~Interface()
{
  Acquisition_Close(m_acq_desc);
  delete m_det_info;
  delete m_sync;
  _aligned_free(m_tmp_buffer);
}

void Interface::_InitDetector(unsigned int &max_columns,unsigned int &max_rows)
{
  DEB_MEMBER_FUNCT();

  unsigned int sensorNum;
  if(Acquisition_EnumSensors(&sensorNum,1,0) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "No camera found";

  ACQDESCPOS sensorId = 0;
  if(Acquisition_GetNextSensor(&sensorId,&m_acq_desc) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't get detector" << DEB_VAR1(sensorId);
  
  int channel_id;
  const char *channel_type;
  if(!get_channel_type_n_id(m_acq_desc,channel_type,channel_id))
    THROW_HW_ERROR(Error) << "Can't get channel type and number";
  DEB_ALWAYS() << "Acquisition board:" << DEB_VAR2(channel_type,channel_id);

  //Reset Binning
  if(Acquisition_SetCameraBinningMode(m_acq_desc,1) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't reset the Binning";

  unsigned int dwFrames,dwSortFlags,data_type;
  BOOL bEnableIRQ;
  DWORD dwAcqType,system_id,sync_mode,dwHwAccess;
  if(Acquisition_GetConfiguration(m_acq_desc,&dwFrames,
				  &max_rows,&max_columns,&data_type,
				  &dwSortFlags, &bEnableIRQ, &dwAcqType, 
				  &system_id, &sync_mode, &dwHwAccess) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't get detector configuration";
  
  DEB_ALWAYS() << "System Id:" << DEB_VAR1(system_id);
  DEB_ALWAYS() << "Detector size (" << max_columns << "," << max_rows << ")";

  if(Acquisition_SetFrameSyncMode(m_acq_desc,HIS_SYNCMODE_INTERNAL_TIMER) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't set trigger mode to INTERNAL";
}

void Interface::getCapList(CapList &cap_list) const
{
  cap_list = m_cap_list;
}

void Interface::reset(ResetLevel reset_level)
{
  m_acq_started = false;
  m_acq_mode = Normal;
}

void Interface::prepareAcq()
{
  DEB_MEMBER_FUNCT();

  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
  const Size& aSize = buffer_mgr.getFrameDim().getSize();

  if(Acquisition_DefineDestBuffers(m_acq_desc,
				   (unsigned short*)m_tmp_buffer,
				   2,
				   aSize.getHeight(),aSize.getWidth()) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Unable to register destination buffer";

  m_total_acq_frames = m_acq_frame_nb = 0;
  m_first_thrown = false;
}

void Interface::startAcq()
{
  double expo_time;
  m_sync->getExpTime(expo_time);
  m_sync->_setExpTime(expo_time,m_sync->m_keep_first_image);
  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
  buffer_mgr.setStartTimestamp(Timestamp::now());
  m_acq_started = true;
  m_sync->startAcq();
}

void Interface::stopAcq()
{
  DEB_MEMBER_FUNCT();
  Acquisition_Abort(m_acq_desc);
  m_sync->_setExpTimeToMin();
}

void Interface::getStatus(StatusType &status)
{
  DEB_MEMBER_FUNCT();

  if(m_acq_mode == Normal)
    {
      status.set(m_acq_started ? 
	     HwInterface::StatusType::Exposure : HwInterface::StatusType::Ready);
    }
  else
    {
      status.set(m_acq_started ?
		 HwInterface::StatusType::Config : HwInterface::StatusType::Ready);
    }

  DEB_RETURN() << DEB_VAR1(status);
}

int Interface::getNbHwAcquiredFrames()
{
  return m_acq_frame_nb;
}

void Interface::newFrameReady()
{
  DEB_MEMBER_FUNCT();
  if(m_acq_mode != Normal)	// nothing to do
    return;

  if(!m_first_thrown &&			// throw first image
     !m_sync->m_keep_first_image)	// if we don't want it
    {
      m_first_thrown = true;
      return;
    }
  TrigMode aCurrentMode;
  m_sync->getTrigMode(aCurrentMode);
  if(aCurrentMode == ExtStartStop)
    {
      ++m_total_acq_frames;
      if(!(m_total_acq_frames & 0x1))  // we keep odd frame
	return;
    }

  int nb_frame_2_acquire;
  m_sync->getNbHwFrames(nb_frame_2_acquire);
  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_obj.getBuffer();
  HwFrameInfoType frame_info;
  frame_info.acq_frame_nb = m_acq_frame_nb;
  if(!nb_frame_2_acquire || m_acq_frame_nb < nb_frame_2_acquire)
    { 
      void* framePt = buffer_mgr.getFrameBufferPtr(m_acq_frame_nb);
      int bufferid = m_sync->m_keep_first_image ? m_acq_frame_nb++ : ++m_acq_frame_nb;
      const FrameDim& fDim = buffer_mgr.getFrameDim();
      int bufferNb = aCurrentMode == ExtStartStop ? 
	int(!m_sync->m_keep_first_image) : (bufferid & 0x1);
      void* srcPt = ((char*)m_tmp_buffer) + (bufferNb * fDim.getMemSize());
      DEB_TRACE() << "memcpy:" << DEB_VAR2(srcPt,framePt);
      memcpy(framePt,srcPt,fDim.getMemSize());
      bool continueAcq = buffer_mgr.newFrameReady(frame_info);
      if(!continueAcq || m_acq_frame_nb == nb_frame_2_acquire)
	{
	  _StopAcq *aStopAcqPt = new _StopAcq(*this);
	  TaskMgr *mgr = new TaskMgr();
	  mgr->addSinkTask(0,aStopAcqPt);
	  aStopAcqPt->unref();

	  PoolThreadMgr::get().addProcess(mgr);
	}
    }
}

void Interface::SetEndAcquisition()
{
  DEB_MEMBER_FUNCT();
  m_acq_started = false;
  m_acq_mode = Normal;
}

void Interface::setKeepFirstImage(bool aFlag)
{
  m_sync->m_keep_first_image = aFlag;
  //hard expo time sync
  if(!aFlag)
    {
      double expo_time;
      m_sync->getExpTime(expo_time);
      m_sync->_setExpTime(expo_time,true);
    }
}

bool Interface::getKeepFirstImage() const
{
  return m_sync->m_keep_first_image;
}

Interface::CorrMode Interface::getCorrectionMode() const
{
  return m_sync->m_corr_mode;
}

void Interface::setCorrectionMode(Interface::CorrMode aMode)
{
  DEB_MEMBER_FUNCT();
  switch(aMode)
    {
    case OffsetAndGain:
      if(!m_sync->m_gain_data)
	THROW_HW_ERROR(Error) << "Should acquire gain image correction before";
    case OffsetOnly:
      if(!m_sync->m_offset_data)
	THROW_HW_ERROR(Error) << "Should acquire offset image correction before";
      break;
    default:
      break;
    }
  m_sync->m_corr_mode = aMode;
}

int Interface::getGain() const
{
  return m_gain;
}

void Interface::setGain(int gain)
{
  DEB_MEMBER_FUNCT();
  if(gain < 0 || gain > 0x3f)
    THROW_HW_ERROR(Error) << "Gain can be between 0 and 63";
  if(Acquisition_SetCameraGain(m_acq_desc,(WORD)gain) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Could not set gain";
  m_gain = gain;
}

void Interface::startAcqOffsetImage(int nbframes,double time)
{
  DEB_MEMBER_FUNCT();

  Size image_size;
  m_det_info->getDetectorImageSize(image_size);
  m_acq_mode = Offset;
  m_sync->setExpTime(time);
  m_sync->reallocOffset(image_size);
  m_acq_started = true;

  if(Acquisition_Acquire_OffsetImage(m_acq_desc,
				     m_sync->m_offset_data,
				     image_size.getHeight(),
				     image_size.getWidth(),
				     nbframes) != HIS_ALL_OK)
    {

      m_acq_mode = Normal,m_acq_started = false;
      THROW_HW_ERROR(Error) << "Could not start acquisition of Offset image";
    }
  m_sync->m_corr_expo_time = time;
}

void Interface::startAcqGainImage(int nbframes,double time)
{
  DEB_MEMBER_FUNCT();

  if(fabs(time - m_sync->m_corr_expo_time) > 1e-6)
    THROW_HW_ERROR(Error) << "Gain image should be taken with the same time of Offset image";
  if(!m_sync->m_offset_data)
    THROW_HW_ERROR(Error) << "Must take Offset image correction before";

  Size image_size;
  m_det_info->getDetectorImageSize(image_size);
  m_acq_mode = Gain;
  m_sync->setExpTime(time);
  m_sync->reallocGain(image_size);
  
  DEB_TRACE() << DEB_VAR2(m_sync->m_offset_data,m_sync->m_gain_data);

  m_acq_started = true;
  if(Acquisition_Acquire_GainImage(m_acq_desc,
				   m_sync->m_offset_data,
				   m_sync->m_gain_data,
				   image_size.getHeight(),
				   image_size.getWidth(),
				   nbframes) != HIS_ALL_OK)
    {
      m_acq_mode = Normal,m_acq_started = false;
      THROW_HW_ERROR(Error) << "Could not start acquisition of Gain image";
    }
}
/*============================================================================
			   Static Methodes
============================================================================*/
bool Interface::get_channel_type_n_id(HANDLE &acq_desc,
				      const char* &channel_type,
				      int &channel_id)
{
  UINT nChannelType;

  if(Acquisition_GetCommChannel(acq_desc,&nChannelType,&channel_id) != HIS_ALL_OK)
    return false;

  switch(nChannelType)
    {
    case HIS_BOARD_TYPE_ELTEC: 
      channel_type = "XRD-FG or XRD-FGe Frame Grabber";break;
    case HIS_BOARD_TYPE_ELTEC_XRD_FGX:
      channel_type = "XRD-FGX frame grabber";break;
    case HIS_BOARD_TYPE_ELTEC_XRD_FGE_Opto:
      channel_type = "XRD-FGe Opto";break;
    case HIS_BOARD_TYPE_ELTEC_GbIF:
      channel_type = "GigabitEthernet";break;
    default:
      channel_type = "Unknown";break;
    }
  return true;
}

std::ostream& lima::PerkinElmer::operator<<(std::ostream &os,Interface::CorrMode aMode)
{
  const char *name;
  switch(aMode)
    {
    case Interface::OffsetOnly: name = "Offset correction";break;
    case Interface::OffsetAndGain: name = "Offset and gain correction";break;
    default: name = "No Correction";break;
    }
  return os << name;
}
