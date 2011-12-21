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
#include "PerkinElmerDetInfoCtrlObj.h"
using namespace lima;
using namespace lima::PerkinElmer;
DetInfoCtrlObj::DetInfoCtrlObj(HANDLE &acq_desc,
			       int max_rows,int max_columns) :
  m_acq_desc(acq_desc),
  m_max_rows(max_rows),
  m_max_columns(max_columns)
{
}

DetInfoCtrlObj::~DetInfoCtrlObj()
{
}

void DetInfoCtrlObj::getMaxImageSize(Size& max_image_size)
{
  max_image_size = Size(m_max_columns,m_max_rows);
}

void DetInfoCtrlObj::getDetectorImageSize(Size& det_image_size)
{
  DEB_MEMBER_FUNCT();
  DWORD dwFrames, dwSortFlags,rows,columns,data_type,sync_mode,dwAcqType,system_id,dwHwAccess;
  if(Acquisition_GetConfiguration(m_acq_desc,&dwFrames,
				  &rows,&columns,&data_type,
				  &dwSortFlags, &bEnableIRQ, &dwAcqType, 
				  &system_id, &sync_mode, &dwHwAccess) != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't get detector image size";
    
  det_image_size = Size(columns,rows);
  DEB_RETURN() << DEB_VAR1(det_image_size);
}

void DetInfoCtrlObj::getDefImageType(ImageType& def_image_type)
{
  def_image_type = Bpp16;
}

void DetInfoCtrlObj::getCurrImageType(ImageType& curr_image_type)
{
  curr_image_type = Bpp16;
}

void DetInfoCtrlObj::setCurrImageType(ImageType  curr_image_type)
{
  DEB_MEMBER_FUNCT();
  DEB_PARAM() << DEB_VAR1(curr_image_type);

  if(curr_image_type != Bpp16)
    THROW_HW_ERROR(Error) << "Only support Bpp16";
}

void DetInfoCtrlObj::getPixelSize(double& pixel_size)
{
  pixel_size = -1.;		// @todo find the pixel size
}

void DetInfoCtrlObj::getDetectorType(std::string& det_type)
{
  det_type = "PerkinElmer"
}

void DetInfoCtrlObj::getDetectorModel(std::string& det_model)
{
  int channel_id;
  const char* channel_type;
  Interface::get_channel_type_n_id(m_acq_desc,channel_type,channel_id);
  det_model = channel_type;
}

void DetInfoCtrlObj::registerMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.registerMaxImageSizeCallback(cb);
}

void DetInfoCtrlObj::unregisterMaxImageSizeCallback(HwMaxImageSizeCallback& cb)
{
  m_mis_cb_gen.unregisterMaxImageSizeCallback(cb);
}
