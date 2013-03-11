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

BinCtrlObj::BinCtrlObj(HANDLE &acq_desc) :
  m_acq_desc(acq_desc)
{
}

BinCtrlObj::~BinCtrlObj()
{
}

void BinCtrlObj::setBin(const Bin& bin)
{
  DEB_MEMBER_FUNCT();

  int returnStatus;
  if(Bin(2,2) == bin)
    returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,2);
  else
    returnStatus = Acquisition_SetCameraBinningMode(m_acq_desc,1);

  if(returnStatus != HIS_ALL_OK)
    THROW_HW_ERROR(Error) << "Can't set Hardware binnig";
}

void BinCtrlObj::getBin(Bin& bin)
{
  WORD binMode;
  Acquisition_GetCameraBinningMode(m_acq_desc,&binMode);
  if(binMode == 2)
    bin = Bin(2,2);
  else
    bin = Bin(1,1);
}

void BinCtrlObj::checkBin(Bin& bin)
{
  int x = bin.getX();
  int y = bin.getY();
  if(x > 1 && y > 1)
    bin = Bin(2,2);
  else
    bin = Bin(1,1);
}
