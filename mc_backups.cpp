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
#include <string>
#include <string.h>
#include <vector>
#include <stdlib.h>

#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
//#include <sys/types.h>
//#include <ctype.h>
#include <errno.h>

#include "zip.h"
using namespace std;

class mcbkp {

	protected:
	time_t tms;

	public:
	
	mcbkp()
	{
		this->init_date();
	}

	void init_date ()
	{
		tms = time (NULL);
	}

	vector<string> arch_names (const char *dir)
	{
		DIR *dp;
		struct stat stFileInfo;
		struct dirent *DirEntry;
		char fileFullName [200];
		
		if ((dp=opendir(dir)) == NULL)
		{
			perror (dir);
			exit (1);
		}
		vector<string> files;

		while (DirEntry = readdir(dp))
		{
			sprintf(fileFullName, "%s/%s", dir, DirEntry->d_name);
			
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
		return files;
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
		char buf[10];
		sprintf (buf, "%d", val);
		string tmp = buf;
		//delete buf;
		return tmp;
	}

	int find_last_day (vector<string> archs)
	{
		vector<string> tmp;
		int last_day = (this->tms - 3600 * 24 * 5);
		string aname = tochar (last_day);
		
		for (int i=0; archs.size()>i; i++)
		{
			tmp = parse_fname (archs[i]);

			if (tmp[0] == "mcbkp" && (atoi (tmp[1].c_str()) <= last_day))
			{
				aname = "/home/sb0y/mc_bkps/mcbkp_"+tmp[1]+".zip";
				cout << "delete file: " << aname << endl;
				unlink (aname.c_str());
				aname = "";
			}
		}
	}

	int new_backup (string backups_dir)
	{
		string backup = backups_dir+"/mcbkp_" + tochar(this->tms) + ".zip";
		//string cmd = "zip "+backups_dir+"/"+backup+" -rj /var/lib/minecraft/*";
		//~ cout << cmd << endl;
		//~ system (cmd.c_str());
		
		struct zip *za;
		int zip_err;
		int err;
		char errstr[1024];
	
		if ((za=zip_open(backup.c_str(), ZIP_CREATE, &err)) == NULL)
		{
			zip_error_to_str(errstr, sizeof(errstr), err, errno);
			fprintf(stderr, "cannot open zip archive `%s': %s\n",
			backup.c_str(), errstr);
			exit(1);
		}
	
		if (zip_add_dir (za, "/home/sb0y/workspace/") <= 0)
		{
			cout << "hello" << endl;
			zip_error_to_str(errstr, sizeof(errstr), err, errno);
			fprintf(stderr, "`%s': %s\n",
			backup.c_str(), errstr);
			zip_close(za);
			exit(1);
		}
	
		zip_close(za);
	}
};

int main (int argc, char **argv)
{
	mcbkp *main;
	main = new mcbkp;
	
	vector<string> pfiles = main->arch_names ("/home/sb0y/mc_bkps");

	main->new_backup ("/home/sb0y/mc_bkps");

	if (!pfiles.empty())
	{
		main->find_last_day (pfiles);
	}

	delete main;

	return 0;
}

