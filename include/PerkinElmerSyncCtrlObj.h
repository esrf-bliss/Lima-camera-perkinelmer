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
#ifndef PERKINELMERSYNCCTRLOBJ_H
#define PERKINELMERSYNCCTRLOBJ_H
#include "PerkinElmerCompatibility.h"
#include "HwSyncCtrlObj.h"

#include "PerkinElmerInterface.h"
namespace lima
{
  namespace PerkinElmer
  {
    class LIBPERKINELMER_API SyncCtrlObj : public HwSyncCtrlObj
    {
      friend class Interface;
      DEB_CLASS_NAMESPC(DebModCamera, "SyncCtrlObj", "PerkinElmer");
    public:
      SyncCtrlObj(HANDLE&);
      virtual ~SyncCtrlObj();

      virtual bool checkTrigMode(TrigMode trig_mode);
      virtual void setTrigMode(TrigMode  trig_mode);
      virtual void getTrigMode(TrigMode& trig_mode);

      virtual void setExpTime(double  exp_time);
      virtual void getExpTime(double& exp_time);

      virtual void setLatTime(double  lat_time);
      virtual void getLatTime(double& lat_time);

      virtual void setNbHwFrames(int  nb_frames);
      virtual void getNbHwFrames(int& nb_frames);

      virtual void getValidRanges(ValidRangesType& valid_ranges);

      void startAcq();
      void reallocOffset(const Size&);
      void reallocGain(const Size&);
      void invalidateCorrectionImage();

    private:
      HANDLE& 			m_acq_desc;
      TrigMode 			m_trig_mode;
      unsigned short*		m_offset_data;
      DWORD*			m_gain_data;
      double			m_expo_time;
      int			m_acq_nb_frames;
      Interface::CorrMode 	m_corr_mode;
      double			m_corr_expo_time;
    };
  }
}
#endif
