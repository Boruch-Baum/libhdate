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
 * Algorithm from http://www.srrb.noaa.gov/highlights/sunrise/calcdetails.html
 * The low accuracy solar position equations are used.
 * These routines are based on Jean Meeus's book Astronomical Algorithms.
 */

#include <time.h>
#include <stdio.h>
#include <math.h>

#include "hdate.h"
#include "support.h"

/**
 @brief days from 1 january
  
 @parm day this day of month
 @parm month this month
 @parm year this year
 @return the days from 1 jan
*/
int
hdate_get_day_of_year (int day, int month, int year)
{
	int jd;
	
	/* get todays julian day number */
	jd = (1461 * (year + 4800 + (month - 14) / 12)) / 4 +
		(367 * (month - 2 - 12 * ((month - 14) / 12))) / 12 -
		(3 * ((year + 4900 + (month - 14) / 12) / 100)) / 4 + day;
	
	/* substruct the julian day of 1/1/year and add one */
	jd = jd - ((1461 * (year + 4799)) / 4 +
		367 * 11 / 12 - (3 * ((year + 4899) / 100)) / 4);

	return jd;
}

/**
 @brief utc sunrise/set time for a gregorian date
  
 @parm day this day of month
 @parm month this month
 @parm year this year
 @parm longitude longitude to use in calculations
 @parm latitude latitude to use in calculations
 @parm sunrise return the utc sunrise in minutes
 @parm sunset return the utc sunset in minutes
*/
void
hdate_get_utc_sun_time (int day, int month, int year, double latitude, double longitude, int *sunrise, int *sunset)
{
	double gama; /* location of sun in yearly cycle in radians */
	double eqtime; /* diffference betwen sun noon and clock noon */
	double decl; /* sun declanation */
	double ha; /* solar hour engle */
	double sunrise_angle = M_PI * 90.833 / 180.0; /* sun angle at sunrise/set */
	
	int day_of_year;
	
	/* get the day of year */
	day_of_year = hdate_get_day_of_year (day, month, year);
	
	/* get radians of sun orbit around erth =) */
	gama = 2.0 * M_PI * ((double)(day_of_year - 1) / 365.0);
	
	/* get the diff betwen suns clock and wall clock in minutes */
	eqtime = 229.18 * (0.000075 + 0.001868 * cos (gama)
		- 0.032077 * sin (gama) - 0.014615 * cos (2.0 * gama)
		- 0.040849 * sin (2.0 * gama));
	
	/* calculate suns declanation at the equater in radians */
	decl = 0.006918 - 0.399912 * cos (gama) + 0.070257 * sin (gama)
		- 0.006758 * cos (2.0 * gama) + 0.000907 * sin (2.0 * gama)
		- 0.002697 * cos (3.0 * gama) + 0.00148 * sin (3.0 * gama);
	
	/* we use radians, ratio is 2pi/360 */
	latitude = M_PI * latitude / 180.0;
	
	/* the sun real time diff from noon at sunset/rise in radians */
	ha = acos ((cos (sunrise_angle) - sin (latitude) * sin (decl)) / (cos (latitude) * cos (decl)));
	
	/* we use minutes, ratio is 1440min/2pi */
	ha = 720.0 * ha / M_PI;
	
	/* get sunset/rise times in utc wall clock in minutes from 00:00 time */
	*sunrise = (int)(720.0 + 4.0 * longitude - ha - eqtime);
	*sunset = (int)(720.0 + 4.0 * longitude + ha - eqtime);
	
	return;
}
