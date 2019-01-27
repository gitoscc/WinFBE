'    WinFBE - Programmer's Code Editor for the FreeBASIC Compiler
'    Copyright (C) 2016-2019 Paul Squires, PlanetSquires Software
'
'    This program is free software: you can redistribute it and/or modify
'    it under the terms of the GNU General Public License as published by
'    the Free Software Foundation, either version 3 of the License, or
'    (at your option) any later version.
'
'    This program is distributed in the hope that it will be useful,
'    but WITHOUT any WARRANTY; without even the implied warranty of
'    MERCHANTABILITY or FITNESS for A PARTICULAR PURPOSE.  See the
'    GNU General Public License for more details.

#pragma once

#DEFINE IDC_FRMSTATUSBAREDITOR_LSTPANELS                  1000
#DEFINE IDC_FRMSTATUSBAREDITOR_CMDINSERT                  1001
#DEFINE IDC_FRMSTATUSBAREDITOR_CMDDELETE                  1002
#DEFINE IDC_FRMSTATUSBAREDITOR_CMDUP                      1003
#DEFINE IDC_FRMSTATUSBAREDITOR_CMDDOWN                    1004
#DEFINE IDC_FRMSTATUSBAREDITOR_COMBOSTYLE                 1005
#DEFINE IDC_FRMSTATUSBAREDITOR_TXTSIMPLETEXT              1006
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL1                     1007
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL2                     1008

#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL3                     1011
#DEFINE IDC_FRMSTATUSBAREDITOR_CMDBROWSEEXE               1012
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL4                     1013
#DEFINE IDC_FRMSTATUSBAREDITOR_TXTWORKINGFOLDER           1014
#DEFINE IDC_FRMSTATUSBAREDITOR_CMDBROWSEFOLDER            1015
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL5                     1016
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL6                     1017
#DEFINE IDC_FRMSTATUSBAREDITOR_CHKCTRL                    1018
#DEFINE IDC_FRMSTATUSBAREDITOR_CHKALT                     1019
#DEFINE IDC_FRMSTATUSBAREDITOR_CHKSHIFT                   1020
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL7                     1021
#DEFINE IDC_FRMSTATUSBAREDITOR_CHKPROMPT                  1022
#DEFINE IDC_FRMSTATUSBAREDITOR_CHKMINIMIZED               1023
#DEFINE IDC_FRMSTATUSBAREDITOR_CHKWAIT                    1024
#DEFINE IDC_FRMSTATUSBAREDITOR_COMBOACTION                1025
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL8                     1026
#DEFINE IDC_FRMSTATUSBAREDITOR_LABEL9                     1027
#Define IDC_FRMSTATUSBAREDITOR_CHKDISPLAYMENU             1028
#Define IDC_FRMSTATUSBAREDITOR_CMDOK                      1029
#Define IDC_FRMSTATUSBAREDITOR_TABCONTROL                 1030


declare Function frmStatusBarEditor_Show( ByVal hWndParent As HWnd ) as LRESULT
