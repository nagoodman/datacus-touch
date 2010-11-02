/*
 Touch2Tuio - Windows 7 Touch to TUIO Bridge
 http://dm.tzi.de/research/hci/touch2tuio
 
 Copyright (c) 2010 Marc Herrlich and Benjamin Walther-Franks, Research Group Digital Media, TZI, University of Bremen
 
 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.
 
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 
 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/


// Touch2Tuio.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

int main(int argc, char *argv[])
{
	if (argc < 2)
	{
		std::cout << "No window title provided!" << std::endl;
		//char c = getchar();
		return 1;
	}

	std::string s;
	for (int i = 1; i < argc; ++i)
	{
		s.append(argv[i]);
		if (i < argc - 1)
			s.append(" ");
	}

	SetConsoleId(GetCurrentProcessId());
	SetTuioServerAddress("127.0.0.1", 3333);

	if (!InstallHookFromWindowTitle(s.c_str()))
	{
		std::cout << "Could not install hook for \"" << s << "\"" << std::endl;
		//char c = getchar();
		return 1;
	}
	std::cout << "Successfully installed hook and started TUIO server" << std::endl;

	char c = getchar();

	if (!RemoveHook())
	{
		std::cout << "Could not remove hook" << std::endl;
		//char c = getchar();
		return 1;
	}

	std::cout << "Successfully removed hook and killed TUIO server" << std::endl;

	return 0;
}
