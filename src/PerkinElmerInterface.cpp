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
#include <Acq.h>

#include <SinkTask.h>
#include <TaskMgr.h>
 
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
  m_acq_mode(Normal)
{
  DEB_CONSTRUCTOR();

  unsigned int max_columns,max_rows;
  _InitDetector(max_columns,max_rows);
  m_det_info = new DetInfoCtrlObj(m_acq_desc,max_columns,max_rows);
  m_sync = new SyncCtrlObj(m_acq_desc);
  // TMP Double Buffer
  m_tmp_buffer = _aligned_malloc(max_columns * max_rows * sizeof(unsigned short) * 2,16);
  if(Acquisition_DefineDestBuffers(m_acq_desc,
				   (unsigned short*)m_tmp_buffer,
				   2,
				   max_rows,max_columns) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Unable to register destination buffer";

  if(Acquisition_SetCallbacksAndMessages(m_acq_desc, NULL, 0,
					 0, _OnEndFrameCallback, _OnEndAcqCallback) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Could not set callback";
  
  if(Acquisition_SetAcqData(m_acq_desc,(DWORD)this) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Unable to register Acq Data";

  m_cap_list.push_back(HwCap(m_det_info));
  m_cap_list.push_back(HwCap(&m_buffer_ctrl_mgr));
  m_cap_list.push_back(HwCap(m_sync));

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

  //Reset Roi
  if(Acquisition_SetCameraROI(m_acq_desc,0xf) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't reset roi";

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
  m_acq_frame_nb = 0;
}

void Interface::startAcq()
{
  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_mgr.getBuffer();
  buffer_mgr.setStartTimestamp(Timestamp::now());
  m_acq_started = true;
  m_sync->startAcq();
}

void Interface::stopAcq()
{
  DEB_MEMBER_FUNCT();
  Acquisition_Abort(m_acq_desc);
}

void Interface::getStatus(StatusType &status)
{
  DEB_MEMBER_FUNCT();
  status.set(m_acq_started ? 
	     HwInterface::StatusType::Exposure : HwInterface::StatusType::Ready);
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

  int nb_frame_2_acquire;
  m_sync->getNbHwFrames(nb_frame_2_acquire);
  StdBufferCbMgr& buffer_mgr = m_buffer_ctrl_mgr.getBuffer();
  HwFrameInfoType frame_info;
  frame_info.acq_frame_nb = m_acq_frame_nb;
  if(!nb_frame_2_acquire || m_acq_frame_nb < nb_frame_2_acquire)
    { 
      void* framePt = buffer_mgr.getFrameBufferPtr(m_acq_frame_nb);
      Size max_image_size;
      m_det_info->getMaxImageSize(max_image_size);
      void* srcPt = ((char*)m_tmp_buffer) + ((m_acq_frame_nb & 0x1) * 
					     max_image_size.getWidth() *
					     max_image_size.getHeight() *
					     sizeof(unsigned short));
      const FrameDim& fDim = buffer_mgr.getFrameDim();
      DEB_TRACE() << "memcpy:" << DEB_VAR2(srcPt,framePt);
      memcpy(framePt,srcPt,fDim.getMemSize());
      bool continueAcq = buffer_mgr.newFrameReady(frame_info);
      ++m_acq_frame_nb;
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

void Interface::startAcqOffsetImage(int nbframes,double time)
{
  DEB_MEMBER_FUNCT();

  Size image_size;
  m_det_info->getDetectorImageSize(image_size);
  m_acq_mode = Offset;
  m_sync->setExpTime(time);
  m_sync->reallocOffset(image_size);
      
  if(Acquisition_Acquire_OffsetImage(m_acq_desc,
				     m_sync->m_offset_data,
				     image_size.getHeight(),
				     image_size.getWidth(),
				     nbframes) != HIS_ALL_OK)
    {
      m_acq_mode = Normal;
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

  if(Acquisition_Acquire_GainImage(m_acq_desc,
				   m_sync->m_offset_data,
				   m_sync->m_gain_data,
				   image_size.getHeight(),
				   image_size.getWidth(),
				   nbframes) != HIS_ALL_OK)
    {
      m_acq_mode = Normal;
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
      channel_type = "Unknow";break;
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
