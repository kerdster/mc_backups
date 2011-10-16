//      mc_backups.cpp
//      
//      Copyright 2011 Andrei Bagrintsev <andrey@bagrincev.ru>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.


#include <iostream>
#include <stdio.h>
#include <time.h>
#include <cstring>
#include <vector>
#include <stdlib.h>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libtar.h>
#include <bzlib.h>
#include <unistd.h>
#include <errno.h>
#include "mcmd.h"

#include <sstream>
//#include "zip.h"
using namespace std;

class mcbkp {

	protected:
	time_t tms;
	const char* game_dir;
	const char* bdir;
	public:
	vector<string> pfiles;
	
	mcbkp(char* _game_dir, char* _bdir)
	{
		game_dir = check_slash(_game_dir);
		bdir = check_slash(_bdir);
		time (&tms);
	}

	~mcbkp()
	{
		//delete this->game_dir;
		//delete this->bdir;
	}

	char* check_slash (char* str)
	{
		if (!str)
		{
			cout << "MEGOFAIL." << endl;
			exit (0);
		}

		int len = strlen (str);
		
		if (str[len-1] != '/')
		{
			char* tmp = NULL;

			if ((tmp=new(nothrow) char[len+2]) == NULL)
			{
				exit(1);
			}

			strcpy(tmp, str);
			tmp[len] = '/';
			tmp[len+1] = '\0';

			delete tmp;
			return tmp;
		}
		return str;
	}
	
	bool arch_names ()
	{
		DIR *dp;
		struct stat stFileInfo;
		struct dirent *DirEntry;
		char fileFullName [200];
		
		if ((dp=opendir(bdir)) == NULL)
		{
			perror (bdir);
			exit (1);
		}
		vector<string> files;

		while (DirEntry = readdir(dp))
		{
			sprintf(fileFullName, "%s/%s", bdir, DirEntry->d_name);
			
			if (lstat(fileFullName, &stFileInfo) < 0)
			{
				perror ( fileFullName );
			}
			
			if (S_ISDIR(stFileInfo.st_mode))
			{
				//cout << "Directory: " << DirEntry->d_name << endl;
				continue;
			}

			files.push_back (DirEntry->d_name);
			//cout << DirEntry->d_name << endl;
		}
		closedir (dp);
		pfiles = files;
		return true;
	}

	vector<string> parse_fname(string fname)
	{
		string tmp;
		string part;
		register int i;
		int read_part = 1;
		vector<string> parts;

		for (i=0; fname[i]!=0; i++)
		{
			tmp = fname[i];

			// символы не попадающие в строку
			if (tmp == "_" || tmp == ".")
			{
				read_part = 0;
			}

			// собираем строку по букве
			if (read_part)
			{
				part += tmp;
			}

			// записать строку в массив
			if (tmp == "_" || fname[(i+1)]==0)
			{
				parts.push_back (part);
				part = "";
				read_part = 1;
				continue;
			}
		}
		
		/*for (i=0; parts.size()>i; i++)
			cout << parts[i] << endl;*/
		return parts;
	}

	string tochar (int val)
	{
		//~ char *buf = new char[64];
		//~ sprintf (buf, "%d", val);
		//~ string tmp = buf;
		//~ delete buf;
		//~ return tmp;
		stringstream out;
		out << val;
		return out.str();
	}

	bool find_last_day()
	{
		vector<string> tmp;
		int last_day = (this->tms - 3600 * 24 * 5);
		string aname = tochar (last_day);
		
		for (int i=0; pfiles.size()>i; i++)
		{
			tmp = parse_fname (pfiles[i]);

			if (tmp[0] == "mcbkp" && (atoi (tmp[1].c_str()) <= last_day))
			{
				aname = string(bdir)+"/mcbkp_"+tmp[1]+".tar";
				cout << "deleting file: " << aname << endl;
				unlink (aname.c_str());
				aname = "";
			}
		}

		return true;
	}

	bool new_backup()
	{
		string backup = string(bdir)+"mcbkp_" + tochar(this->tms) + ".tar";
		//string backup = bdir;
		//backup = backup+"/"+"mcbkp_" +  tochar(tms) + ".tar";
		 
		TAR *pTar;
		char extractTo[] = ".";
		string gd = game_dir;

		if (tar_open(&pTar, (char*)backup.c_str(), NULL, O_WRONLY | O_CREAT, 0644, TAR_GNU) != 0)
		{
			cout << "error: " << strerror(errno) << endl;
			exit (1);
		}

		if (tar_append_tree(pTar, (char*)gd.c_str(), extractTo) != 0)
		{
			cout << "error: " << strerror(errno) << endl;
			exit (1);
		}
		 
		close(tar_fd(pTar));

		return true;
	}

	bool check()
	{
		struct stat st;

		if(stat(game_dir,&st) < 0)
		{
			cout << "game directory does not exist." << endl;
			return false;
		}

		if(stat(bdir,&st) < 0)
		{
			cout << "directory for backups saving does not exist" << endl;
			return false;
		}

		return true;
	}
};

int main (int argc, char **argv)
{
	if (argc <= 2)
	{
		cout << "Fail. Usage: " << argv[0] << " <game_directory> <backups_directory>" << endl;
		exit (0);
	}
	
	mcbkp main (argv[1], argv[2]);

	if (!main.check())
	{
		exit(1);
	}
	
	main.arch_names();
	
	run_cmd ("say Beginning backup the world! Hold on, niggas!", 0);
	run_cmd ("save-all", 0);
	run_cmd ("save-off", 0);
	if (main.new_backup())
	{
		run_cmd ("say All is ok. Have Fun!", 0);
	}
	run_cmd ("save-on", 0);

	if (!main.pfiles.empty())
	{
		main.find_last_day();
	}

	return 0;
}
