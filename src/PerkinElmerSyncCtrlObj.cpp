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
#include "PerkinElmerSyncCtrlObj.h"
#include <Acq.h>

using namespace lima;
using namespace lima::PerkinElmer;

SyncCtrlObj::SyncCtrlObj(HANDLE &acq_desc) :
  m_acq_desc(acq_desc),
  m_trig_mode(IntTrig),
  m_offset_data(NULL),
  m_gain_data(NULL),
  m_expo_time(-1.),
  m_acq_nb_frames(1)
{
}

SyncCtrlObj::~SyncCtrlObj()
{
}

bool SyncCtrlObj::checkTrigMode(TrigMode trig_mode)
{
  bool valid_mode = false;
  switch (trig_mode)
    {
    case IntTrig:
    case IntTrigMult:
    case ExtTrigMult:
      valid_mode = true;
      break;

    default:
      valid_mode = false;
      break;
    }
  return valid_mode;
}

void SyncCtrlObj::setTrigMode(TrigMode trig_mode)
{
  DEB_MEMBER_FUNCT();
  DWORD trigMode;
  switch(trig_mode)
    {
    case ExtTrigMult:
      trigMode = HIS_SYNCMODE_EXTERNAL_TRIGGER;break;
    default:
      trigMode = HIS_SYNCMODE_INTERNAL_TIMER;break;
    }
  if(Acquisition_SetFrameSyncMode(m_acq_desc,trigMode) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't set trigger mode to:" << DEB_VAR1(trig_mode);

  m_trig_mode = trig_mode;
}

void SyncCtrlObj::getTrigMode(TrigMode& trig_mode)
{
  trig_mode = m_trig_mode;
}

void SyncCtrlObj::setExpTime(double exp_time)
{
  DEB_MEMBER_FUNCT();
  UINT selectedReadout;
  if(exp_time >= 200e-3)
    {
      for(selectedReadout = 1;selectedReadout < 7;++selectedReadout)
	{
	  int readoutValue = 200 << selectedReadout;
	  if(exp_time < readoutValue * 1e-3) break;
	}
    }
  else
    selectedReadout = 0;

  DEB_TRACE() << DEB_VAR1(selectedReadout);
  if(Acquisition_SetCameraMode(m_acq_desc,selectedReadout) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't change readout time";
  
  if(m_trig_mode != ExtTrigMult)
    {						
      DWORD expTime = DWORD(exp_time * 1e6);
      if(Acquisition_SetTimerSync(m_acq_desc,&expTime) != HIS_ALL_OK)
	THROW_HW_ERROR(Error) << "Can't change exposition time";
      m_expo_time = expTime / 1e-6;
    }
  else
    m_expo_time = exp_time;
}

void SyncCtrlObj::getExpTime(double& exp_time)
{
  exp_time = m_expo_time;
}

void SyncCtrlObj::setLatTime(double lat_time)
{
  //Not managed
}

void SyncCtrlObj::getLatTime(double& lat_time)
{
  lat_time = 134e-3;		// Readout
}

void SyncCtrlObj::setNbHwFrames(int nb_frames)
{
  m_acq_nb_frames = nb_frames;
}

void SyncCtrlObj::getNbHwFrames(int& nb_frames)
{
  nb_frames = m_acq_nb_frames;
}

void SyncCtrlObj::getValidRanges(ValidRangesType& valid_ranges)
{
  valid_ranges.min_exp_time = 134e-3;
  valid_ranges.max_exp_time = 5.;
  valid_ranges.min_lat_time = 134e-3;
  valid_ranges.max_lat_time = 134e-3;
}

void SyncCtrlObj::startAcq()
{
  DEB_MEMBER_FUNCT();

  if(Acquisition_Acquire_Image(m_acq_desc,m_acq_nb_frames,0,
			       HIS_SEQ_CONTINUOUS, 
			       m_offset_data,
			       m_gain_data,
			       NULL)!= HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Could not start the acquisition";
  
}
