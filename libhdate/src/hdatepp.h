/*  libhdate
 *  Copyright  2004-2005  Yaacov Zamir <kzamir@walla.co.il>
 *  
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

/*
 * See Amos Shapir hdate_README file attached for 
 * Copyright notice for functions based on Amos's code
 */
 
#ifndef __HDATE_PP_H__
#define __HDATE_PP_H__

#include <hdate.h>

namespace hdate 
{
	
	class Hdate
	{
		
	public:
		
		Hdate()
		{
			h = new_hdate();
		}
		
		~Hdate()
		{
			delete_hdate(h);
		}
		
		set_gdate (int d, int m, int y)
		{
			hdate_set_gdate (h, d, m, y)
		}
		
		set_hdate (int d, int m, int y)
		{
			hdate_set_hdate (h, d, m, y)
		}
		
		set_jd (int jd)
		{
			hdate_set_jd (h, jd)
		}
		
		char *
		get_format_date (int diaspora, int s)
		{
			return hdate_get_format_date (h, diaspora, s);
		}

		int
		get_parasha (int diaspora)
		{
			return hdate_get_parasha (h, diaspora);
		}

		int
		get_holyday (int diaspora)
		{
			return hdate_get_holyday (h, diaspora);
		}
		
		int
		get_gday ()
		{
			return hdate_get_gday (h)
		}
		
		int
		get_gmonth ()
		{
			return hdate_get_gmonth (h);
		}
		
		int
		get_gyear ()
		{
			return hdate_get_gyear (h);
		}
		
		int
		get_hday ()
		{
			return hdate_get_hday (h);
		}
		
		int
		get_hmonth ()
		{
			return hdate_get_hmonth (h);
		}
		
		int
		get_hyear ()
		{
			return hdate_get_hyear (h);
		}
		
		int
		get_day_of_the_week ()
		{
			return hdate_get_day_of_the_week (h);
		}
		
		int
		get_size_of_year ()
		{
			return hdate_get_size_of_year (h);
		}
		
		int
		get_new_year_day_of_the_week ()
		{
			return hdate_get_new_year_day_of_the_week (h);
		}
		
		int
		get_julian ()
		{
			return hdate_get_julian (h);
		}
		
		int
		get_days ()
		{
			return hdate_get_days (h);
		}
		
		int
		get_weeks ()
		{
			return hdate_get_weeks (h);
		}

	private:
		
		hdate_struct *h;
	
	};

} // name space

#endif
