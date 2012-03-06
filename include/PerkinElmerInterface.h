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
#ifndef PERKINELMERINTERFACE_H
#define PERKINELMERINTERFACE_H
#include "Debug.h"
#include "PerkinElmerCompatibility.h"
#include "HwInterface.h"

namespace lima
{
  namespace PerkinElmer
  {
    class DetInfoCtrlObj;
    class SyncCtrlObj;
    void CALLBACK _OnEndFrameCallback(HANDLE);
    void CALLBACK _OnEndAcqCallback(HANDLE);

    class LIBPERKINELMER_API Interface : public HwInterface
    {
      friend void CALLBACK _OnEndFrameCallback(HANDLE);
      friend void CALLBACK _OnEndAcqCallback(HANDLE);
      DEB_CLASS_NAMESPC(DebModCamera, "Interface", "PerkinElmer");
    public:
      enum CorrMode {No,OffsetOnly,OffsetAndGain};

      Interface();
      virtual ~Interface();

      virtual void getCapList(CapList &) const;
      
      virtual void reset(ResetLevel reset_level);
      virtual void prepareAcq();
      virtual void startAcq();
      virtual void stopAcq();
      virtual void getStatus(StatusType& status);
      
      virtual int getNbHwAcquiredFrames();


      CorrMode getCorrectionMode() const;
      void setCorrectionMode(CorrMode);

      int getGain() const;
      void setGain(int);

      void startAcqOffsetImage(int nbframes,double time);
      void startAcqGainImage(int nbframes,double time);

      static bool get_channel_type_n_id(HANDLE& acq_desc,
					const char* &channel_type,
					int &channel_id);

      void newFrameReady();
      void SetEndAcquisition();
    private:
      enum AcqMode {Normal,Offset,Gain};

      void _InitDetector(unsigned int &max_columns,
			 unsigned int &max_rows);

      HANDLE 		m_acq_desc;
      DetInfoCtrlObj* 	m_det_info;
      SyncCtrlObj*	m_sync;
      SoftBufferCtrlObj m_buffer_ctrl_obj;
      bool		m_acq_started;
      int		m_acq_frame_nb;
      CapList		m_cap_list;
      void*		m_tmp_buffer;
      AcqMode		m_acq_mode;
      unsigned int	m_max_columns;
      unsigned int	m_max_rows;
      int		m_gain;
      bool		m_first_thrown;
    };
    LIBPERKINELMER_API std::ostream& operator <<(std::ostream &os,Interface::CorrMode);
  }
}
#endif
