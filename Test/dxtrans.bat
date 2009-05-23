
:: dxtrans.bat
:: Author: Sijtih <sijithh@softnotions.com>
:: Date: 23-MAY-09
:: Release: 1.0
:: Purpose: Used to copy dxtrans file, dxtrans.dll is a module that contains functions
:: used by DirectX Transform Core
 
::  LICENSE:
:: 
::  This file is part of bbPress.
::  
::  shareaza is free software; you can redistribute it and/or modify
::  it under the terms of the GNU General Public License as published by
::  the Free Software Foundation; either version 2 of the License, or
::  (at your option) any later version.
::  
::  bbPress is distributed in the hope that it will be useful,
::  but WITHOUT ANY WARRANTY; without even the implied warranty of
::  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
::  GNU General Public License for more details.
::  
:: You should have received a copy of the GNU General Public License
::  along with shareaza; if not, write to the Free Software
::  Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
::  
::  ----------------------------------------------------------------------------------
::  
::  PHP version 4 and 5
::  
::  ----------------------------------------------------------------------------------
::  
::  @copyright 2005 Automattic Inc.
::  @license   http://www.gnu.org/licenses/gpl.txt GNU General Public License v2
::  @link      http://automattic.com Automattic
:: 






@echo off

XCOPY "E:\Test\dxtrans.h" "C:\Program Files\Microsoft SDKs\Windows\v6.0A\Include"    /S  /y

exit
 