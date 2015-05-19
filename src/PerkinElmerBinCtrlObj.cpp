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
#include "PerkinElmerBinCtrlObj.h"

using namespace lima;
using namespace lima::PerkinElmer;

BinCtrlObj::BinCtrlObj(HANDLE &acq_desc, unsigned int cam_type) :
  m_acq_desc(acq_desc),
  camera_type(cam_type)
{
}

BinCtrlObj::~BinCtrlObj()
{
}


void BinCtrlObj::setBin(const Bin& bin)
{
  DEB_MEMBER_FUNCT();

  int returnStatus;

  DEB_ALWAYS() << "Camera type = " << DEB_VAR1(camera_type);

  if (camera_type == 15)
    {
    if(Bin(4,1) == bin)
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,5);
    else if(Bin(4,2) == bin)
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,4);
    else if(Bin(4,4) == bin)
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,3);
    else if(Bin(2,2) == bin)
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,2);
    else
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,1);
    }
  else
    {
    if(Bin(2,2) == bin)
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,2);
    else
      returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,1);
    }
    
  if(returnStatus != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't set Hardware binnig";
}

void BinCtrlObj::getBin(Bin& bin)
{
  WORD binMode;
  Acquisition_GetCameraBinningMode(m_acq_desc,&binMode);
  if (camera_type == 15)
    {
    if(binMode == 5)
      bin = Bin(4,1);
    else if(binMode == 4)
      bin = Bin(4,2);
    else if(binMode == 3)
      bin = Bin(4,4);
    else if(binMode == 2)
      bin = Bin(2,2);
    else
      bin = Bin(1,1);
    }
  else
    {
    if(binMode == 2)
      bin = Bin(2,2);
    else
      bin = Bin(1,1);
    }
}

void BinCtrlObj::checkBin(Bin& bin)
{
  int x = bin.getX();
  int y = bin.getY();
  if (camera_type == 15)
    {
    if(x >= 4 && y >= 4)
      bin = Bin(4,4);
    else if(x == 2 && y == 2)
      bin = Bin(2,2);
    else if(x == 1 && y == 4)
      bin = Bin(4,1);
    else if(x == 1 && y == 2)
      bin = Bin(4,2);
    else
      bin = Bin(1,1);
    }
  else
    {
    if(x > 1 && y > 1)
      bin = Bin(2,2);
    else
      bin = Bin(1,1);
    }
}
